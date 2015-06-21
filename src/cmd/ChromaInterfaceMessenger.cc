// This file is part of the GenericLAND software library.
// $Id: ChromaInterfaceMessenger.cc,v 1.2 2005/09/15 21:57:04 sekula Exp $
//
// ChromaInterfaceMessenger.cc by Glenn Horton-Smith, Dec 1999
////////////////////////////////////////////////////////////////
// ChromaInterfaceMessenger
////////////////////////////////////////////////////////////////

#include "RAT/ChromaInterfaceMessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithoutParameter.hh"
#include "RAT/ChromaInterface.hh"

#include <cstdlib>   // for strtol
#include <fstream>    // for file streams
#include <sstream>    // for string streams
#include <iomanip>    // for setw(), etc..

using namespace std;

// GEANT4.9.6 and later
#define G4std std

namespace RAT {

  ChromaInterfaceMessenger::ChromaInterfaceMessenger( ChromaInterface* chroma)
    : fChroma(chroma)
  {
    // the glg4debug directory
    G4UIdirectory* DebugDir = new G4UIdirectory("/chroma/");
#ifdef _HAS_ZMQ
    DebugDir->SetGuidance("Chroma Interface (Available).");
#else
    DebugDir->SetGuidance("Chroma Interface (Unavailable).");
#endif
        
    // activate command
    ActivateCmd = new G4UIcmdWithoutParameter("/chroma/activate",this);
    ActivateCmd->SetGuidance("Optical Photon propagation performed by Chroma. Opens socket connection to Chroma Server.");
    
    // deactivate command
    DeactivateCmd = new G4UIcmdWithoutParameter("/chroma/deactivate",this);
    DeactivateCmd->SetGuidance("Returns Optical Photon propagation to GLG4Sim/Geant4. Closes socket connection to Chroma Server.");
    
    ReactivateCmd = NULL; // later
    CheckConnectionCmd = NULL; // later
  }
  
  
  ChromaInterfaceMessenger::~ChromaInterfaceMessenger()
  {
    delete ActivateCmd;
    delete DeactivateCmd;
  }

  void ChromaInterfaceMessenger::SetNewValue(G4UIcommand * command,G4String newValues)
  {
    
    if ( command == ActivateCmd ) {
      fChroma->initializeServerConnection();
    }
    else if ( command == DeactivateCmd ) {
      fChroma->closeServerConnection();
    }
    // invalid command
    else {
      G4cerr <<  "invalid ChromaInterface command\n" << G4std::flush;
    }
  }
  
  G4String ChromaInterfaceMessenger::GetCurrentValue(G4UIcommand * /*command*/)
  { 
    return G4String("invalid ChromaInterfaceMessenger \"get\" command");
  }

}//end of namespace RAT

