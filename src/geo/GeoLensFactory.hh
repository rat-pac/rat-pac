#ifndef __RAT_GeoLensFactory__
#define __RAT_GeoLensFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoLensFactory : public GeoSolidFactory {
 public:
   GeoLensFactory() : GeoSolidFactory("lens") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
