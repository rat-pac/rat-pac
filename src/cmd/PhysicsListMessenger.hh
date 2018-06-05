/**
 * @class RAT::PhysicsListMessenger
 *
 * Commands for modifying the physics list.
 *
 * @author L. Bignell, BNL
 */
#ifndef __RAT_PhysicsListMessenger__
#define __RAT_PhysicsListMessenger__

#include <G4UImessenger.hh>

class G4UIcmdWithAString;

namespace RAT {

class PhysicsList;

class PhysicsListMessenger : public G4UImessenger {
public:
  PhysicsListMessenger(PhysicsList* physicsList);
  virtual ~PhysicsListMessenger();
  
  G4String GetCurrentValue(G4UIcommand* command);
  void SetNewValue(G4UIcommand* command, G4String newValue);

  PhysicsList* fPhysicsList;

protected:
  G4UIcmdWithAString* fSetOpWLSCmd;
};
  
}  // namespace RAT

#endif  // __RAT_PhysicsListMessenger__

