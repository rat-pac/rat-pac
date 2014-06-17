/** @class PosGen_Multipoint
 *  Draws event vertices from a finite list of points in sequence.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 * Takes a list of event points, either from a RATDB table or
 * generated from some parameter list, and returns a point from that
 * list in order, on each call.  When the list is exhausted, it loops
 * back to the beginning and goes through the list again (and again).
 *
 */

#ifndef __RAT_PosGen_Multipoint__
#define __RAT_PosGen_Multipoint__

#include <RAT/GLG4PosGen.hh>
#include <vector>
#include <string>

namespace RAT {


class PosGen_Multipoint : public GLG4PosGen {
public:
  /** Create a new position generator.
   *
   *  The default is an empty list, triggering an error if you don't configure
   *  this generator with SetState before calling GeneratePosition.
   */
  PosGen_Multipoint(const char *arg_dbname="multipoint");

  /** Returns the next position in the list of points. */
  virtual void GeneratePosition( G4ThreeVector &argResult );

  /** Sets the point list.
   *
   *  There are two options:
   *  /generator/pos/set uniform [# of points] [inner radius] [outer radius]
   *    and
   *  /generator/pos/set table   [table name]
   *
   *  The first version generates a list of points uniformly
   *  distributed in volume between the inner radius and outer radius,
   *  also uniform in theta and phi.  The method used to generate this
   *  list is non-random so you will get the same list on every job.
   *  Moreover, the sequence is fixed so that jobs with a greater
   *  number of points will start with the same sequency of points as
   *  a job with fewer points.
   *
   *  The second version reads the point list from the named table.  You can
   *  include a table index using the bracket notation.  For example: 
   *     MULTIPOINT[led]
   *
   */
  void SetState( G4String newValues );

  /** Return state as string in same format as SetState() */
  G4String GetState() const;

protected:
  void LoadUniformPoints(unsigned numPoints, 
			 double innerRadius, double outerRadius);
  void LoadTablePoints(std::string tableName, std::string tableIndex);

  enum { MULTIPOINT_TABLE, MULTIPOINT_UNIFORM } MultipointTypeList;
  int fType; /** Type of multipoint generator.  Value must be from above enum. */

  std::string fTableName;  /** Only set if using table variant of this generator */
  std::string fTableIndex;

  double fInnerRadius, fOuterRadius;

  std::vector<G4ThreeVector> fPos;
  int fNextPosIndex;
};


} // namespace RAT

#endif
