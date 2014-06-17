/// \class RAT::GeoSurfaceFactory
///
/// \brief  Define a logical border between two solids
///
/// \author name P Gorel <pgorel@ualberta.ca>
///
/// REVISION HISTORY:\n
///   25/03/2011 : P Gorel - New file \n
///
/// \detail Implement the possibility to have a logical border between two solids
///         Created in order to have TPB only between two defined solids, and not as a skin
///
////////////////////////////////////////////////////////////////////////

#ifndef __RAT_GeoSurfaceFactory__
#define __RAT_GeoSurfaceFactory__

#include <RAT/GeoFactory.hh>

namespace RAT {

class GeoSurfaceFactory : public GeoFactory {
public:
  GeoSurfaceFactory() : GeoFactory("border") { };
  virtual G4VPhysicalVolume *Construct(DBLinkPtr table);
};

} // namespace RAT
#endif
