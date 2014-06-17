#ifndef __RAT_GeoRevolutionFactory__
#define __RAT_GeoRevolutionFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoRevolutionFactory : public GeoSolidFactory {
 public:
   GeoRevolutionFactory() : GeoSolidFactory("revolve") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
