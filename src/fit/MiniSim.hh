#ifndef __RAT__MiniSim__
#define __RAT__MiniSim__

#include <G4UserRunAction.hh>
#include <G4UserEventAction.hh>
#include <G4UserStackingAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4UserSteppingAction.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

namespace RAT {
  
  class MiniSim : public G4UserEventAction, G4UserTrackingAction, G4VUserPrimaryGeneratorAction, G4UserSteppingAction{
  public:
    
    MiniSim();
    virtual ~MiniSim();
    
    virtual void BeamOn(int nevents);
    
    // Override these for your simulation needs
    virtual void GeneratePrimaries(G4Event * argEvent) = 0; // You must override at least this
    virtual void BeginOfEventAction(const G4Event* /*anEvent*/) {};
    virtual void EndOfEventAction(const G4Event* /*anEvent*/) {};
    virtual void PreUserTrackingAction(const G4Track* /*aTrack*/) {};
    virtual void PostUserTrackingAction(const G4Track* /*aTrack*/) {};
    virtual void UserSteppingAction(const G4Step *step); // defaults to running scintillation/reemission
        
  protected:
    virtual void TakeSimControl();
    virtual void ReleaseSimControl();
    
    bool fHaveControl;
    bool fUseGLG4;
    const G4UserRunAction      *fOrigRunAction;
    const G4UserEventAction    *fOrigEventAction;
    const G4UserStackingAction *fOrigStackingAction;
    const G4UserTrackingAction *fOrigTrackingAction;
    const G4UserSteppingAction *fOrigSteppingAction;
    const G4VUserPrimaryGeneratorAction *fOrigPrimaryGeneratorAction;
  };
  
  
} // namespace RAT

#endif
