#ifndef __RAT_GeoPMTFactoryBase__
#define __RAT_GeoPMTFactoryBase__

#include <RAT/GeoFactory.hh>
#include <RAT/DS/PMTInfo.hh>

namespace RAT {
  class GeoPMTFactoryBase : public GeoFactory {
  public:
    GeoPMTFactoryBase(const std::string &name) : GeoFactory(name) { };
    static const DS::PMTInfo& GetPMTInfo() { return pmtinfo; }
  protected:
    virtual G4VPhysicalVolume *ConstructPMTs(DBLinkPtr table, std::vector<double> pmt_x, std::vector<double> pmt_y, std::vector<double> pmt_z);
    
    static DS::PMTInfo pmtinfo; ///keeps track of all the PMTs built into the geometry
  };
  
} // namespace RAT

#endif


