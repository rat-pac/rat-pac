#ifndef __RAT_CubicPMTConstruction__
#define __RAT_CubicPMTConstruction__

#include <string>
#include <vector>
#include <RAT/DB.hh>
#include <G4Material.hh>
#include <RAT/GLG4TorusStack.hh>
#include <G4OpticalSurface.hh>
#include <G4VSensitiveDetector.hh>
#include <G4VSolid.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <RAT/WaveguideFactory.hh>
#include <RAT/Factory.hh>
#include <RAT/PMTConstruction.hh>

namespace RAT {
  

struct CubicPMTConstructionParams {
  CubicPMTConstructionParams () { 
    efficiencyCorrection = 1.0;
    invisible = false;
  };
  
  bool invisible;
  
  // Body
  double caseThickness; // mm
  double glassThickness; // mm
  double pmtWidth; // mm
  double photocathodeWidth; // mm
  
  G4Material *outerCase;
  G4Material *glass;
  G4Material *vacuum;
  
  G4OpticalSurface *photocathode;
  
  double efficiencyCorrection; // default to 1.0 for no correction

};

class CubicPMTConstruction : public PMTConstruction {
public:
  CubicPMTConstruction(DBLinkPtr params, G4LogicalVolume *mother);
  virtual ~CubicPMTConstruction() { }
  
  virtual G4LogicalVolume *BuildVolume(const std::string &prefix);
  virtual G4VSolid *BuildSolid(const std::string &prefix);
  virtual G4PVPlacement* PlacePMT(G4RotationMatrix *pmtrot, 
				  G4ThreeVector pmtpos, 
				  const std::string &name, 
				  G4LogicalVolume *logi_pmt, 
				  G4VPhysicalVolume *mother_phys, 
				  bool booleanSolid, int copyNo);
  
protected:
  
  // physical volumes
  G4PVPlacement* glass_phys;
  G4PVPlacement* vacuum_phys;
  
  G4LogicalVolume *log_pmt;
  CubicPMTConstructionParams fParams;

};

} // namespace RAT

#endif
