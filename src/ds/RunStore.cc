#include <RAT/DS/RunStore.hh>
#include <RAT/DS/Root.hh>
#include <RAT/Log.hh>

namespace RAT {
  namespace DS {
    
RunStore* RunStore::fgStore = 0;

RunStore::RunStore() : fReadTree(0), fWriteTree(0) {
  fReadRun = new Run();
  fWriteRun = new Run();
}

RunStore::~RunStore() {
  for (std::map<int, RunRecord *>::iterator i = fCache.begin(); 
       i != fCache.end(); i++) {
    RunRecord *record = i->second;
    delete record; // deletes attached Run as well
  }
}

Run* RunStore::InstanceGetRun(Root* ds) {
  return InstanceGetRun(ds->GetRunID());
}

Run* RunStore::InstanceGetRun(int run) {
  // Check cache first
  if (fCache.count(run) != 0)
    return fCache[run]->run;

  // No read tree means nowhere else to look
  if (!fReadTree)
    return 0;
  
  // Scan quickly through tree
  //fReadTree->SetBranchStatus("*", 0); //for some reason this segfaults
  fReadTree->SetBranchStatus("runID", 1);
  
  // Have to reset branch address after calling SetBranchStatus
  // in case fReadTree is TChain
  fReadTree->SetBranchAddress("run", &fReadRun);

  int i;
  for (i=0; i < fReadTree->GetEntries(); i++) {
    fReadTree->GetEntry(i);
    if (fReadRun->GetID() == run)
      break;
  }

  fReadTree->SetBranchStatus("*", 1);

  // Also need to reset SetBranchAddress again here
  fReadTree->SetBranchAddress("run", &fReadRun);      

  // found run, read all branches, copy to cache
  if (i < fReadTree->GetEntries()) {
    fReadTree->GetEntry(i);
    RunRecord *record = new RunRecord();
    record->run = new Run(*fReadRun);
    fCache[run] = record;
    return fCache[run]->run;
  } else
    return 0;
}

void RunStore::InstanceSetReadTree(TTree* tree) {
  fReadTree = tree;
  fReadTree->SetBranchAddress("run", &fReadRun);
}

void RunStore::InstanceSetWriteTree(TTree* tree) {
  fWriteTree = tree;
  fWriteTree->SetBranchAddress("run", &fWriteRun);
}

void RunStore::InstanceFlushWriteTree() {
  if (!fWriteTree)
    return; // no output tree, nothing to do

  for (std::map<int, RunRecord *>::iterator i = fCache.begin(); 
       i != fCache.end(); i++) {

    RunRecord *record = i->second;

    if (!record->writtenToDisk) {
      *fWriteRun = *(record->run);
      fWriteTree->Fill();
      record->writtenToDisk = true;
    }

  }
}

void RunStore::InstanceAddNewRun(Run* run) {
  RunRecord *record = new RunRecord();
  record->run = run;
  fCache[run->GetID()] = record;
}

void RunStore::InstancePreloadFromTree(TTree* tree, bool writtenToDisk) {
  Run* run = new Run();
  tree->SetBranchAddress("run", &run);

  for (int i=0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);
    RunRecord *record = new RunRecord();
    record->run = new Run(*run);
    record->writtenToDisk = writtenToDisk;
    fCache[run->GetID()] = record;
  }
}

  } // namespace DS
} // namespace RAT

