#ifndef __RAT_GeoSolidArrayFactoryBase__
#define __RAT_GeoSolidArrayFactoryBase__

#include <RAT/GeoFactory.hh>

namespace RAT {
    class GeoSolidArrayFactoryBase : public GeoFactory {
    public:
	GeoSolidArrayFactoryBase(const std::string &name) : GeoFactory(name) { };
	using GeoFactory::Construct;
    protected:
	virtual G4VPhysicalVolume *Construct(G4VSolid *BaseSolid, DBLinkPtr table);
	virtual G4VPhysicalVolume *Construct(G4LogicalVolume *logiSolid, DBLinkPtr table);
    };
    
} // namespace RAT

#endif
