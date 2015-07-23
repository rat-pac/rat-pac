#include <algorithm>
#include <RAT/Log.hh>
#include <RAT/Materials.hh>
#include <RAT/GLG4PMTOpticalModel.hh>
#include <RAT/CubicPMTConstruction.hh>
#include <G4Box.hh>
#include <G4Region.hh>
#include <G4VisAttributes.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalSkinSurface.hh>

using namespace std;

namespace RAT {

CubicPMTConstruction::CubicPMTConstruction(DBLinkPtr table, G4LogicalVolume *mother) : PMTConstruction("cubic") {
    
    glass_phys = 0;
    vacuum_phys = 0;
    
    log_pmt = 0;

    // Setup PMT parameters
    fParams.pmtWidth = table->GetD("pmt_width");
    fParams.photocathodeWidth = table->GetD("photocathode_width");
    fParams.caseThickness = table->GetD("case_thickness");
    fParams.glassThickness = table->GetD("glass_thickness");

    // Materials
    fParams.outerCase = G4Material::GetMaterial(table->GetS("case_material"));
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
    Log::Assert(fParams.pmtWidth>0, "CubicPMTConstruction: " + pmt_model + " width must be postive");
    Log::Assert(fParams.photocathodeWidth>0, "CubicPMTConstruction: " + pmt_model + " photocathode width must be postive");
    Log::Assert(fParams.caseThickness>0, "CubicPMTConstruction: " + pmt_model + " case thickness must be postive");
    Log::Assert(fParams.glassThickness>0, "CubicPMTConstruction: " + pmt_model + " glass thickness must be postive");
    Log::Assert(fParams.pmtWidth > fParams.caseThickness + fParams.photocathodeWidth, "CubicPMTConstruction: " + pmt_model + " width is too small");
    Log::Assert(fParams.outerCase, "CubicPMTConstruction: " + pmt_model + " has an invalid case material");
    Log::Assert(fParams.glass, "CubicPMTConstruction: " + pmt_model + " has an invalid glass material");
    Log::Assert(fParams.vacuum, "CubicPMTConstruction: " + pmt_model + " has an invalid vacuum material");
    Log::Assert(fParams.photocathode, "CubicPMTConstruction: " + pmt_model + " has an invalid photocathode material");

}
  
  G4LogicalVolume* CubicPMTConstruction::BuildVolume(const std::string &prefix)
  { 

    if (log_pmt) return log_pmt;

    // Case envelope body
    G4Box* body_solid  = (G4Box*)BuildSolid(prefix+"_body_solid");
        
    // Glass body
    G4Box* glass_solid = new G4Box( prefix + "_glass_solid", fParams.pmtWidth - fParams.caseThickness, fParams.pmtWidth - fParams.caseThickness, fParams.pmtWidth - fParams.caseThickness);
    
    // Construct inners
    const double vacuumHeight = fParams.pmtWidth - fParams.caseThickness - fParams.glassThickness;
    G4Box* vacuum_solid = new G4Box( prefix + "_vacuum_solid", fParams.photocathodeWidth, fParams.photocathodeWidth, vacuumHeight);

    // ------------ Logical Volumes -------------
    G4LogicalVolume *body_log, *glass_log, *vacuum_log;
    
    body_log = new G4LogicalVolume( body_solid, fParams.outerCase, prefix + "_body_logic" );
    glass_log = new G4LogicalVolume( glass_solid, fParams.glass, prefix + "_glass_logic" );
    vacuum_log = new G4LogicalVolume( vacuum_solid, fParams.vacuum, prefix + "_vacuum_logic" );

    // ------------ Physical Volumes -------------
    G4ThreeVector noTranslation(0., 0., 0.);
    
    // Place the inner solids in the glass solid to produce the physical volumes
    glass_phys= new G4PVPlacement
      ( 0,                   // no rotation
	G4ThreeVector(0.0, 0.0, fParams.caseThickness), //place glass surface at case surface
	glass_log,            // the logical volume
	prefix+"_glass_phys", // a name for this physical volume
	body_log,                // the mother volume
	false,               // no boolean ops
	0 );                 // copy number
    
    vacuum_phys= new G4PVPlacement
      ( 0,                   // no rotation
	noTranslation,       // must share the same origin than the mother volume
	                     // if we want the PMT optical model working properly
	vacuum_log,          // the logical volume
	prefix+"_vacuum_phys",         // a name for this physical volume
	glass_log,           // the mother volume
	false,               // no boolean ops
	0 );                 // copy number

    // ------------ FastSimulationModel -------------
    // 28-Jul-2006 WGS: Must define a G4Region for Fast Simulations
    // (change from Geant 4.7 to Geant 4.8).
     G4Region* body_region = new G4Region(prefix+"_GLG4_PMTOpticalRegion");
     body_region->AddRootLogicalVolume(glass_log);
     new GLG4PMTOpticalModel(prefix+"_optical_model", body_region, glass_log,
			    fParams.photocathode, fParams.efficiencyCorrection,
			     0.0, 0.0, 0.0 /*prepusling handled after absorption*/);
     
    // ------------ Vis Attributes -------------
     G4VisAttributes * visAtt;
     if (fParams.invisible) {
       visAtt = new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
       body_log->SetVisAttributes(  G4VisAttributes::Invisible );
       glass_log->SetVisAttributes(  G4VisAttributes::Invisible );
       vacuum_log->SetVisAttributes(G4VisAttributes::Invisible);
     } else {
       // PMT case
       visAtt= new G4VisAttributes(G4Color(0.5,0.0,0.0,0.5));
       body_log->SetVisAttributes( visAtt );
       // PMT glass
       visAtt= new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
       glass_log->SetVisAttributes( visAtt );
       // (surface of) interior vacuum is clear orangish gray on top (PC),
       // silvery blue on bottom (mirror)
       visAtt= new G4VisAttributes(G4Color(0.7,0.5,0.3,0.27));
       vacuum_log->SetVisAttributes (visAtt);
     }
     
     log_pmt = body_log;
     
     return log_pmt;
  }
  
  G4VSolid* CubicPMTConstruction::BuildSolid(const string &name) {
    G4Box* body = new G4Box(name, fParams.pmtWidth, fParams.pmtWidth, fParams.pmtWidth);
    return body;
  }

  G4PVPlacement* CubicPMTConstruction::PlacePMT(
            G4RotationMatrix *pmtrot, 
            G4ThreeVector pmtpos, 
            const std::string &name, 
            G4LogicalVolume *logi_pmt, 
            G4VPhysicalVolume *mother_phys, 
            bool booleanSolid, int copyNo) {
    
    G4PVPlacement *body_phys = new G4PVPlacement(pmtrot, pmtpos, name, logi_pmt, mother_phys,  booleanSolid, copyNo);

    // photocathode surface
    new G4LogicalBorderSurface(name+"_photocathode_logsurf1", vacuum_phys, glass_phys, fParams.photocathode);
    
    return body_phys;

  }
  
} //namespace RAT
