// This file is part of the GenericLAND software library.
// $Id: GLG4PrimaryGeneratorMessenger.cc,v 1.2 2005/11/19 17:59:39 volsung Exp $
//
// GLG4PrimaryGeneratorMessenger.cc by Glenn Horton-Smith, Feb. 1999
// updated Aug. 3-17, 2001, for new GLG4PrimaryGeneratorAction

////////////////////////////////////////////////////////////////
// GLG4PrimaryGeneratorMessenger
////////////////////////////////////////////////////////////////

#include "GLG4PrimaryGeneratorMessenger.hh"
#include "GLG4PrimaryGeneratorAction.hh"

#include <RAT/Factory.hh>

#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4ParticleTable.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4UImanager.hh"
#include "G4ios.hh"
#include "G4Event.hh"
#include "globals.hh"
#include <CLHEP/Units/SystemOfUnits.h>

#include "fstream"  // for file streams
#include <sstream>  // for string streams
#include "iomanip"  // for G4std::setw(), etc..

#include "GLG4Gen.hh"
#include "GLG4StringUtil.hh"

#include "local_g4compat.hh"

GLG4PrimaryGeneratorMessenger::GLG4PrimaryGeneratorMessenger(GLG4PrimaryGeneratorAction* myaction)
:myAction(myaction)
{
  G4UIdirectory *GenDir= new G4UIdirectory("/generator/");
  GenDir->SetGuidance("Control the primary event generator.");

  ListCmd = new G4UIcommand("/generator/list",this);
  
  EventWindowCmd= new G4UIcommand("/generator/event_window", this);
  EventWindowCmd->SetGuidance("Set/show event time window");
  EventWindowCmd->SetParameter(new G4UIparameter("window(ns)",'d',true));

  GunCmd = new G4UIcommand("/generator/gun",this);
  GunCmd->SetGuidance("Set/show gun parameters.");
  GunCmd->SetGuidance("  particle_name:           name of particle");
  GunCmd->SetGuidance("  x_mm, y_mm, z_mm:        position of gun in mm");
  GunCmd->SetGuidance("  px_MeV, py_MeV, pz_MeV:  momentum in MeV/c");
  GunCmd->SetGuidance("  K_MeV:                   kinetic energy override");
  GunCmd->SetGuidance("  pol_x, pol_y, pol_z:     polarization [optional]");
  GunCmd->SetGuidance("  mult:                    multiplicity [optional]");
  GunCmd->SetGuidance("For isotropic, leave px,py,pz zero and set K_MeV.");
  GunCmd->SetGuidance("For random polarization, leave pol_* zero.");
  GunCmd->SetGuidance("Note: this command is an alias for the two commands:\n"
   "   /generator/pos/set 9 \"x y z\"\n"
   "   /generator/vtx/set 17 \"particle_name px py pz K polx poly polz mult\"\n"
   "Use the above two commands directly for more control.\n"
   "Type the */set commands with only one argument for current state and help."
		      );
  
  GunCmd->SetParameter(new G4UIparameter("particle_name", 's', true));
  GunCmd->SetParameter(new G4UIparameter("x_mm", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("y_mm", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("z_mm", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("px_MeV", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("py_MeV", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("pz_MeV", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("K_MeV", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("pol_x", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("pol_y", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("pol_z", 'd', true));
  GunCmd->SetParameter(new G4UIparameter("mult", 'd', true));

  GenAddCmd = new G4UIcommand("/generator/add", this);
  GenAddCmd->SetGuidance("Enable and add new event generator to simulation.");
  GenAddCmd->SetGuidance("Usage: /generator/add generator_name generator_state");
  
  GenAddCmd->SetParameter(new G4UIparameter("gen_name", 's', true));
  GenAddCmd->SetParameter(new G4UIparameter("gen_state", 's', true));


  RateSetCmd= new G4UIcommand("/generator/rate/set",this);
  RateSetCmd->SetGuidance("Set generator rate");
  RateSetCmd->SetParameter(new G4UIparameter("setting", 's', true));

  VtxSetCmd= new G4UIcommand("/generator/vtx/set",this);
  VtxSetCmd->SetGuidance("Set vertex generator state");
  VtxSetCmd->SetParameter(new G4UIparameter("setting", 's', true));
  
  PosSetCmd= new G4UIcommand("/generator/pos/set",this);
  PosSetCmd->SetGuidance("Set position generator state");
  PosSetCmd->SetParameter(new G4UIparameter("setting", 's', true));
  
  GenClearCmd = new G4UIcommand("/generator/clear", this); 
  GenClearCmd->SetGuidance("Clear all event generators in this simulation.");
  GenClearCmd->SetGuidance("Usage: /generator/clear ");

  lastGen = 0;
}


GLG4PrimaryGeneratorMessenger::~GLG4PrimaryGeneratorMessenger()
{
}

void pop_first_word(G4String in, G4String &first, G4String &rest)
{
  in = util_strip_default(in);
  G4String::size_type pos = in.find(" ");
  if (pos != G4String::npos) {
    first = in.substr(0, pos);
    rest = util_strip_default(in.substr(pos+1));
  } else {
    first = in;
    rest = "";
  }
}

void GLG4PrimaryGeneratorMessenger::
SetNewValue(G4UIcommand * command,G4String newValues)
{
  G4String first, rest;

  if (command == ListCmd) {
  } else if (command == GunCmd) {
  } else if (command == GenAddCmd) {
    pop_first_word(newValues, first, rest);
    try {
      lastGen = RAT::GlobalFactory<GLG4Gen>::New(first);
      lastGen->SetState(rest);
      myAction->AddGenerator(lastGen);
    } catch (RAT::FactoryUnknownID &unknown) {
      G4Exception(__FILE__, "No Active Generator", FatalException, ("Unknown generator " + unknown.id).c_str());
    }
  } else if (command == RateSetCmd) {
    if (lastGen)
      lastGen->SetTimeState(newValues);
    else
      G4Exception(__FILE__, "No Active Generator", FatalException, "Cannot use /generator/rate/set without active generator");
  } else if (command == VtxSetCmd) {
    if (lastGen)
      lastGen->SetVertexState(newValues);
    else
      G4Exception(__FILE__, "No Active Generator", FatalException, "Cannot use /generator/vtx/set without active generator");
  } else if (command == PosSetCmd) {
    if (lastGen)
      lastGen->SetPosState(newValues);
    else
      G4Exception(__FILE__, "No Active Generator", FatalException, "Cannot use /generator/pos/set without active generator");
  } else if (command == EventWindowCmd) {
    G4double newWindow = util_to_double(newValues);
    if (newWindow <= 0.0)
      G4cerr << "Time window must be positive" << G4endl;
    else
      myAction->SetEventWindow(newWindow*CLHEP::ns);
  } else if (command == GenClearCmd){   
    myAction->ClearGenerators(); // clear all event generators
  } else {
    G4cerr <<  "invalid GLG4 \"set\" command";
  }
}


G4String GLG4PrimaryGeneratorMessenger::
GetCurrentValue(G4UIcommand * /*command*/)
{ 
  return G4String("invalid GLG4PrimaryGeneratorMessenger \"get\" command");
}


