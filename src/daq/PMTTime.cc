#include <cmath>
#include <algorithm>
#include <vector>
#include <Randomize.hh>
#include <RAT/PMTTime.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>

using namespace std;

namespace RAT {

PMTTime::PMTTime() {
  DBLinkPtr ltransit = DB::Get()->GetLink("PMTTRANSIT");
  x = ltransit->GetDArray("x");
  y = ltransit->GetDArray("y");
  cableDelay = ltransit->GetD("cable_delay");
}

PMTTime::~PMTTime() {}

double PMTTime::PickTime(double time) {
  double ttransit = 0;
  double frontend = 0;
  double rand = G4UniformRand();
  unsigned int up = 0;

  for (unsigned int i = 0; i < x.size(); i++) {
    if ( rand < y[i] ){
      up = i;
      break;
    }
  }
  // Check for up > 0 so up-1 doesn't cause a segfault.
  // This case, switch to extrapolation, since 0<rand<y[0]
  if (up == 0) {
    up = 1;
  }
  ttransit = (rand - y[(up-1)]) * (x[up] - x[(up-1)])/(y[up] - y[(up-1)]) + x[(up-1)];
  frontend = time + ttransit + cableDelay;
  return frontend;
}
  
} // namespace RAT

