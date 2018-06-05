#include <RAT/ConeWaveguideFactory.hh>
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

void ConeWaveguideFactory::SetTable(std::string table, std::string index)
{
  WaveguideFactory::SetTable(table, index);
  
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

G4LogicalVolume *ConeWaveguideFactory::Construct(const std::string &name,
                                                 G4LogicalVolume */*mother*/, 
                                                 bool invisible)
{
  double OuterRadiusTop = fRadiusTop + 1.0;
  double OuterRadiusBottom = fRadiusBottom + 1.0;
  G4VSolid *cone_solid_in = new G4Cons(name+"_solid_in", 
                                    fInnerRadiusTop, fRadiusTop,
                                    fInnerRadiusBottom, fRadiusBottom,
                                    (fZTop - fZBottom)/2.0, 0.0, CLHEP::twopi);
  G4VSolid *cone_solid_out = new G4Cons(name+"_solid_out", 
                                    fRadiusTop, OuterRadiusTop,
                                    fRadiusBottom, OuterRadiusBottom,
                                    (fZTop - fZBottom)/2.0, 0.0, CLHEP::twopi);
  G4VSolid *cone_solid_whole = new G4Cons(name+"_solid_whole", 
                                    fInnerRadiusTop, OuterRadiusTop +1.0,
                                    fInnerRadiusBottom, OuterRadiusBottom +1.0,
                                    (fZTop - fZBottom)/2.0, 0.0, CLHEP::twopi);
  if (fPMTBody) {
    cone_solid_in->SetName(name+"_in");
    cone_solid_in = new G4SubtractionSolid(name, cone_solid_in, fPMTBody, 0, -GetPlacementOffset());
    cone_solid_out->SetName(name+"_out");
    cone_solid_out = new G4SubtractionSolid(name, cone_solid_out, fPMTBody, 0, -GetPlacementOffset());
    cone_solid_whole->SetName(name+"_full");
    cone_solid_whole = new G4SubtractionSolid(name, cone_solid_whole, fPMTBody, 0, -GetPlacementOffset());
  }
  
  G4LogicalVolume *cone_logi_in = new G4LogicalVolume(cone_solid_in, fMaterial, name+"_logi_in");
  G4LogicalVolume *cone_logi_out = new G4LogicalVolume(cone_solid_out, fMaterial, name+"_logi_out");
  G4LogicalVolume *cone_logi_whole = new G4LogicalVolume(cone_solid_whole, fMaterial, name+"_logi_whole");

  G4ThreeVector no_offset(0, 0, 0);
  G4PVPlacement* in_phys= new G4PVPlacement
    ( 0,                   // no rotation
      no_offset,       // puts face equator in right place
      cone_logi_in,                    // the logical volume
      name+"_in",         // a name for this physical volume
      cone_logi_whole,           // the mother volume
      false,               // no boolean ops
      0 );                 // copy number
  G4PVPlacement* out_phys= new G4PVPlacement
    ( 0,                   // no rotation
      no_offset,       // puts face equator in right place
      cone_logi_out,                    // the logical volume
      name+"_out",         // a name for this physical volume
      cone_logi_whole,           // the mother volume
      false,               // no boolean ops
      0 );                 // copy number

  if (fSurface) {
    new G4LogicalBorderSurface(name+"_surface1",
                             in_phys, out_phys, fSurface);
    new G4LogicalBorderSurface(name+"_surface2",
                             out_phys, in_phys, fSurface);
}
  
  if (invisible)
    cone_logi_whole->SetVisAttributes(G4VisAttributes::Invisible);
  else
    cone_logi_whole->SetVisAttributes(G4Color(0.9,0.9,0.9,0.5));
   
/*  if (fSurface)
    new G4LogicalSkinSurface(name+"_surf", cone_logi, fSurface);
*/    
  return cone_logi_whole;
}

G4ThreeVector ConeWaveguideFactory::GetPlacementOffset()
{
  return G4ThreeVector(0., 0., fZBottom + (fZTop - fZBottom)/2.0);
}

double ConeWaveguideFactory::GetZTop()
{
  return fZTop;
}

double ConeWaveguideFactory::GetRadius()
{
  return std::max(fRadiusTop, fRadiusBottom);
}

} // namespace RAT
