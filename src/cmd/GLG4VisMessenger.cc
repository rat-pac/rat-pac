// This file is part of the GenericLAND software library.
// $Id: GLG4VisMessenger.cc,v 1.1 2005/08/30 19:55:23 volsung Exp $
//
// GenericLAND visualization manager messenger
//   -- main purpose of defining our own is to reorient "up" vector
//
// Author:  Glenn Horton-Smith, Jan 28, 2000
//
#ifdef G4VIS_USE

#include "GLG4VisMessenger.hh"
#include "GLG4VisManager.hh"

#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4ios.hh"
#include "G4ViewParameters.hh"
#include "globals.hh"

#include "local_g4compat.hh"

#include <sstream>

GLG4VisMessenger::GLG4VisMessenger(GLG4VisManager* pVMan_)
  : pVMan(pVMan_)
{
  // the glg4vis directory
  G4UIdirectory* Dir = new G4UIdirectory("/glg4vis/");
  Dir->SetGuidance("Visualization commands more suited to GenericLAND.");
  
  // the camera reset command
  G4UIcommand *CameraResetCmd = new G4UIcommand("/glg4vis/camera/reset", this);
  CameraResetCmd->SetGuidance
    ("Reset to nominal viewing position, up vector, etc. **DEPRECATED**");

  // the viewer reset command (same as camera/reset)
  CameraResetCmd = new G4UIcommand("/glg4vis/viewer/reset", this);
  CameraResetCmd->SetGuidance
    ("Reset to nominal viewing position, up vector, etc.");

  // the glg4vis reset command (same as camera/reset)
  CameraResetCmd = new G4UIcommand("/glg4vis/reset", this);
  CameraResetCmd->SetGuidance
    ("Reset to nominal viewing position, up vector, etc.");

  // the upvector command
  G4UIcommand *UpVectorCmd = new G4UIcommand("/glg4vis/upvector", this);
  UpVectorCmd->SetGuidance ("Set \"up\" direction");
  UpVectorCmd->SetParameter(new G4UIparameter("x", 'd', true));
  UpVectorCmd->SetParameter(new G4UIparameter("y", 'd', true));
  UpVectorCmd->SetParameter(new G4UIparameter("z", 'd', true));
}

GLG4VisMessenger::~GLG4VisMessenger()
{
}

void GLG4VisMessenger::SetNewValue (G4UIcommand * command,G4String newValues) {
  
  G4String commandname= command->GetCommandName();
  G4std::istringstream is(newValues);  
  
  G4VViewer* currentViewer = pVMan->GetCurrentViewer();
  if (!currentViewer) {
    G4cerr << "GLG4VisMessenger::SetNewValue: no current viewer."
	   << G4endl;
    return;
  }
  G4ViewParameters vp = currentViewer->GetViewParameters();
  
  if (commandname == "reset") {
    vp.SetCurrentTargetPoint ( G4Point3D(0.0,0.0,0.0) );
    vp.SetZoomFactor (1.);
    vp.SetDolly (0.);
    vp.SetViewpointDirection (G4Vector3D (0., 1., 0.));
    vp.SetUpVector (G4Vector3D (0., 0., 1.));
    G4cout << "Target point reset to (0.0,0.0,0.0)\n";
    G4cout << "Zoom factor reset to 1.\n";
    G4cout << "Dolly distance reset to 0.\n";
    G4cout << "Viewpoint direction reset to +y.\n";
    G4cout << "Up vector set to +z.";
    G4cout << G4endl;
  }
  else if (commandname == "upvector") {
    G4double x,y,z;
    is >> x >> y >> z;
    if (is.fail()) {
      G4cerr << "GLG4VisMessaneger::SetNewValue: "
	     << "Could not understand arguments, up vector left as "
	     << vp.GetUpVector() << G4endl << G4std::flush;
      return;
    }
    else {
      vp.SetUpVector (G4Vector3D (x,y,z));
    }
  }
  else {
    G4cerr << "GLG4VisMessaneger::SetNewValue: I do not recognize this command: "
	   << commandname << G4endl;
    return;
  }
  
  currentViewer->SetViewParameters(vp);
}


G4String GLG4VisMessenger::GetCurrentValue (G4UIcommand * )
{
  return G4String("invalid GLG4VisMessenger \"get\" command");
}
#endif
