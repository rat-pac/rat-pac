#include <algorithm>
#include <RAT/Log.hh>
#include <RAT/Materials.hh>
#include <RAT/GLG4PMTOpticalModel.hh>
#include <RAT/RevolutionPMTConstruction.hh>
#include <G4Tubs.hh>
#include <G4Region.hh>
#include <G4Polycone.hh>
#include <G4VisAttributes.hh>
#include <G4SubtractionSolid.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalBorderSurface.hh>

using namespace std;

namespace RAT {

RevolutionPMTConstruction::RevolutionPMTConstruction(DBLinkPtr table, G4LogicalVolume *mother) : PMTConstruction("toroidal") {
    
    string pmt_model = table->GetS("index");
    
    fParams.rEdge = table->GetDArray("rho_edge");
    fParams.zEdge = table->GetDArray("z_edge");
    fParams.rInner = table->GetDArray("rho_inner");
    fParams.zInner = table->GetDArray("z_inner");
    fParams.dynodeRadius = table->GetD("dynode_radius");
    fParams.dynodeTop = table->GetD("dynode_top");
    fParams.dynodeHeight = table->GetD("dynode_height");
    
    Log::Assert(fParams.rEdge.size() == fParams.zEdge.size(),"RevolutionPMTConstruction: " + pmt_model + " must have equal length rho_edge, z_edge arrays");
    Log::Assert(fParams.rInner.size() == fParams.zInner.size(),"RevolutionPMTConstruction: " + pmt_model + " must have equal length rho_inner, z_inner arrays");
    
    // Materials
    fParams.exterior = mother->GetMaterial();
    fParams.glass = G4Material::GetMaterial(table->GetS("glass_material"));
    fParams.dynode = G4Material::GetMaterial(table->GetS("dynode_material"));
    fParams.vacuum = G4Material::GetMaterial(table->GetS("pmt_vacuum_material")); 
    fParams.photocathode = Materials::optical_surface[table->GetS("photocathode_surface")];
    fParams.mirror = Materials::optical_surface[table->GetS("mirror_surface")];
    fParams.dynode_surface = Materials::optical_surface[table->GetS("dynode_surface")];

    Log::Assert(fParams.exterior,"RevolutionPMTConstruction: " + pmt_model + " has an invalid exterior material");
    Log::Assert(fParams.glass,"RevolutionPMTConstruction: " + pmt_model + " has an invalid glass material");
    Log::Assert(fParams.vacuum,"RevolutionPMTConstruction: " + pmt_model + " has an invalid vacuum material");
    Log::Assert(fParams.dynode,"RevolutionPMTConstruction: " + pmt_model + " has an invalid dynode material");
    Log::Assert(fParams.photocathode,"RevolutionPMTConstruction: " + pmt_model + " has an invalid photocathode material");
    Log::Assert(fParams.mirror,"RevolutionPMTConstruction: " + pmt_model + " has an invalid mirror surface");
    Log::Assert(fParams.dynode_surface,"RevolutionPMTConstruction: " + pmt_model + " has an invalid dynode material");

    // Set new overall correction if requested (not included in individual)
    try {
        float efficiency_correction = table->GetD("efficiency_correction");
        fParams.efficiencyCorrection = efficiency_correction;
    } catch (DBNotFoundError &e) { }
    
    body_log = NULL; //initilize null
    
}
  
G4LogicalVolume* RevolutionPMTConstruction::BuildVolume(const std::string &prefix) {

    if (body_log) return body_log;
                                    
    // glass body
    G4Polycone *body_solid  = (G4Polycone*)BuildSolid(prefix+"_body_solid");
    
    size_t inner_equator_idx; //defined as zero in z
    for (inner_equator_idx = 0; inner_equator_idx < fParams.zInner.size(); inner_equator_idx++) {
        if (fParams.zInner[inner_equator_idx] <= 0.0) break;
    }
    size_t edge_equator_idx; //defined as zero in z
    for (edge_equator_idx = 0; edge_equator_idx < fParams.zEdge.size(); edge_equator_idx++) {
        if (fParams.zEdge[edge_equator_idx] <= 0.0) break;
    }
    
    vector<double> zeros(fParams.rInner.size(),0.0);
    
    // inner vacuum
    G4Polycone *inner1_solid = new G4Polycone(prefix + "_inner1_solid",0.0,2.0*pi,inner_equator_idx+1,&fParams.zInner[0],&zeros[0],&fParams.rInner[0]);
    G4Polycone *inner2_solid = new G4Polycone(prefix + "_inner2_solid",0.0,2.0*pi,fParams.rInner.size()-inner_equator_idx,&fParams.zInner[inner_equator_idx],&zeros[0],&fParams.rInner[inner_equator_idx]);

    // dynode (FIXME solid like other tube...?)
    G4Tubs *dynode_solid = new G4Tubs(prefix+"_dynode_solid", 0.0, fParams.dynodeRadius, fParams.dynodeHeight/2.0, 0., twopi);

    // tolerance gap between inner1 and inner2, needed to prevent overlap due to floating point roundoff
    G4double hhgap = 0.5e-7; // half the needed gap between the front and back of the PMT
    G4double toleranceGapRadius = fParams.rInner[inner_equator_idx]; // gap goes at equator
                                                                       
    G4Tubs *central_gap_solid = new G4Tubs(prefix+"_central_gap_solid", 0.0, toleranceGapRadius, hhgap, 0., twopi);

    // Logical volumes
    G4LogicalVolume *body_log, *inner1_log, *inner2_log, *dynode_log, *central_gap_log;
    body_log = new G4LogicalVolume(body_solid, fParams.glass, prefix+"_body_log");
    inner1_log = new G4LogicalVolume(inner1_solid, fParams.vacuum, prefix+"_inner1_log");
    inner2_log = new G4LogicalVolume(inner2_solid, fParams.vacuum, prefix+"_inner2_log");
    dynode_log = new G4LogicalVolume(dynode_solid, fParams.dynode, prefix+"_dynode_log");
    central_gap_log = new G4LogicalVolume(central_gap_solid, fParams.vacuum, prefix+"_central_gap_log");

    // Physical volumes
    G4ThreeVector noTranslation(0., 0., 0.);
    // place inner solids (vacuum) in outer solid (glass)
    inner1_phys= new G4PVPlacement(0,       // no rotation
        G4ThreeVector(0.0, 0.0, 2.*hhgap),  // puts face equator in right place, in front of tolerance gap
        inner1_log,                         // the logical volume
        prefix+"_inner1_phys",              // a name for this physical volume
        body_log,                           // the mother volume
        false,                              // no boolean ops
        0 );                                // copy number
    inner2_phys= new G4PVPlacement(0,       // no rotation
        noTranslation,                      // puts face equator in right place, behind the tolerance gap
        inner2_log,                         // the logical volume
        prefix+"_inner2_phys",              // a name for this physical volume
        body_log,                           // the mother volume
        false,                              // no boolean ops
        0 );                                // copy number
    // place gap between inner1 and inner2
    central_gap_phys= new G4PVPlacement( 0, // no rotation
        G4ThreeVector(0.0, 0.0, hhgap),     // puts face equator in right place, between inner1 and inner2
        central_gap_log,                    // the logical volume
        prefix+"_central_gap_phys",         // a name for this physical volume
        body_log,                           // the mother volume
        false,                              // no boolean ops
        0 );                                // copy number 
    // place dynode in back (inner2)
    dynode_phys= new G4PVPlacement(0,
        G4ThreeVector(0.0, 0.0, fParams.dynodeTop - fParams.dynodeHeight/2.0),
        prefix+"_dynode_phys",
        dynode_log,
        inner2_phys,
        false,
        0 );
        
    // build the optical surface for the dynode
    new G4LogicalSkinSurface(prefix+"_dynode_logsurf", dynode_log, fParams.dynode_surface);

    // setup optical model
    G4Region* body_region = new G4Region(prefix+"_GLG4_PMTOpticalRegion");
    body_region->AddRootLogicalVolume(body_log);
    new GLG4PMTOpticalModel(prefix+"_optical_model", body_region, body_log,
		         fParams.photocathode, fParams.efficiencyCorrection,
		         fParams.dynodeTop, fParams.dynodeRadius,
		         0.0 /*prepusling handled after absorption*/);

    // Visualization attributes
    G4VisAttributes * visAtt;
    if (fParams.invisible) {
        visAtt = new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
        body_log->SetVisAttributes(G4VisAttributes::Invisible );
        dynode_log->SetVisAttributes(G4VisAttributes::Invisible);
        inner1_log->SetVisAttributes(G4VisAttributes::Invisible);
        inner2_log->SetVisAttributes(G4VisAttributes::Invisible);  
        central_gap_log->SetVisAttributes(G4VisAttributes::Invisible); 
    } else {
        visAtt= new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
        body_log->SetVisAttributes(visAtt);
        visAtt= new G4VisAttributes(G4Color(0.5,0.5,0.5,1.0));
        dynode_log->SetVisAttributes(visAtt);
        visAtt= new G4VisAttributes(G4Color(0.7,0.5,0.3,0.27));
        inner1_log->SetVisAttributes(visAtt);
        visAtt= new G4VisAttributes(G4Color(0.6,0.7,0.8,0.67));
        inner2_log->SetVisAttributes(visAtt);
        central_gap_log->SetVisAttributes (G4VisAttributes::Invisible);
    }

    return body_log;
}

G4VSolid* RevolutionPMTConstruction::BuildSolid(const string &name) {
    vector<double> zeros(fParams.rEdge.size(),0.0);
    G4Polycone *body = new G4Polycone(name,0,2*pi,fParams.rEdge.size(),&fParams.zEdge[0],&zeros[0],&fParams.rEdge[0]);
    return body;
}

G4PVPlacement* RevolutionPMTConstruction::PlacePMT(
        G4RotationMatrix *pmtrot, 
        G4ThreeVector pmtpos, 
        const std::string &name, 
        G4LogicalVolume *logi_pmt, 
        G4VPhysicalVolume *mother_phys, 
        bool booleanSolid, int copyNo) {
        
    G4PVPlacement *body_phys = new G4PVPlacement(pmtrot, pmtpos, name, logi_pmt, mother_phys,  booleanSolid, copyNo);
    
    // photocathode surface
    new G4LogicalBorderSurface(name+"_photocathode_logsurf1", inner1_phys, body_phys, fParams.photocathode);

    // build the mirrored surface
    new G4LogicalBorderSurface(name+"_mirror_logsurf1", inner2_phys, body_phys, fParams.mirror);
    new G4LogicalBorderSurface(name+"_mirror_logsurf2", body_phys, inner2_phys, fParams.mirror);
    
    // treate tolerance gap as mirrored also
    new G4LogicalBorderSurface(name+"_central_gap_logsurf1", central_gap_phys, body_phys, fParams.mirror);
    new G4LogicalBorderSurface(name+"_central_gap_logsurf2", body_phys, central_gap_phys, fParams.mirror);

    return body_phys;
}
  
} //namespace RAT
