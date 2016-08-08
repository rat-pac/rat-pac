#ifndef __RAT_LessSimpleDAQProc__
#define __RAT_LessSimpleDAQProc__

#include <RAT/Processor.hh>

namespace RAT {


class LessSimpleDAQProc : public Processor {
public:
  LessSimpleDAQProc();
  virtual ~LessSimpleDAQProc() { };
  virtual Processor::Result DSEvent(DS::Root *ds);

protected:
  int fEventCounter;
  std::vector<double> fSPECharge;
};


} // namespace RAT

#endif
