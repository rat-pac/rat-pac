#include <RAT/PMTConcentrator.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <G4Box.hh>
#include <G4Polycone.hh>
#include <G4PVPlacement.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4VisAttributes.hh>
#include <RAT/Log.hh>
#include <cmath>
#include <iostream>

using namespace std;

namespace RAT {


PMTConcentrator::PMTConcentrator
     (const G4String &name,
      G4double _upper_radius,
      G4double _lower_radius,
      G4double _a, // SEE Mike Lay's Thesis
      G4double _b,
      G4double _d,
      G4Material *outerMaterial,
      G4Material *bulkMaterial,
      G4OpticalSurface *reflective_surf
      ) : G4LogicalVolume(
                         new G4Box("temp",10,10,10), bulkMaterial, name)
{
  this->a=_a;
  this->b=_b;
  this->d=_d;
  this->upper_radius=_upper_radius;
  this->lower_radius=_lower_radius;
  
  // Make Solid
  const int n = 20;
  G4double z[n],  r_inner_oil[n], r_oil_plastic_boundary[n], r_outer_plastic[n];

  for (int i = 0; i < n; i++) {
    z[i] = z_from_r(lower_radius) + i*(z_from_r(upper_radius) -
		    z_from_r(lower_radius))/n;
    r_oil_plastic_boundary[i] = r_from_z(z[i]);
    r_inner_oil[i] = r_oil_plastic_boundary[i] - 0.01*CLHEP::mm;
    r_outer_plastic[i] = r_oil_plastic_boundary[i] + 1.0 * CLHEP::mm;
    z[i] = z[i] - z_from_r(lower_radius);
  }
  
  G4Polycone *outer_solid = new G4Polycone(name, 0, CLHEP::twopi, n, z, r_inner_oil,
					   r_outer_plastic);
  SetSolid(outer_solid);

  // Split into plastic concentrator and oil layer so we can add reflective
  // surface in between
  G4Polycone *conc_solid = new G4Polycone(name+"_plastic", 0, CLHEP::twopi, n, z, 
					  r_oil_plastic_boundary,
					  r_outer_plastic);

  conc_logi = new G4LogicalVolume(conc_solid,
				  bulkMaterial,
				  name+"_plastic");


  G4Polycone *oil_solid = new G4Polycone(name+"_oil", 0, CLHEP::twopi, n, z, r_inner_oil,
					 r_oil_plastic_boundary);
  oil_logi = new G4LogicalVolume(oil_solid,
				 outerMaterial,
				 name+"_oil");

  G4ThreeVector no_offset(0, 0, 0);


  G4PVPlacement* oil_phys= new G4PVPlacement
    ( 0,                   // no rotation
      no_offset,       // puts face equator in right place
      oil_logi,                    // the logical volume
      name+"_oil",         // a name for this physical volume
      this,           // the mother volume
      false,               // no boolean ops
      0 );                 // copy number

  G4PVPlacement* conc_phys= new G4PVPlacement
    ( 0,                   // no rotation
      no_offset,       // puts face equator in right place
      conc_logi,                    // the logical volume
      name+"_plastic",         // a name for this physical volume
      this,           // the mother volume
      false,               // no boolean ops
      0 );                 // copy number

  new G4LogicalBorderSurface(name+"_conc_surface1",
			     oil_phys, conc_phys, reflective_surf);
  new G4LogicalBorderSurface(name+"_conc_surface2",
			     conc_phys, oil_phys, reflective_surf);

}

double PMTConcentrator::GetHeight()
{
  return z_from_r(upper_radius) - z_from_r(lower_radius);
}

void PMTConcentrator::SetInvisible()
{
  this->SetVisAttributes(G4VisAttributes::Invisible);
  conc_logi->SetVisAttributes(G4VisAttributes::Invisible);
  oil_logi->SetVisAttributes(G4VisAttributes::Invisible);
}

double PMTConcentrator::z_from_r (double r)
{
  return -sqrt( 1 - ((r-d)/a)*((r-d)/a) )*b;
}

double PMTConcentrator::r_from_z (double z)
{
  return d + a*sqrt( 1 - (z/b)*(z/b) );
}


} // namespace RAT
