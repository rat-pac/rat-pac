#ifndef __RAT_GeoPolygonFactory__
#define __RAT_GeoPolygonFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoPolygonFactory : public GeoSolidFactory {
 public:
   GeoPolygonFactory() : GeoSolidFactory("polygon") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
