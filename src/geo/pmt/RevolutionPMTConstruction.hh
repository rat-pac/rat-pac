#ifndef __RAT_RevolutionPMTConstruction__
#define __RAT_RevolutionPMTConstruction__

#include <string>
#include <vector>
#include <RAT/DB.hh>
#include <RAT/PMTConstruction.hh>
#include <G4VSolid.hh>
#include <G4Material.hh>
#include <G4PVPlacement.hh>
#include <G4LogicalVolume.hh>
#include <G4OpticalSurface.hh>
#include <G4VSensitiveDetector.hh>

namespace RAT {


struct RevolutionPMTConstructionParams {
    RevolutionPMTConstructionParams () { 
        invisible = false;
        efficiencyCorrection = 1.0; // default to 1.0 for no correction
    };
    
    bool invisible;
    
    double dynodeRadius; // mm
    double dynodeTop; // mm
    double dynodeHeight; // mm

    std::vector<double> rInner, zInner, rEdge, zEdge;

    G4Material *exterior;
    G4Material *glass;
    G4Material *vacuum;
    G4Material *dynode;

    G4OpticalSurface *photocathode;
    G4OpticalSurface *mirror;
    G4OpticalSurface *dynode_surface;

    double efficiencyCorrection;
};

// Construction for PMTs based on G4Polycon
class RevolutionPMTConstruction : public PMTConstruction {
public:
    RevolutionPMTConstruction(DBLinkPtr params, G4LogicalVolume *mother);
    virtual ~RevolutionPMTConstruction() { }
    
    virtual G4LogicalVolume *BuildVolume(const std::string &prefix);
    virtual G4VSolid *BuildSolid(const std::string &prefix);
    virtual G4PVPlacement* PlacePMT(
            G4RotationMatrix *pmtrot, 
            G4ThreeVector pmtpos, 
            const std::string &name, 
            G4LogicalVolume *logi_pmt, 
            G4VPhysicalVolume *mother_phys, 
            bool booleanSolid, int copyNo);
    
protected:
                       
    G4LogicalVolume* body_log;
    
    G4PVPlacement* inner1_phys;
    G4PVPlacement* inner2_phys;
    G4PVPlacement* central_gap_phys; 
    G4PVPlacement* dynode_phys;
    
    RevolutionPMTConstructionParams fParams;
};

}

#endif
