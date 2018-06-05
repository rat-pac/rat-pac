#include <RAT/GeoPolygonFactory.hh>
#include <RAT/PolygonOrientation.hh>
#include <RAT/TubeFacetSolid.hh>
#include <G4Tubs.hh>
#include <G4SubtractionSolid.hh>
#include <G4TwoVector.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {

G4VSolid *GeoPolygonFactory::ConstructSolid(DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  G4double size_z = table->GetD("size_z") * CLHEP::mm;      // half thickness of plate
  string poly_table_name = table->GetS("poly_table");
  DBLinkPtr lpoly_table = DB::Get()->GetLink(poly_table_name);
  const vector<G4double> &vertex_pnts_x = lpoly_table->GetDArray("x");
  const vector<G4double> &vertex_pnts_y = lpoly_table->GetDArray("y"); 

  // Optional parameters
  G4double scale_fac = 1.0;      
  try { scale_fac = table->GetD("scale_fac"); } 
  catch (DBNotFoundError &e) { };
  
  G4double scale_fac_in = 0.0;      
  try { scale_fac_in = table->GetD("scale_fac_in"); } 
  catch (DBNotFoundError &e) { };
  
  // end optional parms

  G4double poly_max = 0.0 * CLHEP::mm;
  G4double poly_max_tmp = 0.0 * CLHEP::mm;
  vector<G4TwoVector> g4Polygon;
  for ( G4int i=0; i < G4int(vertex_pnts_x.size()); ++i ) 
  {
      g4Polygon.push_back( G4TwoVector(vertex_pnts_x[i] * CLHEP::mm, vertex_pnts_y[i] *CLHEP::mm ) );
      poly_max_tmp = sqrt((vertex_pnts_x[i] * CLHEP::mm * vertex_pnts_x[i] * CLHEP::mm) + ( vertex_pnts_y[i] * CLHEP::mm *vertex_pnts_y[i] * CLHEP::mm));
      if (poly_max_tmp >= poly_max)
	  poly_max = poly_max_tmp;
  }
  
  poly_max = poly_max * scale_fac;
  
  //Check the orientation of polygon to be defined clockwise
  CheckOrientation(g4Polygon);
  
  G4VSolid* base_solid = MakeTubeFacetSolid(volume_name,
					    g4Polygon,
					    scale_fac,
					    size_z,
					    0.0,
					    poly_max);
  
  if ((scale_fac_in > 0) && (scale_fac_in < scale_fac))
  {
      G4VSolid* sub_solid = MakeTubeFacetSolid("sub_solid",
						g4Polygon,
						scale_fac_in,
						size_z*1.1,
						0.0,
						poly_max/scale_fac*scale_fac_in);
      
      base_solid = new G4SubtractionSolid(volume_name, base_solid, sub_solid, 0, G4ThreeVector(0.0,0.0,0.0));
  }

  return base_solid;
}

} // namespace RAT
