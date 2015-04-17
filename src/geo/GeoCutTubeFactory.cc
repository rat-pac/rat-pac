#include <RAT/GeoCutTubeFactory.hh>
#include <G4CutTubs.hh>

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

using namespace std;

namespace RAT {

G4VSolid *GeoCutTubeFactory::ConstructSolid(DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  G4double r_max = table->GetD("r_max") * mm;
  G4double size_z = table->GetD("size_z") * mm;

  // Optional parameters
  G4double r_min = 0.0;
  try { r_min = table->GetD("r_min") * mm; } 
  catch (DBNotFoundError &e) { };
  G4double phi_start = 0.0;
  try { phi_start = table->GetD("phi_start") * deg; } 
  catch (DBNotFoundError &e) { };
  G4double phi_delta = twopi;
  try { phi_delta = table->GetD("phi_delta") * deg; } 
  catch (DBNotFoundError &e) { };
  
  G4ThreeVector low_norm = G4ThreeVector( 0.0, 0.0, -1.0);
  try{ 
    std::vector<double> low_vector = table->GetDArray("low_norm"); 
    low_norm.set(low_vector[0],low_vector[1],low_vector[2]);
  }
  catch (DBNotFoundError &e) { };

  G4ThreeVector high_norm = G4ThreeVector( 0.0, 0.0, 1.0);
  try{ 
    std::vector<double> high_vector = table->GetDArray("high_norm"); 
    high_norm.set(high_vector[0],high_vector[1],high_vector[2]);
  }
  catch (DBNotFoundError &e) { };


  return new G4CutTubs(volume_name,
		    r_min, r_max, size_z,
		    phi_start, phi_delta,
                    low_norm, high_norm);
}

} // namespace RAT
