// This file is part of the GenericLAND software library.
// $Id: ChromaInterfaceMessenger.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
// ChromaInterfaceMessenger.hh by Glenn Horton-Smith, Dec. 1999
#ifndef __ChromaInterfaceMessenger_hh__
#define __ChromaInterfaceMessenger_hh__ 1

#include "G4UImessenger.hh"

class G4UIcommand;
namespace RAT {

  class ChromaInterface;

  class ChromaInterfaceMessenger: public G4UImessenger
  {
  public:
    ChromaInterfaceMessenger( ChromaInterface * chroma);
    ~ChromaInterfaceMessenger();
    
    void SetNewValue(G4UIcommand * command,G4String newValues);
    G4String GetCurrentValue(G4UIcommand * command);
    
  private:
    ChromaInterface* fChroma;
    G4UIcommand*  ActivateCmd;
    G4UIcommand*  ReactivateCmd;
    G4UIcommand*  CheckConnectionCmd;
    G4UIcommand*  DeactivateCmd;
    
  };
}

#endif
