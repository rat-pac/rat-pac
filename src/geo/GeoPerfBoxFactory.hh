/* Creates a rectangular solid with a set of holes punched out.
 */

#ifndef __RAT_GeoPerfBoxFactory__
#define __RAT_GeoPerfBoxFactory__

#include <RAT/GeoSolidFactory.hh>
#include <RAT/DB.hh>
#include <G4VSolid.hh>

namespace RAT{
  class GeoPerfBoxFactory : public GeoSolidFactory{
  public:
    GeoPerfBoxFactory() : GeoSolidFactory("perfbox"){};
    virtual G4VSolid* ConstructSolid(DBLinkPtr table);
  };

}

#endif
