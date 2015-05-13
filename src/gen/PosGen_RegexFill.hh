// Position generator to fill all volumes with names that match a regular 
// expression. Excludes daughters unless they also match. Positions uniformly
// distributed by volume among matched volumes.

#ifndef __RAT_PosGen_RegexFill__
#define __RAT_PosGen_RegexFill__

#include <RAT/GLG4PosGen.hh>

#include <G4VPhysicalVolume.hh>
#include <G4VSolid.hh>
#include <G4LogicalVolume.hh>
#include <G4AffineTransform.hh>
#include <G4ThreeVector.hh>

#include <vector>
#include <utility>
#include <map>

//C POSIX Regex library, switch to <regex> in c++11
#include <regex.h> 

class G4VPhysicalVolume;
class G4VSolid;

namespace RAT {

typedef struct {
    // Logical volume representing found volume
    G4LogicalVolume *log;
    
    // Physical volume that was found
    G4VPhysicalVolume *phys;
    
    // Transformation from volume coordinates to global coordinates
    G4AffineTransform transform;
    
    // Volume of solid and bounding box
    double solidVolume, boundVolume;
    
    // Bounding box in volume coodinates
    double x0,x1,y0,y1,z0,z1;
    
    // Pairs of Solids (in daughter coords) and transformations from volume to daughter coords
    std::vector<std::pair<G4VSolid*,G4AffineTransform> > daughters;
} FillVolume;

class PosGen_RegexFill : public GLG4PosGen {
    public:
    
        // Position generator to be called `regexfill`
        PosGen_RegexFill(const char* dbname = "regexfill"): GLG4PosGen(dbname) { }
        virtual ~PosGen_RegexFill() { }
        
        // Generates a random position in a volume that matched a regex
        // Positions are uniformly distributed (by volume) across all matches
        // Positions exclude daughters of mathched volumes unless the daughter also matches
        virtual void GeneratePosition(G4ThreeVector& pos);
        
        // Adds all volumes that match the regex to the fill list
        virtual void SetState(G4String regex);
        
        // Returns a space separated list of regexes
        virtual G4String GetState() const;
        
    protected:
        
        // Recursively traverses daughters of mother to find any volume names mathching the regex
        // Adds found volumes to the `found` parameter and calculates all necessary fields of FillVolume
        static void FindVolumes(G4LogicalVolume *mother, regex_t *re, std::vector<FillVolume> &found);
        
        // Arbitrary volume calculations are terribly slow, so cache solids that
        // have been calculated already. G4VSolid lifetime is the same as the 
        // simulation so presumably this is safe.
        static double GetVolume(G4VSolid *solid);
        
        // Accumulates set regexes
        std::string fState;
        
        // Volumes that matched the regexes
        std::vector<FillVolume> fVolumes;
        
        // Stores the sum of the volume up to the requested index
        std::vector<double> fVolumeCumu;
        
        // Calculated volume cache
        static std::map<G4VSolid*,double> fSolidVolumes;
};

} // namespace RAT

#endif
