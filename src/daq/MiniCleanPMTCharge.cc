#include <cmath>
#include <algorithm>
#include <vector>
#include <CLHEP/Random/RandGauss.h>
#include <CLHEP/Random/RandGeneral.h>
#include <CLHEP/Random/RandFlat.h>
#include <TMath.h>
#include <TH1.h>
#include <MiniCleanPMTCharge.hh>

namespace RAT {

MiniCleanPMTCharge::MiniCleanPMTCharge() {
  m = 8.1497;
  a1 = 0.27235E-01; 
  b1 = 0.7944E-03; 
  c1 = 0.81228E-01; 
  qP0 = .59; 
  qPa = 4.41;
  gamM = TMath::Gamma(m);
  beta = pow(m,m)/gamM;
  norm = 1.0/PIntegrate(50);
}

MiniCleanPMTCharge::~MiniCleanPMTCharge() {}

double MiniCleanPMTCharge::Polya(double Q) const {
  return norm*(beta*pow(Q,m-1)*(a1*exp(-m*Q)+(b1*pow(1/qPa, m-1)*exp(-m*Q/qPa)))+(c1*exp(-Q/qP0)));
}

double MiniCleanPMTCharge::Gamma(double A, double X) const {
  double d = TMath::Gamma(A);
  double g = TMath::Gamma(A,X);
  return d*(1.0-g);
}

double MiniCleanPMTCharge::PIntegrate(double R) const {
  double gamMR = Gamma(m, m*R);
  double gamMRQ = Gamma(m, m*R/qPa);
  double firstTerm = c1*qP0*(1.0-exp(-1.0*R/qP0));
  double secondTerm = a1*(1.0-(gamMR/gamM));
  double thirdTerm = b1*qPa*(1.0-(gamMRQ/gamM));
  return firstTerm+secondTerm+thirdTerm;
}

double MiniCleanPMTCharge::Bisection(double FLAT) const {
  double L1 = 0.000000001;
  double L2 = 50;
  double L3 = 0.0;
  double f1;
  double f2;
  double f3;
  for(int a = 0; a <= 35; a++)
  {
    f1 = FLAT-norm*PIntegrate(L1);
    f2 = FLAT-norm*PIntegrate(L2);
    if(f1*f2<0)  L3 = L2-(fabs(L1-L2)/2.00);
    f3 = FLAT-norm*PIntegrate(L3);
    if(f1*f3<0)  L2 = L3;
    else  L1 = L3;
  }
  return (L1+L2)/2.00;
}

double MiniCleanPMTCharge::PickCharge() const {
  double RFLAT = CLHEP::RandFlat::shoot(1);
  return Bisection(RFLAT);
}

double MiniCleanPMTCharge::PDF(double q) const {
  return Polya(q);
}

} // namespace RAT

