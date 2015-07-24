#include <RAT/ESgenMessenger_2.hh>
#include <RAT/ESgen_2.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

namespace RAT {

  ESgenMessenger_2::ESgenMessenger_2(ESgen_2* e): esgen_2(e){
      
    // Commands will in a /generator/es_2/ directory
    G4UIdirectory* dir = new G4UIdirectory("/generator/es_2/");
    dir->SetGuidance("Control the physics parameters of the D.Hellfeld elastic-scattering generator");

    ffU235Cmd = new G4UIcmdWithADouble("/generator/es_2/U235", this);
    ffU235Cmd->SetGuidance("Sets the U235 fission fraction");
    ffU235Cmd->SetParameterName("u235",false);
    ffU235Cmd->SetDefaultValue(0.496);
      
    ffU238Cmd = new G4UIcmdWithADouble("/generator/es_2/U238", this);
    ffU238Cmd->SetGuidance("Sets the U238 fission fraction");
    ffU238Cmd->SetParameterName("u238",false);
    ffU238Cmd->SetDefaultValue(0.087);
      
    ffPu239Cmd = new G4UIcmdWithADouble("/generator/es_2/Pu239", this);
    ffPu239Cmd->SetGuidance("Sets the Pu239 fission fraction");
    ffPu239Cmd->SetParameterName("pu239",false);
    ffPu239Cmd->SetDefaultValue(0.351);
      
    ffPu241Cmd = new G4UIcmdWithADouble("/generator/es_2/Pu241", this);
    ffPu241Cmd->SetGuidance("Sets the Pu241 fission fraction");
    ffPu241Cmd->SetParameterName("pu241",false);
    ffPu241Cmd->SetDefaultValue(0.066);
  }

  ESgenMessenger_2::~ESgenMessenger_2() {;}
  
  void ESgenMessenger_2::SetNewValue(G4UIcommand* command, G4String newValue){
/*
       
    if ( command == ffU235Cmd ){
        
        G4double ffU235 = ffU235Cmd->GetNewDoubleValue( newValue );
        esgen_2->SetU235FissionFrac( ffU235 );
    }
    
    else if ( command == ffU238Cmd ){
        
        G4double ffU238 = ffU238Cmd->GetNewDoubleValue( newValue );
        esgen_2->SetU238FissionFrac( ffU238 );
      }
      
    else if ( command == ffPu239Cmd ){
        
        G4double ffPu239 = ffPu239Cmd->GetNewDoubleValue( newValue );
        esgen_2->SetPu239FissionFrac( ffPu239 );
    }
      
    else if ( command == ffPu241Cmd ){
        
        G4double ffPu241 = ffPu241Cmd->GetNewDoubleValue( newValue );
        esgen_2->SetPu241FissionFrac( ffPu241 );
    }
      
*/
      G4double ffU235  = ffU235Cmd ->GetNewDoubleValue(newValue);
      G4double ffU238  = ffU238Cmd ->GetNewDoubleValue(newValue);
      G4double ffPu239 = ffPu239Cmd->GetNewDoubleValue(newValue);
      G4double ffPu241 = ffPu241Cmd->GetNewDoubleValue(newValue);

      
      if (ffU235 + ffU238 + ffPu239 + ffPu241 == 1){
          esgen_2->SetFissionFractions(ffU235, ffU238, ffPu239, ffPu241);
      }
      
      else {
          G4cerr << "Error: Your fission fractions do not add up to 1" << G4endl;
      }
  }

} // namespace RAT
