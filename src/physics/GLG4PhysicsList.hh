// This file is part of the GenericLAND software library.
// $Id: GLG4PhysicsList.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
// GLG4PhysicsList.hh by Glenn Horton-Smith, Feb. 1999
#ifndef __GLG4PhysicsList_hh__
#define __GLG4PhysicsList_hh__ 1

#include "G4VModularPhysicsList.hh"

class GLG4PhysicsList : public G4VModularPhysicsList
{
public:
  GLG4PhysicsList();  // constructor
  ~GLG4PhysicsList(); // destructor

  void ConstructParticle(); // to instantiate desired Particle's

  void ConstructProcess(); // to instantiate desired Process'es

  void SetCuts();          // to set the default cut values for all particles

  void SetAltOpWLS(bool);
  bool GetAltOpWLS();

private:
  G4double cutForGamma;
  G4double cutForElectron;
  G4double cutForPositron;
  G4double currentDefaultCut;
  bool useAltOpWLS;
  G4VPhysicsConstructor* WLSmodel;

  // these methods Construct physics processes and register them
    virtual void AddParameterisation();
    virtual void ConstructGeneral();
    virtual void ConstructHadronic();
    virtual void ConstructEM();
    virtual void ConstructOp();
  //virtual void ConstructWLS();
};

#endif
