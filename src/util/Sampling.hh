#ifndef __RAT_Sampling__
#define __RAT_Sampling__

#include <G4MaterialPropertyVector.hh>
#include <G4PhysicsOrderedFreeVector.hh>

namespace RAT {

  /** Convert a photon spectrum in a material property vector into
      a cumulative distribution for random sampling.

      With the resulting G4PhysicsOrderedFreeVector, the recipe
      for obtaining a random sample is:
      
      G4PhysicsOrderedFreeVector *pofv = [your vector];
      double integralValue = G4UniformRand()*pofv->GetMaxValue();
      double sample = pofv->GetEnergy(integralValue);
   */
  G4PhysicsOrderedFreeVector*
  Integrate_MPV_to_POFV( G4MaterialPropertyVector* inputVector );

} // namespace RAT

#endif
