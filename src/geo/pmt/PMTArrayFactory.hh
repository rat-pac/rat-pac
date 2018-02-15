#ifndef __RAT_PMTArrayFactory__
#define __RAT_PMTArrayFactory__

#include <RAT/PMTFactoryBase.hh>

namespace RAT {
 class PMTArrayFactory : public PMTFactoryBase {
 public:
   PMTArrayFactory() : PMTFactoryBase("pmtarray") {};
   virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
