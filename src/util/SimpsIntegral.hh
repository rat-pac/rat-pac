#ifndef __RAT_SimpsIntegral__
#define __RAT_SimpsIntegral__

#include <vector>

using namespace std;

namespace RAT{
  /* Integrate the samples from sample number start to end the samples 
     assuming a fixed baseline using simpson's method. */
  double SimpsIntegral(const vector<double>& samples, double baseline,
		       int start, int end);
}

#endif
