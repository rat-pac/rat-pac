// RAT::FermiFunction
// 11-Jan-2006 WGS

// I've copied these constants and functions from the original code
// from Joe Formaggio and Jason Detwiler, even though they duplicate
// some definitions that can be found in Geant4.  As a precaution,
// I've put them in the RAT namespace.  If one were being complete,
// one would revise contstants like "ElectronMass" to be the Geant4
// definitions.

#ifndef __RAT_FermiFunction__
#define __RAT_FermiFunction__

#include <complex>

namespace RAT {

  static const double ElectronMass = 0.510998918;
  static const double AlphaMass    = 3727.42;
  static const double UnitMass = 931.502; // MeV

  double GetRandomNumber(double rmin = 0., double rmax = 1.);

  double Nucl_Beta(int Beta, double Z, double A, double W, double W0,
		   int N, double vMass);

  double Nucl_Wave(int Beta, double Z, double A, double W, int k);

  double Nucl_Size(int Beta, double Z, double W, int k);

  double Nucl_Radius(double A);

  double Nucl_Mass(double A, double Z);

  double Nucl_Wave_Phase(int Beta, double Z, double A, double W, int k,
			 int sel);

  double Screening_Potential(double Z, double p, int Beta);

  std::complex < double >Hyper1F1Norm(std::complex < double >A,
				      std::complex < double >B,
				      std::complex < double >Z);

  double Factorial(int N);

  double HyperGeometric_PQF(double A[], int nA, double B[], int nB,
			    double Z);

  double GammaLn(double xx);

  std::complex < double >GammaLn_Complex(std::complex < double >xx);

} // namespace RAT

#endif
