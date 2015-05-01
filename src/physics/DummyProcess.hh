/**
 * @class DummyProcess
 *
 * @detail Dummy classes used as placeholders in new opticalphoton tracks so
 * that G4Track users can figure out the name of the process which
 * created the track.
 */

#ifndef __DummyProcess__
#define __DummyProcess__

#include <G4VProcess.hh>

class DummyProcess : public G4VProcess {
public:
  DummyProcess(const G4String& aName="NoName", G4ProcessType aType=fNotDefined) 
    : G4VProcess(aName, aType) { };

  virtual G4double AlongStepGetPhysicalInteractionLength(
      const G4Track&, G4double, G4double, G4double&, G4GPILSelection*) {
    return 0;
  }
 
  virtual G4double AtRestGetPhysicalInteractionLength(
      const G4Track&, G4ForceCondition*) {
    return 0;
  }
 
  virtual G4double PostStepGetPhysicalInteractionLength(
      const G4Track&, G4double, G4ForceCondition*) {
    return 0;
  }

  virtual G4VParticleChange* PostStepDoIt(const G4Track&, const G4Step&) {
    return 0;
  }
 
  virtual G4VParticleChange* AlongStepDoIt(const G4Track&, const G4Step&) {
    return 0;
  }

  virtual G4VParticleChange* AtRestDoIt(const G4Track&, const G4Step&) {
    return 0;
  }
};

#endif  // __DummyProcess__

