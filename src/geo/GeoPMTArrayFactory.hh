#ifndef __RAT_GeoPMTArrayFactory__
#define __RAT_GeoPMTArrayFactory__

#include <RAT/GeoPMTFactoryBase.hh>

namespace RAT {
 class GeoPMTArrayFactory : public GeoPMTFactoryBase {
 public:
   GeoPMTArrayFactory() : GeoPMTFactoryBase("pmtarray") {};
   virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
