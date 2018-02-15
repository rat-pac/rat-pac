#ifndef __RAT_PMTFactoryBase__
#define __RAT_PMTFactoryBase__

#include <RAT/GeoFactory.hh>
#include <RAT/DS/PMTInfo.hh>

namespace RAT {
  class PMTFactoryBase : public GeoFactory {
  public:
    PMTFactoryBase(const std::string &name) : GeoFactory(name) { };
    static const DS::PMTInfo& GetPMTInfo() { return pmtinfo; }
  protected:
  
    virtual G4VPhysicalVolume *ConstructPMTs(DBLinkPtr table, 
        const std::vector<G4ThreeVector> &pmt_pos, 
        const std::vector<G4ThreeVector> &pmt_dir, 
        const std::vector<int> &pmt_type, 
        const std::vector<double> &pmt_effi_corr);
    
    static DS::PMTInfo pmtinfo; ///keeps track of all the PMTs built into the geometry
  };
  
} // namespace RAT

#endif


