// RAT::IsotopeMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the Isotope parameters via the command line.

//#include <RAT/IsotopeMessenger.hh>
//#include <RAT/VertexGen_Isotope.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

#include <RAT/VertexGen_Isotope.hh>

namespace RAT {
    
    IsotopeMessenger::IsotopeMessenger(VertexGen_Isotope* e) :
    isotope(e)
    {
        // Commands will in a /generator/isotope/ directory
        G4UIdirectory* dir = new G4UIdirectory("/generator/isotope/");
        dir->SetGuidance("Control the physics parameters of the isotope selector");
        
        ACmd = new G4UIcmdWithADouble("/generator/isotope/A", this);
        ACmd->SetGuidance("Sets the value of sine-squared theta (the weak mixing angle)");
        ACmd->SetParameterName("valueA",false);
        ACmd->SetDefaultValue( isotope->GetIsotopeA() );
        
        ZCmd = new G4UIcmdWithADouble("/generator/isotope/Z", this);
        ZCmd->SetGuidance("Sets Z");
        ZCmd->SetParameterName("valueZ",false);
        ZCmd->SetDefaultValue( isotope->GetIsotopeZ() );
        
        ECmd = new G4UIcmdWithADouble("/generator/isotope/E", this);
        ECmd->SetGuidance("Set the excitation energy (MeV) ");
        ECmd->SetParameterName("valueE",false);
        ECmd->SetDefaultValue( isotope->GetIsotopeE() );
    }
    
    IsotopeMessenger::~IsotopeMessenger() {;}
    
    void IsotopeMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
    {
        if ( command == ACmd )
        {
            G4double a = ACmd->GetNewDoubleValue( newValue );
            isotope->SetIsotopeA( a );
        }
        else if ( command == ZCmd )
        {
            G4double z  = ZCmd->GetNewDoubleValue( newValue );
            isotope->SetIsotopeZ( z  );
        }else  if ( command == ECmd )
        {
            double e  = ECmd->GetNewDoubleValue( newValue );
            isotope->SetIsotopeE ( e  );
        }
        else
        {
            G4cerr << "Error: Invalid IsotopeMessenger \"set\" command" << G4endl;
        }
    }
    
    G4String IsotopeMessenger::GetCurrentValue(G4UIcommand* command)
    {
        if ( command == ACmd )
            return ACmd->ConvertToString( isotope->GetIsotopeA() );
        else if ( command == ZCmd )
            return ZCmd->ConvertToString( isotope->GetIsotopeZ() );
        else if ( command == ECmd )
            return ECmd->ConvertToString( isotope->GetIsotopeE() );
        // Error if we reach here.
        return G4String("Error: Invalid IsotopeMessenger \"get\" command");
    }
    
} // namespace RAT
