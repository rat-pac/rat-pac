// This file is part of the GenericLAND software library.
// $Id: GLG4PrimaryGeneratorMessenger.hh,v 1.2 2005/11/19 17:59:39 volsung Exp $
//
// GLG4PrimaryGeneratorMessenger.hh by Glenn Horton-Smith, Feb. 1999
// updated Aug. 3-17, 2001, for new GLG4PrimaryGeneratorAction

#ifndef __GLG4PrimaryGeneratorMessenger_hh__
#define __GLG4PrimaryGeneratorMessenger_hh__ 1

#include "G4UImessenger.hh"

class GLG4PrimaryGeneratorAction;
class G4UIcommand;
class GLG4Gen;

class GLG4PrimaryGeneratorMessenger: public G4UImessenger
{
  public:
    GLG4PrimaryGeneratorMessenger(GLG4PrimaryGeneratorAction* myGun);
    ~GLG4PrimaryGeneratorMessenger();
    
    void SetNewValue(G4UIcommand * command,G4String newValues);
    G4String GetCurrentValue(G4UIcommand * command);
    
  private:
    GLG4PrimaryGeneratorAction* myAction;
 
    G4UIcommand*       ListCmd;
    G4UIcommand*       GunCmd;
    G4UIcommand*       GenAddCmd;
    G4UIcommand*       RateSetCmd;
    G4UIcommand*       VtxSetCmd;
    G4UIcommand*       PosSetCmd;
    G4UIcommand*       EventWindowCmd;
    G4UIcommand*       GenClearCmd;

  GLG4Gen *lastGen;
};

#endif
