#ifndef __RAT_TrackingMessenger__
#define __RAT_TrackingMessenger__

#include <G4UImessenger.hh>
#include <G4UIcmdWithABool.hh>
#include <G4UIcmdWithAString.hh>
#include <G4UIcmdWithADouble.hh>
#include <G4String.hh>

namespace RAT {

class TrackingMessenger : public G4UImessenger {
public:
  TrackingMessenger();
  ~TrackingMessenger();

  G4String GetCurrentValue(G4UIcommand * command);
  void SetNewValue(G4UIcommand * command, G4String newValue);

protected:
  G4UIcmdWithABool *FillPointContCmd;
  G4UIcmdWithAString *storeParticleTrajCmd;
  G4UIcmdWithAString *discardParticleTrajCmd;
  G4UIcmdWithABool *storeMuonTrajSpecialCmd;
  G4UIcmdWithADouble *setMaxGlobalTimeCmd;
  G4UIcmdWithABool *storeOpticalTrackIDCmd;
};


} // namespace RAT

#endif
