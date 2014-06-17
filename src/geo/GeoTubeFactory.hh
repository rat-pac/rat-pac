#ifndef __RAT_GeoTubeFactory__
#define __RAT_GeoTubeFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoTubeFactory : public GeoSolidFactory {
 public:
   GeoTubeFactory() : GeoSolidFactory("tube") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
