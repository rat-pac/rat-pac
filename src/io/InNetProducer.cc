#include <RAT/InNetProducer.hh>
#include <RAT/ProcBlock.hh>
#include <RAT/DS/Root.hh>
#include <RAT/SignalHandler.hh>
#include <RAT/Log.hh>
#include <RAT/Config.hh>

#include <G4UIdirectory.hh>
#include <G4UIcmdWithAnInteger.hh>

#include <TServerSocket.h>
#include <TSocket.h>
#include <TMonitor.h>
#include <TMessage.h>
#include <TStopwatch.h>
#include <TSystem.h>

#include <list>

namespace RAT {


InNetProducer::InNetProducer()
{
    mainBlock = 0;
    Init();
}

InNetProducer::InNetProducer(ProcBlock *block)
{
    SetMainBlock(block);
    Init();
}
    
InNetProducer::~InNetProducer()
{
}

void InNetProducer::Init()
{
    // Build commands
    G4UIdirectory* DebugDir = new G4UIdirectory("/rat/innet/");
    DebugDir->SetGuidance("Read DS::Root from Network");
    
    // info message command
    fListenCmd = new G4UIcmdWithAnInteger("/rat/innet/listen", this);
    fListenCmd->SetGuidance("port number to accept connections on");
    fListenCmd->SetParameterName("port", false);  // required
}

G4String InNetProducer::GetCurrentValue(G4UIcommand * /*command*/)
{
    Log::Die("invalid innet \"get\" command");
    return G4String("You should never see this.");
}


void InNetProducer::SetNewValue(G4UIcommand * command, G4String newValue)
{
    if (command == fListenCmd) {
	int port = G4UIcmdWithAnInteger::GetNewIntValue(newValue);
	if (!mainBlock)
	    Log::Die("innet: No main block declared! (should never happen)");
	else if (!Listen(port, 0 /* read events forever */))
	    Log::Die(dformat("innet: Error listening on port %d ", port));
    } else
	Log::Die("invalid innet \"set\" command");
}
    
bool InNetProducer::Listen(int port, int event_limit)
{
    TServerSocket *srv = new TServerSocket(port);
    if (!srv->IsValid()) {
	delete srv;
	return false;
    }
    srv->SetOption(kNoBlock, 1);
    
    TMonitor monitor;
    std::list <TSocket *> sockets;
    int event_count = 0;
    bool error = false;
    TStopwatch lastEvent;
    lastEvent.Start();

    info << "InNet: Listening on port " << port << ".\n";

    while ( !error && (event_limit == 0 || event_count < event_limit)
	    && !SignalHandler::IsTermRequested() ) {
	
	TSocket *newsock = srv->Accept();
	if (newsock == 0) {
	    error = true;
	    continue;
	} else if ((void *)newsock != (void *) -1) {
	    // -1 returned by Accept() if no new connection to accept
	    // BTW: Whoever thought that returning -1 in a pointer
	    // was an acceptable way to flag a special return condition
	    // should be flogged with the C++ standard.
	    
	    newsock->SetOption(kNoBlock, 0);

	    // Send initial header info
	    TMessage header;
	    header.WriteFastArrayString(RATVERSION.c_str(),RATVERSION.length());
	    // Use number of clients as our load measure
	    double load = sockets.size();
	    header.WriteDouble(load);

	    if (newsock->Send(header) == -1) {
		warn << "innet: Unable to send header to " 
		     << newsock->GetInetAddress().GetHostName()
		     << "\n";
		delete newsock;
	    } else {
		info << "innet: Connection from " << newsock->GetInetAddress().GetHostName()
		     << "\n";
		sockets.push_back(newsock);
		monitor.Add(newsock, TMonitor::kRead);
	    }
	}
	
	TSocket *sockWithData = monitor.Select(50 /* wait up to 50 ms */);
	
	if (sockWithData != 0 && (void *) sockWithData != (void *) -1) {
	    TMessage *msg;
	    Int_t read_result = sockWithData->Recv(msg);
	    if (read_result == 0) {
		// Other side closed connection
		warn << "innet: Host " << sockWithData->GetInetAddress().GetHostName()
		     << " has disconnected\n";
		monitor.Remove(sockWithData);
		sockets.remove(sockWithData);
		sockWithData->Close();
		delete sockWithData;	
	    } else if (read_result == -1) {
		// Real error
		error = true;
	    } else if (read_result == -4) {
		// Socket has no data to read.  How'd this happen?
		warn << "innet: Socket selected which has no data.\n";
	    } else {
		lastEvent.Start(); // restart timer since we got another event

		// Stuff event through event loop
		DS::Root *ds = dynamic_cast<DS::Root *>
		    (msg->ReadObject(DS::Root::Class()));
		
		if (ds == 0)
		    error = true;
		else {
		    mainBlock->DSEvent(ds);
		    event_count++;
		}
		
		
		// Send updated event back to client
		if (sockWithData->SendObject(ds) == -1) {
		    
		    // Failed to send, shut down the connection
		    warn << "innet: Cannot send event to " 
			 << sockWithData->GetInetAddress().GetHostName()
			 << newline;
		    warn << "Closing socket.\n";
		    monitor.Remove(sockWithData);
		    sockets.remove(sockWithData);
		    sockWithData->Close();
		}
		
		delete ds;
	    }
	    
	    delete msg;
	}

	// TMonitor performs a busy wait for socket data, which wastes
	// power if nothing is going on.  If we haven't gotten an
	// event for 30 seconds, go into a low power mode by sleeping
	// for 500 ms out of every 50 ms.  Once an event comes in we
	// will go back to busy wait.
	if (lastEvent.RealTime() > 30.0 /*sec*/)
	    gSystem->Sleep(500 /*msec*/);
	lastEvent.Continue(); // querying the stopwatch stops it
    }
    
    // Done, now free up sockets
    std::list<TSocket *>::iterator s;
    for (s = sockets.begin(); s != sockets.end(); s++)
	delete *s;
    
    delete srv;
    
    return !error;
}
    

} // namespace RAT
