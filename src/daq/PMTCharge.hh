#ifndef __RAT_PMTCharge__
#define __RAT_PMTCharge__

#include <vector>
#include <cstddef>
#include <CLHEP/Random/RandGeneral.h>
#include "TH1.h"
namespace RAT {

class PMTCharge {
public:
  PMTCharge();
  virtual ~PMTCharge();
  /** Returns charge for one photoelectron. */
  virtual float PickCharge() const;

  /** Value of charge PDF at charge q (not normalized) */
  virtual float PDF(float q) const;

protected:
  float Bisection(double Flat) const;
  double PIntegrate(double R) const;
  float Polya(float Q) const;
  double Gamma(double A, double X) const;
  double norm;
  float m; float gamM; float beta;  
  float a1; float b1; float c1; float qP0; float qPa;
};

} // namespace RAT

#endif

