#ifndef __RAT_IBDgen__
#define __RAT_IBDgen__

#include <RAT/LinearInterp.hh>
#include <CLHEP/Random/Randomize.h>
#include <G4ThreeVector.hh>
#include <G4LorentzVector.hh>

namespace RAT {

// Generate inverse beta decay event
class IBDgen {
public:
  IBDgen();
  
  // Generate random event vectors
  //    Pass in the neutrino direction (unit vector)
  //    Returns 4-momentum vectors for neutrino and resulting positron
  //    and neutron.  Neutrino energy and positron direction drawn from
  //    GenInteraction() distribution.
  void GenEvent(const G4ThreeVector &nu_dir,
		G4LorentzVector &neutrino,
		G4LorentzVector &positron,
		G4LorentzVector &neutron);

  // Generate random inverse beta decay interaction
  //
  //   Selects neutrino energy and cos(theta) of the produced
  //   positron relative to neutrino direction.  They are pulled
  //   from 2D distribution of reactor neutrinos which have interacted
  //   with a proton, so both the incident flux, and the (relative) 
  //   differential cross-section are factored in.
  void GenInteraction(float &E, float &CosThetaLab);

  // Differential cross section for inverse beta decay
  static double CrossSection(double Enu, double CosThetaLab);
  
  // Flux as a function of energy.  Interpolated from table in IBD RATDB table
  double Flux(float E) const { return rmpflux(E); };

protected:
  LinearInterp<double> rmpflux;
  double Emax;
  double Emin;
  double XCmax;
  double FluxMax;
};


} // namespace RAT

#endif
