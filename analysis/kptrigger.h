#ifndef __KPTRIGGER__
#define __KPTRIGGER__

#include <iostream>
#include <string>
#include <vector>

#include "RAT/DS/MC.hh"

class KPPulse {
  
 public:
  
  KPPulse();
  ~KPPulse();

  double tstart;
  double tend;
  double tpeak;
  double peakamp;

  typedef enum { kRising, kFalling, kDefined } Status;
  Status fStatus;

  double last_max;
  int nfallingbins;
  
};

typedef std::vector< KPPulse* > KPPulseList;
typedef std::vector< KPPulse* >::iterator KPPulseListIter;

int find_trigger( RAT::DS::MC* mc, double threshold, double window_ns, double tave_ns, double decay_constant, KPPulseList& pulses, int first_od_sipmid=90000, bool veto=false );


#endif
