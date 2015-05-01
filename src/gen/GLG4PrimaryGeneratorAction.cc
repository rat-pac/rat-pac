// This file is part of the GenericLAND software library.
// $Id: GLG4PrimaryGeneratorAction.cc,v 1.4 2006/04/27 00:12:29 volsung Exp $
//
// new GLG4PrimaryGeneratorAction.cc by Glenn Horton-Smith, August 3-17, 2001

////////////////////////////////////////////////////////////////
// GLG4PrimaryGeneratorAction
////////////////////////////////////////////////////////////////

// include files (boring)
#include "GLG4PrimaryGeneratorAction.hh"
#include "GLG4PrimaryGeneratorMessenger.hh"

#include "globals.hh"
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4Track.hh"
#include "G4ios.hh"
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include "Randomize.hh"

#include "GLG4Gen.hh"
#include "GLG4TimeGen.hh"
#include "GLG4VertexGen.hh"            // for vertex generator
#include "GLG4PosGen.hh"               // for global position generator
#include <RAT/Factory.hh>
#include <RAT/PosGen_Line.hh>
#include <RAT/PosGen_FillShell.hh>
#include <RAT/PosGen_Radial.hh>
#include <RAT/PosGen_FillShell.hh>
#include <RAT/PosGen_Multipoint.hh>
#include <RAT/PosGen_TriMeshSurface.hh>
#include <RAT/VertexGen_PhotonBomb.hh>
#include <RAT/VertexGen_WIMP.hh>
#include <RAT/Gen_RandomTrigger.hh>
#include <RAT/Log.hh>
#include <stdio.h>                   // for sprintf

#include <RAT/EventInfo.hh>
#include <RAT/TimeUtil.hh>

GLG4PrimaryGeneratorAction*
GLG4PrimaryGeneratorAction::theGLG4PrimaryGeneratorAction=0;

GLG4PrimaryGeneratorAction::
GLG4PrimaryGeneratorAction()
{
  if (theGLG4PrimaryGeneratorAction == 0) {
    theGLG4PrimaryGeneratorAction= this;
  }
  else {
    G4Exception(__FILE__, "Multiple Primary Generator Action", FatalException, "Error, more than one GLG4PrimaryGeneratorAction instantiated.\n"
    "Sorry, but this is a no-no because GLG4SteppingAction relies on\n"
    "GLG4PrimaryGeneratorAction::GetTheGLG4PrimaryGeneratorAction().\n"
    "This is yucky, I know -- please rewrite GLG4SteppingAction AND\n"
    "all main() programs so that constructor accepts a pointer to\n"
    "the GLG4PrimaryGeneratorAction you really want them to use.");
  }

  // initialize messenger and time fields
  myMessenger = new GLG4PrimaryGeneratorMessenger(this);
  myUniversalTime= 0.0;
  myUniversalTimeSincePriorEvent= 0.0;
  
  myEventWindow= 0*CLHEP::ns;

  // load up factories with known generators

  // Generic generators
  RAT::GlobalFactory<GLG4Gen>::Register("combo", 
					new RAT::Alloc<GLG4Gen, GLG4Gen_Combo>);
  RAT::GlobalFactory<GLG4Gen>::Register("external", 
					new RAT::Alloc<GLG4Gen, GLG4Gen_External>);
  RAT::GlobalFactory<GLG4Gen>::Register("random_trigger",
          new RAT::Alloc<GLG4Gen, Gen_RandomTrigger>);

  // These generators are used by combo to make a "custom" generator
  // Rate generators
  RAT::GlobalFactory<GLG4TimeGen>::Register("uniform", 
					    new RAT::Alloc<GLG4TimeGen, 
					    GLG4TimeGen_Uniform>);
  RAT::GlobalFactory<GLG4TimeGen>::Register("poisson",
					    new RAT::Alloc<GLG4TimeGen, 
					    GLG4TimeGen_Poisson>);

  // Vertex generators
  RAT::GlobalFactory<GLG4VertexGen>::Register("gun", 
					      new RAT::Alloc<GLG4VertexGen, 
					      GLG4VertexGen_Gun>);
  RAT::GlobalFactory<GLG4VertexGen>::Register("gun2", 
					      new RAT::Alloc<GLG4VertexGen, 
					      GLG4VertexGen_Gun2>);
  RAT::GlobalFactory<GLG4VertexGen>::Register("pbomb", 
					      new RAT::Alloc<GLG4VertexGen, 
					      RAT::VertexGen_PhotonBomb>);
  RAT::GlobalFactory<GLG4VertexGen>::Register("wimp",
					      new RAT::Alloc<GLG4VertexGen,
					      RAT::VertexGen_WIMP>);
  RAT::GlobalFactory<GLG4VertexGen>::Register("HEPEvt",
					      new RAT::Alloc<GLG4VertexGen,
					      GLG4VertexGen_HEPEvt>);

  // Position generators
  RAT::GlobalFactory<GLG4PosGen>::Register("point", 
					   new RAT::Alloc<GLG4PosGen, 
					   GLG4PosGen_Point>);
  RAT::GlobalFactory<GLG4PosGen>::Register("line", 
					   new RAT::Alloc<GLG4PosGen, 
					   RAT::PosGen_Line>);
  RAT::GlobalFactory<GLG4PosGen>::Register("fillshell", 
					   new RAT::Alloc<GLG4PosGen, 
					   RAT::PosGen_FillShell>);
  RAT::GlobalFactory<GLG4PosGen>::Register("radial", 
					   new RAT::Alloc<GLG4PosGen, 
					   RAT::PosGen_Radial>);
  RAT::GlobalFactory<GLG4PosGen>::Register("fillshell", 
					   new RAT::Alloc<GLG4PosGen, 
					   RAT::PosGen_FillShell>);
  RAT::GlobalFactory<GLG4PosGen>::Register("paint", 
					   new RAT::Alloc<GLG4PosGen, 
					   GLG4PosGen_Paint>);
  RAT::GlobalFactory<GLG4PosGen>::Register("fill", 
					   new RAT::Alloc<GLG4PosGen, 
					   GLG4PosGen_Fill>);
  RAT::GlobalFactory<GLG4PosGen>::Register("multipoint", 
					   new RAT::Alloc<GLG4PosGen, 
					   RAT::PosGen_Multipoint>);
  RAT::GlobalFactory<GLG4PosGen>::Register("triMeshSurface",
					   new RAT::Alloc<GLG4PosGen, 
					   RAT::PosGen_TriMeshSurface>);

  needReset = true;
}

GLG4PrimaryGeneratorAction::~GLG4PrimaryGeneratorAction()
{
}

void GLG4PrimaryGeneratorAction::SetEventWindow(double argEventWindow)
{
  myEventWindow= argEventWindow;
}

void GLG4PrimaryGeneratorAction::AddGenerator(GLG4Gen *gen)
{
  myGenList.push(gen);
  needReset = true;
}
void GLG4PrimaryGeneratorAction::ClearGenerators(void)
{
  RAT::warn << "Clearing all event generators!" << endl;
  myGenList.clear();
  needReset = true;
}
// GeneratePrimaries (this is the interesting part!)
void GLG4PrimaryGeneratorAction::GeneratePrimaries(G4Event* argEvent)
{  
  RAT::EventInfo *eventInfo = new RAT::EventInfo;
  eventInfo->utc = RAT::AddNanoseconds(runUTC, (long) GetUniversalTime());
  argEvent->SetUserInformation((G4VUserEventInformation*) eventInfo);  
  
  if (needReset) {
    // Need to reset the times of all the generators now that we've started
    // running.

    GLG4GenList temp;
    while (!myGenList.empty()) {
      GLG4Gen *gen = myGenList.top();
      myGenList.pop();
      gen->ResetTime();
      temp.push(gen);
    }
    
    myGenList = temp; // copy them all back
    needReset = false;
  }

  // Find time increment until next event on queue
  // (priority queue will always have next event on top)
  if (myGenList.empty())
    G4Exception(__FILE__, "No Active Generator", FatalException, "GLG4PrimaryGeneratorAction: No generators selected!");
  
  double timeToNextEvent = myGenList.top()->NextTime();
  myUniversalTimeSincePriorEvent = timeToNextEvent;
  myUniversalTime += timeToNextEvent;

  // Offset time of all queued generators, so top generator's NextTime() == 0
  myGenList.SubtractTime(timeToNextEvent);

  // Add all events in event window, includes pileup and deferred tracks
  while (!myGenList.empty()
	 && myGenList.top()->NextTime() <= myEventWindow) {

    GLG4Gen *nextGen = myGenList.top();
    myGenList.pop();

    double vertexTime = nextGen->NextTime();
    nextGen->GenerateEvent(argEvent);

    if (nextGen->IsRepeatable()) {
      // Reset time relative to time to previous generator time
      // to get pilup correct
      nextGen->ResetTime(vertexTime);
      myGenList.push(nextGen);
    } else
      delete nextGen;
  }
}


void GLG4PrimaryGeneratorAction::
DeferTrackToLaterEvent(const G4Track * track)
{
  myGenList.push(new GLG4Gen_DeferTrack(track));
}
