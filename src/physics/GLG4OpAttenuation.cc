// This file is part of the GenericLAND software library.
// $Id: GLG4OpAttenuation.cc,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
//  "Attenuation" (absorption or scattering) of optical photons
//
//   GenericLAND Simulation
//
//   Original: Glenn Horton-Smith, Dec 2001
//
// GLG4OpAttenuation.cc
// 

#include "G4ios.hh"
#include "GLG4OpAttenuation.hh"

#include "G4DynamicParticle.hh"
#include "G4Material.hh"
#include "G4OpticalPhoton.hh"
#include "G4GeometryTolerance.hh"
#include "globals.hh"
#include "templates.hh"
#include "Randomize.hh"
#include "G4Step.hh"

using namespace std;

        /////////////////
        // Hidden static variables and functions
        /////////////////

// values in Cos2ThetaTable are used in equation to invert the equation
// for generating an angular distribution for scalar scattering:
//  dP/d(cos\theta) = (3/4) sin^2\theta
//   P_{cumulative} = (2+3*cos\theta-cos^3\theta)/4
//      cos(\theta) = 4*(P_{cumulative}-1/2) / (3-cos^2\theta)
//                                                ^^^^^^^^^^^ table value used
#define N_COSTHETA_ENTRIES 129
static double Cos2ThetaTable[N_COSTHETA_ENTRIES]; 
static int TableInitialized= 0;

DummyProcess GLG4OpAttenuation::fgAttenuation("Attenuation");
DummyProcess GLG4OpAttenuation::fgScattering("Scattering");


static void InitializeTable(void)
{
  double angTolerance = G4GeometryTolerance::GetInstance()->GetAngularTolerance();

  double cos2th=0.0;

  for (int i=0; i<N_COSTHETA_ENTRIES-1; i++) {
    double x= i/(double)(N_COSTHETA_ENTRIES-1);
    double old_cos2th;
    do { // find exact root by iterating to convergence
      old_cos2th= cos2th;
      double costh= 2.0*x/(3.0-cos2th);
      cos2th= costh*costh;
    } while (fabs(old_cos2th-cos2th) > angTolerance);
    Cos2ThetaTable[i]= cos2th;
  }
  Cos2ThetaTable[N_COSTHETA_ENTRIES-1]= 1.0;
  TableInitialized=1;
}

        /////////////////
        // Constructors and Destructor
        /////////////////

GLG4OpAttenuation::GLG4OpAttenuation(const G4String& processName)
              : G4OpAbsorption(processName)
{
  if (!TableInitialized)
    InitializeTable();
}

GLG4OpAttenuation::~GLG4OpAttenuation(){}

        ////////////
        // Methods
        ////////////

// PostStepDoIt
// -------------
//
G4VParticleChange*
GLG4OpAttenuation::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep)
{
        aParticleChange.Initialize(aTrack);

	const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
        const G4Material* aMaterial = aTrack.GetMaterial();
	G4StepPoint* postStepPoint = aStep.GetPostStepPoint();

	G4double thePhotonMomentum = aParticle->GetTotalMomentum();

	G4MaterialPropertiesTable* aMaterialPropertyTable;
	G4MaterialPropertyVector* OpScatFracVector;
	
        G4double OpScatFrac = 0.0;

	aMaterialPropertyTable = aMaterial->GetMaterialPropertiesTable();

	if ( aMaterialPropertyTable ) {
	   OpScatFracVector = aMaterialPropertyTable->
	           GetProperty("OPSCATFRAC");
           if ( OpScatFracVector ) {
             OpScatFrac = OpScatFracVector->
	             Value(thePhotonMomentum);
           }
	}

	if ( OpScatFrac > 0.0 && G4UniformRand() < OpScatFrac ) {
	  // photon scattered coherently -- use scalar scattering (Rayleigh):
	  // for fully polarized light, angular distribution \prop sin^2 \theta
	  // where theta is angle between initial polarization and final
	  // momentum vectors.  All light in Geant4 is fully polarized...
	  G4double urand= G4UniformRand()-0.5;
	  G4double Cos2Theta0=
	    Cos2ThetaTable[(int)(fabs(urand)*2.0*(N_COSTHETA_ENTRIES-1)+0.5)];
	  G4double CosTheta= 4.0*urand/(3.0-Cos2Theta0);
#ifdef G4DEBUG	  
	  if (fabs(CosTheta)>1.0) {
	    cerr << "GLG4OpAttenution: Warning, CosTheta=" << CosTheta
		 << " urand=" << urand
		 << endl;
	    CosTheta= CosTheta>0.0 ? 1.0 : -1.0;
	  }
#endif
	  G4double SinTheta= sqrt(1.0-CosTheta*CosTheta);
	  G4double Phi= (2.0*G4UniformRand()-1.0)*M_PI;
	  G4ThreeVector e2( aParticle->GetMomentumDirection()
			    .cross( aParticle->GetPolarization() ) );
	  
	  G4ThreeVector NewMomentum=
	    ( CosTheta * aParticle->GetPolarization() +
	      (SinTheta*cos(Phi)) * aParticle->GetMomentumDirection() +
	      (SinTheta*sin(Phi)) * e2 ).unit();

	  // polarization is normal to new momentum and in same plane as
	  // old new momentum and old polarization
	  G4ThreeVector NewPolarization=
	    ( aParticle->GetPolarization() - CosTheta*NewMomentum).unit();
	  
	  aParticleChange.ProposeMomentumDirection(NewMomentum);
	  aParticleChange.ProposePolarization(NewPolarization);

	  postStepPoint->SetProcessDefinedStep(&fgScattering);
	}
	else {
	  // photon absorbed (may be re-radiated... but that is GLG4Scint's job)
	  aParticleChange.ProposeTrackStatus(fStopAndKill);
	  postStepPoint->SetProcessDefinedStep(&fgAttenuation);
	  
	  if (verboseLevel>0) {
	    G4cout << "\n** Photon absorbed! **" << G4endl;
	  }
	}
	
	return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
}


