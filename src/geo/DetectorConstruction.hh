/**
 * @class DetectorConstruction
 * @brief Load detector geometry and materials.
 */

#ifndef __RAT_DetectorConstruction__
#define __RAT_DetectorConstruction__

#include <G4VUserDetectorConstruction.hh>

class G4VPhysicalVolume;

namespace RAT {

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction() {}
  virtual ~DetectorConstruction() {}

  // Make volumes, return pointer to world
  virtual G4VPhysicalVolume* Construct();

  // Called by Construct, provided for convenience
  void ConstructMaterials();

  G4VPhysicalVolume* GetWorld() const { return fWorldPhys; }

  static DetectorConstruction *GetDetectorConstruction();

protected:
  static DetectorConstruction* sDetectorConstruction;
  G4VPhysicalVolume* fWorldPhys;

  void SetupGDMLSD();
};

}  // namespace RAT

#endif  // __RAT_DetectorConstruction__

