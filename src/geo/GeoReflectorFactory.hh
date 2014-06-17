#ifndef __RAT_GeoReflectorFactory__
#define __RAT_GeoReflectorFactory__

#include <RAT/GeoFactory.hh>

namespace RAT {
 class GeoReflectorFactory : public GeoFactory {
 public:
   GeoReflectorFactory() : GeoFactory("reflector") {};


   virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
