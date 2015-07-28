#ifndef RAT_ESgenMessenger_hh
#define RAT_ESgenMessenger_hh

#include "G4UImessenger.hh"
#include "G4String.hh"

// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;

namespace RAT {

  // Foward declarations in namespace
  class ESgen_2;

  class ESgenMessenger_2: public G4UImessenger
  {
  public:
    ESgenMessenger_2(ESgen_2*);
    ~ESgenMessenger_2();
    
    void SetNewValue(G4UIcommand* command, G4String newValues);
	G4String GetCurrentValue(G4UIcommand* command);
      
  private:
    ESgen_2* esgen_2;
    
    G4UIcmdWithADouble* ffU235Cmd;
    G4UIcmdWithADouble* ffU238Cmd;
    G4UIcmdWithADouble* ffPu239Cmd;
    G4UIcmdWithADouble* ffPu241Cmd;
      
      
  };

} // namespace RAT

#endif // RAT_ESgenMessenger_hh
