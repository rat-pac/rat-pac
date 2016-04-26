/**
 * @class G4OpWLSBuilder
 *
 * @author L. Bignell, BNL
 */
#ifndef __G4OpWLSBuilder__
#define __G4OpWLSBuilder__ 

#include <G4VPhysicsConstructor.hh>

class G4OpWLSBuilder : public G4VPhysicsConstructor {
public:
  G4OpWLSBuilder() {}
  virtual ~G4OpWLSBuilder() {}

  // To instantiate desired Particles
  void ConstructParticle();

  // To instantiate desired Processes
  void ConstructProcess();

};

#endif  // __G4OpWLSBuilder__

