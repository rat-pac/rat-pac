#ifndef __RAT_EventSplitter__
#define __RAT_EventSplitter__

#include <RAT/Processor.hh>

namespace RAT {


class EventSplitter : public Processor {
public:
  EventSplitter();
  virtual ~EventSplitter() { };

  virtual void SetI(std::string param, int value);

  virtual Processor::Result DSEvent(DS::Root *ds);


protected:
  int fEventCounter;
  double clockSpeed;
  double slidingWindow;
  double collectionWindow;
  double nhitThresh;
  std::vector<double> fSPECharge;
  std::vector<double> chargeVec;
  std::vector<double> timeVec;
  std::vector<double> pmtIDVec;
};


} // namespace RAT

#endif
