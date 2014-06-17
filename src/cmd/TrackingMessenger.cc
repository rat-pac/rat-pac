#include <RAT/TrackingMessenger.hh>
#include <RAT/Gsim.hh>
#include <RAT/Trajectory.hh>
#include <RAT/GLG4SteppingAction.hh>
#include <RAT/Log.hh>

namespace RAT {

TrackingMessenger::TrackingMessenger()
{
  FillPointContCmd=new G4UIcmdWithABool("/tracking/FillPointCont", this);
  FillPointContCmd->SetGuidance("Enables/disables G4 track visualization via /vis commands.");
  FillPointContCmd->SetGuidance("This requires the RAT trajectories to duplicate info, consuming more space.");
  FillPointContCmd->SetParameterName("on", true, false);
  FillPointContCmd->SetDefaultValue(false);

  storeParticleTrajCmd = new G4UIcmdWithAString("/tracking/storeParticleTrajectory",this);
  storeParticleTrajCmd->SetGuidance("Store trajectory of a particle with given name");
  storeParticleTrajCmd->SetParameterName("particleName",false); // required string: particle name

  discardParticleTrajCmd = new G4UIcmdWithAString("/tracking/discardParticleTrajectory",this);
  discardParticleTrajCmd->SetGuidance("Discard trajectory of a particle with given name");
  discardParticleTrajCmd->SetParameterName("particleName",false); // required string: particle name

  storeMuonTrajSpecialCmd = new G4UIcmdWithABool("/tracking/storeMuonTrajectorySpecial", this);
  storeMuonTrajSpecialCmd->SetGuidance("Store abbreviated tracking information for muons: store only those steps with process \"Transportation\"");
  storeMuonTrajSpecialCmd->SetParameterName("state", true, false);
  storeMuonTrajSpecialCmd->SetDefaultValue(false);
  
  storeOpticalTrackIDCmd = new G4UIcmdWithABool("/tracking/storeOpticalTrackID", this);
  storeOpticalTrackIDCmd->SetGuidance("Store original parent track ID and creation step for each MC photon.");
  storeOpticalTrackIDCmd->SetParameterName("state", true, false);
  storeOpticalTrackIDCmd->SetDefaultValue(false);

  setMaxGlobalTimeCmd=new G4UIcmdWithADouble("/tracking/setMaxGlobalTime",this);
  setMaxGlobalTimeCmd->SetGuidance("Kill any track whose global time exceeds this (double) value, in ns.  value <= 0.0 means do not kill any tracks.");
  setMaxGlobalTimeCmd->SetParameterName("value",true, false);
  setMaxGlobalTimeCmd->SetDefaultValue(0.0);

}

TrackingMessenger::~TrackingMessenger()
{
  delete FillPointContCmd;
  delete storeParticleTrajCmd;
  delete discardParticleTrajCmd;
  delete storeMuonTrajSpecialCmd;
  delete setMaxGlobalTimeCmd;
  delete storeOpticalTrackIDCmd;
}

G4String TrackingMessenger::GetCurrentValue(G4UIcommand * command)
{
  if(command==FillPointContCmd){
    return Gsim::GetFillPointCont() ? "True" : "False";
  } else if(command==storeOpticalTrackIDCmd) {
    return Gsim::GetStoreOpticalTrackID() ? "True" : "False";
  } else if(command==storeParticleTrajCmd) {
    return Gsim::GetStoreParticleTrajString(true);
  } else if(command==discardParticleTrajCmd) {
    return Gsim::GetStoreParticleTrajString(false);
  } else if(command==storeMuonTrajSpecialCmd) {
    return Trajectory::GetDoAppendMuonStepSpecial();
  } else if(command==setMaxGlobalTimeCmd) {
    return setMaxGlobalTimeCmd->ConvertToString(GLG4SteppingAction::max_global_time);
  } else{
    Log::Die("TrackingMessenger sent unknown get command: "+command->GetCommandPath());    
    return ""; // never get here
  }
}

void TrackingMessenger::SetNewValue(G4UIcommand * command, G4String newValue)
{
  if(command==FillPointContCmd){
    bool on = G4UIcmdWithABool::GetNewBoolValue(newValue);
    if(on)
      detail<<"Tracking: trajectories will fill the container needed by G4 to draw tracks"<<newline;
    else
      detail<<"Tracking: trajectories won't fill the container needed by G4 to draw tracks"<<newline;
    Gsim::SetFillPointCont(on);
  } else if(command==storeOpticalTrackIDCmd){
    bool on = G4UIcmdWithABool::GetNewBoolValue(newValue);
    if(on)
      detail<<"Tracking: Storing the optical photon creation track ID and originating step" << newline;
    else
      detail<<"Tracking: Not storing optical photon creation track ID and originating step" << newline;
    Gsim::SetStoreOpticalTrackID(on);
  } else if(command==storeParticleTrajCmd){
    G4String particleName = newValue;
    detail << "Tracking: storing trajectory information for " << newValue << "\n";
    Gsim::SetStoreParticleTraj(newValue,true);
  } else if(command==discardParticleTrajCmd){
    G4String particleName = newValue;
    detail << "Tracking: discarding trajectory information for " << newValue << "\n";
    Gsim::SetStoreParticleTraj(newValue,false);    
  } else if(command==storeMuonTrajSpecialCmd){
    bool state = G4UIcmdWithABool::GetNewBoolValue(newValue);
    if (state)
      detail << "Tracking: Storing abbreviated tracking information for muons" << newline;
    else
      detail << "Tracking: Not storing abbreviated tracking information for muons" << newline;
    Trajectory::SetDoAppendMuonStepSpecial(state);
  } else if(command==setMaxGlobalTimeCmd){
    G4double val = G4UIcmdWithADouble::GetNewDoubleValue(newValue);
    if (val==0.0)
      detail << "Tracking: No max global time set for tracks." << newline;
    else
      detail << "Tracking: Max global time for tracks set to " << val << newline;
    GLG4SteppingAction::max_global_time=val;
  } else{
    Log::Die("TrackingMessenger sent unknown set command: "+command->GetCommandPath());    
  }
}

} // namespace RAT
