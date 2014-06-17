#include <RAT/SimpsIntegral.hh>

using namespace std;

namespace RAT{

  double SimpsIntegral(const vector<double>& samples, double baseline,
		       int start, int end){
    int len;
    double qsum = 0.0;
    if ((end - start) % 2 == 0){
      /* If there are an even number of samples, then there are an odd
	 number of intervals; but Simpson's rule works only on an even
	 number of intervals. Therefore we use Simpson's method on the
	 all but the final sample, and integrate the last interval
	 using the trapezoidal rule */
      len = end - start - 1;
      qsum += (samples[end-1] + samples[end-2] - 2*baseline)/2.0;
    }
    else
      len = end - start;
    
    double qsimps;
    qsimps = samples[start]-baseline;
    for (int i=start; i<start+len; i+=2)
      qsimps += (samples[i]-baseline)*4;
    for (int i=start+1; i<len+start-1; i+=2)
      qsimps += (samples[i]-baseline)*2;
    qsimps += samples[start+len-1]-baseline;
    qsimps /= 3.0;
    
    qsum += qsimps;
    return qsum;
  }
}
