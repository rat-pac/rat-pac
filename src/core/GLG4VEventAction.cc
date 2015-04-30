// This file is part of the GenericLAND software library.
// $Id: GLG4VEventAction.cc,v 1.1 2005/08/30 19:55:23 volsung Exp $
//
//  GLG4 version by Glenn Horton-Smith December, 2004.
//  Based on earlier work by H. Ikeda and G. Horton-Smith

#include "local_g4compat.hh"

#include "GLG4VEventAction.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4VVisManager.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4DigiManager.hh"
#include "G4UImanager.hh"
#include "G4UnitsTable.hh"
#include "G4ios.hh"
#include "G4UIcmdWithAString.hh"

#include "GLG4Scint.hh"  // for doScintilllation and total energy deposition info


//GLG4HitPhotonCollection GLG4VEventAction :: theHitPhotons= GLG4HitPhotonCollection();
GLG4HitPMTCollection GLG4VEventAction :: theHitPMTCollection= GLG4HitPMTCollection();
G4bool GLG4VEventAction :: flagFullOutputMode = false;
G4bool GLG4VEventAction :: fgDoParameterizedScintillation = true;

GLG4VEventAction::GLG4VEventAction()
:drawFlag("all")
{
  fDrawCmd = new G4UIcmdWithAString("/event/drawTracks",this);
  fDrawCmd->SetGuidance("Draw the tracks in the event");
  fDrawCmd->SetGuidance("  Choice : none, charged(default), all, allnonopt");
  fDrawCmd->SetParameterName("choice",true);
  fDrawCmd->SetDefaultValue("charged");
  fDrawCmd->SetCandidates("none charged all allnonopt");
  fDrawCmd->AvailableForStates(G4State_Idle);

  fFileCmd= new G4UIcommand("/event/output_file",this);
  fFileCmd->SetGuidance("Set the file name for event output.");
  fFileCmd->SetParameter( new G4UIparameter("filename", 's', true) );

  fModeCmd = new G4UIcmdWithAString("/event/output_mode",this);
  fModeCmd->SetGuidance("Select how much hit data you want output:");
  fModeCmd->SetGuidance("  Choice : basic(default), full");
  fModeCmd->SetParameterName("choice",true);
  fModeCmd->SetDefaultValue("basic");
  fModeCmd->SetCandidates("basic full");
  fModeCmd->AvailableForStates(G4State_PreInit, G4State_Idle, G4State_GeomClosed);

  G4UIcommand *other_cmd;
  other_cmd= new G4UIcommand("/event/doParameterizedScintillation",this);
  other_cmd->SetGuidance("Turn on/off fast simulation of PMT response to scintillator energy deposition.");
  other_cmd->SetGuidance("Note that fast (parameterized) simulation only occurs if detailed tracking of\n"
			 "optical photons is disabled via /glg4scint/off");
  other_cmd->SetParameter(new G4UIparameter("status", 'i', false));
}

GLG4VEventAction::~GLG4VEventAction()
{
  delete fDrawCmd;
  delete fFileCmd;
  delete fModeCmd;
}

G4String GLG4VEventAction::GetCurrentValue(G4UIcommand * command)
{
  if(command->GetCommandName() == "drawTracks")
    return drawFlag;
  else if (command->GetCommandName() == "output_mode")
    return ( flagFullOutputMode ? "full" : "basic" );
  else if (command->GetCommandName() == "doParameterizedScintillation")
    return ( fgDoParameterizedScintillation ? "1" : "0" );

  return G4String("Unhandled command passed to "
		  "GLG4EventAction::GetCurrentValue");
}

void GLG4VEventAction::SetNewValue(G4UIcommand * command,
				G4String newValue)
{
  if(command->GetCommandName() == "drawTracks")
    drawFlag= newValue;
  else if (command->GetCommandName() == "output_mode") {
    G4bool old_flagFullOutputMode= flagFullOutputMode;
    flagFullOutputMode=  ( newValue == "full" );
    if ( flagFullOutputMode != old_flagFullOutputMode ) {
      G4cout << "Switching output mode to " <<
	( flagFullOutputMode ? "full" : "basic" ) << G4endl;
    }
    else {
      G4cout << "FYI: new output mode == old output mode" << G4endl;
    }
  }
  else if (command->GetCommandName() == "output_file")
    {
      // always call CloseFile() here, in case file is open
      // (user code should ignore CloseFile() if file is not open)
      CloseFile();
      // open new file, if new filename given
      // (it is okay not to give a name, in case user just wants to close file)
      if (newValue.length() <= 0)
	{
	  // G4cerr << "Null Output File Name" << G4endl;
	  return;
	}
      else
	  OpenFile(newValue,flagFullOutputMode);

    }
  else if (command->GetCommandName() == "doParameterizedScintillation")
    {
      fgDoParameterizedScintillation= (atoi((const char*)newValue)!=0);
    }
  else
    {
      G4cerr << "Unknown command ``" << command->GetCommandName()
	     << " passed to GLG4EventAction::SetNewValue\n";
    }

}


void GLG4VEventAction::BeginOfEventAction(const G4Event* )
{
  GLG4Scint::ResetTotEdep();
  // clearing theHitPMTCollection clears away the HitPhotons and HitPMTs
  theHitPMTCollection.Clear(); 
  Clear();  
}

void GLG4VEventAction::EndOfEventAction(const G4Event* evt)
{
  G4TrajectoryContainer * trajectoryContainer = evt->GetTrajectoryContainer();
  G4int n_trajectories = 0;
  if(trajectoryContainer)
    { n_trajectories = trajectoryContainer->size(); }
  
  FillData(evt);

  // draw trajectories
  if(G4VVisManager::GetConcreteInstance() && drawFlag != "none")
  {
    for(G4int i=0; i<n_trajectories; i++) 
      {
	// GLG4UDGE: the explicit cast on the next line is not a good thing.
	   G4Trajectory* trj = (G4Trajectory *)((*trajectoryContainer)[i]);
           if ( (drawFlag == "all") 
		|| ((drawFlag == "allnonopt")
		    &&(trj->GetParticleName() != "opticalphoton"))
                || ((drawFlag == "charged")&&(trj->GetCharge() != 0.)) )
#if G4VERSION_NUMBER > 1000
	     trj->DrawTrajectory();
#else
	     trj->DrawTrajectory(50);
#endif
      }
  }
}


