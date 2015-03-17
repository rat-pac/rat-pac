#include <RAT/PhysicsListMessenger.hh>
#include <RAT/Gsim.hh>
#include <RAT/GLG4PhysicsList.hh>
#include <RAT/Log.hh>
#include <GLG4PhysicsList.hh>

namespace RAT {

PhysicsListMessenger::PhysicsListMessenger(GLG4PhysicsList* PLptr)
  :pPhysList(PLptr)
{
  SetAltOpWLSCmd=new G4UIcmdWithABool("/PhysicsList/SetAltOPWLS", this);
  SetAltOpWLSCmd->SetGuidance("Enables/disables the alternative WLS model.");
  SetAltOpWLSCmd->SetGuidance("This requires the user to supply data for the quantum yield and excitation/emission spectra.");
  SetAltOpWLSCmd->SetParameterName("on", true, false);
  SetAltOpWLSCmd->SetDefaultValue(false);
}

PhysicsListMessenger::~PhysicsListMessenger()
{
  delete SetAltOpWLSCmd;
}

G4String PhysicsListMessenger::GetCurrentValue(G4UIcommand * command)
{
  if(command==SetAltOpWLSCmd){
    return pPhysList->GetAltOpWLS() ? "True" : "False";
  } else{
    Log::Die("PhysicsListMessenger sent unknown get command: "+command->GetCommandPath());    
    return ""; // never get here
  }
}

void PhysicsListMessenger::SetNewValue(G4UIcommand * command, G4String newValue)
{
  if(command==SetAltOpWLSCmd){
    bool on = G4UIcmdWithABool::GetNewBoolValue(newValue);
    if(on)
      detail<<"PhysicsList: using alternate WLS model"<<newline;
    else
      detail<<"PhysicsList: using G4 WLS model"<<newline;
    pPhysList->SetAltOpWLS(on);
  } else{
    Log::Die("PhysicsListMessenger sent unknown set command: "+command->GetCommandPath());    
  }
}

} // namespace RAT
