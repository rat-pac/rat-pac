#ifndef __RAT_GeoSolidFactory__
#define __RAT_GeoSolidFactory__

#include <RAT/GeoFactory.hh>

namespace RAT {

  class GeoSolidFactory : public GeoFactory {
  public:
    GeoSolidFactory(const std::string &name) : GeoFactory(name) { };
    virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
    virtual G4LogicalVolume *ConstructLogicalVolume(G4VSolid *solid, DBLinkPtr table);
    virtual G4VSolid *ConstructSolid(DBLinkPtr table) = 0;
  };

} // namespace RAT
#endif
