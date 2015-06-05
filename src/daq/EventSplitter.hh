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
  double offsetToWindow;
  double collectionWindow;
  double nhitThresh;
  //Standard vector to fill the two matrices
  std::vector<double> Vec;
  //MAtrix that include all PMT time, charge and ID
  std::vector <std::vector <double> > matr;
  //Matrix of subevent information, include event trig time,
  //and PMT count in subevent event window
  std::vector <std::vector <double> > recordSubTime;


};


} // namespace RAT

#endif
