#ifndef __CHROMA_INTERFACE__
#define __CHROMA_INTERFACE__

#include <vector>
#include <string>

#include "ratchromadata.pb.h"

class G4Track;
class G4VParticleChange;
class G4Step;

namespace RAT {
class ChromaInterface {

public:

  ChromaInterface();
  ~ChromaInterface();

  void initializeServerConnection();
  void closeServerConnection();

  void readStoreKillCherenkovPhotons( std::vector< G4Track* >* secondaries );
  void readStoreKillScintillationPhotons( const G4Step* astep, G4VParticleChange* scint_photons );

  void ClearData();

  void SendPhotonData();
  void ReceivePhotonData();
  void SendDetectorConfigData();
  void MakePhotonHitData();

protected:

  ratchroma::ChromaData message;
  std::string fStrQueueAddress;

};

}// end of RAT namespace

#endif
