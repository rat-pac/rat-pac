#ifndef __RAT_PMTCoverageFactory__
#define __RAT_PMTCoverageFactory__

#include <RAT/PMTFactoryBase.hh>

namespace RAT {
 class PMTCoverageFactory : public PMTFactoryBase {
 public:
   PMTCoverageFactory() : PMTFactoryBase("pmtcoverage") {};
   virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
 };
} // namespace RAT

#endif
