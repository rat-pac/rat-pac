#ifndef __RAT_GeoWatchmanShieldFactory__
#define __RAT_GeoWatchmanShieldFactory__

#include <GeoSolidFactory.hh>

#include <G4VSolid.hh>

namespace RAT {
 class GeoWatchmanShieldFactory : public GeoSolidFactory {
 public:
   GeoWatchmanShieldFactory() : GeoSolidFactory("watchmanshield") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
