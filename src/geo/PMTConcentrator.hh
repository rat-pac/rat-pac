#ifndef __RAT_PMTConcentrator__
#define __RAT_PMTConcentrator__

#include <G4LogicalVolume.hh>
#include <G4Polycone.hh>
#include <G4Material.hh>
#include <G4OpticalSurface.hh>

namespace RAT {


class PMTConcentrator : public G4LogicalVolume {
public:
  PMTConcentrator(const G4String &name,
		     double upper_radius,
		     double lower_radius,
		     double a, // SEE Mike Lay's Thesis
		     double b,
		     double d,
		     G4Material *outerMaterial,
		     G4Material *bulkMaterial,
		     G4OpticalSurface *reflective_surf);
  double GetHeight();
  void SetInvisible();

protected:
  G4LogicalVolume *conc_logi;
  G4LogicalVolume *oil_logi;

  G4Polycone *concentrator;
  
  double a, b, d, upper_radius, lower_radius;

  double z_from_r (double r);
  double r_from_z (double z);
    
};


} // namespace RAT

#endif
