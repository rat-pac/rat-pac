// Created by Daniel Hellfeld (07/2015)
// Directly follows from ESgenMessenger.hh

#ifndef RAT_ReactorESgenMessenger_hh
#define RAT_ReactorESgenMessenger_hh

#include "G4UImessenger.hh"
#include "G4String.hh"

// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;

namespace RAT {

  // Foward declarations in namespace
  class ReactorESgen;

  class ReactorESgenMessenger: public G4UImessenger{
	  
  public:
    ReactorESgenMessenger(ReactorESgen*);
    ~ReactorESgenMessenger();
    
    void SetNewValue(G4UIcommand* command, G4String newValues);
	G4String GetCurrentValue(G4UIcommand* command);
      
  private:
    ReactorESgen* reactoresgen;
    
    G4UIcmdWithADouble* ffU235Cmd;
    G4UIcmdWithADouble* ffU238Cmd;
    G4UIcmdWithADouble* ffPu239Cmd;
    G4UIcmdWithADouble* ffPu241Cmd;
	G4UIcmdWithADouble* ReactorPowerCmd;
	G4UIcmdWithADouble* EperFissCmd;
	G4UIcmdWithADouble* StandoffCmd;
	G4UIcmdWithADouble* TimeCmd;
	G4UIcmdWithADouble* WvolumeCmd;

  };

} // namespace RAT

#endif // RAT_ReactorESgenMessenger_hh
