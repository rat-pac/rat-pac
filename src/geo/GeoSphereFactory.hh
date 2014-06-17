#ifndef __RAT_GeoSphereFactory__
#define __RAT_GeoSphereFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoSphereFactory : public GeoSolidFactory {
 public:
   GeoSphereFactory() : GeoSolidFactory("sphere") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
