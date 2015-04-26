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
  double pe;
  double z;
  int hits_assigned;
  
};

typedef std::vector< KPPulse* > KPPulseList;
typedef std::vector< KPPulse* >::iterator KPPulseListIter;

int find_trigger( RAT::DS::MC* mc, 
		  double threshold, double window_ns, double tave_ns, 
		  int n_decay_constants, double decay_weights[], double decay_constants_ns[],
		  KPPulseList& pulses, int first_od_sipmid=90000, bool veto=false );
void assign_pulse_charge( RAT::DS::MC* mc, std::string pmtinfo, KPPulseList& pulselist, double decay_const, int first_od_sipmid=90000, bool veto=false );

#endif
