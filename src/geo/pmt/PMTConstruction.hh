#ifndef __RAT_PMTConstruction__
#define __RAT_PMTConstruction__

#include <string>
#include <RAT/DB.hh>
#include <G4VSolid.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4RotationMatrix.hh>

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
    virtual G4PVPlacement* PlacePMT(
            G4RotationMatrix *pmtrot, 
            G4ThreeVector pmtpos, 
            const std::string &name, 
            G4LogicalVolume *logi_pmt, 
            G4VPhysicalVolume *mother_phys, 
            bool booleanSolid, int copyNo) = 0;
            
protected:
    
    std::string name;
};

} // namespace RAT

#endif

