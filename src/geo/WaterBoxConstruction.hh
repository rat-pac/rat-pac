#ifndef __RAT_WaterBoxConstruction__
#define __RAT_WaterBoxConstruction__

#include <RAT/GeoSolidFactory.hh>

namespace RAT {
  /// Class for construction opf logical volume for water box.
  /// Code lifted from Fraser Duncan. As the standard line goes: 
  ///  All errors are mine."
  /// \author Chris Jillings, jillings@snolab.ca
  class WaterBoxConstruction {
 public:
   WaterBoxConstruction() {};
   virtual G4LogicalVolume* ConstructLogicalVolume(DBLinkPtr table);

 };
  
} // namespace RAT

#endif


