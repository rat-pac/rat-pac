#include <cmath>
#include <algorithm>
#include <vector>
#include <CLHEP/Random/RandGauss.h>
#include <CLHEP/Random/RandGeneral.h>
#include <CLHEP/Random/RandFlat.h>
#include <TMath.h>
#include <TH1.h>
#include <PMTCharge.hh>

namespace RAT {

PMTCharge::PMTCharge() {
  m = 8.1497;
  a1 = 0.27235E-01; 
  b1 = 0.7944E-03; 
  c1 = 0.81228E-01; 
  qP0 = .59; 
  qPa = 4.41;
  gamM = TMath::Gamma(m);
  beta = pow(m,m)/gamM;
  norm = 1.0/PMTCharge::PIntegrate(50);
}

PMTCharge::~PMTCharge() {}

float PMTCharge::Polya(float Q) const {
  return norm*(beta*pow(Q,m-1)*(a1*exp(-m*Q)+(b1*pow(1/qPa, m-1)*exp(-m*Q/qPa)))+(c1*exp(-Q/qP0)));
}

double PMTCharge::Gamma(double A, double X) const {
  double d = TMath::Gamma(A);
  double g = TMath::Gamma(A,X);
  return d*(1.0-g);
}

double PMTCharge::PIntegrate(double R) const {
  double gamMR = PMTCharge::Gamma(m, m*R);
  double gamMRQ = PMTCharge::Gamma(m, m*R/qPa);
  double firstTerm = c1*qP0*(1.0-exp(-1.0*R/qP0));
  double secondTerm = a1*(1.0-(gamMR/gamM));
  double thirdTerm = b1*qPa*(1.0-(gamMRQ/gamM));
  return firstTerm+secondTerm+thirdTerm;
}

float PMTCharge::Bisection(double FLAT) const {
  float L1 = 0.000000001;
  float L2 = 50;
  float L3 = 0.0;
  float f1;
  float f2;
  float f3;
  for(int a = 0; a <= 35; a++)
  {
    f1 = FLAT-norm*PMTCharge::PIntegrate(L1);
    f2 = FLAT-norm*PMTCharge::PIntegrate(L2);
    if(f1*f2<0)  L3 = L2-(fabs(L1-L2)/2.00);
    f3 = FLAT-norm*PMTCharge::PIntegrate(L3);
    if(f1*f3<0)  L2 = L3;
    else  L1 = L3;
  }
  return (L1+L2)/2.00;
}

float PMTCharge::PickCharge() const {
  double RFLAT = CLHEP::RandFlat::shoot(1);
  return PMTCharge::Bisection(RFLAT);
}

float PMTCharge::PDF(float q) const {
  return Polya(q);
}

} // namespace RAT

