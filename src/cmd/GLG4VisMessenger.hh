// This file is part of the GenericLAND software library.
// $Id: GLG4VisMessenger.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
// GenericLAND visualization manager messenger
//   -- main purpose of defining our own is to reorient "up" vector
//
// Author:  Glenn Horton-Smith, Jan 28, 2000
//
#ifdef G4VIS_USE
#ifndef GLG4VISMESSENGER_HH
#define GLG4VISMESSENGER_HH

#include "G4UImessenger.hh"

class GLG4VisManager;

class GLG4VisMessenger: public G4UImessenger
{
  public:
    GLG4VisMessenger(GLG4VisManager* pVMan_);
    ~GLG4VisMessenger();
    
    void SetNewValue(G4UIcommand * command,G4String newValues);
    G4String GetCurrentValue(G4UIcommand * command);

  protected:
    GLG4VisManager* pVMan;
};

#endif
#endif
