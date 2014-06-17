/* Creates a base revolved solid with a set of tubes intersecting the base.
   The base tube lies along the z-axis, and all intersections are 
   perpendicular to this axis.  The intersecting tube length is measured
   from the outer radius of the base tube at the center of the intersection.
   The intersecting tube can be used to create holes in the base tube
   or included to make it part of the base solid.  The intersection can 
   also be offset to make holes that partially penetrate the base solid.
 */

#ifndef __RAT_GeoTubeIntersectionFactory__
#define __RAT_GeoTubeIntersectionFactory__

#include <RAT/GeoSolidFactory.hh>
#include <RAT/DB.hh>
#include <G4VSolid.hh>

namespace RAT{
  class GeoTubeIntersectionFactory : public GeoSolidFactory{
  public:
    GeoTubeIntersectionFactory() : GeoSolidFactory("tubeinter"){};
    virtual G4VSolid* ConstructSolid(DBLinkPtr table);
  };

}

#endif
