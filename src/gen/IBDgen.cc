#include <CLHEP/Units/PhysicalConstants.h>
#include <RAT/IBDgen.hh>
#include <RAT/DB.hh>

using namespace CLHEP;

namespace RAT {


// Additional constants
const double DELTA = neutron_mass_c2 - proton_mass_c2;
const double GFERMI = 1.16639e-11 / MeV / MeV;
 
IBDgen::IBDgen()
{
  // Get parameters from database
  DBLinkPtr libd = DB::Get()->GetLink("IBD");

  Emin = libd->GetD("emin");
  Emax = libd->GetD("emax");
  // Flux function
  rmpflux.Set(libd->GetDArray("spec_e"), libd->GetDArray("spec_flux"));
  
  // Other useful numbers
  XCmax = CrossSection(Emax,-1);
  FluxMax = rmpflux(Emin);
}

  
void IBDgen::GenEvent(const Hep3Vector &nu_dir,
		      HepLorentzVector &neutrino,
		      HepLorentzVector &positron,
		      HepLorentzVector &neutron)
{
  float Enu, CosThetaLab;

  // Pick energy of neutrino and relative direction of positron
  GenInteraction(Enu, CosThetaLab);
  
  // Zero'th order approximation of positron quantities (infinite nucleon mass)
  double E0 = Enu - DELTA;
  double p0 = sqrt(E0*E0-electron_mass_c2*electron_mass_c2); 
  double v0 = p0/E0;
  // First order correction for finite nucleon mass
  double Ysquared = (DELTA*DELTA-electron_mass_c2*electron_mass_c2)/2;
  double E1 = E0*(1-Enu/proton_mass_c2*(1-v0*CosThetaLab))
               - Ysquared/proton_mass_c2;
  double p1 = sqrt(E1*E1-electron_mass_c2*electron_mass_c2);

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


void IBDgen::GenInteraction(float &E, float &CosThetaLab)
{
  
  bool passed=false;
  
  while(!passed){
    // Pick E and cos(theta) uniformly
    E = Emin+(Emax-Emin)*HepUniformRand();
    CosThetaLab = -1.0+2.0*HepUniformRand();
    
    // Decided whether to draw again based on relative cross-section.
    float XCtest = XCmax * FluxMax * HepUniformRand();
    double XCWeight = CrossSection(E, CosThetaLab);
    double FluxWeight = rmpflux(E);
    passed = XCWeight * FluxWeight > XCtest;
  }
}


double IBDgen::CrossSection(double Enu, double CosThetaLab)
{
  //
  // Cross section constants.  Some for overall scale are just
  // to allow absolute comparison to published article.
  //
  const double CosThetaC = (0.9741+0.9756)/2;
  //
  // Radiative correction constant
  //
  const double RadCor = 0.024;
  //
  // check for threshold
  //
  const double EminBeta = 
    ((proton_mass_c2+DELTA+electron_mass_c2)*(proton_mass_c2+electron_mass_c2+DELTA)
     -proton_mass_c2*proton_mass_c2)/2/proton_mass_c2;

  if(Enu<EminBeta) return 0;

  //
  // overall scale
  //
  const double Sigma0 = GFERMI*GFERMI*CosThetaC*CosThetaC/pi*(1+RadCor);
  //
  // couplings
  //
  const double f = 1.00;
  const double f2 = 3.706;
  const double g = 1.26;

  //
  // order 0 terms
  //
  double E0 = Enu - DELTA;
  if(E0<electron_mass_c2) E0=electron_mass_c2;
  double p0 = sqrt(E0*E0-electron_mass_c2*electron_mass_c2);
  double v0 = p0/E0;

  //
  //  order 1 terms
  //
  const double Ysquared = (DELTA*DELTA-electron_mass_c2*electron_mass_c2)/2;
  double E1 = E0*(1-Enu/proton_mass_c2*(1-v0*CosThetaLab))-Ysquared/proton_mass_c2;
  if(E1<electron_mass_c2) E1=electron_mass_c2;
  double p1 = sqrt(E1*E1-electron_mass_c2*electron_mass_c2);
  double v1 = p1/E1;

  double Gamma = 
    2*(f+f2)*g*((2*E0+DELTA)*(1-v0*CosThetaLab)-
		electron_mass_c2*electron_mass_c2/E0)
    +(f*f+g*g)*(DELTA*(1+v0*CosThetaLab)+electron_mass_c2*electron_mass_c2/E0)
    +(f*f+3*g*g)*((E0+DELTA)*(1-CosThetaLab/v0)-DELTA)
    +(f*f-g*g)*((E0+DELTA)*(1-CosThetaLab/v0)-DELTA)*v0*CosThetaLab;

  double XC = 
    ((f*f+3*g*g)+(f*f-g*g)*v1*CosThetaLab)*E1*p1
    -Gamma/proton_mass_c2*E0*p0;
  XC *= Sigma0/2;
  XC *= hbarc * hbarc; // Convert from MeV^-2 to mm^2 (native units for GEANT4)

  return XC;  
}


} // namespace RAT
