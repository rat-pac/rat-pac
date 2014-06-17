// This file is part of the GenericLAND software library.
// $Id: GLG4DeferTrackProc.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
// Process to limit step length to stay within event time
// and defer long tracks (AND tracks which start after event time) using
// defered particle "generator".
//
// Written: G. Horton-Smith, 29-Oct-2001
//
#ifndef GLG4DeferTrackProc_h
#define GLG4DeferTrackProc_h 1

#include "G4ios.hh"
#include "globals.hh"
#include "G4VProcess.hh"

////////////////////////////////////////////////////////////////

class GLG4PrimaryGeneratorAction;
class G4HadronCaptureProcess;

class GLG4DeferTrackProc : public G4VProcess 
{
 public:  //with description     

  GLG4DeferTrackProc(const G4String& processName="DeferTrackProc");

  ~GLG4DeferTrackProc();

  virtual G4double PostStepGetPhysicalInteractionLength(
                             const G4Track& track,
			     G4double   previousStepSize,
			     G4ForceCondition* condition
			    );

  virtual G4VParticleChange* PostStepDoIt(
			     const G4Track& ,
			     const G4Step& 
			    );
			    
 public:  //without description 
     			    
     //  no operation in  AtRestGPIL
     virtual G4double AtRestGetPhysicalInteractionLength(
                             const G4Track& ,
			     G4ForceCondition* 
			    ){ return -1.0; };
			    
     //  no operation in  AtRestDoIt      
     virtual G4VParticleChange* AtRestDoIt(
			     const G4Track& ,
			     const G4Step&
			    ){return NULL;};

     //  no operation in  AlongStepGPIL
     virtual G4double AlongStepGetPhysicalInteractionLength(
                             const G4Track&,
			     G4double  ,
			     G4double  ,
			     G4double& ,
                             G4GPILSelection*
			    ){ return -1.0; };

     //  no operation in  AlongStepDoIt
     virtual G4VParticleChange* AlongStepDoIt(
			     const G4Track& ,
			     const G4Step& 
			    ) {return NULL;};

 private:
  
  // hide assignment operator as private 
      GLG4DeferTrackProc(GLG4DeferTrackProc&);
      GLG4DeferTrackProc& operator=(const GLG4DeferTrackProc& right);

 private:
  GLG4PrimaryGeneratorAction* _generator;
};

#endif

