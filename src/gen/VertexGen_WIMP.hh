#ifndef __RAT_VertexGen_WIMP__
#define __RAT_VertexGen_WIMP__

#include <RAT/GLG4VertexGen.hh>
#include <CLHEP/Random/RandGeneral.h>

namespace RAT {

class VertexGen_WIMP : public GLG4VertexGen {
public:
  VertexGen_WIMP(const char *arg_dbname="wimp");
  virtual ~VertexGen_WIMP();
  virtual void GeneratePrimaryVertex( G4Event *argEvent,
                                      G4ThreeVector &dx,
                                      G4double dt);
  /** State format "nucleus_name WIMP_mass_in_GeV" */
  virtual void SetState( G4String newValues );
  virtual G4String GetState();
  double Helmff(double E,double A);
  double VelIntegral(double vmin,double v0,double vE, double vesc);

protected:
  void Setup();

  std::string fNucleusName;
  G4ParticleDefinition *fNucleus;
  double fWIMPMass; // GeV

  CLHEP::RandGeneral *fEnergyDist;
  double fEnergyDistLo, fEnergyDistHi;
};


} // namespace RAT

#endif
