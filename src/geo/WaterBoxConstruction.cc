#include <RAT/WaterBoxConstruction.hh>
#include <G4Box.hh>
#include <G4SubtractionSolid.hh>
#include <G4Sphere.hh>
#include <G4Orb.hh>
#include <G4VisAttributes.hh>
#include <G4IntersectionSolid.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>

using namespace std;

namespace RAT {
  /// \brief Log volume for water box
  ///
  /// Logical volume for water box.
  G4LogicalVolume* WaterBoxConstruction::ConstructLogicalVolume(DBLinkPtr table) {
    // First make the box itself
    string volume_name = table->GetIndex();
    const vector<double> &size = table->GetDArray("boxsize");
    G4Box* thebox = new G4Box("waterbox_box", size[0]/2, size[1]/2, size[2]/2);
    G4Material* cardboard = G4Material::GetMaterial("cardboard");
    G4Material* air = G4Material::GetMaterial("air");
    G4String logbasename = "waterbox";
    G4LogicalVolume* thebox_log = 
      new G4LogicalVolume(thebox, cardboard, logbasename);
    G4VisAttributes* thebox_vis = new G4VisAttributes();
    thebox_vis->SetColor(1,0,0);
    thebox_log->SetVisAttributes(thebox_vis);

    G4double thickness = table->GetD("boxthickness");
    G4Box* airbox = new G4Box("waterbox_air", 
			      size[0]/2 -thickness, 
			      size[1]/2 -thickness, 
			      size[2]/2 -thickness);
    G4LogicalVolume* airbox_log = 
      new G4LogicalVolume(airbox, air, logbasename + "_air");
    G4VisAttributes* airbox_vis = new G4VisAttributes();
    airbox_vis->SetColor(0,1,0);
    airbox_log->SetVisAttributes(airbox_vis);

    G4ThreeVector vec(0,0,0);
    new G4PVPlacement((G4RotationMatrix*)0, vec, airbox_log, 
		      logbasename+"_airp", thebox_log, 0, 0);


    G4Box* water_boxpart = new G4Box("water_boxpart", 
				     size[0]/2 -thickness, 
				     size[1]/2 -thickness, 
				     size[2]/2 -thickness);


    G4double cornervoid = table->GetD("airgapincorner");
    G4ThreeVector sz(size[0]/2-thickness, size[1]/2-thickness, 
		     size[2]/2-thickness);
    double_t radiuswater = sz.mag() - cornervoid;
    G4Orb* water_spherepart = new G4Orb(logbasename  + "_sph",
					radiuswater);
    
    G4IntersectionSolid* water_sol = 
      new G4IntersectionSolid("waterbox_water", 
			      water_boxpart, water_spherepart);
    G4Material* water = G4Material::GetMaterial("water");
    G4LogicalVolume* waterboxwater_log = 
      new G4LogicalVolume(water_sol, water, logbasename + "_water"); 
    G4VisAttributes* waterboxwater_vis = new G4VisAttributes();
    waterboxwater_vis->SetColor(0,0,1);
    waterboxwater_log->SetVisAttributes(waterboxwater_vis);

    new G4PVPlacement(0, G4ThreeVector(0,0,0), waterboxwater_log, 
		      logbasename+"_waterp", airbox_log, 0, 0); 


    return thebox_log;

  }





} // namespace RAT
