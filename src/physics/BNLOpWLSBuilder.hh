/**
 * @class BNLOpWLSBuilder
 *
 * A custom wavelength-shifting process developed by L. Bignell at BNL.
 *
 * @author L. Bignell, BNL
 */
#ifndef __BNLOpWLSBuilder__
#define __BNLOpWLSBuilder__

#include <G4VPhysicsConstructor.hh>

class BNLOpWLSBuilder : public G4VPhysicsConstructor {
public:
  BNLOpWLSBuilder() {}
  virtual ~BNLOpWLSBuilder() {}

  // To instantiate desired Particles
  void ConstructParticle();

  // To instantiate desired Processes
  void ConstructProcess();
};

#endif  // __BNLOpWLSBuilder__

