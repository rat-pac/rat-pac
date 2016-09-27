// RAT::SNgenMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the SNgen parameters via the command line.

#ifndef RAT_SNgenMessenger_hh
#define RAT_SNgenMessenger_hh

#include "G4UImessenger.hh"
#include "G4String.hh"

// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;

namespace RAT {

  // Foward declarations in namespace
  class SNgen;

  class SNgenMessenger: public G4UImessenger
  {
  public:
    SNgenMessenger(SNgen*);
    ~SNgenMessenger();
    
    void SetNewValue(G4UIcommand* command, G4String newValues);
    G4String GetCurrentValue(G4UIcommand* command);
    
  private:
    SNgen* sngen;
    
    G4UIcmdWithADouble* wmaCmd;
    G4UIcmdWithADouble* vmuCmd;
      
      G4UIcmdWithADouble* IBDAmpCmd;
      G4UIcmdWithADouble* ESAmpCmd;
      G4UIcmdWithADouble* CCAmpCmd;
      G4UIcmdWithADouble* ICCAmpCmd;
      G4UIcmdWithADouble* NCAmpCmd;
      G4UIcmdWithADouble* ModelCmd;
      
  };

} // namespace RAT

#endif // RAT_SNgenMessenger_hh
