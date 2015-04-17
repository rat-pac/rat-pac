#include "G4WLSbuilder.hh"

#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
#include "G4LossTableManager.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

G4WLSbuilder::G4WLSbuilder()
{}

G4WLSbuilder::~G4WLSbuilder()
{}

#include "G4OpticalPhoton.hh"
#include "RAT/OpWLS.hh"
#include "AltOpWLS.hh"

void G4WLSbuilder::ConstructParticle()
{
  // optical photon
  G4OpticalPhoton::OpticalPhotonDefinition();
}

void G4WLSbuilder::ConstructProcess()
{

  RAT::OpWLS* theWLSProcess;
  theWLSProcess = new RAT::OpWLS();

  if (verboseLevel > 0) {
    theWLSProcess->DumpInfo();
  }

  theWLSProcess->SetVerboseLevel(0);

  G4ProcessManager * pManager = 0;
  pManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
  if(!pManager){
    cout << "ERROR: G4WLSbuilder couldn't find the Optical photon process manager"
	 << endl;
    return;
  }

  theWLSProcess->UseTimeProfile("exponential");
  pManager->AddDiscreteProcess(theWLSProcess);

}



