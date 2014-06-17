#ifndef __RAT_GeoPerfSphereFactory__
#define __RAT_GeoPerfSphereFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoPerfSphereFactory : public GeoSolidFactory {
 public:
   GeoPerfSphereFactory() : GeoSolidFactory("psphere") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
