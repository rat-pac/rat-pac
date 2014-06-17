/// \class RAT::GeoRevolutionChimneyFactory
///
/// \brief  Define the geometry of the a revolution solid with a chimney in the middle
///
/// \author name P Gorel <pgorel@ualberta.ca>
///
/// REVISION HISTORY:\n
///   25/03/2011 : P Gorel - New file \n
///
/// \detail Implement the geometry of the a revolution solid with a chimney in the middle
///
////////////////////////////////////////////////////////////////////////

#ifndef __RAT_GeoRevolutionChimneyFactory__
#define __RAT_GeoRevolutionChimneyFactory__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
 class GeoRevolutionChimneyFactory : public GeoSolidFactory {
 public:
   GeoRevolutionChimneyFactory() : GeoSolidFactory("revolveChimney") {};
   virtual G4VSolid *ConstructSolid(DBLinkPtr table);
 };
  
} // namespace RAT

#endif
