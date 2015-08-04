#include <RAT/GeoRevolutionChimneyFactory.hh>
#include <G4Polycone.hh>
#include <RAT/Log.hh>
#include <G4UnionSolid.hh>
#include <G4SubtractionSolid.hh>
#include <G4Tubs.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {

G4VSolid *GeoRevolutionChimneyFactory::ConstructSolid(DBLinkPtr table) {

  string volume_name = table->GetIndex();

  G4int numZPlanes;

  const vector<double> &z = table->GetDArray("z");
  const vector<double> &r_max = table->GetDArray("r_max");
  const vector<double> &r_min = table->GetDArray("r_min");
  const float rout_chim = table->GetD("rout_chimney");
  float rin_chim=0.0;
  try { rin_chim = table->GetD("rin_chimney");} catch (DBNotFoundError &e) { };
  const float z_bot = table->GetD("bottom_chimney");
  const float z_top = table->GetD("top_chimney");
  const G4ThreeVector offset ((z_top+z_bot)*.5, 0.0, 0.0);

  numZPlanes = G4int(z.size());

  if ((z.size() != r_max.size()) || (z.size() != r_min.size()) || (r_max.size() != r_min.size())) {
    Log::Die("GeoRevolutionChimneyFactory::ConstructSolid: Tables z, r_max and r_min must all be same size for 'revolve'.");
  }


  // Optional parameters
  G4double phi_start = 0.0;
  try { phi_start = table->GetD("phi_start") * CLHEP::deg; } catch (DBNotFoundError &e) { };
  G4double phi_delta = CLHEP::twopi;
  try { phi_delta = table->GetD("phi_delta") * CLHEP::deg; } catch (DBNotFoundError &e) { };

  G4double *z_array;
  G4double *r_array;
  G4double *r_min_array;

  z_array = new G4double[numZPlanes];
  r_array = new G4double[numZPlanes];
  r_min_array = new G4double[numZPlanes];

  for ( G4int i=0; i < numZPlanes; ++i ) {
    z_array[i] = z[i] * CLHEP::mm;
    r_array[i] = fabs(r_max[i]) * CLHEP::mm;
    r_min_array[i] = fabs(r_min[i]) * CLHEP::mm;
  }
  ///This is the revolution
  G4Polycone *vessel = new G4Polycone("vessel", phi_start, phi_delta, numZPlanes,
                                      z_array, r_min_array, r_array);

  ///Here comes the chimney
  G4Tubs *chimney  = new G4Tubs("chimney",0, rout_chim, (z_top-z_bot)*.5, 0., CLHEP::twopi);
  G4RotationMatrix* rot = new G4RotationMatrix();
  rot->rotateY(90. * CLHEP::deg);

  if (rin_chim != 0.0) {
    G4VSolid* temp=0;
    if (rin_chim != rout_chim)
      temp = new G4UnionSolid ("union", vessel, chimney, rot, offset);
    else 
      temp = vessel;
    G4Tubs *empty = new G4Tubs("chimney",0, rin_chim, z_top, 0., CLHEP::twopi);
    return new G4SubtractionSolid (volume_name, temp, empty, rot, offset);
  } else
    return new G4UnionSolid (volume_name, vessel, chimney, rot, offset);

}

} // namespace RAT
