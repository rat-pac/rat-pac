// RAT::FastNeutronMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the FastNeutron parameters via the command line.

//#include <RAT/FastNeutronMessenger.hh>
//#include <RAT/VertexGen_FastNeutron.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

#include <RAT/VertexGen_FastNeutron.hh>

namespace RAT {
    
    FastNeutronMessenger::FastNeutronMessenger(VertexGen_FastNeutron* e) :
    fastneutron(e)
    {
        // Commands will in a /generator/fastneutron/ directory
        G4UIdirectory* dir = new G4UIdirectory("/generator/fastneutron/");
        dir->SetGuidance("Control the physics parameters of the fastneutron selector");
        
        DCmd = new G4UIcmdWithADouble("/generator/fastneutron/depth", this);
        DCmd->SetGuidance("Sets the value of sine-squared theta (the weak mixing angle)");
        DCmd->SetParameterName("valueA",false);
        DCmd->SetDefaultValue( fastneutron->GetDepth() );
    
        ECmd = new G4UIcmdWithADouble("/generator/fastneutron/enthresh", this);
        ECmd->SetGuidance("Set the neutron energy threshold (MeV)");
        ECmd->SetParameterName("valueEt",false);
        ECmd->SetDefaultValue( fastneutron->GetEnThreshold() );

        STCmd = new G4UIcmdWithADouble("/generator/fastneutron/sidewalls", this);
        STCmd->SetGuidance("Set the location of the incoming fastNeutron. Sidewalls = 1, top-bottom  = 0");
        STCmd->SetParameterName("valueST",false);
        STCmd->SetDefaultValue( fastneutron->GetSideBool() );
    
    }
    
    FastNeutronMessenger::~FastNeutronMessenger() {;}
    
    void FastNeutronMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
    {
        if ( command == DCmd )
        {
            G4double a = DCmd->GetNewDoubleValue( newValue );
            fastneutron->SetDepth( a );
        }
        else  if ( command == ECmd )
        {
            double e  = ECmd->GetNewDoubleValue( newValue );
            fastneutron->SetEnThreshold ( e  );
        }
        else  if ( command == STCmd )
        {
            double st   = STCmd->GetNewDoubleValue( newValue );
            fastneutron->SetSideBool (st);
        }
        else
        {
            G4cerr << "Error: Invalid FastNeutronMessenger \"set\" command" << G4endl;
        }
    }
    
    G4String FastNeutronMessenger::GetCurrentValue(G4UIcommand* command)
    {
        if ( command == DCmd )
            return DCmd->ConvertToString( fastneutron->GetDepth() );
        else if ( command == ECmd )
            return ECmd->ConvertToString( fastneutron->GetEnThreshold() );
        else if ( command == STCmd )
            return STCmd->ConvertToString( fastneutron->GetSideBool() );
        // Error if we reach here.
        return G4String("Error: Invalid FastNeutronMessenger \"get\" command");
    }
    
} // namespace RAT
