#include "AltWLSbuilder.hh"

#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
#include "G4LossTableManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

AltWLSbuilder::AltWLSbuilder()
{}

AltWLSbuilder::~AltWLSbuilder()
{}

#include "G4OpticalPhoton.hh"
#include "RAT/OpWLS.hh"
#include "AltOpWLS.hh"

void AltWLSbuilder::ConstructParticle()
{
  // optical photon
  G4OpticalPhoton::OpticalPhotonDefinition();
}

void AltWLSbuilder::ConstructProcess()
{

  AltOpWLS* theAltWLSProcess = new AltOpWLS();;

  //pre-load the Ex/Em data
  char* DataPath = getenv("GLG4DATA");
  if(DataPath!=NULL){
    string thePath = string(DataPath) + "/ExEmMatrix.root";
    theAltWLSProcess->SetExEmData(thePath);
  }
  else{
    G4cout << "Warning: Could not find Ex/Em data path for AltOPWLS model"
 	     << G4endl;
  }


  if (verboseLevel > 0) {
    theAltWLSProcess->DumpInfo();
  }

  theAltWLSProcess->SetVerboseLevel(0);

  G4ProcessManager* pManager = 0;
  pManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
  if(!pManager){
 cout << "ERROR: AltWLSbuilder couldn't find the Optical photon process manager"
	 << endl;
    return;
  }
  
  theAltWLSProcess->UseTimeProfile("exponential");
  pManager->AddDiscreteProcess(theAltWLSProcess);
  
}



