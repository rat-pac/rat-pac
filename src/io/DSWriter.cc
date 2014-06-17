#include <RAT/DSWriter.hh>

#include <TObjString.h>

#include <RAT/DS/RunStore.hh>
#include <RAT/Log.hh>
#include <RAT/ObjInt.hh>

namespace RAT {

DSWriter::DSWriter(const char *filename, bool update, int autosave) : fFilename(filename)
{
  fBranchDS = new DS::Root();
  fBranchRun = new DS::Run();

   if (update) {
    fFile = TFile::Open(filename, "UPDATE");
    fRunTree = (TTree *) fFile->Get("runT");

    if (fRunTree) {
      DS::RunStore::PreloadFromTree(fRunTree, true /* already written to disk */);
    } else {
      // make new runTree now
      fRunTree = new TTree("runT", "RAT Run Tree");
      fRunTree->Branch("run", fBranchRun->ClassName(), &fBranchRun, 32000, 99);
    }
  } else {
    fFile = TFile::Open(filename, "RECREATE");
    fRunTree = new TTree("runT", "RAT Run Tree");
    fRunTree->Branch("run", fBranchRun->ClassName(), &fBranchRun, 32000, 99); 
  }
  DS::RunStore::SetWriteTree(fRunTree);
 
  if ( update && (fTree = (TTree *) fFile->Get("T")) ) {
    // Appending events to existing tree
    fTree->SetBranchAddress("ds", &fBranchDS, 0);
  } else {
    // Setup tree
    fTree = new TTree("T", "RAT Tree");
    fTree->Branch("ds", fBranchDS->ClassName(), &fBranchDS, 32000, 99);
  }
  
  // Allow tree to be recovered up to last N kilobytes if RAT terminates suddenly
  fTree->SetAutoSave(autosave*1024 /*bytes*/); 
}


DSWriter::~DSWriter() {
  Close();
}

int DSWriter::Fill(DS::Root *ds)
{
  // Faster to just switch addresses
  fTree->SetBranchAddress("ds", &ds, 0);
  
  int errcode = fTree->Fill();

  // And switch back to avoid dangling pointer if ds is deleted elsewhere
  fTree->SetBranchAddress("ds", &fBranchDS, 0);

  return errcode;
}

void DSWriter::WriteLog()
{
  fFile->cd();

  TObjString *log = new TObjString(Log::GetLogBuffer().c_str());
  log->Write("log");

  TObjString *macro = new TObjString(Log::GetMacro().c_str());
  macro->Write("macro");
  
  TMap *dbtrace = Log::GetDBTraceMap();
  dbtrace->Write("db", TObject::kSingleKey);
}

void DSWriter::WriteStatus(int _status)
{
  fFile->cd();
  ObjInt status(_status);
  status.Write("status");
}

void DSWriter::Close()
{
  if (fFile) {
    if (fFile->IsWritable()) {
      fFile->cd(); // In case another ROOT file has been opened recently
      fTree->Write();
      fRunTree->Write();
      fFile->Close(); // also deletes TTree and TObjString?
    } else {
      warn << "DSWriter: ROOT closed the TFile before we could write!  You probably lost data.\n";
      warn << "DSWriter: In Python, use try/finally block to call Close() function.\n";
    }
  } 
  
  delete fFile;
  fFile = 0;
}

} // namespace RAT
