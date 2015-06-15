#ifndef __CHROMA_INTERFACE__
#define __CHROMA_INTERFACE__

#include <vector>
#include "ratchromadata.pb.h"

class G4Track;

class ChromaInterface {

public:

  ChromaInterface();
  ~ChromaInterface();

  void initializeServerConnection();
  void closeServerConnection();

  void readStoreKillCherenkovPhotons( const std::vector< const G4Track* >& secondaries );

protected:

  ratchroma::ChromaData message;

};


#endif
