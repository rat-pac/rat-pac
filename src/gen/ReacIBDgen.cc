#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Random/RandFlat.h>
#include <CLHEP/Random/RandGeneral.h>

#include <RAT/ReacIBDgenMessenger.hh>
#include <RAT/ReacIBDgen.hh>
#include <RAT/DB.hh>
#include <fstream>
#include <iostream>

using namespace CLHEP;

namespace RAT {

//#define DEBUG

// Additional constants
const double DELTA = neutron_mass_c2 - proton_mass_c2;

//We start with the Reactor Isotope components given in Marc Bergevin's original
//IBDgenerator file

const double ReacIBDgen::U235DEFAULT = 0.496;
const double ReacIBDgen::U238DEFAULT = 0.087;
const double ReacIBDgen::Pu239DEFAULT = 0.391;
const double ReacIBDgen::Pu241DEFAULT = 0.066;
 
ReacIBDgen::ReacIBDgen()
{
  //initialize your initial isotope values.
  Reset();
  // Get parameters from database
  DBLinkPtr libd = DB::Get()->GetLink("IBD");

  //initialize the messenger to adjust isotope parameters in mac files
  messenger = new ReacIBDgenMessenger(this);

  Emin = 1.806;  //CHANGED TO MATCH THE ENERGY RANGES IN MARC'S FILE FOR NOW
  Emax = 14.000;

}

ReacIBDgen::~ReacIBDgen()
{
  //If there's no messenger adjustments, delete the messenger pointer.
  if ( messenger != 0 )
    {
      delete messenger;
      messenger = 0;
    }
}

  
void ReacIBDgen::GenEvent(const Hep3Vector &nu_dir,
		      HepLorentzVector &neutrino,
		      HepLorentzVector &positron,
		      HepLorentzVector &neutron)
{
  float Enu, CosThetaLab;

  // Pick energy of neutrino and relative direction of positron
  GenInteraction(Enu, CosThetaLab);
  
  // Zero'th order approximation of positron quantities (infinite nucleon mass)
  double E0 = Enu - DELTA;
  double p0 = sqrt(E0*E0 - electron_mass_c2*electron_mass_c2); 
  double v0 = p0/E0;

  // First order correction of positron quantities for finite nucleon mass
  double Ysquared = (DELTA*DELTA - electron_mass_c2*electron_mass_c2)/2;
  double E1 = E0*(1 - Enu/proton_mass_c2*(1 - v0*CosThetaLab))
               - Ysquared/proton_mass_c2;
  double p1 = sqrt(E1*E1 - electron_mass_c2*electron_mass_c2);

  // Compute nu 4-momentum
  neutrino.setVect(nu_dir * Enu); // MeV (divide by c if need real units)
  neutrino.setE(Enu);
  
  // Compute positron 4-momentum
  Hep3Vector pos_momentum(p1*nu_dir);

  // Rotation from nu direction to pos direction.
  double theta = acos(CosThetaLab);
  double phi = 2*pi*HepUniformRand();  // Random phi
  Hep3Vector rotation_axis = nu_dir.orthogonal();
  rotation_axis.rotate(phi, nu_dir);
  pos_momentum.rotate(theta, rotation_axis);

  positron.setVect(pos_momentum);
  positron.setE(E1);
  
  // Compute neutron 4-momentum
  neutron.setVect(neutrino.vect() - positron.vect());
  neutron.setE(sqrt(neutron.vect().mag2() + neutron_mass_c2*neutron_mass_c2));  
}




void ReacIBDgen::GenInteraction(float &E, float &CosThetaLab)
{
    // Pick E from the reactor spectrum and cos(theta) uniformly

    E = GetNuEnergy();
    CosThetaLab = -1.0+2.0*HepUniformRand();
  
}

void ReacIBDgen::SetU235Amplitude (double U235Am)
{
  if ((U235Am < 0.) || (U235Am > 1.))
    {
      G4cerr << "Set your U235 Amplitude between 0 and 1." << G4endl;
      return;
    }
  U235Amp = U235Am;
}

void ReacIBDgen::Reset()
{
  SetU235Amplitude (U235DEFAULT);
  SetU238Amplitude (U238DEFAULT);
  SetPu239Amplitude (Pu239DEFAULT);
  SetPu241Amplitude (Pu241DEFAULT);
}

void ReacIBDgen::SetU238Amplitude (double U238Am)
{
  if ((U238Am < 0.) || (U238Am > 1.))
    {
      G4cerr << "Set your U238 Amplitude between 0 and 1." << G4endl;
      return;
    }
  U238Amp = U238Am;
}

void ReacIBDgen::SetPu239Amplitude (double Pu239Am)
{
  if ((Pu239Am < 0.) || (Pu239Am > 1.))
    {
      G4cerr << "Set your Pu239 Amplitude between 0 and 1." << G4endl;
      return;
    }
  Pu239Amp = Pu239Am;
}

void ReacIBDgen::SetPu241Amplitude (double Pu241Am)
{
  if ((Pu241Am < 0.) || (Pu241Am > 1.))
    {
      G4cerr << "Set your Pu241 Amplitude between 0 and 1." << G4endl;
      return;
    }
  Pu241Amp = Pu241Am;
}


float ReacIBDgen::GetNuEnergy(){
      // This method of setting up probability densities as a function of 
      // Energy is taken from the CfSource.cc file.

      // setup the probability density as a function of energy

      // Random generators according to probability densities.
      static CLHEP::RandGeneral* fGenerate = 0;

      static const float flow = Emin; static const float fhigh = Emax;

      static bool first = true;
      if (first)
	{
	  first = false;

	  // In the original code, the probability densities used the
	  // funlxp and funlux routines in CERNLIB to generate random
	  // numbers.  The following code uses CLHEP to generate the
	  // same "histograms" for the RandGeneral random-number
	  // generator.

	  const int probDensSize = 200;
	  double fspace[probDensSize];

	  // Find function values at bin centers.
	  for ( int i=0; i != probDensSize; i++ )
	    {
	      float value = (float(i) + 0.5) * (fhigh - flow) / (float) probDensSize;
	      fspace[i] = IBDESpectrum(flow + value);

#ifdef DEBUG
	      std::cout << "   i=" << i << " f="
			<< fspace[i] << ","
		        << std::endl;

#endif

#ifdef DEBUG
	  	//Let's write the fspace prob. density function to a text file.
	 	 std::ofstream fout("TheProbFunc.txt");
	  	if(fout.is_open())
	 	 {
			std::cout << "Your file is open.  Let's put the probability density function into it..." << std::endl;
			for(int i = 0; i < probDensSize; i++)
			{
				fout << i << " " << fspace[i] << std::endl;
			}
		 	fout.close();
	 	 }
#endif
    	   }

	  // Define random-number generators.  First argument has your
	  // Array of probablility density values, second input has the
	  // number of array elements.
	  fGenerate = new CLHEP::RandGeneral(fspace,probDensSize);

#ifdef DEBUG
	  std::cout << " Random generator test (f):" << std::endl;
	  for ( int i=0; i != 20; i++ )
	    {
	      std::cout << i << ": "
			<< fGenerate->shoot() * (fhigh - flow) + flow << ", "
		        << std::endl;
	    }

#endif
	} 


	  float nuE = fGenerate->shoot() * (fhigh - flow) + flow;

#ifdef DEBUG
	  std::cout << "Your generated neutrino energy is..." 
		    << nuE
		    << std::endl;
#endif

	  return nuE;
}


double ReacIBDgen::IBDESpectrum(float x){
    //I have replaced the CrossSection function that lives in the original IBDgen
    //With the cross section function given in the original source file from
    //Marc Bergevin.
  
    double mElectron=0.511;  
    double XC=CrossSection(x);
    double EnergyVal = NuReacSpectrum(x)*XC*sqrt(XC*XC - mElectron*mElectron);

#ifdef DEBUG
    std::cout << EnergyVal << " and " << XC << std::endl;
#endif

    return EnergyVal;
  
    //The final units output are in MeV
  }


double ReacIBDgen::CrossSection(float x){
    
    double mNeutron = 939.565378;
    double mProton = 938.27;
    double mElectron = 0.511;
    double delta = mNeutron - mProton;
    double A = 0.5;
    double B = mNeutron*mNeutron;
    double C = 4.0*mProton;
    double D = delta+(delta*delta - mElectron*mElectron)/(2*mProton);
    double E = mNeutron;

    double XC = A*(sqrt(B - C*(D-x)) - E);
  
    return XC;  
  }


float ReacIBDgen::U235ReacSpectrum(const float& x){

    // return the the reactor U235 neutrino flux contribution U235(x)
    float N = 0.;
   
    // Use the current set Amplitude for the U235 contribution 
    double C0 = GetU235Amplitude() ;
    double C1=0.870;
    double C2=0.160;
    double C3=0.091;
    //double C4=201.92;  //Defined in Marc Bergevin's original Reactor neutrino
			 //generator.  Unsure of purpose

    N = C0 * exp(C1 - C2*x - C3*x*x);

    return N;
  }


float ReacIBDgen::Pu239ReacSpectrum(const float& x){

    // return the reactor Pu239 neutrino flux contribution Pu239(x)
    float N = 0.;

    double C0=GetPu239Amplitude();
    double C1=0.896;
    double C2=0.239;
    double C3=0.0981;
    //double C4=209.99;  //Defined in Marc's file; not sure of function
    N = C0 * exp(C1 - C2*x - C3*x*x);

    return N;
  }

float ReacIBDgen::U238ReacSpectrum(const float& x){

    // return the reactor U238 neutrino flux contribution U238(x)
    float N = 0.;

    double C0=GetU238Amplitude();
    double C1=0.976;
    double C2=0.162;
    double C3=0.079;
    //double C4=205.52;  //Defined in Marc's file; not sure of function
    N = C0 * exp(C1 - C2*x - C3*x*x);

    return N;
  }

float ReacIBDgen::Pu241ReacSpectrum(const float& x){

    // return the the reactor Pu241 Neutrino flux contribution Pu241(x)
    float N = 0.;

    double C0=GetPu241Amplitude();
    double C1=0.793;
    double C2=0.080;
    double C3=0.1085;
    //double C4=213.60;  //Defined in Marc's file; not sure of function
    N = C0 * exp(C1 - C2*x - C3*x*x);

    return N;
  }

float ReacIBDgen::NuReacSpectrum(const float& x){

    // return the sum of the neutrino flux contributions from each reactor isotope
    // for a given value x (energy in MeV)

    float tot = U235ReacSpectrum(x) + Pu239ReacSpectrum(x) + U238ReacSpectrum(x) + Pu241ReacSpectrum(x);

    return tot;
  }

} // namespace RAT
