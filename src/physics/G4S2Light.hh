#ifndef G4S2Light_h
#define G4S2Light_h 1

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

// this entire file is adapted from G4Scintillation.hh from Geant4.9.4
class G4S2Light : public G4VRestDiscreteProcess //class definition
{
  // Class inherits publicly from G4VRestDiscreteProcess
private:
 
public: // constructor and destructor

        G4S2Light(const G4String& processName = "S2",
	                   G4ProcessType type = fElectromagnetic);
	~G4S2Light();

public: // methods, with descriptions

        G4bool IsApplicable(const G4ParticleDefinition& aParticleType);
        // Returns true -> 'is applicable', only for thermalelectrons
  
	G4double GetMeanFreePath(const G4Track& aTrack,
				       G4double ,
                                       G4ForceCondition* );

        G4double GetMeanLifeTime(const G4Track& aTrack,
                                 G4ForceCondition* );
  
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
        // re-scaling to for example represent detector effects.
  
        G4double GetScintillationYieldFactor() const;
        // Returns the quantum (thermalelectron) yield factor. (See above.)
  
        void SetScintillationExcitationRatio(const G4double excitationratio);
        // Inherited from the S1 code. Unused...
  
        G4double GetScintillationExcitationRatio() const;

protected:

        G4bool fTrackSecondariesFirst; // see above
        G4double YieldFactor; // turns scint. on/off
        G4double ExcitationRatio; // N_ex/N_i

private:
        //LUXSimManager *luxManager;

};

////////////////////
// Inline methods
////////////////////

inline 
G4bool G4S2Light::IsApplicable(const G4ParticleDefinition& aParticleType)
{
       if (aParticleType.GetParticleName() != "thermalelectron") return false;
       
       return true;
}

inline 
void G4S2Light::SetTrackSecondariesFirst(const G4bool state) 
{ 
	fTrackSecondariesFirst = state;
}

inline
G4bool G4S2Light::GetTrackSecondariesFirst() const
{
        return fTrackSecondariesFirst;
}

inline
void G4S2Light::SetScintillationYieldFactor(const G4double yieldfactor)
{
  YieldFactor = yieldfactor;
}

inline
G4double G4S2Light::GetScintillationYieldFactor() const
{
        return YieldFactor;
}

inline
void G4S2Light::SetScintillationExcitationRatio(const G4double excitationratio)
{
        ExcitationRatio = excitationratio;
}

inline
G4double G4S2Light::GetScintillationExcitationRatio() const
{
        return ExcitationRatio;
}


} // namespace RAT

#endif /* G4S2Light_h */
