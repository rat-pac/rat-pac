#ifndef __RAT_SimpleDAQProc__
#define __RAT_SimpleDAQProc__

#include <RAT/Processor.hh>

namespace RAT {


class SimpleDAQProc : public Processor {
public:
  SimpleDAQProc();
  virtual ~SimpleDAQProc() { };
  virtual Processor::Result DSEvent(DS::Root *ds);

protected:
  int fEventCounter;
  std::vector<double> fSPECharge;
};


} // namespace RAT

#endif
