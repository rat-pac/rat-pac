#ifndef __RAT_EventSplitter__
#define __RAT_EventSplitter__

#include <RAT/Processor.hh>

namespace RAT {


class EventSplitter : public Processor {
public:
  EventSplitter();
  virtual ~EventSplitter() { };
  virtual Processor::Result DSEvent(DS::Root *ds);

protected:
  int fEventCounter;
  std::vector<double> fSPECharge;
};


} // namespace RAT

#endif
