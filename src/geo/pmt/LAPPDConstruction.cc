#include <algorithm>
#include <RAT/Log.hh>
#include <RAT/Materials.hh>
#include <RAT/GLG4PMTOpticalModel.hh>
#include <RAT/LAPPDConstruction.hh>
#include <G4Box.hh>
#include <G4Region.hh>
#include <G4VisAttributes.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalSkinSurface.hh>

using namespace std;

namespace RAT {

LAPPDConstruction::LAPPDConstruction(DBLinkPtr table, G4LogicalVolume *mother) : PMTConstruction("lappd") {
    
    glass_phys = 0;
    vacuum_phys = 0;
    
    log_pmt = 0;

    // Setup PMT parameters
    fParams.width = table->GetD("width");
    fParams.thick = table->GetD("thick");
    fParams.glassThickness = table->GetD("glass_thickness");

    // Materials
    fParams.glass = G4Material::GetMaterial(table->GetS("glass_material"));
    fParams.vacuum = G4Material::GetMaterial(table->GetS("pmt_vacuum_material")); 
    string pc_surface_name = table->GetS("photocathode_surface");
    fParams.photocathode = Materials::optical_surface[pc_surface_name];
    
    if (fParams.photocathode == 0)
      Log::Die("PMTFactoryBase error: Photocathode surface \"" + pc_surface_name + "\" not found");
    
    // Set new overall correction if requested (not included in individual)
    try {
      float efficiency_correction = table->GetD("efficiency_correction");
      fParams.efficiencyCorrection = efficiency_correction;
    } catch (DBNotFoundError &e) { }

    string pmt_model = table->GetS("index");
    Log::Assert(fParams.width>0, "LAPPDConstruction: " + pmt_model + " width must be postive");
    Log::Assert(fParams.thick>0, "LAPPDConstruction: " + pmt_model + " thick must be postive");
    Log::Assert(fParams.glassThickness>0, "LAPPDConstruction: " + pmt_model + " glass thickness must be postive");
    Log::Assert(fParams.glass, "LAPPDConstruction: " + pmt_model + " has an invalid glass material");
    Log::Assert(fParams.vacuum, "LAPPDConstruction: " + pmt_model + " has an invalid vacuum material");
    Log::Assert(fParams.photocathode, "LAPPDConstruction: " + pmt_model + " has an invalid photocathode material");

}
  
  G4LogicalVolume* LAPPDConstruction::BuildVolume(const std::string &prefix)
  { 

    if (log_pmt) return log_pmt;

    // Glass body
    G4Box* body_solid  = (G4Box*)BuildSolid(prefix+"_body_solid");
        
    // Construct inners
    const double vacuumWidth = fParams.width - fParams.glassThickness;
    const double vacuumThick = fParams.thick - fParams.glassThickness;
    G4Box* vacuum_solid = new G4Box( prefix + "_vacuum_solid", vacuumWidth, vacuumWidth, vacuumThick);

    // ------------ Logical Volumes -------------
    G4LogicalVolume *body_log, *vacuum_log;
    
    body_log = new G4LogicalVolume( body_solid, fParams.glass, prefix + "_body_logic" );
    vacuum_log = new G4LogicalVolume( vacuum_solid, fParams.vacuum, prefix + "_vacuum_logic" );

    // ------------ Physical Volumes -------------
    G4ThreeVector noTranslation(0., 0., 0.);
    
    // Place the inner solids in the glass solid to produce the physical volumes
    vacuum_phys= new G4PVPlacement
      ( 0,                   // no rotation
	noTranslation,       // must share the same origin than the mother volume
	                     // if we want the PMT optical model working properly
	vacuum_log,          // the logical volume
	prefix+"_vacuum_phys",         // a name for this physical volume
	body_log,           // the mother volume
	false,               // no boolean ops
	0 );                 // copy number

    // ------------ FastSimulationModel -------------
    // 28-Jul-2006 WGS: Must define a G4Region for Fast Simulations
    // (change from Geant 4.7 to Geant 4.8).
     G4Region* body_region = new G4Region(prefix+"_GLG4_PMTOpticalRegion");
     body_region->AddRootLogicalVolume(body_log);
     new GLG4PMTOpticalModel(prefix+"_optical_model", body_region, body_log,
			    fParams.photocathode, fParams.efficiencyCorrection,
			     0.0, 0.0, 0.0 /*prepusling handled after absorption*/);
     
    // ------------ Vis Attributes -------------
     G4VisAttributes * visAtt;
     if (fParams.invisible) {
       visAtt = new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
       body_log->SetVisAttributes(  G4VisAttributes::Invisible );
       vacuum_log->SetVisAttributes(G4VisAttributes::Invisible);
     } else {
       // PMT glass
       visAtt= new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
       body_log->SetVisAttributes( visAtt );
       // (surface of) interior vacuum is clear orangish gray on top (PC),
       // silvery blue on bottom (mirror)
       visAtt= new G4VisAttributes(G4Color(0.7,0.5,0.3,0.27));
       vacuum_log->SetVisAttributes (visAtt);
     }
     
     log_pmt = body_log;
     
     return log_pmt;
  }
  
  G4VSolid* LAPPDConstruction::BuildSolid(const string &name) {
    G4Box* body = new G4Box(name, fParams.width, fParams.width, fParams.thick);
    return body;
  }

  G4PVPlacement* LAPPDConstruction::PlacePMT(
            G4RotationMatrix *pmtrot, 
            G4ThreeVector pmtpos, 
            const std::string &name, 
            G4LogicalVolume *logi_pmt, 
            G4VPhysicalVolume *mother_phys, 
            bool booleanSolid, int copyNo) {
    
    G4PVPlacement *body_phys = new G4PVPlacement(pmtrot, pmtpos, name, logi_pmt, mother_phys,  booleanSolid, copyNo);

    // photocathode surface
    new G4LogicalBorderSurface(name+"_photocathode_logsurf1", vacuum_phys, body_phys, fParams.photocathode);
    
    return body_phys;

  }
  
} //namespace RAT
