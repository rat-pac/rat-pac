#ifndef G4S1Light_h
#define G4S1Light_h 1

#include "globals.hh"
#include "templates.hh"
#include "Randomize.hh"
#include "G4Poisson.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleMomentum.hh"
#include "G4Step.hh"
#include "G4VRestDiscreteProcess.hh"
#include "G4OpticalPhoton.hh"
#include "G4DynamicParticle.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4ThermalElectron.hh"

namespace RAT {

//#include "LUXSimManager.hh"

#define AVO 6.022e23 //Avogadro's number (#/mol)
#define EMASS 9.109e-31*kg
#define MillerDriftSpeed true

#define GASGAP 0.25*cm //S2 generation region
#define BORDER 0*cm //liquid-gas border z-coordinate

#define QE_EFF 1 //a base or maximum quantum efficiency
#define phe_per_e 1 //S2 gain for quick studies

// different field regions, for gamma-X studies
#define WIN 0*mm //top Cu block (also, quartz window)
#define TOP 0 //top grid wires
#define ANE 0 //anode mesh
#define SRF 0 //liquid-gas interface
#define GAT 0 //gate grid
#define CTH 0 //cathode grid
#define BOT 0 //bottom PMT grid
#define PMT 0 //bottom Cu block and PMTs

// this entire file is adapted from G4Scintillation.hh from Geant4.9.4
class G4S1Light : public G4VRestDiscreteProcess //class definition
{
  // Class inherits publicly from G4VRestDiscreteProcess
private:
public: // constructor and destructor

        G4S1Light(const G4String& processName = "S1",
		  G4ProcessType type = fElectromagnetic);
	~G4S1Light();

public: // methods, with descriptions
        G4bool IsApplicable(const G4ParticleDefinition& aParticleType);
        // Returns true -> 'is applicable', for any particle type except for an
        // 'opticalphoton' and for short-lived particles

	G4double GetMeanFreePath(const G4Track& aTrack,
				       G4double ,
                                       G4ForceCondition* );
        // Returns infinity; i. e. the process does not limit the step, but 
        // sets the 'StronglyForced' condition for the DoIt to be invoked at
        // every step.

        G4double GetMeanLifeTime(const G4Track& aTrack,
                                 G4ForceCondition* );
        // Returns infinity; i. e. the process does not limit the time, but
        // sets the 'StronglyForced' condition for the DoIt to be invoked at
        // every step.
  
        // For in-flight particles losing energy (or those stopped)
	G4VParticleChange* PostStepDoIt(const G4Track& aTrack,
					const G4Step& aStep);
        G4VParticleChange* AtRestDoIt ( const G4Track& aTrack,
					const G4Step& aStep);
  
        // These are the methods implementing the scintillation process.
  
	void SetTrackSecondariesFirst(const G4bool state);
        // If set, the primary particle tracking is interrupted and any
        // produced scintillation quanta are tracked next. When all have been
        // tracked, the tracking of the primary resumes.

        G4bool GetTrackSecondariesFirst() const;
        // Returns the boolean flag for tracking secondaries first.
  
        void SetScintillationYieldFactor(const G4double yieldfactor);
        // Called to set the scintillation quantum yield factor, useful for
        // shutting off scintillation entirely, or for producing a universal 
        // re-scaling to for example represent detector effects. Internally is
        // used for Lindhard yield factor for NR. Default should be user-set
        // to be 1 (for ER) in your simulation -- see NEST readme
  
        G4double GetScintillationYieldFactor() const;
        // Returns the quantum (photon/electron) yield factor. See above.

        void SetScintillationExcitationRatio(const G4double excitationratio);
        // Called to set the scintillation exciton-to-ion ratio, useful for
        // when NumExcitons/NumIons is different for different types of parent
        // particles. This overwrites the ExcitationRatio obtained from the
        // G4MaterialPropertiesTable (e.g., 0.06 for LXe).

        G4double GetScintillationExcitationRatio() const;
        // Returns the ratio of the number of excitons to ions. Read above.

protected:
        G4bool fTrackSecondariesFirst; // see above
        //bools for tracking some special particle cases
        G4bool fExcitedNucleus, fAlpha, fVeryHighEnergy, fMultipleScattering;
        G4double fKr83m;
        G4double YieldFactor; // turns scint. on/off
        G4double ExcitationRatio; // N_ex/N_i, the dimensionless ratio of
        //initial excitons to ions
private:
        //LUXSimManager *luxManager;
};

////////////////////
// Inline methods
////////////////////
inline 
G4bool G4S1Light::IsApplicable(const G4ParticleDefinition& aParticleType)
{
       if (aParticleType.GetParticleName() == "opticalphoton") return false;
       if (aParticleType.IsShortLived()) return false;
       if (aParticleType.GetParticleName() == "thermalelectron") return false;
       //if(abs(aParticleType.GetPDGEncoding())==2112 || //neutron (no E-dep.)
       if(abs(aParticleType.GetPDGEncoding())==12 || //neutrinos (ditto) 
	  abs(aParticleType.GetPDGEncoding())==14 ||
	  abs(aParticleType.GetPDGEncoding())==16) return false;
       
       return true;
}

inline 
void G4S1Light::SetTrackSecondariesFirst(const G4bool state) 
{ 
	fTrackSecondariesFirst = state;
}

inline
G4bool G4S1Light::GetTrackSecondariesFirst() const
{
        return fTrackSecondariesFirst;
}

inline
void G4S1Light::SetScintillationYieldFactor(const G4double yieldfactor)
{
  YieldFactor = yieldfactor;
}

inline
G4double G4S1Light::GetScintillationYieldFactor() const
{
        return YieldFactor;
}

inline
void G4S1Light::SetScintillationExcitationRatio(const G4double excitationratio)
{
        ExcitationRatio = excitationratio;
}

inline
G4double G4S1Light::GetScintillationExcitationRatio() const
{
        return ExcitationRatio;
}


} // namespace RAT

#endif /* G4S1Light_h */
