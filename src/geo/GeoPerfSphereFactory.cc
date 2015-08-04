#include <RAT/GeoPerfSphereFactory.hh>
#include <G4Sphere.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <vector>

#include <RAT/UnionSolidArray.hh>

using namespace std;

namespace RAT {

G4VSolid *GeoPerfSphereFactory::ConstructSolid(DBLinkPtr table)
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
  //  end optional parms
  
  G4VSolid *base_sphere = new G4Sphere(volume_name, r_min, r_max,
                                phi_start, phi_delta,
                                theta_start, theta_delta);

  // Read Solid positions
  string pos_table_name = table->GetS("pos_table");
  DBLinkPtr lpos_table = DB::Get()->GetLink(pos_table_name);
  const vector<double> &pos_x = lpos_table->GetDArray("x");
  const vector<double> &pos_y = lpos_table->GetDArray("y");
  const vector<double> &pos_z = lpos_table->GetDArray("z");
  const vector<double> &r_hole_array = lpos_table->GetDArray("r_hole");
  G4double r_hole = 0.0;

  unsigned num_holes = pos_x.size();
  vector<G4VSolid *> holes(num_holes);

  for (unsigned holeID = 0; holeID < num_holes; holeID++)
  {
      if ( holeID > r_hole_array.size())
	  r_hole = r_hole_array[r_hole_array.size()-1];
      else
	  r_hole = r_hole_array[holeID];
       G4double cutter_r1 = sqrt(r_min*r_min-r_hole*r_hole);
       G4double cutter_r2 = r_max;
       G4double size_z_hole = (cutter_r2 - cutter_r1)*1.02;

      G4VSolid* hole_cutter = new G4Tubs("temp_" + ::to_string(holeID), 0.0, r_hole, size_z_hole, 0.0, CLHEP::twopi);  // the hole cutter
      
      G4ThreeVector solidpos(pos_x[holeID], pos_y[holeID], pos_z[holeID]);
      solidpos.setMag((cutter_r1 + cutter_r2)/2.0);
     
      G4RotationMatrix* solidrot = new G4RotationMatrix();
      G4ThreeVector soliddir;      
      soliddir = solidpos;
      soliddir = soliddir.unit();
      // rotation required to point in direction of soliddir
      
      double angle_y = (-1.0)*atan2(soliddir.x(), soliddir.z());
      double angle_x = atan2(soliddir.y(), sqrt(soliddir.x()*soliddir.x()+soliddir.z()*soliddir.z()));
      solidrot->rotateY(angle_y);
      solidrot->rotateX(angle_x);
      
      holes[holeID] = new G4DisplacedSolid(volume_name + ::to_string(holeID), hole_cutter, 
                                           solidrot, solidpos);
  }
  
  G4VSolid *hole_union = MakeUnionSolidArray("hole_union", holes);
  
  return new G4SubtractionSolid(volume_name+"_cut", base_sphere, hole_union);
}



} // namespace RAT
