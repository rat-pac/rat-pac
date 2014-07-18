#include <RAT/GeoWatchmanShieldFactory.hh>
#include <RAT/DB.hh>

#include <G4VSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Polyhedra.hh>

#include <vector>

using namespace std;

namespace RAT {

G4VSolid *GeoWatchmanShieldFactory::ConstructSolid(DBLinkPtr table) {
    string volumeName = table->GetIndex();
    
    DBLinkPtr pmtinfo = DB::Get()->GetLink(table->GetS("pmtinfo_table"));
    
    vector<G4double> pmtX = pmtinfo->GetDArray("x");
    vector<G4double> pmtY = pmtinfo->GetDArray("y");
    vector<G4double> pmtZ = pmtinfo->GetDArray("z");
    vector<G4double> pmtU = pmtinfo->GetDArray("dir_x");
    vector<G4double> pmtV = pmtinfo->GetDArray("dir_y");
    vector<G4double> pmtW = pmtinfo->GetDArray("dir_z");
    //G4int nPmt = pmtX.size();
  
    G4int numCols = table->GetI("cols");
    //G4int numRows = table->GetI("rows");
    G4double steelThickness = table->GetD("steel_thickness");
    G4double detSize = table->GetD("detector_size");
    G4double shieldThickness = table->GetD("shield_thickness");
    
    G4double cablePosRadius = detSize/2.0-shieldThickness;
    G4double sideHeight = detSize-2.*shieldThickness;
    G4double zPlaneSide[2] = {-sideHeight/2.-steelThickness,sideHeight/2.+steelThickness};
    G4double zPlaneBottop[2] = {-steelThickness/2.,steelThickness/2.};

    G4double rOuterBottop[2] = {cablePosRadius-4.*steelThickness,cablePosRadius-4.*steelThickness};
    G4double rOuterInside[2] = {cablePosRadius-3.*steelThickness,cablePosRadius-3.*steelThickness};
    G4double rOuterOutside[2] = {cablePosRadius-2.*steelThickness,cablePosRadius-2.*steelThickness};

    G4double rInnerBottop[2] = {0.0,0.0};
    G4double rInnerInSide[2] = {cablePosRadius-4.*steelThickness,cablePosRadius-4.*steelThickness};
    G4double rInnerOutSide[2] = {cablePosRadius-3.*steelThickness,cablePosRadius-3.*steelThickness};

    G4Polyhedra *bottopSolid = new G4Polyhedra(volumeName+"_bottop_solid",0,2.*pi,numCols,2,zPlaneBottop,rInnerBottop,rOuterBottop);
    G4Polyhedra *insideSolid = new G4Polyhedra(volumeName+"_inside_solid",0,2.*pi,numCols,2,zPlaneSide,rInnerInSide,rOuterInside);
    G4Polyhedra *outsideSolid = new G4Polyhedra(volumeName+"_outside_solid",0,2.*pi,numCols,2,zPlaneSide,rInnerOutSide,rOuterOutside);
    
    G4UnionSolid *sideSolid = new G4UnionSolid(volumeName+"_side_solid",insideSolid,outsideSolid);
    G4UnionSolid *sideTopSolid = new G4UnionSolid(volumeName+"_side_top_solid",sideSolid,bottopSolid,NULL,G4ThreeVector(0.0,0.0,sideHeight/2.0));
    G4UnionSolid *shieldSolid = new G4UnionSolid(volumeName+"_solid",sideTopSolid,bottopSolid,NULL,G4ThreeVector(0.0,0.0,-sideHeight/2.0));
    
    //Do we need to knock out holes for the PMTs or can the geometry gracefully handle the overlap
    
    return shieldSolid;
}

} // namespace RAT
