///////////////////////////////////////////////////////////////////////////////
/// \class RAT::MiniCleanPMTCharge
///
/// \brief  Implementation of PMTCharge using MiniCLEAN's parameterization.
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details See MiniCLEAN docs for details. This is the default model if no 
///          other model is specified.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_MiniCleanPMTCharge__
#define __RAT_MiniCleanPMTCharge__

#include <vector>
#include <cstddef>
#include <CLHEP/Random/RandGeneral.h>
#include "TH1.h"
#include <RAT/PMTCharge.hh>

namespace RAT {

class MiniCleanPMTCharge : public PMTCharge {
public:
  MiniCleanPMTCharge();
  virtual ~MiniCleanPMTCharge();
  /** Returns charge for one photoelectron. */
  virtual double PickCharge() const;

  /** Value of charge PDF at charge q (not normalized) */
  virtual double PDF(double q) const;

protected:
  double Bisection(double Flat) const;
  double PIntegrate(double R) const;
  double Polya(double Q) const;
  double Gamma(double A, double X) const;
  double norm;
  double m; double gamM; double beta;  
  double a1; double b1; double c1; double qP0; double qPa;
};

} // namespace RAT

#endif

