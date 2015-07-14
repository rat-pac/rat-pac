#ifndef __RAT_VertexGen_ReacIBD__
#define __RAT_VertexGen_ReacIBD__

#include <RAT/GLG4VertexGen.hh>
#include <RAT/ReacIBDgen.hh>

/** vertex generator that can generate the products of a inverse
    beta-decay reaction from a reactor anti-neutrino.  The direction
    of the neutrino is supplied, and the energy and angle of the
    produced positron and neutron are drawn from the distribution
    produced by the differential cross-section and a reactor anti-neutrino
    energy spectrum.
*/

namespace RAT {

class VertexGen_ReacIBD : public GLG4VertexGen {
public:
  VertexGen_ReacIBD(const char *arg_dbname="ibd");
  virtual ~VertexGen_ReacIBD();
  virtual void GeneratePrimaryVertex( G4Event *argEvent,
				      G4ThreeVector &dx,
				      G4double dt);
  // generates a primary vertex with given particle type, direction, energy,
  // and consistent polarization.

  virtual void SetState( G4String newValues );
  // format: dir_x dir_y dir_z
  // If dir_x==dir_y==dir_z==0, the directions are isotropic.

  virtual G4String GetState();
  // returns current state formatted as above

private:
  G4ParticleDefinition *nu, *n, *eplus;
  ReacIBDgen reacibd;
  G4ThreeVector nu_dir;
};


} // namespace RAT

#endif
