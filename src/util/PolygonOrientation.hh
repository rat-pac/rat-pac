#ifndef __RAT_PolygonOrientation__
#define __RAT_PolygonOrientation__

#include <vector>
#include <G4TwoVector.hh>
#include <assert.h>

namespace RAT {

  /** Takes the coordinates of a polygon (in 2-D vector format) and checks
      the orientation. G4 seems to want polygons to be oriented in the
      negative z direction in a right handed sense. The function will
      reorder the given coordinates if necessary
   */

    int CheckOrientation(std::vector<G4TwoVector> &G4Polygon);
 


} // namespace RAT



#endif
