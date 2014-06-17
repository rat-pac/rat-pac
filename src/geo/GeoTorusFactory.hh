#ifndef __RAT_GeoTorusFactory__
#define __RAT_GeoTorusFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoTorusFactory : public GeoSolidFactory {
 public:
   GeoTorusFactory() : GeoSolidFactory("torus") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
