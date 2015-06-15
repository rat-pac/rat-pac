#ifndef __CHROMA_INTERFACE__
#define __CHROMA_INTERFACE__

class ChromaInterface {

  ChromaInterface();
  ~ChromaInterface();

  void initializeServerConnection();
  void closeServerConnection();

  void readStoreKillCherenkovPhotons();

};


#endif
