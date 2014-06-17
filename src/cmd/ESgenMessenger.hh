// RAT::ESgenMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the ESgen parameters via the command line.

#ifndef RAT_ESgenMessenger_hh
#define RAT_ESgenMessenger_hh

#include "G4UImessenger.hh"
#include "G4String.hh"

// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;

namespace RAT {

  // Foward declarations in namespace
  class ESgen;

  class ESgenMessenger: public G4UImessenger
  {
  public:
    ESgenMessenger(ESgen*);
    ~ESgenMessenger();
    
    void SetNewValue(G4UIcommand* command, G4String newValues);
    G4String GetCurrentValue(G4UIcommand* command);
    
  private:
    ESgen* esgen;
    
    G4UIcmdWithADouble* wmaCmd;
    G4UIcmdWithADouble* vmuCmd;
  };

} // namespace RAT

#endif // RAT_ESgenMessenger_hh
