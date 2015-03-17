#ifndef __G4WLSbuilder_hh__
#define __G4WLSbuilder_hh__ 1

#include "G4VPhysicsConstructor.hh"

class G4WLSbuilder : public G4VPhysicsConstructor
{
public:
  G4WLSbuilder();  // constructor
  ~G4WLSbuilder(); // destructor

  void ConstructParticle(); // to instantiate desired Particle's

  void ConstructProcess(); // to instantiate desired Process'es

};

#endif
