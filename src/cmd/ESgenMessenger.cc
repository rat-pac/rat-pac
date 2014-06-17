// RAT::ESgenMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the ESgen parameters via the command line.

#include <RAT/ESgenMessenger.hh>
#include <RAT/ESgen.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

namespace RAT {

  ESgenMessenger::ESgenMessenger(ESgen* e) :
    esgen(e)
  {  
    // Commands will in a /generator/es/ directory
    G4UIdirectory* dir = new G4UIdirectory("/generator/es/");
    dir->SetGuidance("Control the physics parameters of the elastic-scattering generator");

    wmaCmd = new G4UIcmdWithADouble("/generator/es/wma", this);
    wmaCmd->SetGuidance("Sets the value of sine-squared theta (the weak mixing angle)");
    wmaCmd->SetParameterName("sin2th",false);
    wmaCmd->SetDefaultValue( esgen->GetMixingAngle() );

    vmuCmd = new G4UIcmdWithADouble("/generator/es/vmu", this);
    vmuCmd->SetGuidance("Sets the value of the neutrino magnetic moment (units of Bohr magnetons)");
    vmuCmd->SetParameterName("vmu",false);
    vmuCmd->SetDefaultValue( esgen->GetMagneticMoment() );
  }

  ESgenMessenger::~ESgenMessenger() {;}
  
  void ESgenMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
  {
    if ( command == wmaCmd )
      {
	G4double wma = wmaCmd->GetNewDoubleValue( newValue );
	esgen->SetMixingAngle( wma );
      }
    else if ( command == vmuCmd )
      {
	G4double vmu = vmuCmd->GetNewDoubleValue( newValue );
	esgen->SetNeutrinoMoment( vmu );
      }
    else
      {
	G4cerr << "Error: Invalid ESgenMessenger \"set\" command" << G4endl;
      }
  }

  G4String ESgenMessenger::GetCurrentValue(G4UIcommand* command)
  { 
    if ( command == wmaCmd )
      return wmaCmd->ConvertToString( esgen->GetMixingAngle() );
    else if ( command == vmuCmd )
      return vmuCmd->ConvertToString( esgen->GetMagneticMoment() );

    // Error if we reach here.
    return G4String("Error: Invalid ESgenMessenger \"get\" command");
  }

} // namespace RAT
