/** @class PosGen_Line
 *  Generate event vertices at random points along a line.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 * This position generator returns random points distributed uniformly
 * along a line segment.  The line is defined by supplying the x,y,z
 * coordinates of the endpoints
 *
 */

#ifndef __RAT_PosGen_Line__
#define __RAT_PosGen_Line__

#include <RAT/GLG4PosGen.hh>

namespace RAT {


class PosGen_Line : public GLG4PosGen {
public:
  /** Create a new position generator.
   *
   *  The default line segment is from (0,0,0) to (0,0,0), i.e. a point 
   *  at the origin.
   */
  PosGen_Line(const char *arg_dbname="line");

  /** Generate a random position on the line segment */
  virtual void GeneratePosition( G4ThreeVector &argResult );

  /** Set the coordinates of the line endpoints
   *
   *  @param[in] newValues   x1 y1 z1 x2 y2 z2 (in mm)
   */
  void SetState( G4String newValues );

  /** Return state as string in same format as SetState() */
  G4String GetState() const;

protected:
  G4ThreeVector  fPoint1; /**< Endpoint 1 of line */
  G4ThreeVector  fPoint2; /**< Endpoint 2 of line */
};


} // namespace RAT

#endif
