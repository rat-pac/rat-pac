#include <G4ios.hh>  // G4OpticalSurface.hh should have included this
#include <RAT/ConeWaveguideConstruction.hh>
#include <algorithm>
#include <G4Cons.hh>
#include <G4SubtractionSolid.hh>
#include <G4Material.hh>
#include <G4VisAttributes.hh>
#include <RAT/Materials.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4PVPlacement.hh>
#include <G4LogicalBorderSurface.hh>
#include <CLHEP/Units/PhysicalConstants.h>

namespace RAT {

ConeWaveguideConstruction::ConeWaveguideConstruction(std::string table, std::string index)
{
  DBLinkPtr fTable = DB::Get()->GetLink(table, index);
  
  fZTop = fTable->GetD("z_top");
  fZBottom = fTable->GetD("z_bottom");
  fRadiusTop = fTable->GetD("radius_top");
  fRadiusBottom = fTable->GetD("radius_bottom");
  fInnerRadiusTop = fTable->GetD("inner_radius_top");
  fInnerRadiusBottom = fTable->GetD("inner_radius_bottom");
  fMaterial = G4Material::GetMaterial(fTable->GetS("material"));
  std::string surface = fTable->GetS("surface");
  if (surface == "none" || surface == "")
    fSurface = 0;
  else { 
    fSurface = Materials::optical_surface[surface];
    if (!fSurface)
      Log::Die("ConeWaveguideFactory: Unable to locate surface " + surface);
  }
}

G4VSolid *ConeWaveguideConstruction::NewBodySolid(const std::string &name, G4VSolid* fPMTBody)
{
  double OuterRadiusTop = fRadiusTop + 1.0;
  double OuterRadiusBottom = fRadiusBottom + 1.0;
/*  G4VSolid *cone_solid_in = new G4Cons(name+"_solid_in", 
                                    fInnerRadiusTop, fRadiusTop,
                                    fInnerRadiusBottom, fRadiusBottom,
                                    (fZTop - fZBottom)/2.0, 0.0, twopi);
  G4VSolid *cone_solid_out = new G4Cons(name+"_solid_out", 
                                    fRadiusTop, OuterRadiusTop,
                                    fRadiusBottom, OuterRadiusBottom,
                                    (fZTop - fZBottom)/2.0, 0.0, twopi);
*/
  G4VSolid *cone_solid_whole = new G4Cons(name+"_solid_whole", 
                                    fInnerRadiusTop, OuterRadiusTop +1.0,
                                    fInnerRadiusBottom, OuterRadiusBottom +1.0,
                                    (fZTop - fZBottom)/2.0, 0.0, CLHEP::twopi);
/*
  cone_solid_in->SetName(name+"_in");
  cone_solid_in = new G4SubtractionSolid(name, cone_solid_in, fPMTBody, 0, -GetPlacementOffset());
  cone_solid_out->SetName(name+"_out");
  cone_solid_out = new G4SubtractionSolid(name, cone_solid_out, fPMTBody, 0, -GetPlacementOffset());
*/
  G4ThreeVector NullVector(0., 0., 0.);
  cone_solid_whole->SetName(name+"_full");
  cone_solid_whole = new G4SubtractionSolid(name, cone_solid_whole, fPMTBody, 0, -GetPlacementOffset());
  
  return cone_solid_whole;
}

double ConeWaveguideConstruction::GetHeight()
{
  return (fZTop - fZBottom);
}

G4ThreeVector ConeWaveguideConstruction::GetPlacementOffset()
{
  return G4ThreeVector(0., 0., fZBottom + (fZTop - fZBottom)/2.0);
}

} // namespace RAT
