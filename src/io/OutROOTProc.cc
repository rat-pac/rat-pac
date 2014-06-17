#include <RAT/OutROOTProc.hh>
#include <RAT/Log.hh>
#include <RAT/Processor.hh>
#include <RAT/DB.hh>
#include <RAT/ObjInt.hh>
#include <RAT/SignalHandler.hh>
#include <RAT/DS/RunStore.hh>
  
#include <TFile.h>
#include <TTree.h>
#include <TObjString.h>


namespace RAT {



OutROOTProc::OutROOTProc() : Processor("outroot")
{
  f = 0;
  tree = 0;
  autosave = 1024; // kB
  savetree = true;
  branchDS = new DS::Root();
  branchRun = new DS::Run();

  // Extract default filename from database.  Used if no
  // output file is specified by user with /rat/procset
  DB *db = DB::Get();
  DBLinkPtr roottab = db->GetLink("IO");
  try {
    default_filename = roottab->GetS("default_output_filename");
  } catch (DBNotFoundError &e) { 
    default_filename = "out.root";
  }
}

void write_object_with_directory(TDirectory *dir, const std::string &path, TObject *obj)
{
  std::string name = path; // default is path is the object name
  std::string subdir_name = "";

  // Check if there is a directory name in this path
  size_t last_slash = path.rfind('/');
  if (last_slash == 0) {
    name = path.substr(1, std::string::npos); // slice off leading slash
  } else if (last_slash != std::string::npos) {
    subdir_name = path.substr(0, last_slash);
    name = path.substr(last_slash + 1, std::string::npos);
    if (!dir->GetDirectory(subdir_name.c_str()))
      dir->mkdir(subdir_name.c_str());
  }

  dir->cd(subdir_name.c_str());
  obj->Write(name.c_str());
}


OutROOTProc::~OutROOTProc()
{
  if (f) {
    f->cd(); // In case another ROOT file has been opened recently
    if (savetree)
      tree->Write();
    DS::RunStore::FlushWriteTree();
    runTree->Write();

    // Record run information in the file
    TObjString *log = new TObjString(Log::GetLogBuffer().c_str());
    log->Write("log");

    TObjString *macro = new TObjString(Log::GetMacro().c_str());
    macro->Write("macro");
    
    TMap *dbtrace = Log::GetDBTraceMap();
    dbtrace->Write("db", TObject::kSingleKey);

    // Save any objects that processors have logged
    f->mkdir("obj");
    TDirectory *dir = f->GetDirectory("obj");

    std::vector<std::pair<std::string, TObject*> > objects = Log::GetObjects();
    for (unsigned int i=0; i < objects.size(); i++)
      write_object_with_directory(dir, objects[i].first, objects[i].second);

    f->cd();

    // Update status information
    ObjInt status;
    if (SignalHandler::IsTermRequested())
      status = 0;
    else
      status = 1;
    status.Write("status");


    f->Close(); // also deletes TTree and TObjString?

    delete f;
  }
  
  // Don't delete branchDS because it looks like the TTree took ownership of it
}

void OutROOTProc::SetS(std::string param, std::string value)
{
  if (param == "file") {
      if (run_num > 0)
      {
	  std::ostringstream buffer;
	  char temp_str[16];
	  snprintf(temp_str, sizeof(temp_str), "_%04i.root",run_num);
	  buffer << value << temp_str; 
	  value = buffer.str();
      }
    if (!OpenFile(value, false))
      Log::Die("outroot: Cannot open file " + value);
  } else if (param == "updatefile") {
    if (!OpenFile(value, true))
      Log::Die("outroot: Cannot open file " + value);
  }
}

void OutROOTProc::SetI(std::string param, int value)
{
  if (param == "autosave") {
    if (value <= 0)
      Log::Die("outroot: autosave parameter must be a postive integer");
      
    autosave = value;
    if (tree) // autosave might be called after the file is already opened
      tree->SetAutoSave(autosave*1024 /*bytes*/); 
  } else if (param == "savetree") {
    if (value == 0) {
      savetree = false;
      info << "outroot: Not writing event tree!\n";
    }
  }
}

bool OutROOTProc::OpenFile(std::string theFilename, bool update)
{
  if (update) {
    info << "OutROOTProc: Appending to " << theFilename << newline;
    f = TFile::Open(theFilename.c_str(), "UPDATE");

    runTree = (TTree *) f->Get("runT");
    if (runTree) {
      DS::RunStore::PreloadFromTree(tree, true /* already written to disk */);
    } else {
      // make new runTree now
      runTree = new TTree("runT", "RAT Run Tree");
      runTree->Branch("run", branchRun->ClassName(), &branchRun, 32000, 99);
    }
  } else {
    info << "OutROOTProc: Writing to " << theFilename << newline;
    f = TFile::Open(theFilename.c_str(), "RECREATE");

     runTree = new TTree("runT", "RAT Run Tree");
     runTree->Branch("run", branchRun->ClassName(), &branchRun, 32000, 99); 
  }
  DS::RunStore::SetWriteTree(runTree);
 
  if (!f)
    return false;

  filename = theFilename;

  if (savetree) {
    if ( update && (tree = (TTree *) f->Get("T")) ) {
      // Appending events to existing tree
      tree->SetBranchAddress("ds", &branchDS, 0);
    } else {
      // Setup tree
      tree = new TTree("T", "RAT Tree");
      tree->Branch("ds", branchDS->ClassName(), &branchDS, 32000, 99);
    }
    
    // Allow tree to be recovered up to last N kilobytes if RAT terminates suddenly
    tree->SetAutoSave(autosave*1024 /*bytes*/); 
  }

  // Add status integer to file with "crash" value in case we never finish
  ObjInt status(-1);
  status.Write("status");

  return true;
}


Processor::Result OutROOTProc::DSEvent(DS::Root *ds)
{
  if (!f) {
    // Assume no file specified, and so we must open the default
    info << "outroot: No output file specified, opening " << default_filename
	 << newline;
    if (!OpenFile(default_filename))
      Log::Die("outroot: No output file specified and cannot open default: "
		  +default_filename);
  }

  // Short-circuit if we aren't saving events to a tree
  if (!savetree)
    return Processor::OK;

  // Don't let a crazy track with hundreds of thousands of steps flood
  // the output file
  if (ds->ExistMC()) {
    RAT::DS::MC *mc = ds->GetMC();

    int ntracks = mc->GetMCTrackCount();
    for (int i=0; i < ntracks; i++) {
      RAT::DS::MCTrack *mctrack = mc->GetMCTrack(i);
      int nsteps = mctrack->GetMCTrackStepCount();
      bool isMuon = ( mctrack->GetParticleName().compare("mu-")==0
		      || mctrack->GetParticleName().compare("mu+")==0 );
      if ( nsteps > 1e5 && !isMuon ) {
	warn << "outroot: trimming intermediate steps from MC event #"
	<< mc->GetID() << ", track #" << mctrack->GetID()
	<< " which has " << nsteps << " steps" << newline;
	mctrack->PruneIntermediateMCTrackSteps();
      }
    }
  }

  // Write to tree
  tree->SetBranchAddress("ds", &ds, 0);
  
  int errcode = tree->Fill();
  if (errcode < 0)
    Log::Die(std::string("OutROOTProc: Error writing to file ")
	     + f->GetName() + ".\n             Out of disk space?");
  
  return Processor::OK;
}


} // namespace RAT
