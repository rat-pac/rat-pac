#ifndef __RAT_GeoReflectorWaveguideFactory__
#define __RAT_GeoReflectorWaveguideFactory__

#include <RAT/GeoFactory.hh>

namespace RAT {
 class GeoReflectorWaveguideFactory : public GeoFactory {
 public:
   GeoReflectorWaveguideFactory() : GeoFactory("reflector_waveguide") {};


   virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
