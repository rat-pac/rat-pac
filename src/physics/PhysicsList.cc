#include <string>
#include <stdexcept>
#include <Shielding.hh>
#include <G4FastSimulationManagerProcess.hh>
#include <G4OpticalPhoton.hh>
#include <G4ParticleDefinition.hh>
#include <G4ProcessManager.hh>
#include <G4Cerenkov.hh>
#include <G4OpBoundaryProcess.hh>
#include <G4VUserPhysicsList.hh>
#include <G4RunManager.hh>
#include <RAT/GLG4OpAttenuation.hh>
#include <RAT/GLG4Scint.hh>
#include <RAT/GLG4SteppingAction.hh>
#include <RAT/G4OpWLSBuilder.hh>
#include <RAT/BNLOpWLSBuilder.hh>
#include <RAT/PhysicsListMessenger.hh>
#include <RAT/PhysicsList.hh>

namespace RAT {

PhysicsList::PhysicsList() : Shielding(), wlsModel(NULL) {
  new PhysicsListMessenger(this);
}

PhysicsList::~PhysicsList() {}

void PhysicsList::ConstructParticle() {
  Shielding::ConstructParticle();
  G4OpticalPhoton::OpticalPhotonDefinition();
}

void PhysicsList::ConstructProcess() {
  AddParameterization();
  Shielding::ConstructProcess();
  ConstructOpticalProcesses();
}

void PhysicsList::SetOpWLSModel(std::string model) {
  this->wlsModelName = model;
  delete this->wlsModel;

  if (model == "g4") {
    this->wlsModel = new G4OpWLSBuilder();
  }
  else if (model == "bnl") {
    this->wlsModel = new BNLOpWLSBuilder();
  }
  else {
    std::cerr << "PhysicsList::SetOpWLSModel: Unknown model \""
              << model << "\"" << std::endl;
    throw std::runtime_error("Unknown WLS model in PhysicsList");
  }

  std::cout << "PhysicsList::SetOpWLSModel: Set WLS model to \""
            << model << "\"" << std::endl;

  G4RunManager::GetRunManager()->PhysicsHasBeenModified();
}

void PhysicsList::ConstructOpticalProcesses() {
  // Cherenkov: default G4Cerenkov
  //
  // Request that Cerenkov photons be tracked first, before continuing
  // originating particle step.  Otherwise, we get too many secondaries!
  G4Cerenkov* cerenkovProcess = new G4Cerenkov();
  cerenkovProcess->SetTrackSecondariesFirst(true);
  cerenkovProcess->SetMaxNumPhotonsPerStep(4);

  // Attenuation: RAT's GLG4OpAttenuation
  //
  // GLG4OpAttenuation implements Rayleigh scattering.
  GLG4OpAttenuation* attenuationProcess = new GLG4OpAttenuation();

  // Scintillation: RAT's GLG4Scint
  //
  // Create three scintillation processes which depend on the mass.
  G4double protonMass = G4Proton::Proton()->GetPDGMass();
  G4double alphaMass = G4Alpha::Alpha()->GetPDGMass();
  GLG4Scint* defaultScintProcess = new GLG4Scint();
  GLG4Scint* nucleonScintProcess = new GLG4Scint("nucleon", 0.9 * protonMass);
  GLG4Scint* alphaScintProcess = new GLG4Scint("alpha", 0.9 * alphaMass);

  // Optical boundary processes: default G4
  G4OpBoundaryProcess* opBoundaryProcess = new G4OpBoundaryProcess();

  // Wavelength shifting: User-selectable via PhysicsListMessenger
  if (this->wlsModel) {
    wlsModel->ConstructProcess();
  }

  // Set verbosity
  if (verboseLevel > 0) {
    cerenkovProcess->DumpInfo();
    attenuationProcess->DumpInfo();
    defaultScintProcess->DumpInfo();
    nucleonScintProcess->DumpInfo();
    alphaScintProcess->DumpInfo();
    opBoundaryProcess->DumpInfo();
  }

  cerenkovProcess->SetVerboseLevel(verboseLevel-1);
  attenuationProcess->SetVerboseLevel(verboseLevel-1);
  defaultScintProcess->SetVerboseLevel(verboseLevel-1);
  nucleonScintProcess->SetVerboseLevel(verboseLevel-1);
  alphaScintProcess->SetVerboseLevel(verboseLevel-1);
  opBoundaryProcess->SetVerboseLevel(verboseLevel-1);

  // Apply processes to all particles where applicable
  GetParticleIterator()->reset();
  while((*GetParticleIterator())()) {
    G4ParticleDefinition* particle = GetParticleIterator()->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
    if (cerenkovProcess->IsApplicable(*particle)) {
      pmanager->AddProcess(cerenkovProcess);
      pmanager->SetProcessOrdering(cerenkovProcess, idxPostStep);
    }
    if (particleName == "opticalphoton") {
      pmanager->AddDiscreteProcess(attenuationProcess);
      pmanager->AddDiscreteProcess(opBoundaryProcess);
    }
  }
}

void PhysicsList::AddParameterization() {
  G4FastSimulationManagerProcess* fastSimulationManagerProcess =
    new G4FastSimulationManagerProcess();
  GetParticleIterator()->reset();
  while((*GetParticleIterator())()) {
    G4ParticleDefinition* particle = GetParticleIterator()->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    if (particle->GetParticleName() == "opticalphoton") {
      pmanager->AddProcess(fastSimulationManagerProcess, -1, -1, 1);
    }
  }
}

}  // namespace RAT
