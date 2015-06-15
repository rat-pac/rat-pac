#include "ChromaInterface.hh"

#include "G4Track.hh"
#include "G4VProcess.hh"

ChromaInterface::ChromaInterface() {
}


ChromaInterface::~ChromaInterface() {
}

void ChromaInterface::initializeServerConnection() {}

void ChromaInterface::closeServerConnection() {}

void ChromaInterface::readStoreKillCherenkovPhotons( const std::vector< const G4Track* >& secondaries ) {

  for (  std::vector< const G4Track* >::const_iterator it=secondaries.begin(); it!=secondaries.end(); it++ ) {
    if ( (*it)->GetParticleDefinition()->GetParticleName()=="opticalphoton" && (*it)->GetCreatorProcess()->GetProcessName()=="cerenkov" ) {
      std::cout << "  " << (*it)->GetParticleDefinition()->GetParticleName() << " " << (*it)->GetCreatorProcess()->GetProcessName() << std::endl;
    }
  }

}
