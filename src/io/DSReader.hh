#ifndef __RAT_DSReader___
#define __RAT_DSReader___

#include <RAT/DS/Root.hh>
#include <TObject.h>
#include <TChain.h>
#include <TTree.h>
#include <string>

namespace RAT {

// Convenience class for ROOT scripts
class DSReader : public TObject {
public:
  DSReader(const char *filename);
  virtual ~DSReader();

  void Add(const char* filename);
  void SetBranchStatus(const char *bname, bool status=1) { 
    T.SetBranchStatus(bname, status);
  };

  TTree* GetT() { return &T; };
  DS::Root* GetDS() { return ds; };
  Long64_t GetTotal() { return total; };

  // Load event.  Returns ds (which will now point to specified event)
  DS::Root* GetEvent(Long64_t num) { next = num+1; T.GetEntry(num); return ds; };
  DS::Root* NextEvent();

  ClassDef(DSReader,0)

protected:
  TChain T;
  DS::Root *ds;
  Long64_t next;
  Long64_t total;
};


} // namespace RAT

#endif
