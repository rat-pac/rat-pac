// This file is part of the GenericLAND software library.
// $Id: GLG4VEventAction.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
//
//  GenericLAND Simulation
//
//  GLG4 version by Glenn Horton-Smith December, 2004.
//  Based on earlier work by H. Ikeda, O. Tajima and G. Horton-Smith
//

#ifndef GLG4VEventAction_h
#define GLG4VEventAction_h 1

#include "globals.hh"
#include "G4UserEventAction.hh"
#include "G4UImessenger.hh"

#include "fstream"
//#include "GLG4HitPhotonCollection.hh"
#include "GLG4HitPMTCollection.hh"

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

class G4UIcmdWithAString;
class G4UIcmdWithAnInteger;

class GLG4VEventAction :
  public G4UserEventAction,
  public G4UImessenger
{
public:
  // constructor, destructor
  GLG4VEventAction();
  ~GLG4VEventAction();

  // overrides for G4UserEventAction methods
  virtual void BeginOfEventAction(const G4Event*);
  virtual void EndOfEventAction(const G4Event*);

  // overrides for G4UImessenger methods
  virtual void SetNewValue(G4UIcommand * command, G4String newValue);
  G4String GetCurrentValue(G4UIcommand * command);

  //  static GLG4HitPhotonCollection*  GetTheHitPhotons() { return &theHitPhotons; }
  static GLG4HitPMTCollection*  GetTheHitPMTCollection() { return &theHitPMTCollection; }
  static G4bool GetDoParameterizedScintillation() { return fgDoParameterizedScintillation; }

protected:
  //  static GLG4HitPhotonCollection theHitPhotons;
  static GLG4HitPMTCollection theHitPMTCollection;
  
  static G4bool flagFullOutputMode;
  G4String drawFlag;

public:
  virtual void OpenFile(const G4String filename,G4bool outputMode) = 0;
  virtual void CloseFile() = 0;
  virtual void FillData(const G4Event*) = 0;
  virtual void Clear() = 0;
  
protected:
  G4UIcmdWithAString *fDrawCmd;
  G4UIcommand *fFileCmd;
  G4UIcmdWithAString *fModeCmd;

  static G4bool  fgDoParameterizedScintillation;
};

#endif
