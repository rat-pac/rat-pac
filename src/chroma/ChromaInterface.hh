#ifndef __CHROMA_INTERFACE__
#define __CHROMA_INTERFACE__

#include <vector>
#include <string>

#include "ratchromadata.pb.h"

//#include "zhelpers.hpp"

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
  zmq::socket_t * S_Client_Socket (zmq::context_t & context);

protected:
  zmq::socket_t *client;
  zmq::context_t *context;
  ratchroma::ChromaData message;
  std::string fStrQueueAddress;
  std::string ClientIdentity;
};

}// end of RAT namespace

#endif
