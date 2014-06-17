/** @class PosGen_Radial 
 *  Generate event vertices at random points along a line.
 *
 * This position generator returns random points distributed uniformly
 * in radius and direction around the sphere's center.  The sphere is defined by supplying the radius and the
 * coordinates of the center
 *
 */

#ifndef __RAT_PosGen_Radial__
#define __RAT_PosGen_Radial__

#include <RAT/GLG4PosGen.hh>

namespace RAT {


class PosGen_Radial : public GLG4PosGen {
public:
  /** Create a uniform spherical position generator.
  The default sphere is from (0,0,0) the origin.
   */
  PosGen_Radial(const char *arg_dbname="radial");

  /** Generate a random position in the sphere, uniformly ditributed in radial coordinate*/
  virtual void GeneratePosition( G4ThreeVector &argResult );

  /** Set the center and max radius of the distribution
   *
   *  @param[in] newValues   x y z R (in mm)
   */
  void SetState( G4String newValues );

  /** Return state as string in same format as SetState() */
  G4String GetState() const;

protected:
  G4ThreeVector  fCenter; /*distribution center */
  G4double  fMaxRadius; /**distribution radius*/
  G4ThreeVector fPoint;//generated point
};


} // namespace RAT

#endif
