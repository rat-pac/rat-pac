#ifndef __RAT_GeoConvexLensFactory__
#define __RAT_GeoConvexLensFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoConvexLensFactory : public GeoSolidFactory {
 public:
   GeoConvexLensFactory() : GeoSolidFactory("convexlens") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
