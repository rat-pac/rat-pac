#ifndef __RAT_MultiChargeDist__
#define __RAT_MultiChargeDist__

#include <vector>
#include <TGraph.h>

namespace RAT {

  /** Generates the normalized n-photoelectron charge distributions 

      maxPE: Generate n-photoelectron charge distributions from 0 to maxPE (inclusive)
      qMaxStepSize: Compute distribution with at most this  step size in charge units
      qBins: compute the distribution with this many bins unless the step size is too larg
      
      meanSinglePE, rmsSinglePE: Return the mean and RMS of the 1 pe charge distribution.
   */
  std::vector<TGraph*> MultiChargeDist(int maxPE, double qMaxStepSize,
				       int qBins, double meanSinglePE);
					 

} // namespace RAT


#endif
