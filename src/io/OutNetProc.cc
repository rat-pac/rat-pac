#include <RAT/OutNetProc.hh>
#include <RAT/Log.hh>
#include <RAT/Config.hh>

#include <vector>
#include <string_utilities.hpp>

#include <TMessage.h>

using namespace std;

namespace RAT {

// utility func
std::string to_string(const TSocket *socket)
{
    return dformat("%s:%d", socket->GetInetAddress().GetHostName(), socket->GetPort());
}

OutNetProc::OutNetProc() : Processor("outnet"), fSocket(0), fVersionCheck(true)
{
}


OutNetProc::~OutNetProc()
{
    delete fSocket;
}

void OutNetProc::SetI(std::string param, int value)
{
    if (param == "versioncheck") {
	if (value == 0 || value == 1)
	    fVersionCheck = (value == 1);
	else
	    throw ParamInvalid(param, dformat("Invalid value: %d", value));
    } else
	throw ParamUnknown(param);
}

void OutNetProc::SetS(std::string param, std::string value)
{
    if (param == "host") {
	fHostlist = split(value, ",");

	PickServer(fHostlist);
    } else
	throw ParamUnknown(param);
}

Processor::Result OutNetProc::DSEvent(DS::Root *ds)
{
    int retries = 5;
    
    // Send event for processing
  send_event:
    while (retries >= 0 && fSocket->SendObject(ds) == -1) {
	warn << "outnet: Cannot send event to " << to_string(fSocket)
	     << ".  Searching for server\n";

	// Find a new server
	PickServer(fHostlist);
	retries--;
    } 

    // Get event back after processing
    TMessage *msg = 0;
    Int_t read_result = fSocket->Recv(msg);

    if (retries >= 0 && read_result <= 0) {
	// Didn't work, we need to reconnect and retry
	if (read_result == 0) {
	    // Other side closed connection
	    warn << "outnet: Host " << to_string(fSocket)
		 << " has disconnected\n";
	} else if (read_result == -1) {
	    warn << "outnet: Read error getting event from server.\n";
	} else if (read_result == -4) {
	    // Socket has no data to read.  How'd this happen?
	    warn << "outnet: Socket has no data.  Odd?\n";
	} 

	delete msg;

	if (retries > 0) {
	    PickServer(fHostlist);
	    retries--;
	    goto send_event; // OMG, I am going to programmer hell.
	}
    }

    
    if (read_result > 0) {
	// It did work! Get message
	// Extract updated DS object from message
	DS::Root *newDS = dynamic_cast<DS::Root *>
	    (msg->ReadObject(DS::Root::Class()));
	
	if (newDS == 0)
	    Log::Die("outnet: Aborting run due to network error.");
	else {
	    // Update our copy of ds
	    *ds = *newDS;
	    delete newDS;
	}
	delete msg;
    } else
	Log::Die("outnet: Aborting run due to network error.");

    return Processor::OK;
}

    
void OutNetProc::PickServer(std::vector<std::string> &hostlist)
{
    double minLoad = 1e9;
    TSocket *minLoadSocket = 0;
    int minLoadGITVersion = 0;

    for (unsigned i=0; i < hostlist.size(); i++) {
	std::vector<std::string> parts = split(hostlist[i], ":");
	if (parts.size() != 2)
	    throw ParamInvalid("host", "host needs two parts - host:port");
	
	std::string host = parts[0];

	// Allow for a port range
	std::vector<std::string> portParts = split(parts[1], "-");
	int minPort, maxPort;

	if (portParts.size() == 1) {
	    // only one port specified
	    minPort = to_int(portParts[0]);
	    maxPort = minPort;
	} else if (portParts.size() == 2) {
	    // inclusive port range specified
	    minPort = to_int(portParts[0]);
	    maxPort = to_int(portParts[1]);
	} else
	    throw ParamInvalid("host", 
			       "port specifier should be integer or min-max range");


	// Try all specified ports on host
	for (int port = minPort; port < maxPort + 1; port++) {

	    double load;
	    int svnVersion;
	    TSocket *socket = Connect(host, port, load, svnVersion);	
	    
	    if (socket && (minLoadSocket == 0 || load < minLoad)) {
		// This is a candidate lowest load server process
		delete minLoadSocket;
		minLoad = load;
		minLoadGITVersion = svnVersion;
		minLoadSocket = socket;
	    } else {
		// This socket is worse than one we already found, close it now
		delete socket;
	    }
	}
    }

    
    if (minLoadSocket) {
	info << "outnet: Using server " << to_string(minLoadSocket) << "\n";
	info << dformat("outnet: Local GIT = %s, Remote GIT = %s, Remote load = %1.2f\n",
			RATVERSION.c_str(), minLoadGITVersion, minLoad);
	fSocket = minLoadSocket;
    } else {
	throw ParamInvalid("host", "Could not locate a suitable server!");
    }
}    
    
TSocket *OutNetProc::Connect(std::string host, int port, double &load, int &svnVersion)
{
    TSocket *socket = new TSocket(host.c_str(), port);
    
    if (socket->IsValid()) {
	// Get initial header
	TMessage *msg  = 0;
	Int_t read_result = socket->Recv(msg);
	
	if (read_result == 0) {
	    warn << "outnet: Host " << to_string(socket)
		 << " has disconnected while receiving header.\n";
	    delete socket;
	    delete msg;
	    return 0;
	} else if (read_result == -1) {
	    // Real error
	    warn << "outnet: Error reading from " << to_string(socket) << "\n";
	    delete socket;
	    delete msg;
	    return 0;
	} else if (read_result == -4) {
	    // Socket has no data to read.  How'd this happen?
	    warn << "outnet: No data read while receiving header from"
		 << to_string(socket) << ".\n";

	    delete socket;
	    delete msg;
	    return 0;
	} else {
	    msg->ReadInt(svnVersion);
	    msg->ReadDouble(load);
	    delete msg;

	    if (fVersionCheck && svnVersion != RATVERSION) {
		warn << "outnet: Host " << to_string(socket)
		     << " is running RAT SVN version " << svnVersion << ".\n";
		warn << "outnet: Does not match local SVN version " << RATVERSION << ".\n";
		warn << "Include /rat/procset versioncheck 0 in your macro to skip this test.\n";
		socket->Close();
		delete socket;
		return 0;
	    }

	    // Actually worked!
	    return socket;
	}
	
    } else {
	warn << "outnet: Could not connect to " << to_string(socket) << "\n";
	return 0;
    }
}


} // namespace RAT
