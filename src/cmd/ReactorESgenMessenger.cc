// Created by Daniel Hellfeld (07/2015)
// Follows similarly to ESgenMessenger.cc

#include <RAT/ReactorESgenMessenger.hh>
#include <RAT/ReactorESgen.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

namespace RAT {

  ReactorESgenMessenger::ReactorESgenMessenger(ReactorESgen* e): reactoresgen(e){
      
    // Commands will in a /generator/reactor_es/ directory
    G4UIdirectory* dir = new G4UIdirectory("/generator/reactor_es/");
    dir->SetGuidance("Control the physics parameters of the D.Hellfeld elastic-scattering generator");

	// Set up ability to specify U235 fission fraction
    ffU235Cmd = new G4UIcmdWithADouble("/generator/reactor_es/U235", this);
    ffU235Cmd->SetGuidance("Sets the U235 fission fraction");
    ffU235Cmd->SetParameterName("u235",false);
    ffU235Cmd->SetDefaultValue(0.496); // See ReactorESgen.cc for reason behind this value
      
	// Set up ability to specify U238 fission fraction
    ffU238Cmd = new G4UIcmdWithADouble("/generator/reactor_es/U238", this);
    ffU238Cmd->SetGuidance("Sets the U238 fission fraction");
    ffU238Cmd->SetParameterName("u238",false);
    ffU238Cmd->SetDefaultValue(0.087); // See ReactorESgen.cc for reason behind this value
      
	// Set up ability to specify Pu239 fission fraction
    ffPu239Cmd = new G4UIcmdWithADouble("/generator/reactor_es/Pu239", this);
    ffPu239Cmd->SetGuidance("Sets the Pu239 fission fraction");
    ffPu239Cmd->SetParameterName("pu239",false);
    ffPu239Cmd->SetDefaultValue(0.351); // See ReactorESgen.cc for reason behind this value
      
	// Set up ability to specify Pu241 fission fraction
    ffPu241Cmd = new G4UIcmdWithADouble("/generator/reactor_es/Pu241", this);
    ffPu241Cmd->SetGuidance("Sets the Pu241 fission fraction");
    ffPu241Cmd->SetParameterName("pu241",false);
    ffPu241Cmd->SetDefaultValue(0.066); // See ReactorESgen.cc for reason behind this value
  }

  ReactorESgenMessenger::~ReactorESgenMessenger() {;}
  
  void ReactorESgenMessenger::SetNewValue(G4UIcommand* command, G4String newValue){

    if ( command == ffU235Cmd ){
        
		// Retrieve and set value
        G4double ffU235 = ffU235Cmd->GetNewDoubleValue( newValue );
        reactoresgen->SetU235FissionFrac( ffU235 );
    }
    
    else if ( command == ffU238Cmd ){
        
        G4double ffU238 = ffU238Cmd->GetNewDoubleValue( newValue );
        reactoresgen->SetU238FissionFrac( ffU238 );
      }
      
    else if ( command == ffPu239Cmd ){
        
        G4double ffPu239 = ffPu239Cmd->GetNewDoubleValue( newValue );
        reactoresgen->SetPu239FissionFrac( ffPu239 );
    }
      
    else if ( command == ffPu241Cmd ){
        
        G4double ffPu241 = ffPu241Cmd->GetNewDoubleValue( newValue );
        reactoresgen->SetPu241FissionFrac( ffPu241 );
		
		// Typically Pu241 is listed last, so at this point we want to check whether the fractions add
		//   up to 1. If they dont, we set the fractions to the default values. See ReactorESgen.cc.
		reactoresgen->CheckFissionFractions();
    }
	  
	else
		G4cerr << "\nError: Invalid ReactorESgenMessenger \"set\" command\n";
	  
  }
	
	
  G4String ReactorESgenMessenger::GetCurrentValue(G4UIcommand* command){
	  
	  if (command == ffU235Cmd)
		  return ffU235Cmd->ConvertToString(reactoresgen->GetU235FissionFrac());
	  
	  else if (command == ffU238Cmd)
		  return ffU238Cmd->ConvertToString(reactoresgen->GetU238FissionFrac());
	  
	  else if (command == ffPu239Cmd)
		  return ffPu239Cmd->ConvertToString(reactoresgen->GetPu239FissionFrac());
	  
	  else if (command == ffPu241Cmd)
		  return ffPu241Cmd->ConvertToString(reactoresgen->GetPu241FissionFrac());
	  
	  else
		  return G4String("Error: Invalid ReactorESgenMessenger \"get\" command");
	}

} // namespace RAT
