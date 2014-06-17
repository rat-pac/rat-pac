#ifndef __RAT_VertexGen_ES__
#define __RAT_VertexGen_ES__

#include <RAT/ESgen.hh>
#include <RAT/GLG4VertexGen.hh>

#include <G4Event.hh>
#include <G4ThreeVector.hh>
#include <globals.hh>

// Calculates the cross-section for neutrino-elastic scattering  
// as function of neutrino energy and the electron's recoil energy.
// Allow for variations in the weak mixing angle and the possibility 
// of a neutrino magnetic moment
//
// J. Formaggio (UW) -02/09/2005

// Converted to Geant4+GLG4Sim+RAT by Bill Seligman (21-Jan-2006).

namespace RAT {

  class VertexGen_ES : public GLG4VertexGen {
  public:

    // Note that the database named is "ibd" by default in the
    // constructor.  In other words, we assume the anti-neutrino flux
    // is the same for both inverse beta-decay (IBD) and elastic
    // scattering (ES)... at least for now.

    VertexGen_ES(const char* arg_dbname="ibd");
    virtual ~VertexGen_ES();
    virtual void GeneratePrimaryVertex( G4Event* argEvent,
					G4ThreeVector& dx,
					G4double dt);
    // generates a primary vertex with given particle type, direction, and energy.
    virtual void SetState( G4String newValues );
    // format: dir_x dir_y dir_z
    // If dir_x==dir_y==dir_z==0, the directions are isotropic.
    virtual G4String GetState();
    // returns current state formatted as above

  private:
    G4ParticleDefinition* electron;
    G4ThreeVector nu_dir;

    // Separate class to generate the elastic-scattering event.
    // Concrete definition; will invoke the ESgen() constructor.
    ESgen esgen;

    // Electron mass
    G4double m_electron;
  };

} // namespace RAT

#endif
