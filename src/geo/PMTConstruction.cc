#include <algorithm>
#include <RAT/Log.hh>
#include <RAT/PMTConstruction.hh>
#include <RAT/GLG4PMTOpticalModel.hh>
#include <G4Tubs.hh>
#include <G4SubtractionSolid.hh>
#include <G4Region.hh>
#include <G4VisAttributes.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <CLHEP/Units/PhysicalConstants.h>

namespace RAT {
  
  PMTConstruction::PMTConstruction(const PMTConstructionParams &params) : fParams(params)
  {
    assert(fParams.zEdge.size() == fParams.rhoEdge.size());
    assert(fParams.zEdge.size() == fParams.zOrigin.size()+1);
    assert(fParams.exterior);
    assert(fParams.glass);
    assert(fParams.vacuum);
    assert(fParams.dynode);
    assert(fParams.photocathode);
    assert(fParams.mirror);
    // Do not assert fParams.detector.  If detector==0, then do not associate PMT
    // with sensitive volume
    //assert(fParams.detector);
  }
  
  G4LogicalVolume *PMTConstruction::NewPMT(const std::string &prefix, bool simpleVis)
  {
    // envelope cylinder
    G4VSolid *envelope_solid=0;
    if (fParams.useEnvelope)
      envelope_solid = NewEnvelopeSolid(prefix+"_envelope_solid");
                                        
    // glass body
    GLG4TorusStack *body_solid = NewBodySolid(prefix+"_body_solid");
    
    // inner vacuum
    GLG4TorusStack *inner1_solid= new GLG4TorusStack(prefix + "_inner1_solid");
    GLG4TorusStack *inner2_solid= new GLG4TorusStack(prefix + "_inner2_solid");
    std::vector<double> innerZEdge, innerRhoEdge;
    G4double zLowestDynode;
    int equatorIndex;
    CalcInnerParams(body_solid, innerZEdge, innerRhoEdge, equatorIndex, zLowestDynode);
    inner1_solid->SetAllParameters(equatorIndex, &innerZEdge[0], &innerRhoEdge[0],
                                   &fParams.zOrigin[0]);
    inner2_solid->SetAllParameters(fParams.zOrigin.size()-equatorIndex,
                                   &innerZEdge[equatorIndex],
                                   &innerRhoEdge[equatorIndex],
                                   &fParams.zOrigin[equatorIndex]);
    
    // dynode volume
    G4double hhDynode= (fParams.dynodeTop - zLowestDynode)/2.0;
    G4Tubs* dynode_solid = new G4Tubs(prefix+"_dynode_solid",
                                      0.0, fParams.dynodeRadius,// solid cylinder (FIXME?)
                                      hhDynode,                // half height of cylinder
                                      0., CLHEP::twopi );            // cylinder complete in phi
    
    // tolerance gap between inner1 and inner2, needed to prevent overlap due to floating point roundoff
    G4double hhgap = 0.5e-7 ;                                            // half the needed gap between the front and back of the PMT
    G4double toleranceGapRadius = innerRhoEdge[equatorIndex];            // the outer radius of the gap needs to be equal to the
                                                                       // inner radius of the PMT where inner1 and inner2 join 
                                                                       
    G4Tubs* central_gap_solid = new G4Tubs(prefix+"_central_gap_solid",
                                      0.0 , toleranceGapRadius,        // solid cylinder with same radius as PMT
                                      hhgap,                           // half height of cylinder
                                      0., CLHEP::twopi );                   // cylinder complete in phi 

    // ------------ Logical Volumes -------------
    G4LogicalVolume *envelope_log=0, *body_log, *inner1_log, *inner2_log, *dynode_log, *central_gap_log;
    
    if (fParams.useEnvelope)
      envelope_log = new G4LogicalVolume(envelope_solid, fParams.exterior, prefix+"envelope_log");
    
    body_log= new G4LogicalVolume(body_solid, fParams.glass, prefix+"_body_log");
    if (fParams.detector)
      body_log->SetSensitiveDetector(fParams.detector);

    inner1_log= new G4LogicalVolume(inner1_solid, fParams.vacuum, prefix+"_inner1_log");
    inner1_log->SetSensitiveDetector(fParams.detector);
    
    inner2_log= new G4LogicalVolume(inner2_solid, fParams.vacuum, prefix+"_inner2_log");

    dynode_log= new G4LogicalVolume(dynode_solid, fParams.dynode, prefix+"_dynode_log");
    
    central_gap_log = new G4LogicalVolume(central_gap_solid, fParams.vacuum, prefix+"_central_gap_log");

    // ------------ Physical Volumes -------------
    G4ThreeVector noTranslation(0., 0., 0.);
    body_phys=0;
    if (fParams.useEnvelope) {
      // place body in envelope
      body_phys= new G4PVPlacement
        ( 0,                   // no rotation
          noTranslation,      // Bounding envelope already constructed to put equator at origin
          body_log,            // the logical volume
          prefix+"_body_phys", // a name for this physical volume
          envelope_log,                // the mother volume
          false,               // no boolean ops
          0 );                 // copy number
    }
    
    // place inner solids in outer solid (vacuum)
    inner1_phys= new G4PVPlacement
      ( 0,                   // no rotation
        G4ThreeVector(0.0, 0.0, 2.*hhgap),       // puts face equator in right place, in front of tolerance gap
	inner1_log,                    // the logical volume
        prefix+"_inner1_phys",         // a name for this physical volume
        body_log,           // the mother volume
        false,               // no boolean ops
        0 );                 // copy number
    
    inner2_phys= new G4PVPlacement
      ( 0,                   // no rotation
        noTranslation,       // puts face equator in right place, behind the tolerance gap
	inner2_log,                    // the logical volume
        prefix+"_inner2_phys",         // a name for this physical volume
        body_log,           // the mother volume
        false,               // no boolean ops
        0 );                 // copy number
    // place gap between inner1 and inner2
    central_gap_phys= new G4PVPlacement
      ( 0,                   // no rotation
        G4ThreeVector(0.0, 0.0, hhgap),        // puts face equator in right place, between inner1 and inner2
	central_gap_log,                       // the logical volume
        prefix+"_central_gap_phys",            // a name for this physical volume
        body_log,           // the mother volume
        false,               // no boolean ops
        0 );                 // copy number 
    // place dynode in stem/back
    dynode_phys= new G4PVPlacement
      ( 0,
        G4ThreeVector(0.0, 0.0, fParams.dynodeTop - hhDynode),
        prefix+"_dynode_phys",
        dynode_log,
        inner2_phys,
        false,
        0 );
    // build the optical surface for the dynode straight away since we already have the logical volume
    new G4LogicalSkinSurface(prefix+"_dynode_logsurf",dynode_log,fParams.dynode_surface);
     
    //--------------Exterior Optical Surface----------------- 
    // If we're using an envelope, body_phys has been created and we can therefore
    // set the optical surfaces, otherwise this must be done later once the physical volume
    // has been placed
    if (fParams.useEnvelope) {
             SetPMTOpticalSurfaces(body_phys,prefix);
    } 
    
    // Go ahead and place the cathode optical surface---this can always be done at this point
     G4LogicalBorderSurface *pc_log_surface = 
            new G4LogicalBorderSurface(prefix+"_photocathode_logsurf1",
                               inner1_phys, body_phys,
                               fParams.photocathode);
    // ------------ FastSimulationModel -------------
    // 28-Jul-2006 WGS: Must define a G4Region for Fast Simulations
    // (change from Geant 4.7 to Geant 4.8).
     G4Region* body_region = new G4Region(prefix+"_GLG4_PMTOpticalRegion");
     body_region->AddRootLogicalVolume(body_log);
     /*GLG4PMTOpticalModel * pmtOpticalModel =*/
     new GLG4PMTOpticalModel(prefix+"_optical_model", body_region, body_log,
			     pc_log_surface, fParams.efficiencyCorrection,
			     fParams.dynodeTop, fParams.dynodeRadius,
			     fParams.prepulseProb);
    
    // ------------ Vis Attributes -------------
    G4VisAttributes * visAtt;
    if (simpleVis) {
      visAtt = new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
      if (fParams.useEnvelope) envelope_log->SetVisAttributes(visAtt);
      body_log->SetVisAttributes(  G4VisAttributes::Invisible );
      dynode_log->SetVisAttributes(G4VisAttributes::Invisible);
      inner1_log->SetVisAttributes(G4VisAttributes::Invisible);
      inner2_log->SetVisAttributes(G4VisAttributes::Invisible);  
      central_gap_log->SetVisAttributes(G4VisAttributes::Invisible); 
    } else {
      if (fParams.useEnvelope) envelope_log-> SetVisAttributes (G4VisAttributes::Invisible);
    // PMT glass
      visAtt= new G4VisAttributes(G4Color(0.0,1.0,1.0,0.05));
      body_log->SetVisAttributes( visAtt );
    // dynode is medium gray
      visAtt= new G4VisAttributes(G4Color(0.5,0.5,0.5,1.0));
      dynode_log->SetVisAttributes( visAtt );
    // (surface of) interior vacuum is clear orangish gray on top (PC),
    // silvery blue on bottom (mirror)
      visAtt= new G4VisAttributes(G4Color(0.7,0.5,0.3,0.27));
      inner1_log->SetVisAttributes (visAtt);
      visAtt= new G4VisAttributes(G4Color(0.6,0.7,0.8,0.67));
      inner2_log->SetVisAttributes (visAtt);
    // central gap is invisible  
      central_gap_log->SetVisAttributes (G4VisAttributes::Invisible);
    }

    if (fParams.useEnvelope)
      return envelope_log;
    else
      return body_log;
  }
  
  GLG4TorusStack *PMTConstruction::NewBodySolid(const std::string &name)
  {
    GLG4TorusStack *body = new GLG4TorusStack(name);
    body->SetAllParameters(fParams.zOrigin.size(), 
                           &fParams.zEdge[0], &fParams.rhoEdge[0], &fParams.zOrigin[0]);
    return body;
  }
  void PMTConstruction::SetPMTOpticalSurfaces(G4PVPlacement *_body_phys, const std::string &prefix)
  { 
    /* Set the optical surfaces for a PMT. This must be called *after* the physical PMT has been placed  
       If this is not done, the mirror surface is not created.
    */
    //build the mirrored surface
    new G4LogicalBorderSurface(prefix+"_mirror_logsurf1",
                                inner2_phys, _body_phys,
                                fParams.mirror);
    new G4LogicalBorderSurface(prefix+"_mirror_logsurf2",
                               _body_phys, inner2_phys,
                               fParams.mirror);
    // also include the tolerance gap
    new G4LogicalBorderSurface(prefix+"_central_gap_logsurf1",
                               central_gap_phys, _body_phys,
                               fParams.mirror);
    new G4LogicalBorderSurface(prefix+"_central_gap_logsurf2",
                               _body_phys,central_gap_phys,
                               fParams.mirror);
  }
  
  G4VSolid *PMTConstruction::NewEnvelopeSolid(const std::string &name)
  {
    G4double zTop = fParams.zEdge[0] + fParams.faceGap;
    G4double zBottom = fParams.zEdge[fParams.zEdge.size()-1];
    G4double rho = fParams.minEnvelopeRadius;
    for (unsigned i = 0; i < fParams.rhoEdge.size(); i++)
      if (fParams.rhoEdge[i] > rho)
        rho = fParams.rhoEdge[i];
        
    G4double mainCylHalfHeight = std::max(zTop, -zBottom);
    G4double subCylHalfHeight = (mainCylHalfHeight - std::min(zTop, -zBottom))/2.0;
    G4double subCylOffset;
    if (zTop < -zBottom) subCylOffset = zTop + subCylHalfHeight;
    else subCylOffset = zBottom - subCylHalfHeight;
    
    G4Tubs *mainEnvelope = new G4Tubs(name+"_main", 0.0, rho, mainCylHalfHeight, 
                                      0.0, CLHEP::twopi);
    G4Tubs *subEnvelope  = new G4Tubs(name+"_sub", 0.0, rho*1.1, subCylHalfHeight, 
                                      0.0, CLHEP::twopi);
                                      
    return new G4SubtractionSolid(name, mainEnvelope, subEnvelope, 
                                  0, G4ThreeVector(0.0, 0.0, subCylOffset));
  }
  
  void PMTConstruction::CalcInnerParams(GLG4TorusStack *body,
                                  std::vector<double> &innerZEdge,
                                  std::vector<double> &innerRhoEdge,
                                  int &equatorIndex,
                                  double &zLowestDynode)
  {
    // Local references
    const G4double wall = fParams.wallThickness;
    const G4double dynodeRadius = fParams.dynodeRadius;
    const std::vector<G4double> &outerZEdge = fParams.zEdge;
    const std::vector<G4double> &outerRhoEdge = fParams.rhoEdge;
    const int nEdge = fParams.zEdge.size() - 1;
    
    // set shapes of inner volumes, scan for lowest allowed point of dynode
    zLowestDynode = fParams.dynodeTop;
    innerZEdge.resize(fParams.zEdge.size());
    innerRhoEdge.resize(fParams.rhoEdge.size());
    
    // We will have to calculate the inner dimensions of the PMT.
    G4ThreeVector norm;
    equatorIndex = -1;
    
    // calculate inner surface edges, check dynode position, and find equator
    innerZEdge[0]  = outerZEdge[0] - wall;
    innerRhoEdge[0]= 0.0;
    for (int i=1; i < nEdge; i++) {
      norm = body->SurfaceNormal( G4ThreeVector(0.0, outerRhoEdge[i], outerZEdge[i]) );
      innerZEdge[i]  = outerZEdge[i]   - wall * norm.z();
      innerRhoEdge[i] = outerRhoEdge[i] - wall * norm.y();
      if (innerRhoEdge[i] > dynodeRadius && innerZEdge[i] < zLowestDynode)
        zLowestDynode= innerZEdge[i];
      if (outerZEdge[i] == 0.0 || innerZEdge[i] == 0.0)
        equatorIndex= i;
    }

    innerZEdge[nEdge]  = outerZEdge[nEdge]   + wall;
    innerRhoEdge[nEdge]= outerRhoEdge[nEdge] - wall;
    
    // one final check on dynode allowed position
    if (innerRhoEdge[nEdge] > dynodeRadius && innerZEdge[nEdge] < zLowestDynode)
      zLowestDynode= innerZEdge[nEdge];
      
    // sanity check equator index
    if (equatorIndex < 0)
      Log::Die("PMTConstruction::CalcInnerParams: Pathological PMT shape with no equator edge");
    // sanity check on dynode height
    if (fParams.dynodeTop > innerZEdge[equatorIndex])
      Log::Die("PMTConstruction::CalcInnerParams: Top of PMT dynode cannot be higher than equator.");
  }
  
  
  
} // namespace RAT

