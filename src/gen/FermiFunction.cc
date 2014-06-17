#include <RAT/FermiFunction.hh>

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <complex>

namespace RAT {

  double Nucl_Beta(int Beta, double Z, double A, double W,
		   double W0, int N, double vMass)
  {
    double Ft = 0.;
    double P = sqrt(pow(W, 2) - 1.);
    double V0 = Screening_Potential(Z, P, Beta);
    if (W0 < 1. + V0) V0 = 0.;
    if (W <= 1. + V0)
      return Ft;
    if (W >= W0)
      return Ft;
    if (Beta == 0)
      return Ft;

    for (int k = 1; k <= N + 1; k++) {
      double Lambda = Nucl_Wave(Beta, Z, A, W, k);
      int L = 2 * k - 1;
      int M = 2 * (N - k + 1) + 1;
      double Phase = sqrt(pow((W0 - W), 2) - pow(vMass, 2)) * (W0 - W);
      double S = Factorial(2 * N + 1) * P * W * Phase;
      double Df = S * Lambda * pow(P, (L - 1)) * pow((W0 - W), (M - 1));
      Df = Df / Factorial(L) / Factorial(M);
      Ft = Ft + Df;
    }
    return Ft;
  }

  double Nucl_Wave(int Beta, double Z, double A, double W,
		   int k)
  {

    double Wave = 0.;
    double P = sqrt(pow(W, 2) - 1.);
    double V0 = Screening_Potential(Z,P,Beta);
    double WPrime = W - V0;
    double PPrime = sqrt(pow(WPrime,2) - 1.);
    double R = Nucl_Radius(A);
    double FK = Nucl_Wave_Phase(Beta, Z, A, WPrime, +k, +1);
    double GK = Nucl_Wave_Phase(Beta, Z, A, WPrime, -k, -1);
    double Q = k + 0.5;
    double L = pow(2., k) * exp(GammaLn(Q)) / sqrt(M_PI);
    double Lambda = (pow(GK, 2) + pow(FK, 2)) * pow(L,
						    2) / (2. * pow(P, 2)) /
      pow((P * R),
	  (2. * k - 2.));
  
    double Coulomb_corr = (PPrime * WPrime)/(P*W) * pow((PPrime/P),2.*k-2.);

    double Lambda_corr = Nucl_Size(Beta, Z, W, k);

    Wave = Lambda * Lambda_corr * Coulomb_corr;
    return Wave;
  }

  double Nucl_Size(int Beta, double Z, double W, int k)
  {
  
    double Size = 1.;
    double R = 0.;
  
    if ((k > 1) || (Beta == 0))
      return Size;
  
    if ((Beta < 0) && (Z < 50.))
      return Size;
  
    if ((Beta > 0) && (Z < 80.))
      return Size;
  
    if (Beta < 0) {
      R = (Z - 50.) * (-25.e-04 - 4e-06 * W * (Z - 50.));
    } else {
      R = (Z - 80.) * (-17.e-05 * W + 6.3e-04 / W - 8.8e-03 / pow(W, 2));
    }
    Size = 1. + R;
    return Size;
  }

  double Nucl_Mass(double A, double Z)
  {
    const double m1H = UnitMass * (1.0079);
    const double mn  = UnitMass * (1.00866501);

    double Mass = 0. ;

    double av   = 15.5 ; // MeV
    double as   = 16.8 ; // MeV
    double ac   =  0.72; // MeV
    double asym = 23.0 ; // MeV
    double ap   = 34.0 ; // MeV

    double delta = 0.;
    int iZ = (int) Z;
    int iA = (int) A;
    int iN = iA - iZ;

    bool evenZ = ((iZ%2)==0);
    bool evenN = ((iN%2)==0);
      
    if ((evenZ) && (evenN))   delta +=ap * pow(A,-0.75);
    if ((!evenZ) && (!evenN)) delta -=ap * pow(A,-0.75);

    double BindE = av * A - as * pow(A,2./3.) - ac*Z*(Z-1.)*pow(A,-1./3.);
    BindE += delta -asym * pow(A-2.*Z,2.)/A;

    Mass = Z * m1H + (A - Z) * mn - BindE;

    return Mass;

  }


  double Nucl_Radius(double A)
  {
  
    double Radius = 0.;
    Radius = (0.002908 * pow(A,(1./3.)) - 0.002437 * pow(A, (-1./3.)));
    return Radius;
  
  }

  double Nucl_Wave_Phase(int Beta, double Z, double A,
			 double W, int k, int sel)
  {
  
    const double AlphaConstant = 1. / 137.03599911;
    std::complex < double >Phase_1, Phase_2, Phase;
  
    double F = 0.;
    double Gamma = sqrt(k * k - pow(AlphaConstant, 2) * pow(Z, 2));
    double P = sqrt(pow(W, 2) - 1.);
    double R = Nucl_Radius(A);
    double Y = Beta * AlphaConstant * Z * W / P;
    double Factor = pow((2. * P * R),
			Gamma) * exp(M_PI * Y / 2.) / 2. / R / sqrt(W);
  
    Phase_1 = exp(std::complex < double >(0., 2. * P * R));
    Phase_2 =
      -(std::complex < double >(k, -Y / W)) /(std::complex < double >(Gamma, Y));
    Phase = Phase_1 * Phase_2;
    std::complex < double >Factor_C;
    if ((1. - sel * W) > 0) {
      Factor_C = std::complex < double >(Factor * sqrt(1. - sel * W), 0.);
    } else {
      Factor_C = std::complex < double >(0., Factor * sqrt(sel * W - 1.));
    }
  
    std::complex < double >A1(Gamma + 1., +Y);
    double B = 2. * Gamma + 1.;
    std::complex < double >C(0., 2. * P * R);
    std::complex < double >F1 = Hyper1F1Norm(A1, B, C);
    std::complex < double >A2(Gamma, +Y);
    std::complex < double >F2 = Hyper1F1Norm(A2, B, C);
    std::complex < double >X(Gamma, +Y);
    std::complex < double >C1(k, -Y );
    std::complex < double >C2(Gamma, -Y );
    std::complex < double >S;
    std::complex < double >Lambda = GammaLn_Complex(X);
  
    if (sel < 0) {
      S = -Factor_C * (C1 * F1 - C2 * F2) * abs(exp(Lambda));
    } else {
      S = -Factor_C * (C1 * F1 + C2 * F2) * abs(exp(Lambda));
    }
    F = abs(S * Phase);
    return F;
  }

  double Screening_Potential(double Z, double p, int Beta){

    /* The Coulomb screening potential comes from a 
       parametrization of J.J. Matese and W.R. Thompson,
       Phys. Rev. 150, 846 (1966). 
       Correction value is returned in units of electron masses in eV.
    */

    double Vc[4] = {27.73915, 3.9061, 0.23268, 0.};
    double As[4] = {-0.1020e-0,+0.238e-2,+0.101e-4,-0.111e-6};
    double Bs[4] = {+0.1560e-1,-0.360e-4,+0.383e-5,+0.242e-7};
    double ATerm = 0.;
    double BTerm = 0.;
    double VTerm = 0.;
    for(int i = 0; i<4; i++){
      ATerm += As[i] * pow(Z,i);
      BTerm += Bs[i] * pow(Z,i);
      VTerm += Vc[i] * pow(log(Z),i);
    }

    double Vs = 0.;
    if (p <= 0.) return Vs;
    Vs = VTerm * pow(Z,4./3.) * exp(-ATerm /p - BTerm / pow(p,2));
    Vs *= Beta; 
    Vs /= (ElectronMass * 1.e+06);

    return Vs;
  }

  std::complex < double >Hyper1F1Norm(std::complex < double >A,
				 std::complex < double >B,
				 std::complex < double >Z)
  {
  
    /*  HyperGeometric 1F1 Normalized function
      
    F = (Sum k = 1, inf) A_k / B_k * Z**k / k!
    where A_k = A + (k-1) and B_k = B + (k-1)
  
    */
  
    const int nCount = 50;
    std::complex < double >F  (1.,0.);
    std::complex < double >An (1.,0.);
    std::complex < double >Bn (1.,0.);
    std::complex < double >ZToTheK (1.,0.);
    double kFactorial = 1.0;
  
    for (int k = 1; k < nCount; k++) {
      std::complex < double > fK (k - 1.,0.);
      An *= (A + fK);
      Bn *= (B + fK);
      ZToTheK *= Z;
      kFactorial *= double(k);
      F += (An / Bn) * ZToTheK / kFactorial;
    }
    std::complex < double >Lambda = GammaLn_Complex(B);
    F = F / exp(Lambda);
    return F;
  }

  double Factorial(int N)
  {
    if(N <= 1) return 1;
    else return exp(GammaLn(double(N) + 1.0));

    /*  
    // Joe's old implementation
    // Factorial function
      
    F(N) = N! = 1 * 2 * 3 * ... N
  
    Limited to N less than 100

  
  
    double F = 1.;
    if (N < 1)
    return F;
    if (N > 100) {
    printf("The factorial call %d is too large.\n", N);
    return F;
    }
  
    for (int i = 1; i <= N; i++) {
    F = F * double (i);
    }
    return F;
    */
  }


  double HyperGeometric_PQF(double A[], int nA, double B[],
			    int nB, double Z)
  {
  
    /*  HyperGeometric PQF function.  Performs a decrete summation
      
    PQF = Sum(k=1,inf) (a1 * a2 * an)_k / (b1 * b2 * bn)_k * z^k / k!
    where a(s)_k = a(s)+k
  
    Requires Factorial function to be defined.
    */
  
  
    const int nK = 10;
  
    double F = 0.;
    double ATerm = 1.;
    double BTerm = 1.;
    double CTerm = 0.;
  
    // Check if indicies are valid
  
    if ((nA < 0) || (nB < 0))
      return F;
  
    // Perform summation up to nK
  
    for (int k = 1; k <= nK; k++) {
    
      for (int i = 0; i < nA; i++) {
	ATerm = ATerm * (A[i] + k - 1.);
      }
    
      for (int i = 0; i < nB; i++) {
	BTerm = BTerm * (B[i] + k - 1.);
      }
    
      CTerm = ATerm / BTerm / Factorial(k);
    
      F = F + CTerm * pow(Z, k);
    
    }
  
    return F;
  
  }

  double GammaLn(double xx)
  {
  
    double cof[6] = { 76.18009172947146,
		      -86.50532032941677,
		      24.01409824083091,
		      -1.231739572450155,
		      .001208650973866179,
		      -.000005395239384953
    };
  
    double stp = 2.5066282746310005;
    double x = xx;
    double y = x;
    double tmp = x + 5.5;
    tmp = (x + 0.5) * log(tmp) - tmp;
    double ser = 1.000000000190015;
    for (int j = 0; j < 6; j++) {
      y = y + 1.;
      ser = ser + cof[j] / y;
    }
    double gammln = tmp + log(stp * ser / x);
    return gammln;
  }

  std::complex < double > GammaLn_Complex(std::complex < double >xx)
  {
  
    double cof[6] = { 76.18009172947146,
		      -86.50532032941677,
		      24.01409824083091,
		      -1.231739572450155,
		      .001208650973866179,
		      -.000005395239384953
    };
  
    double stp = 2.5066282746310005;
    std::complex < double >x = xx;
    std::complex < double >y = x;
    std::complex < double >temp1(5.5, 0.);
    std::complex < double >temp2(0.5, 0.);
    std::complex < double >tmp = x + temp1;
    tmp = (x + temp2) * log(tmp) - tmp;
    std::complex < double >ser(1.000000000190015, 0.);
  
    for (int j = 0; j < 6; j++) {
      y = y + 2. * temp2;
      ser = ser + cof[j] / y;
    }
    std::complex < double >gammln_cpx = tmp + log(stp * ser / x);
    return gammln_cpx;
  }

} // namespace RAT
