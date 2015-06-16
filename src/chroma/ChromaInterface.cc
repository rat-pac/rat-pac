#include "ChromaInterface.hh"

#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4Step.hh"
#include "G4VParticleChange.hh"
#include "G4StepPoint.hh"

namespace RAT {

  ChromaInterface::ChromaInterface() {
    GOOGLE_PROTOBUF_VERIFY_VERSION; // checks protobuf version
    message.Clear();
  }


  ChromaInterface::~ChromaInterface() {
  }

  void ChromaInterface::initializeServerConnection() {}

  void ChromaInterface::closeServerConnection() {}

  void ChromaInterface::readStoreKillCherenkovPhotons( std::vector< G4Track* >* secondaries ) {
  

    for (  std::vector< G4Track* >::const_iterator it=secondaries->begin(); it!=secondaries->end(); it++ ) {

      if ( (*it)->GetParticleDefinition()->GetParticleName()=="opticalphoton" && (*it)->GetCreatorProcess()->GetProcessName()=="cerenkov" ) {
	std::cout << "  " << (*it)->GetParticleDefinition()->GetParticleName() << " " << (*it)->GetCreatorProcess()->GetProcessName() << std::endl;
	// create new data
	ratchroma::CherenkovPhoton* cerenkov = message.add_cherekovdata();
	cerenkov->set_x( (*it)->GetVertexPosition().x() );
	cerenkov->set_y( (*it)->GetVertexPosition().y() );
	cerenkov->set_z( (*it)->GetVertexPosition().z() );
	cerenkov->set_t( (*it)->GetGlobalTime() );
	cerenkov->set_dx( (*it)->GetVertexMomentumDirection().x() );
	cerenkov->set_dy( (*it)->GetVertexMomentumDirection().y() );
	cerenkov->set_dz( (*it)->GetVertexMomentumDirection().z() );
	double ke = (*it)->GetVertexKineticEnergy(); // MeV
	double wavelength = ke; // MeV -> nanometers
	cerenkov->set_wavelength( wavelength );
	cerenkov->set_px( (*it)->GetPolarization().x() );
	(*it)->SetTrackStatus( fStopAndKill ); // we take this over now!
      }

    
    }

  }

  void ChromaInterface::readStoreKillScintillationPhotons( const G4Step* aStep, G4VParticleChange* scint_photons ) {

    G4int iSecondary= scint_photons->GetNumberOfSecondaries();
    if ( iSecondary==0 )
      return;

    ratchroma::ScintStep* scintinfo = message.add_stepdata();
    G4StepPoint* prestep = aStep->GetPreStepPoint();
    G4StepPoint* poststep = aStep->GetPostStepPoint();

    scintinfo->set_nphotons( (int)iSecondary );
    scintinfo->set_step_start_x( prestep->GetPosition().x() );
    scintinfo->set_step_start_y( prestep->GetPosition().y() );
    scintinfo->set_step_start_z( prestep->GetPosition().z() );
    scintinfo->set_step_end_x( poststep->GetPosition().x() );
    scintinfo->set_step_end_y( poststep->GetPosition().y() );
    scintinfo->set_step_end_z( poststep->GetPosition().z() );
    scintinfo->set_material( prestep->GetMaterial()->GetName() );
    scint_photons->Clear();
  }

}// end of namespace RAT
