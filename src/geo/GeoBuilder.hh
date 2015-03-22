#ifndef __RATGeoBuilder__
#define __RATGeoBuilder__

#include <string>
#include <vector>
#include <RAT/DB.hh>
#include <G4VPhysicalVolume.hh>
#include <G4GDMLParser.hh>

namespace RAT {

class GeoBuilder {
public:
  // Initialize factory with default classes
  GeoBuilder();
  // Construct all geometry from database, returns world volume
  G4VPhysicalVolume *ConstructAll(std::string geo_tablename="GEO");
  
  // Construct a volume from particular table (assumes mother volume already
  // exists), returns physical volume
  G4VPhysicalVolume *Construct(DBLinkPtr table);

  typedef enum GeoBuilderSource { RATGEOTABLES, GDMLFILE, DAECOLLADAFILE } GeoBuilderSource_t;
  GeoBuilderSource_t GetBuilderSource() { return geo_source; };

private:
  G4GDMLParser gdml_parser;
  GeoBuilderSource_t geo_source;
};


} // namespace RAT

#endif
