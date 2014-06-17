#ifndef __RAT_GLG4DummyProcess__
#define __RAT_GLG4DummyProcess__

#include <G4VProcess.hh>

// Dummy classes used as placeholders in new opticalphoton tracks so
// that G4Track users can figure out the name of the process which
// created the track.
class GLG4DummyProcess : public G4VProcess {
public:
  GLG4DummyProcess(const G4String &aName = "NoName",
		   G4ProcessType aType = fNotDefined) 
    : G4VProcess(aName, aType) { };

  // Bogus, not a real process
  virtual G4double AlongStepGetPhysicalInteractionLength(
							 const G4Track& /*track*/,
							 G4double  /*previousStepSize*/,
							 G4double  /*currentMinimumStep*/,
							 G4double& /*proposedSafety*/,
							 G4GPILSelection* /*selection*/) { return 0; };
 
  virtual G4double AtRestGetPhysicalInteractionLength(
						      const G4Track& /*track*/,
						      G4ForceCondition* /*condition*/
						      ) { return 0; };
 
  virtual G4double PostStepGetPhysicalInteractionLength(
							const G4Track& /*track*/,
							G4double   /*previousStepSize*/,
							G4ForceCondition* /*condition*/
							) { return 0; };

  virtual G4VParticleChange* PostStepDoIt(
					  const G4Track& /*track*/,
					  const G4Step&  /*stepData*/
					  ) { return 0; };
 
  virtual G4VParticleChange* AlongStepDoIt(
					   const G4Track& /*track*/,
					   const G4Step& /*stepData*/
					   ) { return 0; };
  virtual G4VParticleChange* AtRestDoIt(
					const G4Track& /*track*/,
					const G4Step& /*stepData*/
					) { return 0; };
};

#endif
