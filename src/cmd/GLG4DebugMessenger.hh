// This file is part of the GenericLAND software library.
// $Id: GLG4DebugMessenger.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
// GLG4DebugMessenger.hh by Glenn Horton-Smith, Dec. 1999
#ifndef __GLG4DebugMessenger_hh__
#define __GLG4DebugMessenger_hh__ 1

#include "G4UImessenger.hh"
#include <RAT/DetectorConstruction.hh>

class G4UIcommand;

class GLG4DebugMessenger: public G4UImessenger
{
  public:
    GLG4DebugMessenger(RAT::DetectorConstruction * myDetector);
    ~GLG4DebugMessenger();
    
    void SetNewValue(G4UIcommand * command,G4String newValues);
    G4String GetCurrentValue(G4UIcommand * command);
    
  private:
    RAT::DetectorConstruction * myDetector;
    
    G4UIcommand*       DumpMaterialsCmd;
    G4UIcommand*       DumpGeomCmd;
    G4UIcommand*       TestSolidCmd;
};

#endif
