/** \file CfSource.cc
 *  CfSource C++ file.  Implements the constructor, copy 
 *  constructor, and overloaded = operator and defines the 
 *  Cf252NeutronSpectrum function.
 *
 *  Author: Matthew Worcester
 */
#include <RAT/CfSource.hh>

#include <G4ParticleDefinition.hh>
#include <G4Gamma.hh>
#include <G4Neutron.hh>

#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Random/RandGeneral.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <cmath>
#include <iostream>
#include <fstream>   // file I/O
#include <iomanip>   // format manipulation
#include <cstring>
#include <vector>

#undef DEBUG

namespace RAT {

  long long factorial(int n);

  double CfSource::massNeutron = 0.; // allocate storage for static variable

  CfSource::CfSource(int newIsotope) :
    Isotope(newIsotope)
  {
    // Cf252
    if(Isotope == 252){

      // Verify that all maps and vectors are empty.
      Nneutron = 0;
      Ngamma = 0;
      neutronE.clear();
      Tneutron.clear();
      gammaE.clear();
      Tgamma.clear();

      // setup the probability density as a function of energy

      // Random generators according to probability densities.
      static CLHEP::RandGeneral* fGenerate = 0;
      static CLHEP::RandGeneral* mGenerate = 0;
      static CLHEP::RandGeneral* gGenerate = 0;

      static const float flow = 0.; static const float fhigh = 50.;
      static const float mlow = 0.; static const float mhigh = 25.;
      static const float glow = 0.; static const float ghigh = 25.;

      static bool first = true;
      if (first)
	{
	  first = false;

	  // Initialize the G4 particle definitions.
	  G4ParticleDefinition* neutron = G4Neutron::Neutron();
	  massNeutron = neutron->GetPDGMass() * CLHEP::MeV;

	  // In the original code, the probability densities used the
	  // funlxp and funlux routines in CERNLIB to generate random
	  // numbers.  The following code uses CLHEP to generate the
	  // same "histograms" for the RandGeneral random-number
	  // generator.

	  const size_t probDensSize = 200;
	  double fspace[probDensSize];
	  double mspace[probDensSize];
	  double gspace[probDensSize];

#ifdef DEBUG
	  std::cout << "CfSource initialization" << std::endl;
#endif
	  // Find function values at bin centers.
	  for ( size_t i=0; i != probDensSize; i++ )
	    {
	      float value = (float(i) + 0.5) * (fhigh - flow) / (float) probDensSize;
	      fspace[i] = Cf252NeutronSpectrum(value);
	      value = (float(i) + 0.5) * (mhigh - mlow) / (float) probDensSize;
	      mspace[i] = Cf252GammaMultiplicityFit(value);
	      value = (float(i) + 0.5) * (ghigh - glow) / (float) probDensSize;
	      gspace[i] = Cf252GammaSpectrum(value);
#ifdef DEBUG
	      std::cout << "   i=" << i << " f,m,g="
			<< fspace[i] << ","
			<< mspace[i] << ","
			<< gspace[i] << std::endl;
#endif
	    }

	  // Define random-number generators.
	  fGenerate = new CLHEP::RandGeneral(fspace,probDensSize);
	  mGenerate = new CLHEP::RandGeneral(mspace,probDensSize);
	  gGenerate = new CLHEP::RandGeneral(gspace,probDensSize);

#ifdef DEBUG
	  std::cout << " Random generator test (f,m,g):" << std::endl;
	  for ( size_t i=0; i != 20; i++ )
	    {
	      std::cout << i << ": "
			<< fGenerate->shoot() * (fhigh - flow) + flow << ", "
			<< mGenerate->shoot() * (mhigh - mlow) + mlow << ", "
			<< gGenerate->shoot() * (ghigh - glow) + glow << std::endl;
	    }

#endif
	} 

      //
      // neutron multiplicity distribution (via Los Alamo manual)
      //
      static const double p[maxNeutron+1] = {0.002,0.028,0.155,0.428,
					     0.732,0.917,0.983,0.998,1.000};

      // pick a neutron multiplicity
      bool passed = false;
	
      while(!passed){
	float r = CLHEP::RandFlat::shoot(); // Random number from 0 to 1.

	for(int i=0;i<maxNeutron+1; i++){

	  if(p[i] > r && !passed){
	    Nneutron = i;
	    if (Nneutron) passed = true;
	  }
	}
      }
      //std::cout << "   " << Nneutron << " neutrons" << std::endl;
      //
      // pick a momentum direction for each neutron
      //
      for (int nn=0; nn<Nneutron; nn++)
	{
	  double neutronKE = fGenerate->shoot() * (fhigh - flow) + flow;
	  double energy = massNeutron + neutronKE;
	  // Generate momentum direction uniformly in phi and cos(theta).
	  double phi = CLHEP::RandFlat::shoot(0.,M_PI);
	  double cosTheta = CLHEP::RandFlat::shoot(-1.,1.);
	  double sinTheta = sqrt( 1. - cosTheta*cosTheta );
	  double px = neutronKE * sinTheta * cos(phi);
	  double py = neutronKE * sinTheta * sin(phi);
	  double pz = neutronKE * cosTheta;
#ifdef DEBUG
	  std::cout << "CfSource::CfSource() - neutron energy " 
		    << nn << " = " << energy
		    << ", KE=" << neutronKE
		    << ", (px,py,pz)=("
		    << px << "," << py << "," << pz << ")"
		    << std::endl;
#endif
	  CLHEP::HepLorentzVector momentum(px,py,pz,energy);
	  neutronE.push_back( momentum );
	  Tneutron.push_back( 0. );
	}

      /*
	The total energy in the prompt gammas is a function of the
	material and the number of prompt neutrons produced; these 
	formulae come from T. Valentine's summary
      */
      //double Z = 98.;
      //double A = 252.;
      //
      //double avge;
      //double phi;
      //double etot;
      //
      //double power = 0.3333;
      //avge = -1.33 + 119.6*pow(Z,power)/A;
      //phi = 2.51 - 1.13e-5*Z*Z*sqrt(A);
      //etot = phi*Nneutron+4.0;
      //
      // use the Brunson multiplicity (also via Valentine)
      //
      double m = mGenerate->shoot() * (mhigh - mlow) + mlow;
      Ngamma = (int) m;

#ifdef DEBUG
      std::cout << "CfSource::CfSource - " 
		<< "m=" << m << " => "
		<< Ngamma << " photons" << std::endl;
#endif
      // pick a momentum for each gamma
      //
      double tote = 0.;
      for(int nn=0; nn<Ngamma; nn++)
	{
	  double energy = gGenerate->shoot() * (ghigh - glow) + glow;
	  // Generate momentum direction uniformly in phi and cos(theta).
	  double phi = CLHEP::RandFlat::shoot(0.,M_PI);
	  double cosTheta = CLHEP::RandFlat::shoot(-1.,1.);
	  double sinTheta = sqrt( 1. - cosTheta*cosTheta );
	  double px = energy * sinTheta * cos(phi);
	  double py = energy * sinTheta * sin(phi);
	  double pz = energy * cosTheta;
#ifdef DEBUG
	  std::cout << "CfSource::CfSource() - gamma energy " 
		    << nn << " = " << energy
		    << ", (px,py,pz)=("
		    << px << "," << py << "," << pz << ")"
		    << std::endl;
#endif
	  CLHEP::HepLorentzVector momentum(px,py,pz,energy);
	  gammaE.push_back( momentum );
	  tote += energy;
	  
	  const size_t len2 = 2;
	  float rv[len2];
	  for ( size_t i=0; i<len2; i++ ) rv[i] = CLHEP::RandFlat::shoot();
	  //
	  // 80% of gammas have T_1/2 = 0.01 ns and 20% have T_1/2 = 1 ns
	  //
	  double halflife;
	  if(rv[0] < 0.8)
	    halflife = 0.01;
	  else
	    halflife = 1.0;
	  Tgamma.push_back( halflife*log(1/rv[1]) );
	}
      //std::cout << "          total energy = " << tote << std::endl;

    } // done with Cf 252

    if(Isotope == 255){
      Nneutron = 0;
      Ngamma = 0;
      neutronE.clear();
      Tneutron.clear();
      gammaE.clear();
      Tgamma.clear();
    } // done with Cf 255
  }

  CfSource::~CfSource()
  {;}

  CfSource::CfSource(const CfSource& _CfSource)
  {
    Isotope   = _CfSource.Isotope;
    Nneutron  = _CfSource.Nneutron;
    Ngamma    = _CfSource.Ngamma;
    neutronE  = _CfSource.neutronE;
    Tneutron  = _CfSource.Tneutron;
    gammaE    = _CfSource.gammaE;
    Tgamma    = _CfSource.Tgamma;
  }    

  CfSource& CfSource::operator=(const CfSource& rhs){

    if (this != &rhs)
      {
	Isotope   = rhs.Isotope;  
	Nneutron  = rhs.Nneutron;
	Ngamma    = rhs.Ngamma;
	neutronE  = rhs.neutronE;
	Tneutron  = rhs.Tneutron;
	gammaE    = rhs.gammaE;
	Tgamma    = rhs.Tgamma;
      }
    return *this;
  }    

  float CfSource::Cf252NeutronSpectrum(const float& x){

    // return the neutron spectrum N(x)
    float N = 0.;

    float scale = 1/(2*M_PI*0.359);
    //std::cout << "scale " << scale << std::endl;

    float fminus = exp(-(sqrt(x)-sqrt(0.359))*(sqrt(x)-sqrt(0.359))/1.175);
    float fplus  = exp(-(sqrt(x)+sqrt(0.359))*(sqrt(x)+sqrt(0.359))/1.175);

    N = scale*(fminus-fplus);

    //std::cout << "N " << N << std::endl;
    return N;
  }

  float CfSource::Cf252GammaMultiplicity(const int& x){

    // return the gamma multiplicity M(x)
    double M = 0.;

    double C1 = 0.675;
    double C2 = 6.78;
    double C3 = 9.92;

    M = C1*pow(C2,x)*exp(-C2)/factorial(x) + (1-C1)*pow(C3,x)*exp(-C3)/factorial(x);

    //std::cout << "M(" << x << ") " << M << std::endl;

    return M;
  }

  long long factorial(int n){

    // cannot return above n = 20 because of memory
    if(n < 1)
      return 1;
    else
      return n * factorial(n-1);
  }

  float CfSource::Cf252GammaMultiplicityFit(const float& x){

    // return the gamma multiplicity M(x)
    double M = 0.;

    float gaussian = 0.13345*exp(-0.5*((x-7.0322)/2.6301)*((x-7.0322)/2.6301));
    float exponential = 0.11255*(exp(-(sqrt(x)-sqrt(7.5987))
				     *(sqrt(x)-sqrt(7.5987))/0.56213));

    if(x <= 9.0){
      M = gaussian;
    }
    else{
      M = exponential;
    }

    //std::cout << "M(" << x << ") " << M << std::endl;

    return M;
  }

  float CfSource::Cf252GammaSpectrum(const float& x){

    // return the gamma spectrum N(x)
    float N = 0.;

    float gaussian = 1.8367*exp(-0.5*((x-0.45934)/0.31290)*((x-0.45934)/0.31290));
    float exponential = exp(0.84774-0.89396*x);

    if(x <= 0.744){
      N = gaussian;
    }
    else{
      N = exponential;
    }
  
    //std::cout << "N " << N << std::endl;
    return N;
  }

} // namespace RAT
