#include <RAT/GeoLensFactory.hh>
#include <G4Sphere.hh>
#include <G4Tubs.hh>
#include <G4SubtractionSolid.hh>
#include <RAT/Log.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {

G4VSolid *GeoLensFactory::ConstructSolid(DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  G4double diameter = table->GetD("diameter") * CLHEP::mm;
  G4double R1 = table->GetD("R1") * CLHEP::mm;
  G4double thickness = table->GetD("thickness") * CLHEP::mm;

  // Optional parameters
  G4double R2 = R1;
  try { R2 = table->GetD("R2") * CLHEP::mm; } 
  catch (DBNotFoundError &e) { };
  // end optional parms
  
  
  if ((R1 < thickness) || (R2 < thickness) || (R1 < diameter/2) || (R2 < diameter/2)) 
    Log::Die("GeoBuilder error: Lens Factory : R < diameter/2 or thickness");


  G4VSolid *base_lens =  new G4Tubs(volume_name, 0.0, diameter/2.0, 1.1*thickness/2.0, 0.0, CLHEP::twopi);  // the hole cutter

  G4VSolid *cutter_1 = new G4Sphere("temp_cutter_1", R1, 2*R1, 
				    0, CLHEP::twopi, 0, CLHEP::pi);

  base_lens = new G4SubtractionSolid(volume_name, base_lens, cutter_1, 0, G4ThreeVector(thickness/2.0 - R1, 0, 0));

  G4VSolid *cutter_2 = new G4Sphere("temp_cutter_1", R2, 2*R2, 
				    0, CLHEP::twopi, 0, CLHEP::pi);

  base_lens = new G4SubtractionSolid(volume_name, base_lens, cutter_2, 0, G4ThreeVector(R2 - thickness/2.0, 0, 0));

  return base_lens;

}

} // namespace RAT
