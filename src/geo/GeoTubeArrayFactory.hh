#ifndef __RAT_GeoTubeArrayFactory__
#define __RAT_GeoTubeArrayFactory__

#include <RAT/GeoSolidArrayFactoryBase.hh>

namespace RAT {
 class GeoTubeArrayFactory : public GeoSolidArrayFactoryBase {
 public:
   GeoTubeArrayFactory() : GeoSolidArrayFactoryBase("tubearray") {};
   using GeoSolidArrayFactoryBase::Construct;
   virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
