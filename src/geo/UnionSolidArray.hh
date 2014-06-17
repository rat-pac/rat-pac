#ifndef __RAT_UnionSolidArray__
#define __RAT_UnionSolidArray__

#include <G4VSolid.hh>
#include <string>
#include <vector>

namespace RAT {
   
   // Returns a union solid constructed out of many solids.
   //
   // To support recursion, optional arguments are included to select a subrange
   // of the translations/rotations array.  Normally you can ignore those two arguments.
   G4VSolid *MakeUnionSolidArray(const std::string &name, 
                                 const std::vector<G4VSolid*> &solids,
                                 int firstIndex=0, int length=-1);
   
} // namespace RAT

#endif
