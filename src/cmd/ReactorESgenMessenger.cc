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
	  
	// Set up ability to specify the reactor power level
	ReactorPowerCmd = new G4UIcmdWithADouble("/generator/reactor_es/ReactorPower", this);
	ReactorPowerCmd->SetGuidance("Sets the reactor power level (in GWth)");
	ReactorPowerCmd->SetParameterName("reactorpower",false);
	ReactorPowerCmd->SetDefaultValue(3.758); // Power level of Perry reactor in Ohio
	
	// Set up ability to specify the average energy released per fission
	EperFissCmd = new G4UIcmdWithADouble("/generator/reactor_es/EperFission", this);
	EperFissCmd->SetGuidance("Sets the average energy released per fission (in MeV)");
	EperFissCmd->SetParameterName("EperFiss",false);
	EperFissCmd->SetDefaultValue(200); // Energy released per fission of U235
	
	// Set up ability to specify detector-reactor distance
	StandoffCmd = new G4UIcmdWithADouble("/generator/reactor_es/Standoff", this);
	StandoffCmd->SetGuidance("Sets the detector-reactor standoff (in km)");
	StandoffCmd->SetParameterName("standoff",false);
	StandoffCmd->SetDefaultValue(13); // Distance from Watchman to Perry reactor
	
	// Set up ability to specify the data acquisition time
	TimeCmd = new G4UIcmdWithADouble("/generator/reactor_es/Time", this);
	TimeCmd->SetGuidance("Sets the acquisition time (in years)");
	TimeCmd->SetParameterName("acqTime",false);
	TimeCmd->SetDefaultValue(5); // I have been using 5 years
	
	// Set up ability to specify the size of the sample water volume
	WvolumeCmd = new G4UIcmdWithADouble("/generator/reactor_es/WaterVol", this);
	WvolumeCmd->SetGuidance("Sets the size of the water volume in which we are sampling (in kilotons)");
	WvolumeCmd->SetParameterName("Wvol",false);
	WvolumeCmd->SetDefaultValue(1); // Watchman default fiducial volume
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
	  
	else if ( command == ReactorPowerCmd ){
		
	    G4double power_ = ReactorPowerCmd->GetNewDoubleValue( newValue );
		reactoresgen->SetReactorPower(power_);
	}
	  
	else if ( command == EperFissCmd ){
		  
		G4double eperfission_ = EperFissCmd->GetNewDoubleValue( newValue );
		reactoresgen->SetEnergyPerFission(eperfission_);
	}
	  
	else if ( command == StandoffCmd ){
		  
		G4double standoff_ = StandoffCmd->GetNewDoubleValue( newValue );
		reactoresgen->SetDetectorStandoff(standoff_);
	}
	  
	else if ( command == TimeCmd ){
		  
		G4double time_ = TimeCmd->GetNewDoubleValue( newValue );
		reactoresgen->SetAcquisitionTime(time_);
	}
	  
	else if ( command == WvolumeCmd ){
		  
		G4double watervol_ = WvolumeCmd->GetNewDoubleValue( newValue );
		reactoresgen->SetWaterVolume(watervol_);
		
		// We force this to be the last command so that once it is run, it will calculate the number of events and use beamOn
		reactoresgen->CalculateNumEvents();
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
	  
	  else if (command == ReactorPowerCmd)
		  return ReactorPowerCmd->ConvertToString(reactoresgen->GetReactorPower());
	  
	  else if (command == EperFissCmd)
		  return EperFissCmd->ConvertToString(reactoresgen->GetEnergyPerFission());
	  
	  else if (command == StandoffCmd)
		  return StandoffCmd->ConvertToString(reactoresgen->GetDetectorStandoff());
	  
	  else if (command == TimeCmd)
		  return TimeCmd->ConvertToString(reactoresgen->GetAcquisitionTime());
	  
	  else if (command == WvolumeCmd)
		  return WvolumeCmd->ConvertToString(reactoresgen->GetWaterVolume());
		  
	  else
		  return G4String("Error: Invalid ReactorESgenMessenger \"get\" command");
	}

} // namespace RAT
