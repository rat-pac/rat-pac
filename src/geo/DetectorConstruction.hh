//
//  DetectorConstruction


#ifndef __RAT_DetectorConstruction__
#define __RAT_DetectorConstruction__

#include <G4ThreeVector.hh>
#include <G4VUserDetectorConstruction.hh>
#include <G4VPhysicalVolume.hh>
#include <vector>
#include <map>

using namespace std;

namespace RAT {

  class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction() {};

  virtual G4VPhysicalVolume* Construct();// make volumes, return ptr to world
  void ConstructMaterials(); // Called by Construct, provided for convenience
  G4VPhysicalVolume*  GetWorld() const { return fWorldPhys; }

  static DetectorConstruction *GetDetectorConstruction();

  bool GetPMTPosition(int pmtID, G4ThreeVector &pos);
  vector<G4ThreeVector> GetPMTPositions();

  bool GetVetoPMTPosition(int vetoPMTID, G4ThreeVector &pos);
  vector<G4ThreeVector> GetVetoPMTPositions();

protected:
  static DetectorConstruction *fDetectorConstruction;

  G4VPhysicalVolume* fWorldPhys;
};


} // namespace RAT

#endif
