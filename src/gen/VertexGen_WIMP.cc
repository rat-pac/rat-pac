#include <RAT/VertexGen_WIMP.hh>
#include <RAT/Log.hh>
#include <RAT/DB.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Event.hh>
#include <G4IonTable.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4ThreeVector.hh>
#include "TMath.h"

using namespace TMath;

namespace RAT {
  VertexGen_WIMP::VertexGen_WIMP(const char *arg_dbname)
    : GLG4VertexGen(arg_dbname)
  {

  }
  
  VertexGen_WIMP::~VertexGen_WIMP()
  {
    
  }
  
  void VertexGen_WIMP::
  GeneratePrimaryVertex(G4Event *event,
                        G4ThreeVector &dx,
                        G4double dt)
  {
    // Draw energy from WIMP distribution
    double ke = fEnergyDistLo + 
      (fEnergyDistHi - fEnergyDistLo) * fEnergyDist->shoot();
    double energy = sqrt( ke * ( ke + 2 * fNucleus->GetPDGMass()) );

    // Pick direction isotropically
    G4ThreeVector mom;
    double theta = acos(2.0 * G4UniformRand() - 1.0);
    double phi = 2.0 * G4UniformRand() * CLHEP::pi;
    mom.setRThetaPhi(energy, theta, phi); // Momentum == energy units in GEANT4
    G4ThreeVector dir = mom.unit();
    
    G4PrimaryVertex* vertex= new G4PrimaryVertex(dx, dt);
    G4PrimaryParticle* nucleus = new G4PrimaryParticle(fNucleus, 
						       mom.x(),mom.y(),mom.z());
    // Generate random polarization
    phi = (G4UniformRand()*2.0-1.0)*M_PI;
    G4ThreeVector e1 = dir.orthogonal().unit();
    G4ThreeVector e2 = dir.cross(e1);
    G4ThreeVector rpol = e1*cos(phi)+e2*sin(phi);
    nucleus->SetPolarization(rpol.x(), rpol.y(), rpol.z());

    vertex->SetPrimary(nucleus);
    event->AddPrimaryVertex(vertex);
  }
  
  void VertexGen_WIMP::SetState(G4String newValues)
  {
    if (newValues.length() == 0) {
      // print help and current state
      G4cout << "Current state of this VertexGen_WIMP:\n"
             << " \"" << GetState() << "\"\n" << G4endl;
      G4cout << "Format of argument to VertexGen_WIMP::SetState: \n"
        " \"nucleus_name WIMP_mass_in_GeV\"\n" << G4endl;
      return;
    }

    std::istringstream is(newValues.c_str());
    is >> fNucleusName >> fWIMPMass;
    if (is.fail())
      Log::Die("VertexGen_WIMP: Incorrect vertex setting " + newValues);

    if (fNucleusName == "Ar40")
      fNucleus = G4IonTable::GetIonTable()->GetIon(18, 40, 0.0);
    else if (fNucleusName == "Ne20")
      fNucleus = G4IonTable::GetIonTable()->GetIon(10, 20, 0.0);
    else
      Log::Die("VertexGen_WIMP: Unknown nucleus " + fNucleusName);

    fWIMPMass = fWIMPMass * CLHEP::GeV; // Convert to GEANT4 units

    Setup(); // Configures energy distribution
  }
  
  G4String VertexGen_WIMP::GetState()
  {
    return dformat("%s\t%f", fNucleusName.c_str(), fWIMPMass);
  }

//Helm form factor
double VertexGen_WIMP::Helmff(double E,double mA)
{
  if(E*mA==0)
    return 1;
//hbar c in MeV*fm
  const double hbarcnuc=TMath::Hbar()*TMath::C()/TMath::Qe()*1e9;
  double A=(double)fNucleus->GetBaryonNumber();

  double ra=Sqrt((1.23*pow(A,1./3)-.6)*(1.23*pow(A,1./3)-.6)+7./3*Pi()*Pi()*.52*.52-5);
  double qs=Sqrt(2*E*mA)/hbarcnuc;
  double qr=Sqrt(2*E*mA)*ra/hbarcnuc;

  return 9*((Sin(qr)-qr*Cos(qr))/pow(qr,3))*((Sin(qr)-qr*Cos(qr))/pow(qr,3))*Exp(-qs*qs);
}

//Eq. (19) of C. Savage, G. Gelmini, P. Gondolo, K. Freese, JCAP 0904:010, 2009
double VertexGen_WIMP::VelIntegral(double vmin,double v0,double vE, double vesc)
{
  if(v0==0){
    G4cout<<"VertexGen_WIMP error: zero most likely WIMP velocity relative to the Milky Way. Returning -1\n";
    return -1;
  }
  double xmin=vmin/v0,xE=vE/v0,xesc=vesc/v0;
  if(xesc<xE)
    G4cout<<"VertexGen_WIMP warning (we are not escaping the galaxy): our velocity is "<<vE
      <<", the local Milky Way escape velocity is "<<vesc<<". The results will be unphysical.\n";
  //no extragalactic WIMPs
  if(xesc+xE<=xmin)
    return 0;
  //normal case: we're looking where we expect a good signal
  if(xmin<=Abs(xesc-xE)){
//the ref. above lists this math. possible case, which is unphysical (we'd be flying out of the galaxy)
    if(xesc<=xE)
      return 1/vE;
    if(xesc>xE)
      return (0.25*sqrt(CLHEP::pi)/xE*(erf(xmin+xE)-erf(xmin-xE))-exp(-xesc*xesc))/
        (0.5*sqrt(CLHEP::pi)*erf(xesc)-xesc*exp(-xesc*xesc))/v0;
  }
  //this in case the tail is the best we can get
  return (0.25*sqrt(CLHEP::pi)*(erf(xesc)-erf(xmin-xE))-.5*(xesc+xE-xmin)*exp(-xesc*xesc))/xE/
    (0.5*sqrt(CLHEP::pi)*erf(xesc)-xesc*exp(-xesc*xesc))/v0;
}

  void VertexGen_WIMP::Setup()
  {
    DBLinkPtr lwimp = DB::Get()->GetLink("WIMP");

    const int nsamples = 10000;
    double dRdQ[nsamples];

    fEnergyDistLo = lwimp->GetD("energy_lo") * CLHEP::keV;
//    fEnergyDistHi = lwimp->GetD("energy_hi") * CLHEP::keV;

    // Useful constants
    const double v_esc = lwimp->GetD("v_esc") * CLHEP::km / CLHEP::s; // Local galactic escape velocity
    const double v_0 = lwimp->GetD("v_0") * CLHEP::km / CLHEP::s; // Mean galactic WIMP velocity
    const double v_SunGal = lwimp->GetD("v_SunGal") * CLHEP::km / CLHEP::s; // Sun's velocity around galactic center
    const double v_EarthSun = lwimp->GetD("v_EarthSun") * CLHEP::km / CLHEP::s; // Earth's velocity around sun in direction of Sun's velocity around galactic center

    //const double rho_wimp = lwimp->GetD("rho") * GeV / cm3; // Local dark matter halo density

    const double date = lwimp->GetD("date"); // time since January 1 in days
    // Net velocity of Earth through halo, from R.W.Schnee, http://arxiv.org/abs/1101.5205
    const double v_E = v_SunGal + v_EarthSun * cos(2 * CLHEP::pi/365.25 * (date-152.5));
    
    // Target properties
    const double m_nucleus = fNucleus->GetPDGMass();
    //const int A_nucleus = fNucleus->GetAtomicMass();
    
    const double r = 4 * (fWIMPMass * m_nucleus) / pow( fWIMPMass + m_nucleus, 2.0 );
    //ignore annual and diurnal modulations for now
    fEnergyDistHi=.5*fWIMPMass*r*pow((v_esc+v_E)/CLHEP::c_light,2);
    double step = (fEnergyDistHi - fEnergyDistLo) / nsamples;
    const double E_0 = 0.5 * fWIMPMass * pow(v_0 / CLHEP::c_light, 2.0);
    const double R_0 = 1.0; // Wrong, but we don't care about absolute rate here

    double normal=0;
    for (int i=0; i < nsamples; i++) {
      double ke = fEnergyDistLo + i * step;

      double v_min = v_0 * sqrt(ke / (r * E_0));

      double rate = Helmff(ke,m_nucleus)*R_0/(r* E_0)*VelIntegral(v_min,v_0,v_E,v_esc);
							      

      // Relative rate (constant coefficients may be dropped)
      dRdQ[i] = rate;
      //warn << ke / CLHEP::keV << " " << rate << newline;
      //distribution integral
      normal+=dRdQ[i]*step;
    }

  if(normal>0)
    for(int i=0;i<nsamples;i++)
      dRdQ[i] /=normal;

    fEnergyDist = new CLHEP::RandGeneral(dRdQ, nsamples);    
  }
  
} // namespace RAT
