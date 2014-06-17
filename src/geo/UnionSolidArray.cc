#include <UnionSolidArray.hh>

#include <G4DisplacedSolid.hh>
#include <G4UnionSolid.hh>
#include "RAT/Log.hh"


namespace RAT {

G4VSolid *MakeUnionSolidArray(const std::string &name,
                              const std::vector<G4VSolid*> &solids,
                              int firstIndex, int length)
{      
   if (length == -1)
      length = solids.size();
   
   if (length == 0)
      return 0;
   else if (length == 1) {
      return solids[firstIndex];
   } else {
      // Build a union tree recursively
      int splitLength = length/2;
      G4VSolid *left  = MakeUnionSolidArray(name+"L", solids,
                                            firstIndex, splitLength);
      G4VSolid *right = MakeUnionSolidArray(name+"R", solids,
                                            // pick up remainder on right side
                                            firstIndex+splitLength, length-splitLength);
                                            
      return new G4UnionSolid(name, left, right);                           
   }
}

} // namespace RAT

