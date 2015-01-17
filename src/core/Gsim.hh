#ifndef __RAT_Gsim__
#define __RAT_Gsim__

#include <G4UserRunAction.hh>
#include <G4UserEventAction.hh>
#include <G4UserTrackingAction.hh>
#include <G4TrajectoryContainer.hh>
#include <G4Run.hh>
#include <G4String.hh>
#include <G4RunManager.hh>
#include <G4IonTable.hh>
#include <set>
#include <RAT/Producer.hh>
#include <RAT/DS/Root.hh>
#include <RAT/DS/PMTInfo.hh>
#include <RAT/GLG4VEventAction.hh>
#include <RAT/EventInfo.hh>

class G4RunManager;
class GLG4DebugMessenger;

namespace RAT {

class ProcBlock;
class DetectorConstruction;
class PMTTime;
class PMTCharge;

int get_pdgcode(const G4PrimaryParticle *p);

#ifdef G4VIS_USE
class G4VisExecutive;
#endif

class Gsim : public Producer, G4UserRunAction, G4UserEventAction, G4UserTrackingAction  {
public:
  Gsim();
  Gsim(ProcBlock *theMainBlock);
  virtual ~Gsim();

  // G4UserRunAction methods
  virtual void BeginOfRunAction(const G4Run* aRun);
  virtual void EndOfRunAction(const G4Run* aRun);

  // G4UserEventAction methods
  virtual void BeginOfEventAction(const G4Event* anEvent);
  virtual void EndOfEventAction(const G4Event* anEvent);

  // G4UserTrackingAction methods
  virtual void PreUserTrackingAction(const G4Track* aTrack);
  virtual void PostUserTrackingAction(const G4Track* aTrack);

  // Creates run description and adds it to the RunStore
  void MakeRun(int runID); 

  // Convert G4Event into DS::Root
  void MakeEvent(const G4Event *g4ev, DS::Root *ds);

  static bool GetFillPointCont() { return FillPointCont; }
  static void SetFillPointCont(bool on=false) { FillPointCont=on; }

  // The following methods are used to check whether to store full track info
  // for a particle with particleName.
  static void SetStoreParticleTraj(const G4String& particleName, const bool& gDoStore);
  bool GetStoreParticleTraj(const G4String& particleName);
  static G4String GetStoreParticleTrajString(const bool& gDoStore);
    
  static bool GetStoreOpticalTrackID() { return StoreOpticalTrackID; }
  static void SetStoreOpticalTrackID(bool on=false) { StoreOpticalTrackID=on; }


protected:
  void Init(); // the real constructor
  void AddMCPhoton(DS::MCPMT* rat_mcpmt, const GLG4HitPhoton* photon,
                   bool isDarkHit=false, EventInfo* exinfo=NULL);
  
    /* Storing optical creation track ID and step */
  void PhotonRecurse(std::vector<int> &PhotonIDs, int trackID, int &parentID, int &firstCreatedID);
  void SetOpticalPhotonIDs(std::string particle_type, int trackID, int parentID);
  std::vector<int> OpticalPhotonIDs;

  G4RunManager* theRunManager;
  GLG4DebugMessenger* theDebugMessenger;
  
  RAT::DS::PMTInfo* fPMTInfo;
  std::vector<RAT::PMTTime*> fPMTTime;  //< PMT transit time/delay calculator (indexed by modeltype)
  std::vector<RAT::PMTCharge*> fPMTCharge;  //< PMT single-pe charge calculator (indexed by modeltype)

  int runID;
  TTimeStamp utc;
  int maxpe;
  int nabort;

  /** PMT and noise simulation */
  int npmts;
  double pretriggertime;
  double eventwindow;
  std::vector<double> specharge;
  std::vector<double> transitTime;
  std::vector<double> fTubeRate;
  double fNnoise;
  double noiseRate;
  double channelEfficiency;

  bool fInitialStoreTrajectoryState;

  static bool FillPointCont;
  static bool StoreOpticalTrackID;
  static std::set<G4String> fStoreParticleTraj;
  static std::set<G4String> fDiscardParticleTraj;  

#ifdef G4VIS_USE
  G4VisExecutive* theVisExecutive;
#endif
};

} // namespace RAT

#endif

