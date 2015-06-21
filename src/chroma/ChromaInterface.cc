#include "ChromaInterface.hh"

#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4Step.hh"
#include "G4VParticleChange.hh"
#include "G4StepPoint.hh"

#include "GLG4VEventAction.hh"
#include "GLG4HitPhoton.hh"

#include <RAT/DB.hh>
#include <RAT/Log.hh>

//use this to simplify ZMQ functions

namespace RAT {

  ChromaInterface::ChromaInterface() {
#ifdef _HAS_CHROMA_INTERFACE
    GOOGLE_PROTOBUF_VERIFY_VERSION; // checks protobuf version
    fActive = false;
    ClearData();
#endif
  }


  ChromaInterface::~ChromaInterface() {
  }

  bool ChromaInterface::isActive() {
#ifdef _HAS_CHROMA_INTERFACE
    return fActive;
#else
    return false;
#endif
  }

  void ChromaInterface::initializeServerConnection() {
#ifdef _HAS_CHROMA_INTERFACE
    // Load the Chroma Table
    DB* db = DB::Get();
    DBLinkPtr lChroma = db->GetLink("CHROMA");

    fStrQueueAddress = lChroma->GetS("QueueManagerAddress");
    try {
      info << "Chroma/RAT Queue manager address is " << fStrQueueAddress << newline;
    }
    catch ( DBNotFoundError& e) {
      Log::Die( "Chroma interface initialized without specifying Chroma/RAT Queue manager address." );
    }
    // Here load appropriate socket
    context =  new zmq::context_t(1);//flag=# of i/o threads, apparently
    client = S_Client_Socket (*context);

    // Gather required geometry data
    
    // Talk to Server/Handshake/Send out detector data
    SendDetectorConfigData();
    fActive = true;
#else
    NoSupportWarning();
#endif
  }

  void ChromaInterface::closeServerConnection() {
#ifdef _HAS_CHROMA_INTERFACE
    delete client; // close socket
    delete context;
#endif
    fActive = false;
  }

  void ChromaInterface::readStoreKillCherenkovPhotons( std::vector< G4Track* >* secondaries ) {
#ifdef _HAS_CHROMA_INTERFACE
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
#endif
  }

  void ChromaInterface::readStoreKillScintillationPhotons( const G4Step* aStep, G4VParticleChange* scint_photons ) {
#ifdef _HAS_CHROMA_INTERFACE
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
#endif
  }

  void ChromaInterface::ClearData() {
#ifdef _HAS_CHROMA_INTERFACE
    message.Clear();
#endif
  }

  void ChromaInterface::JoinQueue() {
#ifdef _HAS_CHROMA_INTERFACE
    zhelpers::s_send (*client, "RDY");
#endif
  }
  //must initialize client before setting its identity
  void ChromaInterface::SetIdentity() {
    //uses zhelpers member function to set a random identity.
    //(this method is thread-safe)
#ifdef _HAS_CHROMA_INTERFACE
    ClientIdentity = zhelpers::s_set_id(*client);
#endif
  }
  void ChromaInterface::SendPhotonData() {
    // Send data
    //basic implementation, probably want to handshake or do
    //some check first.
#ifdef _HAS_CHROMA_INTERFACE
    std::string *str_msg = NULL;
    message.SerializeToString(str_msg);
    zhelpers::s_send (*client, *str_msg);
#endif
  }

  void ChromaInterface::ReceivePhotonData() {
    //do some check/configrmation first
#ifdef _HAS_CHROMA_INTERFACE
    std::string msg;
    msg = zhelpers::s_recv (*client);
    //data = message.ParseFromString(msg);
    //std::cout << data << "\n" ;
#endif
  }

  void ChromaInterface::SendDetectorConfigData() {
    // Send geometry information.
    // Includes mesh representation of detector (or activation of cache).  
    // Also, geometry info has to sync. optical detector indexes between Chroma and RAT
  }

  void ChromaInterface::MakePhotonHitData() {
#ifdef _HAS_CHROMA_INTERFACE
    GLG4HitPhoton* hit_photon = new GLG4HitPhoton();
    //hit_photon->SetPMTID((int)iopdet);
    GLG4VEventAction::GetTheHitPMTCollection()->DetectPhoton(hit_photon);
#endif
  }

  //returns a REQ client
#ifdef _HAS_CHROMA_INTERFACE
  zmq::socket_t* ChromaInterface::S_Client_Socket (zmq::context_t & context)
  {
    zmq::socket_t * client = new zmq::socket_t (context, ZMQ_REQ);
    client->connect ("tcp://localhost:5554");
    //can write this more generally (to connect to another
    //port) if we need to
    return client;
  }
#else
  void ChromaInterface::NoSupportWarning() {
    std::cout << "[WARNING] NO CHROMA INTERFACE SUPPORT!" << std::endl;
  }
#endif
}// end of namespace RAT



