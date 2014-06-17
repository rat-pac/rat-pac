// This file is part of the GenericLAND software library.
// $Id: GLG4PrimaryGeneratorAction.hh,v 1.3 2005/11/19 17:59:39 volsung Exp $
//
// new GLG4PrimaryGeneratorAction.hh by Glenn Horton-Smith, August 3-17, 2001

#ifndef __GLG4PrimaryGeneratorAction_hh__
#define __GLG4PrimaryGeneratorAction_hh__ 1
////////////////////////////////////////////////////////////////
// GLG4PrimaryGeneratorAction
////////////////////////////////////////////////////////////////

#include "G4VUserPrimaryGeneratorAction.hh"  // for user primary vertex gen.
#include "GLG4GenList.hh"
#include <TTimeStamp.h>

class GLG4PrimaryGeneratorMessenger;
class G4Event;
class G4Track;

class GLG4PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
  GLG4PrimaryGeneratorAction(); 
  ~GLG4PrimaryGeneratorAction();

  void AddGenerator(GLG4Gen *gen);
  void ClearGenerators(void);
  void GeneratePrimaries(G4Event * argEvent); // generate primary particles

  void SetRunUTC(const TTimeStamp &_utc) { runUTC = _utc; };
  TTimeStamp GetRunUTC() const { return runUTC; };

  double GetUniversalTime() { return myUniversalTime; }
  
  double GetUniversalTimeSincePriorEvent()
  { return myUniversalTimeSincePriorEvent; }
    
  double GetEventWindow() { return myEventWindow; }
  void SetEventWindow(double argEventWindow);
    
  static GLG4PrimaryGeneratorAction* GetTheGLG4PrimaryGeneratorAction()
   { return theGLG4PrimaryGeneratorAction; }

  void DeferTrackToLaterEvent(const G4Track * track);

protected:
  GLG4PrimaryGeneratorMessenger *myMessenger;
  
  TTimeStamp runUTC;
  double myUniversalTime;
  double myUniversalTimeSincePriorEvent;
  double myEventWindow;

  bool needReset;
  GLG4GenList myGenList;

  static GLG4PrimaryGeneratorAction* theGLG4PrimaryGeneratorAction;
};

#endif
