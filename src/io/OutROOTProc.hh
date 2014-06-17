#ifndef __RATOutROOTProc___
#define __RATOutROOTProc___

#include <RAT/Processor.hh>
#include <RAT/DS/Run.hh>

class TFile;
class TTree;

namespace RAT {



class OutROOTProc : public Processor {
public:
  static int run_num;
  OutROOTProc();
  virtual ~OutROOTProc();

  // file - string, name of file to open for output, file will be erased
  // updatefile - string, name of file to append to
  // (do not use both file and update file)
  virtual void SetS(std::string param, std::string value);
  
  // autosave - integer, update root file every N kilobytes
  // savetree 0 - Do not save the event tree.  Must set *before* file or updatefile.
  virtual void SetI(std::string param, int value);

  virtual Processor::Result DSEvent(DS::Root *ds);

  virtual bool OpenFile(std::string theFilename, bool update=false);
  virtual std::string GetFilename() { return filename; };

protected:
  std::string default_filename;
  std::string filename;
  TFile *f;
  TTree *tree;
  TTree  *runTree;
  DS::Root *branchDS;
  DS::Run *branchRun;
  int autosave;
  bool savetree;
};

} // namespace RAT

#endif
