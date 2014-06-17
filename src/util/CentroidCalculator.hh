/** @class CentroidCalculator
 *
 *  Accumulate a weighted series of 3D points and compute the mean and
 *  RMS of the distribution at the end.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  Call Fill() method for each point, and then call GetMean() and
 *  GetRMS() to get the results at the end.
 */

#ifndef __RAT_CentroidCalculator__
#define __RAT_CentroidCalculator__
#include <TVector3.h>
#include <math.h>

namespace RAT {

class CentroidCalculator {
public:
  /** All accumulators cleared by default in constructor */
  CentroidCalculator() { Reset(); };
  
  /** Clear all accumulators */
  void Reset() { 
    fMoment0 = 0.0;
    fMoment1.SetXYZ(0.0,0.0,0.0);
    fMoment2.SetXYZ(0.0,0.0,0.0);
  };

  /** Add another point with given weight. */
  void Fill(const TVector3 &point, double weight=1.0) {
    fMoment0 += weight;
    fMoment1 += weight * point;
    TVector3 square(point.X() * point.X(),
		    point.Y() * point.Y(),
		    point.Z() * point.Z());
    fMoment2 += weight * square;
  };

  /** Add the information from another CentroidCalculator to this one */
  void Add(const CentroidCalculator &other) {
    fMoment0 += other.fMoment0;
    fMoment1 += other.fMoment1;
    fMoment2 += other.fMoment2;
  };

  /** Mean of each coordinate */
  TVector3 GetMean() const { return fMoment1 * (1.0 / fMoment0); };
  
  /** RMS of each coordinate */
  TVector3 GetRMS() const {
    TVector3 squareMoment1(fMoment1.X() * fMoment1.X(),
			   fMoment1.Y() * fMoment1.Y(),
			   fMoment1.Z() * fMoment1.Z());
    TVector3 temp = fMoment0 * fMoment2 - squareMoment1;
    
    return TVector3(sqrt(temp.X()), sqrt(temp.Y()), sqrt(temp.Z())) 
      * (1.0 / fMoment0);
  };

protected:
  double fMoment0;
  TVector3 fMoment1;
  TVector3 fMoment2;
};

} // namespace RAT

#endif
