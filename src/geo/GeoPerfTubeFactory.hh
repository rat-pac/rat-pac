#ifndef __RAT_GeoPerfTubeFactory__
#define __RAT_GeoPerfTubeFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoPerfTubeFactory : public GeoSolidFactory {
 public:
   GeoPerfTubeFactory() : GeoSolidFactory("ptube") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
