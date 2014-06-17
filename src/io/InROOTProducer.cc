#include <RAT/InROOTProducer.hh>
#include <RAT/ProcBlock.hh>
#include <RAT/DS/Root.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/SignalHandler.hh>
#include <RAT/Log.hh>
#include <RAT/DB.hh>

#include <G4UIdirectory.hh>
#include <G4UIcmdWithAString.hh>

#include <TChain.h>
#include <TFile.h>
#include <TString.h>

#include <assert.h>

namespace RAT {


InROOTProducer::InROOTProducer()
{
  mainBlock = 0;
  Init();
}

InROOTProducer::InROOTProducer(ProcBlock *block)
{
  SetMainBlock(block);
  Init();
}

InROOTProducer::~InROOTProducer()
{
}

void InROOTProducer::Init()
{
  // Build commands
  G4UIdirectory* DebugDir = new G4UIdirectory("/rat/inroot/");
  DebugDir->SetGuidance("Read Events from ROOT file");

  readCmd = new G4UIcmdWithAString("/rat/inroot/read", this);
  readCmd->SetGuidance("name of input file");
  readCmd->SetParameterName("filename", false);  // required

  readDefaultCmd = new G4UIcommand("/rat/inroot/read_default", this);
  readDefaultCmd->SetGuidance("read from IO.default_input_filename");
}

G4String InROOTProducer::GetCurrentValue(G4UIcommand * /*command*/)
{
  Log::Die("invalid inroot \"get\" command");
  return G4String("You never see this.");
}


void InROOTProducer::SetNewValue(G4UIcommand * command, G4String newValue)
{
  // readCmd
  if (command == readCmd || command == readDefaultCmd) {
    std::string filename;

    if (command == readDefaultCmd) {
      DBLinkPtr lIO = DB::Get()->GetLink("IO");
      filename = lIO->GetS("default_input_filename");
    } else {
      size_t size = newValue.size();

      // Trim extraneous quotation marks to avoid confusing people
      if (size >= 2 && newValue[(size_t)0] == '\"' && newValue[size-1] == '\"')
	filename = newValue.substr(1, size-2);
      else
	filename = newValue;
    }

    if (!mainBlock)
      Log::Die("inroot: No main block declared! (should never happen)");
    else if (!ReadEvents(filename))
      Log::Die("inroot: Error reading event tree from " + filename);

  } else
    Log::Die("invalid inroot \"set\" command");
}

bool InROOTProducer::ReadEvents(G4String filename)
{
  // Setup
  TChain tree("T");
  if (!tree.Add(filename.c_str()))
      return false;
  
  info << "InROOT: Reading from " << filename << "\n";

  TChain runTree("runT");
  // The run tree is optional to support older files, but it
  // is very awkward to test if filename has wildcards
  if (TString(filename).MaybeWildcard()) {
    // Assume there is a runT in all files
    runTree.Add(filename.c_str());
    DS::RunStore::SetReadTree(&runTree);
  } else {
    // In single file case, we can check
    TFile *ftemp = TFile::Open(filename.c_str());
    if (ftemp->Get("runT")) {
      runTree.Add(filename.c_str());
      DS::RunStore::SetReadTree(&runTree);
    } // else, no runT, so don't register runTree with RunStore

  //delete ftemp;
  }

  DS::Root *branchDS = new DS::Root();
  tree.SetBranchAddress("ds", &branchDS);
  
  // Read
  Int_t num_events = tree.GetEntries();
  for (Int_t i=0; i < num_events && !SignalHandler::IsTermRequested(); i++) {
    tree.GetEntry(i);
    // force the run entry to be loaded into memory so that it
    // can be written later.
    // If no runTree to read from, this will return 0, but we don't care.
    DS::RunStore::GetRun(branchDS);
    
    mainBlock->DSEvent(branchDS);
  }

  // Cleanup
  delete branchDS;

  return true;
}

} // namespace RAT
