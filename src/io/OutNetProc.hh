#ifndef __RAT_OutNetProc__
#define __RAT_OutNetProc__

#include <RAT/Processor.hh>
#include <globals.hh>
#include <vector>

#include <TSocket.h>

namespace RAT {

class OutNetProc : public Processor {
public:
  OutNetProc();
  virtual ~OutNetProc();

  virtual Processor::Result DSEvent(DS::Root *ds);
  
  // host - string, hostname and port in the form "hostname:port"
  virtual void SetS(std::string param, std::string value);

  // versioncheck - int
  // 0 = ignore SVN version of server
  // 1 = verify SVN version of server matches client (default)
  // Must set this option before host option, otherwise 
  // default is assumed
  virtual void SetI(std::string param, int value);


protected:
  
  void PickServer(std::vector<std::string> &hostlist);
  TSocket *Connect(std::string host, int port, double &load, int &svnVersion);


  TSocket *fSocket;
  bool fVersionCheck;
  std::vector<std::string> fHostlist;  
};


} // namespace RAT

#endif
