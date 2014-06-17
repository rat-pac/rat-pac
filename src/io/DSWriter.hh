#ifndef __RAT_DSWriter___
#define __RAT_DSWriter___

#include <RAT/DS/Root.hh>
#include <RAT/DS/Run.hh>
#include <TFile.h>
#include <TObject.h>
#include <TTree.h>
#include <string>

namespace RAT {

// Convenience class for ROOT scripts
class DSWriter : public TObject {
public:
  // Write event tree to `filename`, flushing to disk every `autosave` kB.
  // If update is true, append to file if it exists
  DSWriter(const char *filename, bool update=false, int autosave=1024);
  virtual ~DSWriter();

  // Write an event to tree.  Returns error code from TTree::Fill()
  int Fill(DS::Root *ds);
  TTree* GetTree() { return fTree; };
  TTree* GetRunTree() { return fRunTree; };
  void WriteLog();
  void WriteStatus(int status);
  void Close();

  ClassDef(DSWriter,0)

protected:
  std::string fFilename;
  TFile *fFile;
  TTree *fTree;
  TTree  *fRunTree;
  DS::Root *fBranchDS;
  DS::Run *fBranchRun;
};

} // namespace RAT
#endif
