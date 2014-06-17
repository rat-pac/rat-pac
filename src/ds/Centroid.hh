/**
 *  @class DS::Centroid
 *
 *  Data Structure: Position fit using charge centroid
 */
#ifndef __RAT_DS_Centroid__
#define __RAT_DS_Centroid__

#include <RAT/DS/PosFit.hh>

namespace RAT {
  namespace DS {

class Centroid : public TObject, public PosFit {
public:
  Centroid() : TObject(), PosFit("centroid") {}
  virtual ~Centroid() {}

  /* Position attributes inherited from PosFit */

  /** Exponent used in charge-weighting */
  virtual double GetPower() const { return power; }
  virtual void SetPower(double _power) { power = _power; }
     
  ClassDef(Centroid, 1)
       
protected:
  double power;
};

  } // namespace DS
} // namespace RAT

#endif

