#ifndef __RAT_GeoRevArrayFactory__
#define __RAT_GeoRevArrayFactory__

#include <RAT/GeoSolidArrayFactoryBase.hh>

namespace RAT {
 class GeoRevArrayFactory : public GeoSolidArrayFactoryBase {
 public:
   GeoRevArrayFactory() : GeoSolidArrayFactoryBase("revarray") {};
   using GeoSolidArrayFactoryBase::Construct;
   virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
