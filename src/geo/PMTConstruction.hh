#ifndef __RAT_ToroidalPMTConstruction__
#define __RAT_ToroidalPMTConstruction__

#include <string>
#include <vector>
#include <RAT/DB.hh>
#include <RAT/GLG4TorusStack.hh>
#include <G4Material.hh>
#include <G4OpticalSurface.hh>
#include <G4VSensitiveDetector.hh>
#include <G4VSolid.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <RAT/WaveguideFactory.hh>
#include <RAT/Factory.hh>

namespace RAT {

//Superclass for building all possible PMT types
class PMTConstruction {
public:
    
    // Returns a new PMTConstruction based on the given table
    static PMTConstruction* NewConstruction(DBLinkPtr params, G4LogicalVolume *mother);
    
    PMTConstruction(std::string _name) : name(_name) { }
    
    virtual ~PMTConstruction() { }
    
    // Subclass should build a solid representing just the PMT body
    virtual G4VSolid* BuildSolid(const std::string &prefix) = 0;
    
    // Subclass should build the total single volume to be placed and return
    virtual G4LogicalVolume* BuildVolume(const std::string &prefix) = 0;
    
    // Subclass should do any extra PMT placement finalization here e.g. optical surfaces
    virtual void PlacePMT(G4RotationMatrix *pmtrot, G4ThreeVector pmtpos, const std::string &name, G4LogicalVolume *logi_pmt, G4VPhysicalVolume *mother_phys, bool booleanSolid, int copyNo) = 0;
    
protected:
    
    std::string name;
};

struct ToroidalPMTConstructionParams {
    ToroidalPMTConstructionParams () { 
        efficiencyCorrection = 1.0;
        simpleVis = false;
    };

    bool simpleVis;

    // Envelope control
    bool useEnvelope;
    double faceGap;
    double minEnvelopeRadius;

    // Body
    std::vector<double> zEdge; // n+1
    std::vector<double> rhoEdge; // n+1
    std::vector<double> zOrigin; // n
    double wallThickness; // mm

    double dynodeRadius; // mm
    double dynodeTop; // mm

    G4Material *exterior;
    G4Material *glass;
    G4Material *vacuum;
    G4Material *dynode;

    G4OpticalSurface *photocathode;
    G4OpticalSurface *mirror;
    G4OpticalSurface *dynode_surface;

    double efficiencyCorrection; // default to 1.0 for no correction
};

class ToroidalPMTConstruction : public PMTConstruction {
public:
    ToroidalPMTConstruction(DBLinkPtr params, G4LogicalVolume *mother);
    
    virtual G4LogicalVolume *BuildVolume(const std::string &prefix);
    virtual G4VSolid *BuildSolid(const std::string &prefix);
    virtual void PlacePMT(G4RotationMatrix *pmtrot, G4ThreeVector pmtpos, const std::string &name, G4LogicalVolume *logi_pmt, G4VPhysicalVolume *mother_phys, bool booleanSolid, int copyNo);
    
protected:
    G4VSolid *NewEnvelopeSolid(const std::string &name);
  
    void CalcInnerParams(GLG4TorusStack *body,
                         std::vector<double> &innerZEdge, 
                         std::vector<double> &innerRhoEdge,
                         int &equatorIndex, 
                         double &zLowestDynode); 
                          
    // phyiscal volumes 
    G4PVPlacement* body_phys;
    G4PVPlacement* inner1_phys;
    G4PVPlacement* inner2_phys;
    G4PVPlacement* central_gap_phys; 
    G4PVPlacement* dynode_phys;                  

    WaveguideFactory *fWaveguideFactory;
    
    ToroidalPMTConstructionParams fParams;
};

} // namespace RAT

#endif

