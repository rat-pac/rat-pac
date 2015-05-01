#include <RAT/GeoReflectorWaveguideFactory.hh>
#include <vector>
#include <G4Material.hh>
#include <G4Sphere.hh>
#include <G4SubtractionSolid.hh>
#include <RAT/PMTConstruction.hh>
#include <RAT/ConeWaveguideConstruction.hh>
#include <RAT/GeoPMTParser.hh>
#include <RAT/Log.hh>
#include <G4PVPlacement.hh>
#include <RAT/Materials.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalVolume.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>



using namespace std;

namespace RAT {

G4VPhysicalVolume *GeoReflectorWaveguideFactory::Construct(DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  // Find mother
  string mother_name = table->GetS("mother");
  G4LogicalVolume *mother = FindMother(mother_name);
  if (mother == 0)
    Log::Die("Unable to find mother volume " + mother_name + " for "
                + volume_name);
  // get pointer to physical mother volume
  G4VPhysicalVolume* phys_mother = FindPhysMother(mother_name);
  if (phys_mother == 0)
    Log::Die("GeoBuilder error: PMT mother physical volume " + mother_name
                +" not found");

  G4double r_min = table->GetD("r_min") * CLHEP::mm;
  G4double phi_start = 0.0;
  G4double phi_delta = CLHEP::twopi;
  G4double theta_start = 0.0;
  G4double theta_delta = CLHEP::pi;
  G4double r_mid = r_min + 1.0 * CLHEP::mm;
  G4double r_max = r_min + 2.0 * CLHEP::mm;

  G4Material *inMaterial = G4Material::GetMaterial(table->GetS("inmaterial"));
  G4Material *outMaterial = G4Material::GetMaterial(table->GetS("outmaterial"));
  G4OpticalSurface *RefSurface;
  std::string surface = table->GetS("reflectivesurface");
  if (surface == "none" || surface == "")
    RefSurface = 0;
  else {
    RefSurface = Materials::optical_surface[surface];
    if (!RefSurface)
      Log::Die("GeoReflectorFactory: Unable to locate surface " + surface);
  }

  G4VSolid *ball_in = new G4Sphere(volume_name + "_insolid", r_min, r_mid,
                                  phi_start, phi_delta,
                                  theta_start, theta_delta);

  G4VSolid *ball_out = new G4Sphere(volume_name + "_outsolid", r_mid, r_max,
                                   phi_start, phi_delta,
                                   theta_start, theta_delta);

  G4VSolid *ball_whole = new G4Sphere(volume_name + "_wholesolid", r_min, r_max,
                                   phi_start, phi_delta,
                                   theta_start, theta_delta);

  string pmt_table = table->GetS("pmt_table");
  DBLinkPtr lgeo_pmt = DB::Get()->GetLink("GEO", pmt_table);
  GeoPMTParser pmt_parser(lgeo_pmt);
  PMTConstructionParams params = pmt_parser.GetPMTParams();
  PMTConstruction pmtConstruct(params);
  G4VSolid *pmtBody = pmtConstruct.NewBodySolid("dummy");

  string waveguide = lgeo_pmt->GetS("waveguide");
  string waveguide_desc = lgeo_pmt->GetS("waveguide_desc");
  string waveguide_table, waveguide_index;
  if (!DB::ParseTableName(waveguide_desc, waveguide_table, waveguide_index))
    Log::Die("GeoPMTFactoryBase: Waveguide descriptor name is not a valid RATDB table: "
             +waveguide_desc);
                                                                                                   
  ConeWaveguideConstruction coneConstruct(waveguide_table, waveguide_index);
  G4VSolid *waveBody = coneConstruct.NewBodySolid("dummy", pmtBody);
  G4ThreeVector waveoffset = coneConstruct.GetPlacementOffset();
  vector<G4ThreeVector> pmtloc = pmt_parser.GetPMTLocations();
  int max_pmts = pmtloc.size();

  for (int pmtID = 0; pmtID < max_pmts; pmtID++) {
    G4String name_in = "reflector_in" + ::to_string(pmtID);
    G4String name_out = "reflector_out" + ::to_string(pmtID);
    G4String name_whole = "reflector_whole" + ::to_string(pmtID);
    G4RotationMatrix pmtrot = pmt_parser.GetPMTRotation(pmtID);
    double wavemag = pmtloc[pmtID].mag() - waveoffset.z();
    G4ThreeVector waveloc = pmtloc[pmtID];
    waveloc.setMag(wavemag);
    ball_whole = new G4SubtractionSolid(name_whole, ball_whole, waveBody, & pmtrot, waveloc);
    ball_in = new G4SubtractionSolid(name_in, ball_in, waveBody, & pmtrot, waveloc);
    ball_out = new G4SubtractionSolid(name_out, ball_out, waveBody, & pmtrot, waveloc);
  }

  G4LogicalVolume *whole_logi = new G4LogicalVolume(ball_whole,
                                outMaterial,
                                volume_name);
  G4LogicalVolume *in_logi = new G4LogicalVolume(ball_in,
                                inMaterial,
                                volume_name+"_in_logi");
  G4LogicalVolume *out_logi = new G4LogicalVolume(ball_out,
                                outMaterial,
                                volume_name+"_out_logi");
  G4ThreeVector no_offset(0, 0, 0);

  G4PVPlacement* in_phys= new G4PVPlacement
    ( 0,                   // no rotation
      no_offset,       // puts face equator in right place
      in_logi,                    // the logical volume
      volume_name+"_in",         // a name for this physical volume
      whole_logi,           // the mother volume
      false,               // no boolean ops
      0 );                 // copy number
  G4PVPlacement* out_phys= new G4PVPlacement
    ( 0,                   // no rotation
      no_offset,       // puts face equator in right place
      out_logi,                    // the logical volume
      volume_name+"_out",         // a name for this physical volume
      whole_logi,           // the mother volume
      false,               // no boolean ops
      0 );                 // copy number
  new G4LogicalBorderSurface(volume_name+"_surface1",
                             in_phys, out_phys, RefSurface);
  new G4LogicalBorderSurface(volume_name+"_surface2",
                             out_phys, in_phys, RefSurface);

  /*G4PVPlacement* whole_phys=*/
  new G4PVPlacement
    ( 0,                   // no rotation
      no_offset,       // puts face equator in right place
      volume_name,         // a name for this physical volume
      whole_logi,                    // the logical volu};
      phys_mother,           // the mother volume
      false,               // no boolean ops
      0 );                 // copy number
 
  return 0;
}

}
