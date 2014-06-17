#ifndef GLG4NeutronDiffusionAndCapture_h
#define GLG4NeutronDiffusionAndCapture_h 1
/** @file
    Declares GLG4NeutronDiffusionAndCapture class.
    
    This file is part of the GenericLAND software library.
    $Id: GLG4NeutronDiffusionAndCapture.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $

    @author Glenn Horton-Smith, 2000 (Tohoku, Caltech), 2004 (KSU).
*/


#include "G4ios.hh"
#include "globals.hh"
#include "G4VProcess.hh"

////////////////////////////////////////////////////////////////

class GLG4PrimaryGeneratorAction;
class G4HadronCaptureProcess;

/** Special process that applies simple neutron diffusion to
    epithermal neutrons, then calls neutron decay/capture processes
    "PostStepDoIt" to get decay/capture products.
    This process should be deactivated by default because it is unexpected
    to new users.
    @author Glenn Horton-Smith
*/
class GLG4NeutronDiffusionAndCapture : public G4VProcess 
{
 public:  //with description     

  GLG4NeutronDiffusionAndCapture(const G4String& processName="NeutronDiffusionAndCapture");

  ~GLG4NeutronDiffusionAndCapture();

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
      GLG4NeutronDiffusionAndCapture(GLG4NeutronDiffusionAndCapture&);
      GLG4NeutronDiffusionAndCapture& operator=(const GLG4NeutronDiffusionAndCapture& right);

 private:
  GLG4PrimaryGeneratorAction* _generator;
  G4HadronCaptureProcess* _neutronCaptureProcess;
};

#endif

