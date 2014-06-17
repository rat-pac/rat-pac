#ifndef __RAT_PMTTime__
#define __RAT_PMTTime__

#include <RAT/DB.hh>

namespace RAT {

class PMTTime {
public:
  PMTTime();
  virtual ~PMTTime();

  /** Returns front end time for hit time. */
  virtual double PickTime(double time);

protected:
  std::vector<double> x;
  std::vector<double> y;
  double cableDelay;
};

} // namespace RAT

#endif

