// Created by Daniel Hellfeld (07/2015)
// Directly follows from VertexGen_ES.hh

#ifndef __RAT_VertexGen_ReactorES__
#define __RAT_VertexGen_ReactorES__

#include <RAT/ReactorESgen.hh>
#include <RAT/GLG4VertexGen.hh>

#include <G4Event.hh>
#include <G4ThreeVector.hh>
#include <globals.hh>

namespace RAT {

  class VertexGen_ReactorES : public GLG4VertexGen {
  public:

    // Note that the database named is "ibd" by default in the
    // constructor.  In other words, we assume the anti-neutrino flux
    // is the same for both inverse beta-decay (IBD) and elastic
    // scattering (ES)... at least for now.

    VertexGen_ReactorES(const char* arg_dbname="ibd");
    virtual ~VertexGen_ReactorES();
     
    // generates a primary vertex with given particle type, direction, and energy.
    virtual void GeneratePrimaryVertex(G4Event* argEvent, G4ThreeVector& dx, G4double dt);
    
    // format: dir_x dir_y dir_z
    // If dir_x==dir_y==dir_z==0, the directions are isotropic.
    virtual void SetState(G4String newValues);
    
    // returns current state formatted as above
    virtual G4String GetState();

  private:
    G4ParticleDefinition* electron;
    G4ThreeVector nu_dir;

    // Separate class to generate the elastic-scattering event.
    // Concrete definition; will invoke the ESgen() constructor.
    ReactorESgen reactoresgen;

    // Electron mass
    G4double m_electron;
  };

} // namespace RAT

#endif
