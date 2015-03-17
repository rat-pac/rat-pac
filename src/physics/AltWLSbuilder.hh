#ifndef __AltWLSbuilder_hh__
#define __AltWLSbuilder_hh__ 1

#include "G4VPhysicsConstructor.hh"

class AltWLSbuilder : public G4VPhysicsConstructor
{
public:
  AltWLSbuilder();  // constructor
  ~AltWLSbuilder(); // destructor

  void ConstructParticle(); // to instantiate desired Particle's

  void ConstructProcess(); // to instantiate desired Process'es

};

#endif
