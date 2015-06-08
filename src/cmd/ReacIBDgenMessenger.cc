// Rat::ReacIBDgenMessenger
// 07-June-2015 Teal Pershing

// Provide user commands that allow the user to change
// the Reactor Isotope contents via the command line.

#include <RAT/ReacIBDgenMessenger.hh>
#include <RAT/ReacIBDgen.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

namespace RAT {
  ReacIBDgenMessenger::ReacIBDgenMessenger(ReacIBDgen* re) :
    reacibdgen(re)
  {
    // Commands will be called in the mac with /generator/reacibd/
    G4UIdirectory* dir = new G4UIdirectory("/generator/reacibd/");
    dir->SetGuidance("Control the reactor isotope contents of the Reactor IBD generator");

    U235AmpCmd = new G4UIcmdWithADouble("/generator/reacibd/U235", this);
    U235AmpCmd = SetGuidance("Sets the fractional amount of U235 in the reactor of interest");
    U235AmpCmd->SetParameterName("U235Amp",false);
    U235AmpCmd->SetDefaultValue( reacibdgen->GetU235Content() );


    U238AmpCmd = new G4UIcmdWithADouble("/generator/reacibd/U238", this);
    U238AmpCmd = SetGuidance("Sets the fractional amount of U238 in the reactor of interest");
    U238AmpCmd->SetParameterName("U238Amp",false);
    U238AmpCmd->SetDefaultValue( reacibdgen->GetU238Content() );


    Pu239AmpCmd = new G4UIcmdWithADouble("/generator/reacibd/Pu239", this);
    Pu239AmpCmd = SetGuidance("Sets the fractional amount of Pu239 in the reactor of interest");
    Pu239AmpCmd->SetParameterName("Pu239Amp",false);
    Pu239AmpCmd->SetDefaultValue( reacibdgen->GetPu239Content() );

    Pu241AmpCmd = new G4UIcmdWithADouble("/generator/reacibd/Pu241", this);
    Pu241AmpCmd = SetGuidance("Sets the fractional amount of Pu241 in the reactor of interest");
    Pu241AmpCmd->SetParameterName("Pu241Amp",false);
    Pu241AmpCmd->SetDefaultValue( reacibdgen->GetPu241Content() );
  }

  ReacIBDgenMessenger::~ReacIBDgenMessenger() {;}

  void ReacIBDgenMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
  {
    if ( command == U235AmpCmd )
      {
        G4double U235Amp = U235AmpCmd->GetNewDoubleValue( newValue );
        reacibdgen->SetU235AmpValue ( U235Amp );
      }
    else if ( command == U238AmpCmd )
      {
        G4double U238Amp = U238AmpCmd->GetNewDoubleValue( newValue );
        reacibdgen->SetU238AmpValue ( U238Amp );
      }
    else if ( command == Pu239AmpCmd )
      {
        G4double Pu239Amp = Pu239AmpCmd->GetNewDoubleValue( newValue );
        reacibdgen->SetPu239AmpValue ( Pu239Amp );
      }
    else if ( command == Pu241AmpCmd )
      {
        G4double Pu241Amp = Pu241AmpCmd->GetNewDoubleValue( newValue );
        reacibdgen->SetPu241AmpValue ( Pu241Amp );
      }
    else
      {
        G4cerr << "Error: Invalid ReacIBDgenMessenger \"set\" command" << G4endl;
      }
  }

  G4String ReacIBDgenMessenger::GetCurrentValue(G4UIcommand* command)
  {
    if ( command == U235AmpCmd )
	return U235AmpCmd->ConvertToString( reacibdgen->GetU235Amplitude() );
    else if ( command == U238AmpCmd )
	return U238AmpCmd->ConvertToString( reacibdgen->GetU238Amplitude() );
    else if ( command == Pu239AmpCmd )
	return Pu239AmpCmd->ConvertToString( reacibdgen->GetPu239Amplitude() );
    else if ( command == Pu241AmpCmd )
	return Pu241AmpCmd->ConvertToString( reacibdgen->GetPu241Amplitude() );

    //Get here, you return an error.
    return G4String("Error: Invalid ReacIBDgenMessenger \"get\" command");
  }

}  //namespace RAT
