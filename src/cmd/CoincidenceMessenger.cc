// RAT::CoincidenceMessenger
// 16 July 2008 JR Wilson
// Provide user commands to allow the user to change
// the parameters of the Coincidence_Gen generator via the command line.

#include <RAT/CoincidenceMessenger.hh>
#include <RAT/Coincidence_Gen.hh>

#include <G4UIcommand.hh>
#include <G4UIdirectory.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4UIcmdWithABool.hh>
#include <G4String.hh>

namespace RAT {

	CoincidenceMessenger::CoincidenceMessenger(Coincidence_Gen* cg):Co_gen(cg){  
		// Commands will in a /generator/coincidence/ directory
		G4UIdirectory* dir = new G4UIdirectory("/generator/coincidence/");
		dir->SetGuidance("Control the parameters of the coincidence generator");

		GenAddCmd = new G4UIcommand("/generator/coincidence/add", this);
		GenAddCmd->SetGuidance("Enable and add new interaction to coincidence generator.");
		GenAddCmd->SetGuidance("Usage: /generator/coincidence/add generator_name:generator_state");		
		GenAddCmd->SetParameter(new G4UIparameter("gen_name", 's', true));
		GenAddCmd->SetParameter(new G4UIparameter("gen_state", 's', true));
		
		VtxSetCmd= new G4UIcommand("/generator/coincidence/vtx/set",this);
		VtxSetCmd->SetGuidance("Set vertex generator for most recently added coincidence interaction");
		VtxSetCmd->SetParameter(new G4UIparameter("setting", 's', true));

		PosSetCmd= new G4UIcommand("/generator/coincidence/pos/set",this);
		PosSetCmd->SetGuidance("Set position generator for most recently added coincidence interaction");
		PosSetCmd->SetParameter(new G4UIparameter("setting", 's', true));	
		
		TimeWinCmd = new G4UIcmdWithADouble("/generator/coincidence/timewindow", this);
		TimeWinCmd->SetGuidance("Set time window of coincidences in ns (default = 400ns)");
		TimeWinCmd->SetParameterName("timewin",false);
		TimeWinCmd->SetDefaultValue(400);

		ERangeCmd= new G4UIcommand("/generator/coincidence/energyrange",this);
		ERangeCmd->SetGuidance("Set range for generated total energy");
		ERangeCmd->SetParameter(new G4UIparameter("Erange", 's', true));	

		ExpCmd= new G4UIcommand("/generator/coincidence/exponential",this);
		ExpCmd->SetGuidance("Set time constants in ns for subsequent decays (max of 5)");
		ExpCmd->SetParameter(new G4UIparameter("Exponents", 's', true));	
		
		ExpForceWinCmd= new G4UIcmdWithABool("/generator/coincidence/expoForceWindow",this);
		ExpForceWinCmd->SetGuidance("Force all the events within the time window (which by default is 400ns). Takes a boolean, the default is false");
		ExpForceWinCmd->SetParameterName("state", true, false);
		ExpForceWinCmd->SetDefaultValue(false);
	}

	CoincidenceMessenger::~CoincidenceMessenger(){
		delete GenAddCmd;
		delete VtxSetCmd;
		delete PosSetCmd;
		delete ERangeCmd;
		delete TimeWinCmd;
		delete ExpCmd;
		delete ExpForceWinCmd;
	}

	void CoincidenceMessenger::SetNewValue(G4UIcommand* command, G4String newValues)
	{
		if ( command == GenAddCmd ){
      		Co_gen->AddExtra(newValues);
		}else if ( command == VtxSetCmd ){
			Co_gen->SetExtraVertexState(newValues);
		}else if ( command == PosSetCmd ){
			Co_gen->SetExtraPosState(newValues);	
		}else if ( command == TimeWinCmd ){
			double tw = TimeWinCmd->GetNewDoubleValue( newValues );
			Co_gen->SetTimeWindow(tw);			
		}else if ( command == ERangeCmd ){
			Co_gen->SetEnergyRange(newValues);
		}else if ( command == ExpCmd ){
			Co_gen->SetExponentials(newValues);
		}else if ( command == ExpForceWinCmd ){
			Co_gen->SetExpoForceWindow(ExpForceWinCmd->GetNewBoolValue(newValues));
		}else{
			// Error if we reach here.
			G4cerr << "Error: CoincidenceMessenger invalic command" << G4endl;
		}
	}
        G4String CoincidenceMessenger::GetCurrentValue(G4UIcommand * /*command*/)
	{
		// should add some returns here eventually
		return G4String("invalid Coincidence	Messenger \"get\" command");
	}
}	
