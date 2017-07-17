#include <RAT/ESCrossSecMessenger.hh>
#include <RAT/ESCrossSec.hh>
#include <RAT/Log.hh>
#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4UIcmdWithAnInteger.hh>
#include <G4String.hh>

namespace RAT {

  ESCrossSecMessenger::ESCrossSecMessenger(ESCrossSec* e) :
    fESXS(e)
  {
    // Commands will go in a /generator/es/ directory
    G4UIdirectory* dir = new G4UIdirectory("/generator/es/xsection/");
    dir->SetGuidance("Control the physics parameters of the elastic-scattering generator");

    fWmaCmd = new G4UIcmdWithADouble("/generator/es/xsection/wma", this);
    fWmaCmd->SetGuidance("Sets the value of sine-squared theta (the weak mixing angle)");
    fWmaCmd->SetParameterName("sin2th",false);
    fWmaCmd->SetDefaultValue( fESXS->GetSinThetaW() );


    fStratCmd = new G4UIcmdWithAnInteger("/generator/es/xsection/strategy", this);
    fStratCmd->SetGuidance("Sets the strategy for the ES cross section calculation.");
    fStratCmd->SetGuidance("Usage: /generator/es/xsection/strategy strat");
    fStratCmd->SetGuidance("Options:");
    fStratCmd->SetGuidance("  1 : Original routine from QSNO::PNuE (Bahcall).");
    fStratCmd->SetGuidance("  2 : Improved routine from QSNO::PNuE (without rad. corrections).");
    fStratCmd->SetGuidance("  3 : Improved routine from QSNO::PNuE (with rad. corrections - analytical).");
    fStratCmd->SetGuidance("  4 (default) : Improved routine from QSNO::PNuE (with rad. corrections - table).");
    fStratCmd->SetParameter(new G4UIparameter("strat",'i',false));
    fStratCmd->SetDefaultValue( fESXS->GetRadiativeCorrection() );


    //

  }

  ESCrossSecMessenger::~ESCrossSecMessenger() {;}

  void ESCrossSecMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
  {
    if ( command == fWmaCmd )
      {
        G4double wma = fWmaCmd->GetNewDoubleValue( newValue );
        fESXS->SetSinThetaW( wma );
      }
    else if ( command == fStratCmd )
      {
        G4int strat = fStratCmd->GetNewIntValue( newValue );
        fESXS->SetRadiativeCorrection( strat );
      }
    else
      {
        warn << "Error: Invalid ESCrossSecMessenger \"set\" command" << newline;
      }
  }

  G4String ESCrossSecMessenger::GetCurrentValue(G4UIcommand* command)
  {
    if ( command == fWmaCmd )
      return fWmaCmd->ConvertToString( fESXS->GetSinThetaW() );
    else if ( command == fStratCmd )
      return fStratCmd->ConvertToString( fESXS->GetRadiativeCorrection() );

    // Error if we reach here.
    return G4String("Error: Invalid ESCrossSecMessenger \"get\" command");
  }

} // namespace RAT
