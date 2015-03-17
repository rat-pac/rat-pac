#ifndef __RAT_PhysicsListMessenger__
#define __RAT_PhysicsListMessenger__

#include <G4UImessenger.hh>
#include <G4UIcmdWithABool.hh>

class GLG4PhysicsList;

namespace RAT {

  class PhysicsListMessenger : public G4UImessenger {
  public:
    PhysicsListMessenger(GLG4PhysicsList*);
    ~PhysicsListMessenger();
    
    G4String GetCurrentValue(G4UIcommand * command);
    void SetNewValue(G4UIcommand * command, G4String newValue);

    GLG4PhysicsList* pPhysList;  
  
  protected:
    G4UIcmdWithABool *SetAltOpWLSCmd;
  };
  
  
} // namespace RAT

#endif
