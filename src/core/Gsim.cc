#include <RAT/Gsim.hh>
#include <RAT/ProcBlock.hh>
#include <RAT/SignalHandler.hh>
#include <RAT/Log.hh>
#include <RAT/Trajectory.hh>
#include <RAT/EventInfo.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/DetectorConstruction.hh>
#include <RAT/BWVetGenericChamber.hh>
#include <RAT/BWVetGenericChamberHit.hh>

#include <RAT/Factory.hh>
#include <RAT/GLG4VertexGen.hh>
#include <RAT/VertexGen_IBD.hh>
#include <RAT/Gen_LED.hh>
#include <RAT/VertexGen_ES.hh>
#include <RAT/VertexGen_Spectrum.hh>
#include <RAT/DecayChain_Gen.hh>
#include <RAT/Coincidence_Gen.hh>
#include <RAT/CfGen.hh>
#include <RAT/EventInfo.hh>
#include <RAT/TrackInfo.hh>

#include <RAT/GLG4PrimaryGeneratorAction.hh>
#include <RAT/GLG4Scint.hh>
#include <RAT/GLG4PhysicsList.hh>
#include <RAT/GLG4SteppingAction.hh>
#include <RAT/GLG4DebugMessenger.hh>
#include <RAT/GLG4VertexGen.hh>

#include <RAT/PMTTime.hh>
#include <RAT/PMTCharge.hh>
#include <RAT/TimeUtil.hh>
#include <RAT/PMTTime.hh>
#include <RAT/Config.hh>

#include <Randomize.hh>
#include <vector>
#include <cstdlib>
#include <math.h>

namespace RAT {

// Doesn't waste space unless you want to draw tracks
bool Gsim::FillPointCont = false;
bool Gsim::StoreOpticalTrackID = false;
std::set<G4String> Gsim::fStoreParticleTraj;
std::set<G4String> Gsim::fDiscardParticleTraj;

// 01-Aug-2006 WGS: I'm putting in the default initializers to remind
// us that they're being called anyway, because Gsim inherits from
// these classes.
Gsim::Gsim() : Producer(), G4UserRunAction(), G4UserEventAction(), G4UserTrackingAction() {
  mainBlock = NULL;
  Init();
}

Gsim::Gsim(ProcBlock* theMainBlock) : Producer(), G4UserEventAction(), G4UserTrackingAction() {
  mainBlock = theMainBlock;
  Init();
}

int get_pdgcode(const G4PrimaryParticle* p) {
  G4int glg4pdgcode = p->GetPDGcode();
  if (glg4pdgcode==0 && p->GetG4code()!=0) {
    G4ParticleDefinition* pdef = p->GetG4code();
    if (G4IonTable::IsIon(pdef)) {
      int atomicNumber = G4int(pdef->GetPDGCharge()/eplus);
      int atomicMass = pdef->GetBaryonNumber();
      glg4pdgcode = \
        GLG4VertexGen_HEPEvt::kIonCodeOffset + 1000*atomicNumber + atomicMass;
    }
  }
  return glg4pdgcode;
}

void Gsim::Init() {
  // GEANT4 setup with GLG4sim detector construction and physics processes
  // Adapted from glg4sim.cc. Thanks Glenn!
  // Manages GEANT4 simulation process
  theRunManager = G4RunManager::GetRunManager();
  
  // Detector geometry
  DetectorConstruction* theDetectorConstruction = 
    DetectorConstruction::GetDetectorConstruction();
  theRunManager->SetUserInitialization(theDetectorConstruction);

  // Particle generator
  theRunManager->SetUserAction(new GLG4PrimaryGeneratorAction());

  // Add RAT-specific generators
  GlobalFactory<GLG4VertexGen>::Register("ibd",
                                         new Alloc<GLG4VertexGen,
                                         VertexGen_IBD>);
  GlobalFactory<GLG4VertexGen>::Register("es",
                                         new Alloc<GLG4VertexGen,
                                         VertexGen_ES>);
  GlobalFactory<GLG4VertexGen>::Register("spectrum",
                                         new Alloc<GLG4VertexGen,
                                         VertexGen_Spectrum>);

  GlobalFactory<GLG4Gen>::Register("decaychain",
                                   new Alloc<GLG4Gen,DecayChain_Gen>);
  GlobalFactory<GLG4Gen>::Register("cf",
                                   new Alloc<GLG4Gen,CfGen>);
  GlobalFactory<GLG4Gen>::Register("led",
                                   new Alloc<GLG4Gen,Gen_LED>);
  GlobalFactory<GLG4Gen>::Register("coincidence",
                                   new Alloc<GLG4Gen,Coincidence_Gen>);

  // An additional "messenger" class for user diagnostics
  theDebugMessenger = new GLG4DebugMessenger(theDetectorConstruction);
  
  // User actions to control Run Manager behavior

  // Detect start of run so we can create run description objects
  theRunManager->SetUserAction(static_cast<G4UserRunAction*>(this));

  // Tracking action used to add our specialized Trajectory object
  // to capture particle track information
  theRunManager->SetUserAction(static_cast<G4UserTrackingAction*>(this));
  theRunManager->SetUserAction(new GLG4SteppingAction);

  // ...and finally the hook.  Here's where we trap GEANT4 at the end of
  // each event and do our business.
  theRunManager->SetUserAction(static_cast<G4UserEventAction*>(this));

  // PMT transit time and single-pe charge calculators
  fPMTTime = NULL;
  fPMTCharge = NULL;
}

Gsim::~Gsim() {
  // GEANT4 will try to delete the G4UserEventAction when we delete
  // the Run Manager, but that object is us!!  Clear event action
  // first to avoid circular delete.  Funny casting because
  // SetUserAction() is overloaded.
  theRunManager->SetUserAction(static_cast<G4UserEventAction*>(NULL));
  theRunManager->SetUserAction(static_cast<G4UserTrackingAction*>(NULL));

  delete fPMTTime;
  delete fPMTCharge;
}
  
void Gsim::BeginOfRunAction(const G4Run* /*aRun*/) {
  DBLinkPtr lmc = DB::Get()->GetLink("MC");
  runID = DB::Get()->GetDefaultRun();
  utc = TTimeStamp(); // default to now

  DBLinkPtr lnoise = DB::Get()->GetLink("NOISE");
  noiseRate = lnoise->GetD("noise_rate");

  DBLinkPtr ldaq = DB::Get()->GetLink("DAQ");
  channelEfficiency = ldaq->GetD("channel_efficiency");

  delete fPMTTime;
  fPMTTime = new RAT::PMTTime();

  delete fPMTCharge;
  fPMTCharge = new RAT::PMTCharge();

  // Tell the generator when the run starts
  GLG4PrimaryGeneratorAction* theGLG4PGA= 
      GLG4PrimaryGeneratorAction::GetTheGLG4PrimaryGeneratorAction();
  theGLG4PGA->SetRunUTC(utc);
  
  info << "Gsim: Simulating run " << runID << newline;
  info << "Gsim: Run start at " << utc.AsString() << newline;
  
  if (!DS::RunStore::GetRun(runID)) {
    MakeRun(runID);
  }

  // Find out whether /tracking/storeTrajectory was set by user.
  // fpTrackingManager provided by G4UserTrackingAction parent class
  // We have to restore this state at the end of the event.
  fInitialStoreTrajectoryState = fpTrackingManager->GetStoreTrajectory();

  // Check for a maximum photoelectron limit. Events exceeding this limit
  // are aborted.
  try {
    maxpe = lmc->GetI("max_pe"); 
    warn << "Gsim: Aborting tracking for events exceeding " 
         << maxpe << " photoelectrons" << newline;
  }
  catch (DBNotFoundError& e) {
    maxpe = 0;
  }
  nabort = 0;
}

void Gsim::EndOfRunAction(const G4Run* /*arun*/) {
  if(maxpe > 0){
    info << "Gsim: Tracking aborted for " << nabort 
         << " events exceeding " << maxpe << " photoelectrons" << newline;
  }
}

void Gsim::BeginOfEventAction(const G4Event* anEvent) {
  GLG4Scint::ResetTotEdep();

  // Clearing theHitPMTCollection clears away the HitPhotons and HitPMTs
  GLG4VEventAction::GetTheHitPMTCollection()->Clear();  
  
  EventInfo* eventInfo =
    dynamic_cast<EventInfo* >(anEvent->GetUserInformation()); 

  eventInfo->StorePhotonIDs = StoreOpticalTrackID;

  // This is only necessary if the photons are flagged to be tracked and stored
  if (StoreOpticalTrackID) {
  	OpticalPhotonIDs.resize(10000);
  }
}

void Gsim::EndOfEventAction(const G4Event* anEvent) {
    // Now build data structure out of G4Event
    DS::Root* ds = new DS::Root;
    MakeEvent(anEvent, ds);
    ds->SetRunID(runID);
    ds->AppendProcResult("gsim", Processor::OK);
    ds->SetRatVersion(RATVERSION);

    // Let main processor block process the event
    mainBlock->DSEvent(ds);

    delete ds;

    // Check if the user has requested exit
    if (SignalHandler::IsTermRequested()) {
      warn << "Terminating since Ctrl-C (SIGINT) caught..." << newline;
      theRunManager->AbortRun(true); // Soft abort
    }

    // Reset the store trajectory state to the initial state as
    // specified in the user macro.
    fpTrackingManager->SetStoreTrajectory(fInitialStoreTrajectoryState);

    if (StoreOpticalTrackID) {
    	OpticalPhotonIDs.resize(0);
    }
}

void Gsim::PreUserTrackingAction(const G4Track* aTrack)  {
  // Add storage for our custom tracking information if
  // we haven't already added it by creating a track object elsewhere
  // (like reemitted photons)
  if (!aTrack->GetUserInformation()) {
    // grumble, grumble, C++ const keyword, grumble
    const_cast<G4Track*>(aTrack)->SetUserInformation(new TrackInfo);
  }

  // For very large, complex tracks, it is not sufficient to 
  // discard the track if we do not want it.  We must prevent
  // GEANT4 from saving the information in the first place.
  // To support this on a particle-by-particle basis, we have
  // to flip the state of track storage in the tracking manager
  // on the fly.  We restore the original state at the end of
  // the event, which is critical if beamOn is called more than once
  // in a macro.

  // Store track if requested for this particle.  fpTrackingManager 
  // provided by G4UserTrackingAction parent class
  if(GetStoreParticleTraj(aTrack->GetDefinition()->GetParticleName())) {
    fpTrackingManager->SetStoreTrajectory(true);
    fpTrackingManager->SetTrajectory(new Trajectory(aTrack));    
  }
  else {
    fpTrackingManager->SetStoreTrajectory(false);
  }

  if (aTrack->GetDefinition()->GetParticleName() == "opticalphoton") {
    G4Event* event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
    EventInfo* eventInfo = dynamic_cast<EventInfo* >(event->GetUserInformation());  
    TrackInfo *trackInfo = dynamic_cast<TrackInfo*>(aTrack->GetUserInformation());

    std::string creatorProcessName;
    const G4VProcess* creatorProcess = aTrack->GetCreatorProcess();
    if (creatorProcess)
      creatorProcessName = creatorProcess->GetProcessName();

    // Now deal with creator process naming override from trackInfo
    if (trackInfo && trackInfo->GetCreatorProcess() != "") {
      creatorProcessName = trackInfo->GetCreatorProcess();
    }

    if (creatorProcessName == "Scintillation") {
      eventInfo->numScintPhoton++;
    }
    else if (creatorProcessName == "Reemission") {
      eventInfo->numReemitPhoton++;
    }
  }
}

void Gsim::PostUserTrackingAction(const G4Track* aTrack) {
  // Now that we know how the track was terminated, we can fill
  // the OpticalCentroid
  std::string creatorProcessName;
  std::string destroyerProcessName;

  // The road to hell is paved with global variables, 
  // and GEANT4 is my travelling companion.  
  G4Event* event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
  EventInfo* eventInfo = dynamic_cast<EventInfo* >(event->GetUserInformation());
  TrackInfo* trackInfo = dynamic_cast<TrackInfo*>(aTrack->GetUserInformation());

  // Abort events with too many PE if a limit has been set.
  if (maxpe > 0) {
    int nhits = 0;
    GLG4HitPMTCollection* hitpmts = GLG4VEventAction::GetTheHitPMTCollection();
    for (int ipmt=0; ipmt<hitpmts->GetEntries(); ipmt++) {
      nhits += hitpmts->GetPMT(ipmt)->GetEntries();
    }
    if (nhits > maxpe) {
      G4EventManager::GetEventManager()->AbortCurrentEvent();
      event->SetEventAborted();
      nabort ++;
    }
  }
  
  int TrackID = aTrack->GetTrackID();
  int ParentID = aTrack->GetParentID();
  
  // Determine the creator process
  const G4VProcess* creatorProcess = aTrack->GetCreatorProcess();
  if (creatorProcess) {
    creatorProcessName = creatorProcess->GetProcessName();
  }

  // Now deal with creator process naming override from trackInfo
  if (trackInfo && trackInfo->GetCreatorProcess() != "") {
    creatorProcessName = trackInfo->GetCreatorProcess();
  }

  if(trackInfo) {
    // Fill the energy centroid
    eventInfo->energyCentroid.Add(trackInfo->energyCentroid);
  
    // Finally fill the optical centroid information if we have an optical
    // photon which was not created by the TPB but WAS absorbed by it. This
    // represents the best approximation to the "reconstructable" event
    // vertex, neglecting exotics like TPB scintillation and cerenkov emmission

    // With surface TPB model: Not reemitted, but killed by SurfaceAbsorption
    // With bulk TPB model:    Not made by OpWLS, but killed by OpWLS
    if ((aTrack->GetDefinition()->GetParticleName() == "opticalphoton") &&
        (creatorProcessName != "Reemission") &&
        (creatorProcessName != "OpWLS") ) {
      destroyerProcessName = aTrack->GetStep()->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
      if ((destroyerProcessName == "SurfaceAbsorption") ||
          (destroyerProcessName == "OpWLS")) {
        G4ThreeVector startPosition = aTrack->GetVertexPosition();  
        eventInfo->opticalCentroid.Fill(TVector3(startPosition.x(), startPosition.y(), startPosition.z()));
      }
    }
    if (StoreOpticalTrackID) {
      std::string particle_name = aTrack->GetDefinition()->GetParticleName();
      SetOpticalPhotonIDs(particle_name,TrackID,ParentID);
    }
  }
}

void Gsim::MakeRun(int runID) {
  DBLinkPtr lrun = DB::Get()->GetLink("RUN", "", runID);
  DS::Run* run = new DS::Run();

  run->SetID(runID);
  run->SetType((unsigned) lrun->GetI("runtype"));

  // Load PMT information from the database
  DS::PMTInfo* pmtinfo = run->GetPMTInfo();
  DBLinkPtr lpmt = DB::Get()->GetLink("PMTINFO");
  std::vector<double> pmtx = lpmt->GetDArray("x");
  std::vector<double> pmty = lpmt->GetDArray("y");
  std::vector<double> pmtz = lpmt->GetDArray("z");

  std::vector<double> pmtu(pmtx.size());
  std::vector<double> pmtv(pmtx.size());
  std::vector<double> pmtw(pmtx.size());
  std::vector<int> pmttype(pmtx.size(), 1);

  try {
    pmtu = lpmt->GetDArray("rot_x");
    pmtv = lpmt->GetDArray("rot_y");
    pmtw = lpmt->GetDArray("rot_z");
    pmttype = lpmt->GetIArray("type");
  }
  catch (DBNotFoundError& e) {}

  for (size_t i=0; i<pmtx.size(); i++) {
    pmtinfo->AddPMT(TVector3(pmtx[i], pmty[i], pmtz[i]),
                    TVector3(pmtu[i], pmtv[i], pmtw[i]),
                    pmttype[i]);
  }

  DS::RunStore::AddNewRun(run);
}

void Gsim::MakeEvent(const G4Event* g4ev, DS::Root* ds) {
  DS::MC* mc = ds->GetMC();
  EventInfo* exinfo = dynamic_cast<EventInfo*>(g4ev->GetUserInformation());
  
  // Event Header
  ds->SetRunID(theRunManager->GetCurrentRun()->GetRunID());
  mc->SetID(g4ev->GetEventID());
  mc->SetUTC(exinfo->utc);

  // Vertex Info
  for (int ivert=0; ivert<g4ev->GetNumberOfPrimaryVertex(); ivert++) {
    G4PrimaryVertex* pv = g4ev->GetPrimaryVertex(ivert);

    float t = pv->GetT0();
    TVector3 pos(pv->GetX0(), pv->GetY0(), pv->GetZ0());
    
    for (int ipart=0; ipart<pv->GetNumberOfParticle(); ipart++) {
      DS::MCParticle* rat_mcpart = mc->AddNewMCParticle();

      G4PrimaryParticle* p = pv->GetPrimary(ipart);
      rat_mcpart->SetPDGCode(get_pdgcode(p));
      rat_mcpart->SetParticleName(p->GetG4code()->GetParticleName());
      rat_mcpart->SetMomentum(TVector3(p->GetPx(), p->GetPy(), p->GetPz()));
      rat_mcpart->SetKE(sqrt(p->GetMass()*p->GetMass()+p->GetMomentum().mag2()) - p->GetMass());
      rat_mcpart->SetTime(t);
      rat_mcpart->SetPosition(pos);
      rat_mcpart->SetPolarization(TVector3(p->GetPolX(), p->GetPolY(), p->GetPolZ()));
    }
  }

  // Calibration source information
  const DS::Calib* calib = exinfo->GetCalib();
  // only copy if there is something real to copy
  if (calib->GetID() != -1) {
    *ds->GetCalib() = *calib;
  }

  // Trajectory Info
  G4TrajectoryContainer* traj_list = g4ev->GetTrajectoryContainer();
  if (traj_list) {
    for (size_t itraj=0; itraj<traj_list->size(); itraj++) {
      Trajectory* traj = dynamic_cast<Trajectory*>((*traj_list)[itraj]);
      // Trajectory potentially cut during generation to save space
      if (!traj) {
        continue;
      }
      DS::MCTrack* track = mc->AddNewMCTrack();
      *track = *traj->GetTrack();
    }
  }

  // If the event was aborted because of the maximum number of 
  // photoelectron limit, then store the information about the
  // primary particle and any tracking information, but do not keep
  // the PMT information. 
  if(g4ev->IsAborted()) {
    return;
  }

  // MC summary information
  DS::MCSummary* summary = mc->GetMCSummary();
  summary->SetEnergyCentroid(exinfo->energyCentroid.GetMean());
  summary->SetEnergyRMS(exinfo->energyCentroid.GetRMS());
  summary->SetEnergyLossByVolume(exinfo->energyLoss);
  summary->SetTotalScintEdep(GLG4Scint::GetTotEdep());
  summary->SetTotalScintEdepQuenched(GLG4Scint::GetTotEdepQuenched());
  const G4ThreeVector sCentroid = GLG4Scint::GetScintCentroid();
  TVector3 scintCentroid(sCentroid.x(), sCentroid.y(), sCentroid.z());
  summary->SetTotalScintCentroid(scintCentroid);
  summary->SetNumScintPhoton(exinfo->numScintPhoton);
  summary->SetNumReemitPhoton(exinfo->numReemitPhoton);

  /** PMT and noise simulation */
  GLG4HitPMTCollection* hitpmts = GLG4VEventAction::GetTheHitPMTCollection();
  int numPE = 0;
 
  double firsthittime = 99999;
  double lasthittime = -99999;

  // Get the PMT type for IDPMTs. Then in the loop,
  // increment numPE only when the PE is in an IDPMT.
  // Map from PMT ID numbers to objects for use later in noise calculation
  std::map<int, DS::MCPMT*> mcpmtObjects;

  for (int ipmt=0; ipmt<hitpmts->GetEntries(); ipmt++) {
    GLG4HitPMT* a_pmt= hitpmts->GetPMT(ipmt);
    a_pmt->SortTimeAscending();

    // Create and initialize a RAT DS::MCPMT 
    // note that GLG4HitPMTs are given IDs which are their index
    DS::MCPMT* rat_mcpmt = mc->AddNewMCPMT();
    mcpmtObjects[a_pmt->GetID()] = rat_mcpmt;
    rat_mcpmt->SetID(a_pmt->GetID());
    DS::Run* run = DS::RunStore::GetRun(runID);
    try {
      rat_mcpmt->SetType(run->GetPMTInfo()->GetType(a_pmt->GetID()));
    }
    catch (const std::out_of_range& e) {
      info << "PMT Type out of range: " << a_pmt->GetID() << " min=0 max=" << run->GetPMTInfo()->GetPMTCount() << newline;
      rat_mcpmt->SetType(0);
    }

    numPE += a_pmt->GetEntries();

    /** Add "real" hits from actual simulated photons */     
    for (int i=0; i<a_pmt->GetEntries(); i++) {
      if (StoreOpticalTrackID) {
        AddMCPhoton(rat_mcpmt, a_pmt->GetPhoton(i), false, exinfo);
      }
      else {
        AddMCPhoton(rat_mcpmt, a_pmt->GetPhoton(i), false, NULL);
      }

      /** Update event start and end time */
      double hittime = a_pmt->GetPhoton(i)->GetTime();
      if (hittime < firsthittime) {
        firsthittime = hittime;
        if (i != 0) {
          detail << "Gsim: " << i
                 << "th photon has earliest hit time" << newline;
        }
      }
      if (hittime > lasthittime) {
        lasthittime = hittime;
      }
    }
  }
  mc->SetNumPE(numPE);
  
  /**
   * Add noise hits
   *
   * Generate noise hits in a `noise window' which extends from the first
   * to last photon hits.
   */
  DS::Run* run = DS::RunStore::GetRun(runID);
  DS::PMTInfo* pmtinfo = run->GetPMTInfo();
  double noiseWindowWidth = lasthittime - firsthittime;
  size_t npmts = pmtinfo->GetPMTCount();
  double channelRate = noiseRate * noiseWindowWidth;
  double detectorWideRate = channelRate * npmts / channelEfficiency;
  int noiseHits = \
    static_cast<int>(floor(CLHEP::RandPoisson::shoot(detectorWideRate)));

  for (int ihit=0; ihit<noiseHits; ihit++) {
    GLG4HitPhoton* hit = new GLG4HitPhoton();
    int pmtid = static_cast<int>(G4UniformRand() * npmts);
    hit->SetPMTID(pmtid);
    hit->SetTime(firsthittime + G4UniformRand() * noiseWindowWidth);
    hit->SetCount(1);
    //hit->SetIsNoise();
    // Add the PMT if it did not register a "real" hit
    if (!mcpmtObjects.count(pmtid)) {
      DS::MCPMT* rat_mcpmt = mc->AddNewMCPMT();
      mcpmtObjects[pmtid] = rat_mcpmt;
      rat_mcpmt->SetID(pmtid);
      rat_mcpmt->SetType(pmtinfo->GetType(pmtid));
    }
    AddMCPhoton(mcpmtObjects[pmtid], hit, true, (StoreOpticalTrackID ? exinfo : NULL));
  }
}

void Gsim::AddMCPhoton(DS::MCPMT* rat_mcpmt, const GLG4HitPhoton* photon,
                       bool isDarkHit, EventInfo* exinfo) {
  DS::MCPhoton* rat_mcphoton = rat_mcpmt->AddNewMCPhoton();
  rat_mcphoton->SetDarkHit(isDarkHit);

  // parameters relevant only for actual photon hits, not noise hits
  if (!isDarkHit) {
    rat_mcphoton->SetLambda(photon->GetWavelength());
    float x,y,z;
    photon->GetPosition(x,y,z);
    rat_mcphoton->SetPosition(TVector3(x,y,z));
    photon->GetMomentum(x,y,z);
    rat_mcphoton->SetMomentum(TVector3(x,y,z));
    photon->GetPolarization(x,y,z);
    rat_mcphoton->SetPolarization(TVector3(x,y,z));
    rat_mcphoton->SetOriginFlag( photon->GetOriginFlag() );
    rat_mcphoton->SetTrackID(photon->GetTrackID());
  }
  else {
    // default values
    rat_mcphoton->SetLambda(0.0);
    rat_mcphoton->SetPosition(TVector3(0,0,0));
    rat_mcphoton->SetMomentum(TVector3(0,0,0));
    rat_mcphoton->SetPolarization(TVector3(0,0,0));
    rat_mcphoton->SetTrackID(-1);
  }
  rat_mcphoton->SetHitTime(photon->GetTime());
  rat_mcphoton->SetFrontEndTime(fPMTTime->PickTime(photon->GetTime()));
  rat_mcphoton->SetCharge(fPMTCharge->PickCharge());
}

void Gsim::SetStoreParticleTraj(const G4String& particleName,
                                const bool& gDoStore) {
  if(gDoStore) {
    fStoreParticleTraj.insert(particleName);
  }
  else {
    fDiscardParticleTraj.insert(particleName);
  }
}

bool Gsim::GetStoreParticleTraj(const G4String& particleName) {
  if (!fInitialStoreTrajectoryState) {
    // in case fStoreParticleTraj has an entry, keep 
    // track only if particle name is in fStoreParticleTraj 
    if (!fStoreParticleTraj.empty()) {
      std::set<G4String>::const_iterator it = fStoreParticleTraj.find(particleName);
      if (it == fStoreParticleTraj.end())
        return false;
      else
        return true;
    }
    else
      return false;
  }
  else { 
    // check whether request was made to throw these tracks away by checking
    // whether this particleName exists in fDiscardParticleTraj
    std::set<G4String>::const_iterator it = fDiscardParticleTraj.find(particleName);
    if (it == fDiscardParticleTraj.end())
      return true;
    else
      return false;
  }
}

G4String Gsim::GetStoreParticleTrajString(const bool& gDoStore) {
  G4String ret = "";
  if (gDoStore) {
    for(std::set<G4String>::const_iterator it = fStoreParticleTraj.begin();
        it!=fStoreParticleTraj.end();
        it++){
      ret += *it;
      ret += " ";
    }
  }
  else {
    for(std::set<G4String>::const_iterator it = fStoreParticleTraj.begin();
        it!=fStoreParticleTraj.end();
        it++){
      ret += *it;
      ret += " ";
    }
  }
  return ret;
}

// PhotonRecurse runs back from the PMT photon through the track - parent
// ID pairs in PhotonIDs to find the original track which created the
// optical photon
void Gsim::PhotonRecurse(std::vector<int> &PhotonIDs, int trackID, int &parentID, int &firstCreatedID) {
  if (PhotonIDs[trackID]==0 || PhotonIDs[trackID]==-1) {
    parentID=trackID;
    return;
  }

  if (PhotonIDs[trackID]!=0)
    firstCreatedID = trackID;

  PhotonRecurse(PhotonIDs, PhotonIDs[trackID], parentID, firstCreatedID);
}

void Gsim::SetOpticalPhotonIDs(std::string particle_type, int trackID,
                               int parentID) {
	if (static_cast<size_t>(trackID) > OpticalPhotonIDs.size()) {
  	// this factor of 2 seems pretty random.
  	OpticalPhotonIDs.resize(2*trackID,0);
  }
  if (particle_type.compare("opticalphoton") == 0) {
    OpticalPhotonIDs[trackID] = parentID;
  }
  if (particle_type.compare("opticalphoton") != 0) {
    OpticalPhotonIDs[trackID] = 0;
  }
}

} // namespace RAT

