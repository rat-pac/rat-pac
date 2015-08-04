#include <RAT/GeoWatchmanShieldFactory.hh>
#include <RAT/Materials.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>

#include <G4VSolid.hh>
#include <G4PVPlacement.hh>
#include <G4SubtractionSolid.hh>
#include <G4Polyhedra.hh>
#include <G4Material.hh>
#include <G4Ellipsoid.hh>
#include <G4VPhysicalVolume.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalBorderSurface.hh>
#include <CLHEP/Units/SystemOfUnits.h>

#include <vector>

using namespace std;

namespace RAT {

G4VPhysicalVolume *GeoWatchmanShieldFactory::Construct(DBLinkPtr table) {
    string volumeName = table->GetIndex();
    
    //Get references to the mother volume
    G4LogicalVolume *motherLog = FindMother(table->GetS("mother"));
    G4VPhysicalVolume *motherPhys = FindPhysMother(table->GetS("mother"));
    
    //Get positions and orientations of all PMTs
    DBLinkPtr pmtinfo = DB::Get()->GetLink(table->GetS("pmtinfo_table"));
    vector<G4double> pmtX = pmtinfo->GetDArray("x");
    vector<G4double> pmtY = pmtinfo->GetDArray("y");
    vector<G4double> pmtZ = pmtinfo->GetDArray("z");
    vector<G4double> pmtU = pmtinfo->GetDArray("dir_x");
    vector<G4double> pmtV = pmtinfo->GetDArray("dir_y");
    vector<G4double> pmtW = pmtinfo->GetDArray("dir_z");
    G4int innerStart = table->GetI("inner_start");
    G4int innerLen = table->GetI("inner_len");
    G4int vetoStart = table->GetI("veto_start");
    G4int vetoLen = table->GetI("veto_len");
    
    //Get properties of the pmt covers/backs
    G4double backSemiX = table->GetD("back_semi_x");
    G4double backSemiY = table->GetD("back_semi_y");
    G4double backSemiZ = table->GetD("back_semi_z");
    G4double backThickness = table->GetD("back_thickness");
    G4Material *backMaterial = G4Material::GetMaterial(table->GetS("back_material"));
    G4OpticalSurface *innerBackSurface = GetSurface(table->GetS("inner_back_surface"));
    G4OpticalSurface *vetoBackSurface = GetSurface(table->GetS("veto_back_surface"));
    
    //Get scale and shape paremeters
    G4int numCols = table->GetI("cols");
    //G4int numRows = table->GetI("rows");
    G4double steelThickness = table->GetD("steel_thickness");
    G4double detSize = table->GetD("detector_size");
    G4double shieldThickness = table->GetD("shield_thickness");
    G4double cablePosRadius = detSize/2.0-shieldThickness;
    G4double sideHeight = detSize-2.*shieldThickness;
    G4Material *frameMaterial = G4Material::GetMaterial(table->GetS("frame_material"));    
    G4OpticalSurface *insideSurface = GetSurface(table->GetS("inside_surface"));
    G4OpticalSurface *outsideSurface = GetSurface(table->GetS("outside_surface"));
    
    G4VisAttributes *vis = GetVisAttributes(table);
    G4VisAttributes *innerVis = GetVisAttributes(DB::Get()->GetLink(table->GetS("inner_back_vis")));
    G4VisAttributes *vetoVis = GetVisAttributes(DB::Get()->GetLink(table->GetS("veto_back_vis")));
    
    //Build the pmt covers
  
    G4Ellipsoid *backFilledSolid = new G4Ellipsoid(volumeName+"_back_filled_solid", backSemiX, backSemiY, backSemiZ, -backSemiZ, 0.*CLHEP::cm);
    G4Ellipsoid *backKnockoutSolid = new G4Ellipsoid(volumeName+"_back_knockout_solid", backSemiX-backThickness, backSemiY-backThickness, backSemiZ-backThickness, -backSemiZ,0.*CLHEP::cm);
    G4SubtractionSolid *backSolid = new G4SubtractionSolid(volumeName+"_back_solid",backFilledSolid,backKnockoutSolid,NULL,G4ThreeVector());
    
    G4LogicalVolume *innerBackLog = new G4LogicalVolume(backSolid, backMaterial,volumeName+"_inner_back_log");
    innerBackLog->SetVisAttributes(innerVis);
    
    G4LogicalVolume *vetoBackLog = new G4LogicalVolume(backSolid, backMaterial,volumeName+"_veto_back_log");
    vetoBackLog->SetVisAttributes(vetoVis);
    
    //Place a back on each PMT
    
    //inner PMTs
    for (int i = innerStart; i < (innerStart+innerLen); i++) {
        G4ThreeVector position(pmtX[i],pmtY[i],pmtZ[i]);
        G4ThreeVector orient(pmtU[i],pmtV[i],pmtW[i]);
        orient = orient.unit();
        position = position - 2.0*steelThickness*orient;
        double angle_y = (-1.0)*atan2(orient.x(), orient.z());
        double angle_x = atan2(orient.y(), sqrt(orient.x()*orient.x()+orient.z()*orient.z()));
        double angle_z = atan2(-1*orient.y()*orient.z(), orient.x());
        G4RotationMatrix *rotation = new G4RotationMatrix();
        rotation->rotateY(angle_y);
        rotation->rotateX(angle_x);
        rotation->rotateZ(angle_z);
        G4VPhysicalVolume *backPhys = new G4PVPlacement(rotation,position,innerBackLog,volumeName+"_back_"+::to_string(i),motherLog,false,i);
        new G4LogicalBorderSurface(volumeName+"_back_"+::to_string(i)+"_border",motherPhys,backPhys,innerBackSurface);
    }
    
    //veto PMTs
    for (int i = vetoStart; i < (vetoStart+vetoLen); i++) {
        G4ThreeVector position(pmtX[i],pmtY[i],pmtZ[i]);
        G4ThreeVector orient(pmtU[i],pmtV[i],pmtW[i]);
        orient = orient.unit();
        position = position - 2.0*steelThickness*orient;
        double angle_y = (-1.0)*atan2(orient.x(), orient.z());
        double angle_x = atan2(orient.y(), sqrt(orient.x()*orient.x()+orient.z()*orient.z()));
        double angle_z = atan2(-1*orient.y()*orient.z(), orient.x());
        G4RotationMatrix *rotation = new G4RotationMatrix();
        rotation->rotateY(angle_y);
        rotation->rotateX(angle_x);
        rotation->rotateZ(angle_z);
        G4VPhysicalVolume *backPhys = new G4PVPlacement(rotation,position,vetoBackLog,volumeName+"_back_"+::to_string(i),motherLog,false,i);
        new G4LogicalBorderSurface(volumeName+"_back_"+::to_string(i)+"_border",motherPhys,backPhys,vetoBackSurface);
    }
    
    
    //Build the shield/mounting frame
    
    G4double zPlaneSide[2] = {-sideHeight/2.-steelThickness,sideHeight/2.+steelThickness};
    G4double zPlaneBottop[2] = {-steelThickness/2.,steelThickness/2.};

    G4double rOuterBottop[2] = {cablePosRadius-4.*steelThickness,cablePosRadius-4.*steelThickness};
    G4double rOuterInside[2] = {cablePosRadius-3.*steelThickness,cablePosRadius-3.*steelThickness};
    G4double rOuterOutside[2] = {cablePosRadius-2.*steelThickness,cablePosRadius-2.*steelThickness};

    G4double rInnerBottop[2] = {0.0,0.0};
    G4double rInnerInSide[2] = {cablePosRadius-4.*steelThickness,cablePosRadius-4.*steelThickness};
    G4double rInnerOutSide[2] = {cablePosRadius-3.*steelThickness,cablePosRadius-3.*steelThickness};

    G4Polyhedra *bottopSolid = new G4Polyhedra(volumeName+"_bottop_solid",0,2.*CLHEP::pi,numCols,2,zPlaneBottop,rInnerBottop,rOuterBottop);
    G4Polyhedra *insideSolid = new G4Polyhedra(volumeName+"_inside_solid",0,2.*CLHEP::pi,numCols,2,zPlaneSide,rInnerInSide,rOuterInside);
    G4Polyhedra *outsideSolid = new G4Polyhedra(volumeName+"_outside_solid",0,2.*CLHEP::pi,numCols,2,zPlaneSide,rInnerOutSide,rOuterOutside);
    
    //Do we need to knock out holes for the PMTs or can the geometry gracefully handle the overlap?
    //Apparently it can...
   
    //Place the shield components
   
    G4LogicalVolume *bottopLog = new G4LogicalVolume(bottopSolid,frameMaterial,volumeName+"_bottop_log");
    G4LogicalVolume *insideLog = new G4LogicalVolume(insideSolid,frameMaterial,volumeName+"_inside_log");
    G4LogicalVolume *outsideLog = new G4LogicalVolume(outsideSolid,frameMaterial,volumeName+"_outside_log");
    
    bottopLog->SetVisAttributes(vis);
    insideLog->SetVisAttributes(vis);
    outsideLog->SetVisAttributes(vis);
    
    G4VPhysicalVolume *sideInsidePhys = new G4PVPlacement(NULL,G4ThreeVector(),insideLog,volumeName+"_side_inside",motherLog,false,0);
    G4VPhysicalVolume *sideOutsidePhys = new G4PVPlacement(NULL,G4ThreeVector(),outsideLog,volumeName+"_side_outside",motherLog,false,0);
    G4VPhysicalVolume *topInsidePhys = new G4PVPlacement(NULL,G4ThreeVector(0.0,0.0,sideHeight/2.0),bottopLog,volumeName+"_top_inside",motherLog,false,0);
    G4VPhysicalVolume *topOutsidePhys = new G4PVPlacement(NULL,G4ThreeVector(0.0,0.0,sideHeight/2.0+steelThickness),bottopLog,volumeName+"_top_outside",motherLog,false,0);
    G4VPhysicalVolume *botInsidePhys = new G4PVPlacement(NULL,G4ThreeVector(0.0,0.0,-sideHeight/2.0),bottopLog,volumeName+"bot_inside",motherLog,false,0);
    G4VPhysicalVolume *botOutsidePhys = new G4PVPlacement(NULL,G4ThreeVector(0.0,0.0,-sideHeight/2.0-steelThickness),bottopLog,volumeName+"bot_outside",motherLog,false,0);
    
    new G4LogicalBorderSurface(volumeName+"_side_inside_border", motherPhys, sideInsidePhys, insideSurface);
    new G4LogicalBorderSurface(volumeName+"_top_inside_border", motherPhys, topInsidePhys, insideSurface);
    new G4LogicalBorderSurface(volumeName+"_bot_inside_border", motherPhys, botInsidePhys, insideSurface);
    new G4LogicalBorderSurface(volumeName+"_side_outside_border", motherPhys, sideOutsidePhys, outsideSurface);
    new G4LogicalBorderSurface(volumeName+"_top_outside_border", motherPhys, topOutsidePhys, outsideSurface);
    new G4LogicalBorderSurface(volumeName+"_bot_outside_border", motherPhys, botOutsidePhys, outsideSurface);
    
    return NULL; //Unsure about returning NULL here but it seems not to break anything.
}

G4OpticalSurface *GeoWatchmanShieldFactory::GetSurface(string surface_name) {
    if (Materials::optical_surface.count(surface_name) == 0)
        Log::Die("error: surface "+ surface_name + " does not exist");
    return Materials::optical_surface[surface_name];
}

G4VisAttributes *GeoWatchmanShieldFactory::GetVisAttributes(DBLinkPtr table) {
    try {
        int invisible = table->GetI("invisible");
        if (invisible) return new G4VisAttributes(G4VisAttributes::Invisible);
    } catch (DBNotFoundError &e) { };
    
    G4VisAttributes *vis = new G4VisAttributes();
    
    try {
        const vector<double> &color = table->GetDArray("color");
        if (color.size() == 3) // RGB
            vis->SetColour(G4Colour(color[0], color[1], color[2]));
        else if (color.size() == 4) // RGBA
            vis->SetColour(G4Colour(color[0], color[1], color[2], color[3]));
        else
            warn << "error: " << table->GetName()  << "[" << table->GetIndex() << "].color must have 3 or 4 components" << newline;
    } catch (DBNotFoundError &e) { };
    
    try {
        string drawstyle = table->GetS("drawstyle");
        if (drawstyle == "wireframe")
            vis->SetForceWireframe(true);
        else if (drawstyle == "solid")
            vis->SetForceSolid(true);
        else
            warn << "error: " << table->GetName() << "[" << table->GetIndex() << "].drawstyle must be either \"wireframe\" or \"solid\".";
    } catch (DBNotFoundError &e) { };
    
    try {
        int force_auxedge = table->GetI("force_auxedge");
        vis->SetForceAuxEdgeVisible(force_auxedge == 1);
    } catch (DBNotFoundError &e) { };

    return vis;
}

} // namespace RAT
