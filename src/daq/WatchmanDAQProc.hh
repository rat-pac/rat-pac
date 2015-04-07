#ifndef __RAT_WatchmanDAQProc__
#define __RAT_WatchmanDAQProc__

#include <RAT/Processor.hh>

namespace RAT {


class WatchmanDAQProc : public Processor {
public:
  WatchmanDAQProc();
  virtual ~WatchmanDAQProc() { };
  virtual Processor::Result DSEvent(DS::Root *ds);

protected:
  int fEventCounter;
  std::vector<double> fSPECharge;
};


} // namespace RAT

#endif
