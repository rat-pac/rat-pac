#include "MiniSim.hh"
#include <G4RunManager.hh>
#include <RAT/GLG4Scint.hh>
#include <RAT/DB.hh>
#include <G4VParticleChange.hh>

namespace RAT {
  
  MiniSim::MiniSim() : fHaveControl(false), fUseGLG4(false)
  {
    // check for the standard glg4 scintillation
    G4String scintPhysList;
    try{
      scintPhysList = RAT::DB::Get()->GetLink("MC")->GetS("scintillation");
    }
    catch(DBNotFoundError& e){ scintPhysList = "glg4"; }
    if(scintPhysList == "glg4")
      fUseGLG4 = true;
  }
  
  MiniSim::~MiniSim() 
  {
    if (fHaveControl)
      ReleaseSimControl();
  }
  
  void MiniSim::TakeSimControl()
  {
    if (fHaveControl)
      return;
    
    G4RunManager *runManager = G4RunManager::GetRunManager();
    
    // Store old configuration for later restore
    fOrigRunAction      = runManager->GetUserRunAction();
    fOrigEventAction    = runManager->GetUserEventAction();
    fOrigStackingAction = runManager->GetUserStackingAction();
    fOrigTrackingAction = runManager->GetUserTrackingAction();
    fOrigSteppingAction = runManager->GetUserSteppingAction();
    fOrigPrimaryGeneratorAction = runManager->GetUserPrimaryGeneratorAction();
    
    // Point run manager at us (or nothing)
    runManager->SetUserAction(static_cast<G4UserRunAction*>(0));
    runManager->SetUserAction(static_cast<G4UserEventAction*>(this));
    runManager->SetUserAction(static_cast<G4UserStackingAction*>(0));
    runManager->SetUserAction(static_cast<G4UserTrackingAction*>(this));
    runManager->SetUserAction(static_cast<G4UserSteppingAction*>(this));
    runManager->SetUserAction(static_cast<G4VUserPrimaryGeneratorAction*>(this));
    fHaveControl = true;
  }
  
  void MiniSim::ReleaseSimControl()
  {
    if (!fHaveControl)
      return;

    G4RunManager *runManager = G4RunManager::GetRunManager();

    // Restore original actions
    runManager->SetUserAction(const_cast<G4UserRunAction*>(fOrigRunAction));
    runManager->SetUserAction(const_cast<G4UserEventAction*>(fOrigEventAction));
    runManager->SetUserAction(const_cast<G4UserStackingAction*>(fOrigStackingAction));
    runManager->SetUserAction(const_cast<G4UserTrackingAction*>(fOrigTrackingAction));
    runManager->SetUserAction(const_cast<G4UserSteppingAction*>(fOrigSteppingAction));
    runManager->SetUserAction(const_cast<G4VUserPrimaryGeneratorAction*>(fOrigPrimaryGeneratorAction));
    fHaveControl = false;
  }
  
  void MiniSim::BeamOn(int nevents) {
    G4RunManager *runManager = G4RunManager::GetRunManager();
    TakeSimControl();
    runManager->BeamOn(nevents);
    ReleaseSimControl();
  }
  
  void MiniSim::UserSteppingAction(const G4Step* aStep)
  {
    G4Track* track= aStep->GetTrack();
    static G4int num_zero_steps_in_a_row=0;

    // check for too many zero steps in a row
    if (aStep->GetStepLength() <= 0.0 && track->GetCurrentStepNumber() > 1) {
      ++num_zero_steps_in_a_row;
      if (num_zero_steps_in_a_row >= 4) {
	const G4VPhysicalVolume* pv= track->GetVolume();
	const G4VProcess* lastproc= track->GetStep()->GetPostStepPoint()->GetProcessDefinedStep();
	G4cerr << "MiniSim: Too many zero steps for this track, terminating!"
	       << " type=" << track->GetDefinition()->GetParticleName()
	       << "\n volume=" << (pv!=0 ? pv->GetName() : G4String("NULL"))
	       << " last_process="
	       << (lastproc!=0 ? lastproc->GetProcessName() : G4String("NULL"))
	       << "\n position=" << track->GetPosition()
	       << " momentum=" << track->GetMomentum()
	       << G4endl;
	track->SetTrackStatus( fStopAndKill );
	num_zero_steps_in_a_row= 0;
      }
    }
    else
      num_zero_steps_in_a_row= 0;
    
    // check for very high number of steps
    if (track->GetCurrentStepNumber() > 1000000) {
      const G4VPhysicalVolume* pv= track->GetVolume();
      const G4VProcess* lastproc= track->GetStep()->GetPostStepPoint()
	->GetProcessDefinedStep();
      G4cerr << "MiniSim: Too many steps for this track, terminating!\n"
	     << " step_no=" << track->GetCurrentStepNumber()
	     << " type=" << track->GetDefinition()->GetParticleName()
	     << "\n volume=" << (pv!=0 ? pv->GetName() : G4String("NULL"))
	     << " last_process="
	     << (lastproc!=0 ? lastproc->GetProcessName() : G4String("NULL"))
	     << "\n position=" << track->GetPosition()
	     << " momentum=" << track->GetMomentum()
	     << G4endl;
      track->SetTrackStatus( fStopAndKill );    
    }
    
    if(fUseGLG4){
      // invoke scintillation process
      G4VParticleChange * pParticleChange = GLG4Scint::GenericPostPostStepDoIt(aStep);
      // were any secondaries defined?
      G4int iSecondary= pParticleChange->GetNumberOfSecondaries();
      if (iSecondary > 0) {
	// add secondaries to the list
	while ( (iSecondary--) > 0 ) {
          G4Track *tempSecondaryTrack = pParticleChange->GetSecondary(iSecondary);
          fpSteppingManager->GetfSecondary()->push_back( tempSecondaryTrack );
	}
      }
      pParticleChange->Clear();
    }
  }  
    
} // namespace RAT
