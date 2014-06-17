#ifndef __RAT_GeoPMTFactoryBase__
#define __RAT_GeoPMTFactoryBase__

#include <RAT/GeoFactory.hh>

namespace RAT {
  class GeoPMTFactoryBase : public GeoFactory {
  public:
    GeoPMTFactoryBase(const std::string &name) : GeoFactory(name) { };
  protected:
    virtual G4VPhysicalVolume *ConstructPMTs(DBLinkPtr table, 
                                     std::vector<double> pmt_x, std::vector<double> pmt_y, std::vector<double> pmt_z);
  };
  
} // namespace RAT

#endif


