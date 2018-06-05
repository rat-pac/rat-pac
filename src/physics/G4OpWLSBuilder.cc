#include <G4OpWLS.hh>
#include <G4ProcessManager.hh>
#include <G4OpticalPhoton.hh>
#include <RAT/G4OpWLSBuilder.hh>
#include <RAT/Log.hh>

void G4OpWLSBuilder::ConstructParticle() {
  G4OpticalPhoton::OpticalPhotonDefinition();
}

void G4OpWLSBuilder::ConstructProcess() {
  G4OpWLS* theWLSProcess = new G4OpWLS();

  if (verboseLevel > 0) {
    theWLSProcess->DumpInfo();
  }

  theWLSProcess->SetVerboseLevel(0);

  G4ProcessManager* pManager = NULL;
  pManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
  if (!pManager) {
    RAT::warn << "G4OpWLSBuilder::ConstructProcess: Error: Could not find the "
                 "Optical photon process manager" << newline;
    return;
  }

  theWLSProcess->UseTimeProfile("exponential");
  pManager->AddDiscreteProcess(theWLSProcess);
}

