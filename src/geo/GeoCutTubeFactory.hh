#ifndef __RAT_GeoCutTubeFactory__
#define __RAT_GeoCutTubeFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoCutTubeFactory : public GeoSolidFactory {
 public:
   GeoCutTubeFactory() : GeoSolidFactory("cut_tube") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
