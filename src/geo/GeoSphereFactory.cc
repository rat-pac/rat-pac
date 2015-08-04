#include <RAT/GeoSphereFactory.hh>
#include <G4Sphere.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {

G4VSolid *GeoSphereFactory::ConstructSolid(DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  G4double r_max = table->GetD("r_max") * CLHEP::mm;

  // Optional parameters
  G4double r_min = 0.0;
  try { r_min = table->GetD("r_min") * CLHEP::mm; } 
  catch (DBNotFoundError &e) { };
  G4double phi_start = 0.0;
  try { phi_start = table->GetD("phi_start") * CLHEP::deg; } 
  catch (DBNotFoundError &e) { };
  G4double phi_delta = CLHEP::twopi;
  try { phi_delta = table->GetD("phi_delta") * CLHEP::deg; } 
  catch (DBNotFoundError &e) { };
  G4double theta_start = 0.0;
  try { theta_start = table->GetD("theta_start") * CLHEP::deg; } 
  catch (DBNotFoundError &e) { };
  G4double theta_delta = CLHEP::pi;
  try { theta_delta = table->GetD("theta_delta") * CLHEP::deg; } 
  catch (DBNotFoundError &e) { };
  
  return new G4Sphere(volume_name, r_min, r_max, 
		      phi_start, phi_delta,
		      theta_start, theta_delta);
}

} // namespace RAT
