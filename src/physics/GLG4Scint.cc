/**
 * @file GLG4Scint.cc
 *
 * For GLG4Scint class, providing advanced scintillation process.
 * Distantly based on an extensively modified version of G4Scintillation.cc.
 * 
 * This file is part of the GenericLAND software library.
 * $Id: GLG4Scint.cc,v 1.2 2006/03/08 03:52:41 volsung Exp $
 *
 * @author Glenn Horton-Smith (Tohoku) 28-Jan-1999
 * 
 * 4 January, 2009
 * V.V. Golovko changed method GetPhotonMomentum()
 *                          to GetPhotonEnergy()
 * V.V. Golovko changed method GetMinPhotonMomentum()
 *                          to GetMinPhotonEnergy()
 * V.V. Golovko changed method GetMaxPhotonMomentum()
 *                          to GetMaxPhotonEnergy()
 *
 */

// [see detailed class description in GLG4Scint.hh]

#include "G4UnitsTable.hh"
#include "GLG4Scint.hh"
#include "G4ios.hh"
#include "G4Timer.hh"
#include "Randomize.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"
#include "G4TrackFastVector.hh"
#include <RAT/PhotonThinning.hh>

#include "G4IonTable.hh"

#include "G4hZiegler1985Nuclear.hh"
#include "G4hZiegler1985p.hh"
#include "G4hIonEffChargeSquare.hh"
#include "G4hParametrisedLossModel.hh"
#include "G4PSTARStopping.hh"
#include "TF1.h"
#include "G4AtomicShells.hh"
#include "G4ParticleTable.hh"
#include <RAT/TrackInfo.hh>
#include <RAT/Sampling.hh>

#include <RAT/EventInfo.hh>
#include <RAT/Log.hh>
#include <G4Event.hh>
#include <G4EventManager.hh>
#include <sstream>

////////////////
// Static data members
////////////////

G4std::vector<GLG4Scint*> GLG4Scint::masterVectorOfGLG4Scint;
G4UIdirectory* GLG4Scint::GLG4ScintDir = NULL;
G4int GLG4Scint::maxTracksPerStep = 180000;
G4double GLG4Scint::meanPhotonsPerSecondary = 1.0; 
G4bool GLG4Scint::doScintillation = true;
G4bool GLG4Scint::doReemission = true;
G4double GLG4Scint::totEdep = 0.0;
G4double GLG4Scint::totEdep_quenched = 0.0;
G4double GLG4Scint::totEdep_time = 0.0;
G4ThreeVector GLG4Scint::scintCentroidSum(0.0, 0.0, 0.0);
G4double GLG4Scint::QuenchingFactor = 1.0;
G4bool GLG4Scint::UserQF = false;
G4String GLG4Scint::fPrimaryName = G4String();
G4double GLG4Scint::fPrimaryEnergy = 0;
GLG4DummyProcess GLG4Scint::scintProcess("Scintillation", fUserDefined);
GLG4DummyProcess GLG4Scint::reemissionProcess("Reemission", fUserDefined);
G4std::vector<GLG4DummyProcess*> GLG4Scint::reemissionProcessVector;
G4int GLG4Scint::fPhotonCount;

/////////////////
// Constructors
/////////////////

GLG4Scint::GLG4Scint(const G4String& tablename, G4double lowerMassLimit) {
  verboseLevel = 0;
  myLowerMassLimit = lowerMassLimit;
  
  myPhysicsTable = MyPhysicsTable::FindOrBuild(tablename);
  myPhysicsTable->IncUsedBy();
  if (verboseLevel)
    myPhysicsTable->Dump();

  // Add to ordered list
  if (masterVectorOfGLG4Scint.size() == 0 ||
      lowerMassLimit >= masterVectorOfGLG4Scint.back()->myLowerMassLimit) {
      masterVectorOfGLG4Scint.push_back(this);
  }
  else {
    for (G4std::vector<GLG4Scint*>::iterator i=masterVectorOfGLG4Scint.begin();
         i!=masterVectorOfGLG4Scint.end();
         i++) {
       if (lowerMassLimit < (*i)->myLowerMassLimit) {
          masterVectorOfGLG4Scint.insert(i, this);
          break;
       }
     }
  }

  // Create UI commands if necessary
  if (GLG4ScintDir == NULL) {
    GLG4ScintDir = new G4UIdirectory("/glg4scint/");
    GLG4ScintDir->SetGuidance("scintillation process control.");
    G4UIcommand *cmd;
    cmd= new G4UIcommand("/glg4scint/on", this);
    cmd->SetGuidance("Turn on scintillation");
    cmd= new G4UIcommand("/glg4scint/off", this);
    cmd->SetGuidance("Turn off scintillation");
    cmd= new G4UIcommand("/glg4scint/reemission", this);
    cmd->SetGuidance("Turn on/off reemission of absorbed opticalphotons");
    cmd->SetParameter(new G4UIparameter("status", 's', false));
    cmd= new G4UIcommand("/glg4scint/maxTracksPerStep",this);
    cmd->SetGuidance("Set maximum number of opticalphoton tracks per step\n"
                     "(If more real photons are needed, "
                     "weight of tracked particles is increased.)\n" );
    cmd->SetParameter(new G4UIparameter("maxTracksPerStep", 'i', false));
    cmd= new G4UIcommand("/glg4scint/meanPhotonsPerSecondary",this);
    cmd->SetGuidance("Set mean number of \"real\" photons per secondary\n");
    cmd->SetParameter(new G4UIparameter("meanPhotonsPerSecondary",
                                        'd', false));
    cmd= new G4UIcommand("/glg4scint/verbose",this);
    cmd->SetGuidance("Set verbose level");
    cmd->SetParameter(new G4UIparameter("level", 'i', false));
    cmd= new G4UIcommand("/glg4scint/dump",this);
    cmd->SetGuidance("Dump tables");
    cmd= new G4UIcommand("/glg4scint/setQF",this);
    cmd->SetGuidance("Set a constant quenching factor, default is 1");
    cmd->SetParameter(new G4UIparameter("QuenchingFactor", 'd', false));
    cmd->SetGuidance("Set Time Precision Goal in the scintillation time delay, default is 0.001 ns");
    cmd->SetGuidance("Set maximum number of iterations in the scintillation time delay, default is 1000");
  }
  
#ifdef G4VERBOSE
  G4cout << "GLG4Scint[" << tablename << "]" << " is created " << G4endl;
#endif
}

////////////////
// Destructors
////////////////

GLG4Scint::~GLG4Scint()  {
  myPhysicsTable->DecUsedBy();
  for (G4std::vector<GLG4Scint *>::iterator i=masterVectorOfGLG4Scint.begin();
       i!=masterVectorOfGLG4Scint.end();
       i++) {
    if (*i == this) {
      masterVectorOfGLG4Scint.erase(i);
      break;
    }
  }
}


////////////
// Methods
////////////

// Sets the quenching factor
void GLG4Scint::SetQuenchingFactor(G4double qf=1.0) { QuenchingFactor = qf; }

#ifdef G4DEBUG
G4double GLG4Scint_tottime = 0.0;
G4int GLG4Scint_num_calls= 0;
G4int GLG4Scint_num_phots= 0;
#endif

// This routine is called for each step of any particle
// in a scintillator.  For accurate energy deposition, must be called
// from user-supplied UserSteppingAction, which also must stack
// any particles created.  A pseudo-Poisson-distributed number of
// photons is generated according to the scintillation yield formula,
// distributed evenly along the track segment and uniformly into 4pi.
G4VParticleChange*
GLG4Scint::PostPostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
#ifdef G4DEBUG
  G4Timer timer;
  timer.Start();
  GLG4Scint_num_calls ++;
#endif

  // Below this anonymous namespace is label PostStepDoIt_DONE
  {
    // prepare to generate an event, organizing to
    // check for things that cause an early exit.
    aParticleChange.Initialize(aTrack);
    const G4Material* aMaterial = aTrack.GetMaterial();
    const MyPhysicsTable::Entry* physicsEntry =
      myPhysicsTable->GetEntry(aMaterial->GetIndex());
      
    G4bool flagReemission = false;
    if (aTrack.GetDefinition() == G4OpticalPhoton::OpticalPhoton()) {
      // Adds the original parent track and creation step to PhotonIDParentStep
      G4Event* event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
      RAT::EventInfo* eventInfo = dynamic_cast<RAT::EventInfo*>(event->GetUserInformation());
      RAT::TrackInfo* currentTrackInfo = dynamic_cast<RAT::TrackInfo*>(aTrack.GetUserInformation());
      if (eventInfo && eventInfo->StorePhotonIDs) {
        // Only occurs on first step
        if (aTrack.GetCurrentStepNumber() == 1) { 
          eventInfo->PhotonIDParentStep[aTrack.GetTrackID()].push_back(aTrack.GetParentID());
          eventInfo->PhotonIDParentStep[aTrack.GetTrackID()].push_back(currentTrackInfo->GetCreatorStep()-1);
        }
      }

      flagReemission = (doReemission &&
                        aTrack.GetTrackStatus() == fStopAndKill &&
                        aStep.GetPostStepPoint()->GetStepStatus() != fGeomBoundary);

      if (!flagReemission) {
        goto PostStepDoIt_DONE;
      }
    }

    G4double TotalEnergyDeposit = aStep.GetTotalEnergyDeposit();
  
    if (TotalEnergyDeposit <= 0.0 && !flagReemission)
      goto PostStepDoIt_DONE;

    if (!physicsEntry) {
      G4cout << "physicsEntry empty for material=" << aMaterial->GetName() << G4endl;
      goto PostStepDoIt_DONE;
    }

    // Finds E-dependent QF, unless the user provided an E-independent one
    if (!UserQF && physicsEntry->QuenchingArray != NULL) {
      // This interpolates or uses first/last value if out of range
      SetQuenchingFactor(physicsEntry->QuenchingArray->Value(aTrack.GetVertexKineticEnergy()));
    }
    else {
      SetQuenchingFactor(1.0);
    }

    // Retrieve the Light Yield or Scintillation Integral for this material  
    G4double ScintillationYield=physicsEntry->light_yield;
    G4PhysicsOrderedFreeVector* ScintillationIntegral = physicsEntry->spectrumIntegral;
    G4PhysicsOrderedFreeVector* ReemissionIntegral = physicsEntry->reemissionIntegral;
    
    if (!ScintillationIntegral) {
      if (ReemissionIntegral == NULL) { // If reemits, there's still work to do!
        goto PostStepDoIt_DONE;
      }
    }

    
    // If no LY defined Max Scintillation Integral == ScintillationYield
    if (!ScintillationYield) {
      if (ScintillationIntegral == NULL) {
        ScintillationYield = 0;
      }
      else {
        ScintillationYield = ScintillationIntegral->GetMaxValue();
      }
    }

    
    // Set positions, directions, etc.
    G4StepPoint* pPreStepPoint = aStep.GetPreStepPoint();
    G4StepPoint* pPostStepPoint = aStep.GetPostStepPoint();

    G4ThreeVector x0 = pPreStepPoint->GetPosition();
    G4ThreeVector p0 = pPreStepPoint->GetMomentumDirection();
    G4double t0 = pPreStepPoint->GetGlobalTime();

    // Finally ready to start generating the event
    // figure out how many photons we want to make
    G4int numSecondaries;
    G4double weight;
    //G4double reemissionProb = 0;
    //G4int numComponents = -1;
    //G4int absorberIndex = -1;

    if (flagReemission) {
      G4MaterialPropertiesTable* mpt_scint =
        aMaterial->GetMaterialPropertiesTable();

      // Check if there are multiple components
      if (mpt_scint->ConstPropertyExists("COMPONENTS")) {
        RAT::Log::Die("GLG4Scint: COMPONENTS not yet implemented");
        //numComponents = (G4int) mpt_scint->GetConstProperty("NUM_COMP");
      }

      G4MaterialPropertyVector* mpv_scint_reemission =
        mpt_scint->GetProperty("REEMISSION_PROB");

      if (mpv_scint_reemission == NULL) {
        goto PostStepDoIt_DONE;
      }

      G4double p_reemission =
        mpv_scint_reemission->Value(aTrack.GetKineticEnergy());

      numSecondaries = (G4int)(CLHEP::RandPoisson::shoot(p_reemission));

      if (numSecondaries == 0) {
        goto PostStepDoIt_DONE;
      }
      weight = aTrack.GetWeight();
    }
    else {
      // Apply Birk's law
      // Astr. Phys. 30 (2008) 12 uses custom dE/dx, different from G4/Ziegler's
      G4double birksConstant = physicsEntry->birksConstant;
      G4double QuenchedTotalEnergyDeposit = TotalEnergyDeposit * 1.0;
      if (birksConstant != 0.0) {
        G4double dE_dx = TotalEnergyDeposit /  aStep.GetStepLength();
        QuenchedTotalEnergyDeposit /= (1.0 + birksConstant * dE_dx);
      }

      // Track total edep, quenched edep
      totEdep += TotalEnergyDeposit;
      totEdep_quenched += QuenchedTotalEnergyDeposit;
      totEdep_time = t0;
      scintCentroidSum +=
        QuenchedTotalEnergyDeposit * (x0 + p0*(0.5*aStep.GetStepLength()));

      // Now we are done if we are not actually making photons here
      if (!doScintillation) {
        goto PostStepDoIt_DONE;
      }
  
      // Calculate MeanNumPhotons
      G4double MeanNumPhotons =
        (ScintillationYield *
         GetQuenchingFactor() *
         QuenchedTotalEnergyDeposit *
         (1.0 + birksConstant * (physicsEntry->ref_dE_dx)));

      if (MeanNumPhotons <= 0.0) {
        goto PostStepDoIt_DONE;
      }
      
      // Randomize number of TRACKS (not photons)
      // this gets statistics right for number of PE after applying
      // boolean random choice to final absorbed track (change from
      // old method of applying binomial random choice to final absorbed
      // track, which did want poissonian number of photons divided
      // as evenly as possible into tracks)
      // Note for weight=1, there's no difference between tracks and photons.
      G4double MeanNumTracks =
        (MeanNumPhotons /
         meanPhotonsPerSecondary /
         RAT::PhotonThinning::GetFactor());

      G4double resolutionScale = physicsEntry->resolutionScale;
      if (MeanNumTracks > 12.0) {
        numSecondaries=
          (G4int)(CLHEP::RandGauss::shoot(MeanNumTracks,
                                          resolutionScale
                                          * sqrt(MeanNumTracks)));
      }
      else {
        if (resolutionScale > 1.0) {
          MeanNumTracks =
            CLHEP::RandGauss::shoot(MeanNumTracks,
                                    (sqrt(resolutionScale*resolutionScale-1.0) *
                                     MeanNumTracks));
        }
        numSecondaries=
          (G4int)(CLHEP::RandPoisson::shoot(MeanNumTracks));
      }

      weight = meanPhotonsPerSecondary;
      if (numSecondaries > maxTracksPerStep) {
        // It's probably better to just set meanPhotonsPerSecondary to
        // a big number if you want a small number of secondaries, but
        // this feature is retained for backwards compatibility.
        weight = weight * numSecondaries / maxTracksPerStep;
        numSecondaries = maxTracksPerStep;
      }
    }
    
    // if there are no photons, then we're all done now
    if (numSecondaries <= 0) {
      // Return unchanged particle and no secondaries  
      aParticleChange.SetNumberOfSecondaries(0);
      goto PostStepDoIt_DONE;
    }

    // Okay, we will make at least one secondary.
    // Notify the proper authorities.
    aParticleChange.SetNumberOfSecondaries(numSecondaries);
    if (!flagReemission) {
      if (aTrack.GetTrackStatus() == fAlive) {
        aParticleChange.ProposeTrackStatus(fSuspend);
      }
    }

    // Now look up waveform information we need to add the secondaries
    G4PhysicsOrderedFreeVector* WaveformIntegral =
      physicsEntry->timeIntegral;

    for (G4int iSecondary=0; iSecondary<numSecondaries; iSecondary++) {
      // Determine photon momentum
      G4double sampledMomentum;

      if (!flagReemission && ScintillationIntegral != NULL) {
        // Normal scintillation
        G4double CIIvalue =
          G4UniformRand() * ScintillationIntegral->GetMaxValue();
        sampledMomentum = ScintillationIntegral->GetEnergy(CIIvalue);
#ifdef G4DEBUG
        if (verboseLevel > 1) {
          G4cout << "sampledMomentum = " << sampledMomentum << G4endl;
          G4cout << "CIIvalue =        " << CIIvalue << G4endl;
        }
#endif
      }
      else {
        // Reemission
        G4bool this_is_REALLY_STUPID;
        G4double CIIvalue =
          (G4UniformRand() *
           ReemissionIntegral->GetValue(aTrack.GetKineticEnergy(),
                                        this_is_REALLY_STUPID));
        if (CIIvalue == 0.0) {
          // Return unchanged particle and no secondaries  
          aParticleChange.SetNumberOfSecondaries(0);
          goto PostStepDoIt_DONE;
        }

        sampledMomentum = ReemissionIntegral->GetEnergy(CIIvalue);
        aParticleChange.ProposeLocalEnergyDeposit(aTrack.GetKineticEnergy() -
                                                  sampledMomentum);

        if (sampledMomentum > aTrack.GetKineticEnergy()) {
           goto PostStepDoIt_DONE;
        }

#ifdef G4DEBUG
        if (sampledMomentum > aTrack.GetKineticEnergy()) {
          G4cerr << "Error in GLG4Scint: sampled reemitted photon momentum "
                 << sampledMomentum
                 << " is greater than track energy "
                 << aTrack.GetKineticEnergy() << G4endl;
        }
        if (verboseLevel > 1) {
          G4cout << "oldMomentum = " <<aTrack.GetKineticEnergy() << G4endl;
          G4cout << "reemittedSampledMomentum = " << sampledMomentum
                 << "\nreemittedCIIvalue =        " << CIIvalue << G4endl;
        }
#endif
      }

      // Generate random photon direction
      G4double cost = 1. - 2. * G4UniformRand();
      G4double sint = sqrt(1. - cost * cost);  // FIXED BUG from G4Scint

      G4double phi = 2 * M_PI * G4UniformRand();
      G4double sinp = sin(phi);
      G4double cosp = cos(phi);

      G4double px = sint*cosp;
      G4double py = sint*sinp;
      G4double pz = cost;

      // Create photon momentum direction vector
      G4ParticleMomentum photonMomentum(px, py, pz);

      // Determine polarization of new photon
      G4double sx = cost * cosp;
      G4double sy = cost * sinp; 
      G4double sz = -sint;

      G4ThreeVector photonPolarization(sx, sy, sz);

      G4ThreeVector perp = photonMomentum.cross(photonPolarization);

      phi = 2 * M_PI * G4UniformRand();
      sinp = sin(phi);
      cosp = cos(phi);

      photonPolarization = cosp * photonPolarization + sinp * perp;

      photonPolarization = photonPolarization.unit();

      // Generate a new photon
      G4DynamicParticle* aScintillationPhoton =
        new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(), 
                              photonMomentum);

      aScintillationPhoton->SetPolarization(photonPolarization.x(),
                                            photonPolarization.y(),
                                            photonPolarization.z());

      aScintillationPhoton->SetKineticEnergy(sampledMomentum);

      // Generate new G4Track object
      G4ThreeVector aSecondaryPosition;
      G4double deltaTime;
      if (flagReemission) {
        deltaTime = pPostStepPoint->GetGlobalTime() - t0;
        aSecondaryPosition = pPostStepPoint->GetPosition();
      }
      else {
        G4double delta = G4UniformRand() * aStep.GetStepLength();
        aSecondaryPosition = x0 + delta * p0;

        // Start deltaTime based on where on the track it happened
        deltaTime =
          (delta /
           ((pPreStepPoint->GetVelocity() +
             pPostStepPoint->GetVelocity()) / 2.));
      }

      // Delay for scintillation time
      if (WaveformIntegral) {
        G4double WFvalue = G4UniformRand()*WaveformIntegral->GetMaxValue();
        G4double sampledDelayTime = WaveformIntegral->GetEnergy(WFvalue);
        deltaTime += sampledDelayTime;
      }

      // Set secondary time
      G4double aSecondaryTime = t0 + deltaTime;

      // Create secondary track                
      G4Track* aSecondaryTrack =
        new G4Track(aScintillationPhoton,
                    aSecondaryTime,
                    aSecondaryPosition);

      aSecondaryTrack->SetWeight(weight);
      aSecondaryTrack->SetParentID(aTrack.GetTrackID());
      RAT::TrackInfo* trackInfo = new RAT::TrackInfo();
      
      trackInfo->SetCreatorStep(aTrack.GetCurrentStepNumber());

      if (flagReemission) {
        aSecondaryTrack->SetCreatorProcess(&reemissionProcess);
        trackInfo->SetCreatorProcess(reemissionProcess.GetProcessName());
      }
      else {
        aSecondaryTrack->SetCreatorProcess(&scintProcess);
        trackInfo->SetCreatorProcess(scintProcess.GetProcessName());
      }

      aSecondaryTrack->SetUserInformation(trackInfo);

      // Add the secondary to the ParticleChange object
      aParticleChange.SetSecondaryWeightByProcess(true); // recommended
      aParticleChange.AddSecondary(aSecondaryTrack);
      
      // AddSecondary() overrides our setting of the secondary track weight
      // in Geant4.3.1 & earlier (and also later, at least until Geant4.7?).
      // Maybe not required if SetWeightByProcess(true) called,
      // but we do both, just to be sure
      aSecondaryTrack->SetWeight(weight);
    }
  }

 PostStepDoIt_DONE:
#ifdef G4DEBUG    
  timer.Stop();
  GLG4Scint_tottime += timer.GetUserElapsed();
  GLG4Scint_num_phots += aParticleChange.GetNumberOfSecondaries();
#endif
  
#ifdef G4VERBOSE
  if (verboseLevel > 1) {
    G4cout << "\n Exiting from GLG4Scint::DoIt -- NumberOfSecondaries = " 
	   << aParticleChange.GetNumberOfSecondaries() << " produced by " << aTrack.GetDefinition()->GetParticleName() <<  G4endl;
  }
#endif

  return &aParticleChange;
}


// The generic (static) PostPostStepDoIt
G4VParticleChange* GLG4Scint::GenericPostPostStepDoIt(const G4Step *pStep) {
  G4Track *track = pStep->GetTrack();
  G4double mass = track->GetDynamicParticle()->GetMass();
  G4std::vector<GLG4Scint *>::iterator it = masterVectorOfGLG4Scint.begin();
  for (int i=masterVectorOfGLG4Scint.size(); (i--) > 1;) {
    it++;
    if (mass < (*it)->myLowerMassLimit) {
      return (*(--it))->PostPostStepDoIt(*track,*pStep);
    }
  }
  return (*it)->PostPostStepDoIt(*track, *pStep);
}


////////////////////////////////////////////////////////////////
// MyPhysicsTable (nested class) definitions
////////////////////////////////////////////////////////////////

////////////////
// "Static" members of the class
// [N.B. don't use "static" keyword here, because it means something
// entirely different in this context.]
////////////////

GLG4Scint::MyPhysicsTable* GLG4Scint::MyPhysicsTable::head = NULL;


// Constructor
GLG4Scint::MyPhysicsTable::MyPhysicsTable() {
  name = 0;
  next = 0;
  used_by_count = 0;
  data = 0;
  length = 0;
}


// Destructor
GLG4Scint::MyPhysicsTable::~MyPhysicsTable() {
  if (used_by_count != 0) {
      G4cerr << "Error, GLG4Scint::MyPhysicsTable is being deleted with "
             << "used_by_count = " << used_by_count << G4endl;
      return;
  }
  delete name;
  delete[] data;
}


////////////////
// Member functions
////////////////

void GLG4Scint::MyPhysicsTable::Dump() const {
  G4cout << " GLG4Scint::MyPhysicsTable {\n"
         << "  name=" << (*name) << G4endl
         << "  length=" << length << G4endl
         << "  used_by_count=" << used_by_count << G4endl;

  for (G4int i=0; i<length; i++) {
    G4cout << "  data[" << i << "]= { // "
           << (*G4Material::GetMaterialTable())[i]->GetName() << G4endl;
    G4cout << "   spectrumIntegral=";
    if (data[i].spectrumIntegral)
      (data[i].spectrumIntegral)->DumpValues();
    else
      G4cout << "NULL" << G4endl;
    
    G4cout << "   reemissionIntegral=";
    if (data[i].reemissionIntegral)
      (data[i].reemissionIntegral)->DumpValues();
    else
      G4cout << "NULL" << G4endl;
      
    G4cout << "   timeIntegral=";
    if (data[i].timeIntegral)
      (data[i].timeIntegral)->DumpValues();
    else
      G4cout << "NULL" << G4endl;
      G4cout << "   resolutionScale=" << data[i].resolutionScale
             << "   birksConstant=" << data[i].birksConstant
             << "   ref_dE_dx=" << data[i].ref_dE_dx << G4endl
             << "   light yield=" << data[i].light_yield << G4endl;

    G4cout << "Quenching = \n";
    if (data[i].QuenchingArray != NULL)
      data[i].QuenchingArray->DumpValues();
    else
      G4cout << "NULL" << G4endl << "  }\n";
  }

  G4cout << " }\n";
}


GLG4Scint::MyPhysicsTable*
GLG4Scint::MyPhysicsTable::FindOrBuild(const G4String& name) {
  // Head should always exist and should always be the default (name=="")
  if (head == NULL) {
    head = new MyPhysicsTable;
    head->Build("");
  }

  MyPhysicsTable* rover = head;
  while (rover) {
    if (name == *(rover->name))
      return rover;
    rover = rover->next;
  }

  rover= new MyPhysicsTable;
  rover->Build(name);
  rover->next = head->next;  // Always keep head pointing to default
  head->next = rover;

  return rover;
}


void GLG4Scint::MyPhysicsTable::Build(const G4String& newname) {
  delete name;
  delete[] data;

  // Name in the physics list, i.e. "" or "heavy" or "alpha" etc.
  // This is a suffix on material property vectors in RATDB
  name = new G4String(newname);

  const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
  length = G4Material::GetNumberOfMaterials();

  // vector of Entrys for everything in MATERIALS
  data = new Entry[length];

  // Create new physics tables
  for (G4int i=0; i<length; i++) {
    const G4Material* aMaterial = (*theMaterialTable)[i];
    data[i].Build(*name, i, aMaterial->GetMaterialPropertiesTable());
  }
}


// Constructor for Entry
GLG4Scint::MyPhysicsTable::Entry::Entry() {
  spectrumIntegral = reemissionIntegral = timeIntegral = NULL;
  I_own_spectrumIntegral = I_own_timeIntegral = false;
  resolutionScale = 1.0;
  light_yield = 0.0;
  DMsConstant = birksConstant = ref_dE_dx = 0.0;
  QuenchingArray = NULL;
}


// Destructor for Entry
GLG4Scint::MyPhysicsTable::Entry::~Entry() {
  if (I_own_spectrumIntegral) {
   delete spectrumIntegral;
   delete reemissionIntegral;
  }

  if (I_own_timeIntegral)
    delete timeIntegral;

  delete QuenchingArray;
}


// Build for Entry
void GLG4Scint::MyPhysicsTable::Entry::Build(
    const G4String& name,
    int i,
    G4MaterialPropertiesTable *aMaterialPropertiesTable) {
  // Delete old data
  if (I_own_spectrumIntegral) {
    delete spectrumIntegral;
    delete reemissionIntegral;
  }

  if (I_own_timeIntegral) {
    delete timeIntegral;
  }

  // Set defaults
  spectrumIntegral = reemissionIntegral = timeIntegral = NULL;
  resolutionScale = 1.0;
  birksConstant = ref_dE_dx = 0.0;    
  light_yield = 0.0;    
  QuenchingArray = NULL;

  // Exit, leaving default values, if no material properties
  if (!aMaterialPropertiesTable) {
    return;
  }
  
  // Retrieve vector of scintillation wavelength intensity
  // for the material from the material's optical
  // properties table ("SCINTILLATION")
  std::stringstream property_string;

  property_string.str("");
  property_string << "SCINTILLATION" << name;
  G4MaterialPropertyVector* theScintillationLightVector = 
    aMaterialPropertiesTable->GetProperty(property_string.str().c_str());

  property_string.str("");
  property_string << "SCINTILLATION_WLS" << name;
  G4MaterialPropertyVector* theReemissionLightVector = 
    aMaterialPropertiesTable->GetProperty(property_string.str().c_str());

  if (theScintillationLightVector && !theReemissionLightVector) {
    G4cout << "\nWarning! Found a scintillator without Re-emission spectrum";
    G4cout << " (probably a scintillator without WLS)" << G4endl;
    G4cout << "I will assume that for this material this spectrum is equal ";
    G4cout << "to the primary scintillation spectrum..." << G4endl;
    theReemissionLightVector = theScintillationLightVector;
  }
     
  if (theScintillationLightVector) {
    if (aMaterialPropertiesTable->ConstPropertyExists("LIGHT_YIELD"))
      light_yield=aMaterialPropertiesTable->GetConstProperty("LIGHT_YIELD");
    else { 
      G4cout << "\nWarning! Found a scintillator without LIGHT_YIELD parameter.";
      G4cout << "\nI will assume that for this material this parameter is ";
      G4cout << "implicit in the scintillation integral..." << G4endl;

      // If no light yield, it's no scintillator // THIS IS A BUG
      //theScintillationLightVector=NULL;
    }

    // find the integral
    if (theScintillationLightVector == NULL)
      spectrumIntegral = NULL;
    else
      spectrumIntegral = RAT::Integrate_MPV_to_POFV(theScintillationLightVector);

    reemissionIntegral = RAT::Integrate_MPV_to_POFV(theReemissionLightVector);   
    I_own_spectrumIntegral = true;
  }
  else {
    // Use default integral (possibly null)
    spectrumIntegral =
      MyPhysicsTable::GetDefault()->GetEntry(i)->spectrumIntegral;
    reemissionIntegral = spectrumIntegral;
    I_own_spectrumIntegral = false;
  }

  // Retrieve vector of scintillation time profile
  // for the material from the material's optical
  // properties table ("SCINTWAVEFORM")
  property_string.str("");
  property_string << "SCINTWAVEFORM" << name;
  G4MaterialPropertyVector* theWaveForm = 
    aMaterialPropertiesTable->GetProperty(property_string.str().c_str());
  
  if (theWaveForm) {
    // Do we have time-series or decay-time data?
    if (theWaveForm->GetMinLowEdgeEnergy() >= 0.0) {
      // We have digitized waveform (time-series) data
      // Find the integral
      timeIntegral = RAT::Integrate_MPV_to_POFV(theWaveForm);
      I_own_timeIntegral = true;
    }
    else {
      // We have decay-time data.
      // Sanity-check user's values:
      // Issue a warning if they are nonsense, but continue
      if (theWaveForm->Energy(theWaveForm->GetVectorLength() - 1) > 0.0) {
        G4cerr << "GLG4Scint::MyPhysicsTable::Entry::Build():  "
               << "SCINTWAVEFORM" << name
               << " has both positive and negative X values.  "
               << " Undefined results will ensue!\n";
      }

      G4double maxtime= -3.0 * (theWaveForm->GetMinLowEdgeEnergy());
      G4double mintime= -1.0 * (theWaveForm->GetMaxLowEdgeEnergy());
      G4double bin_width = mintime / 100;
      int nbins = (int) (maxtime/bin_width) + 1;
      G4double* tval = new G4double[nbins];
      G4double* ival = new G4double[nbins];
      {
        for (int i=0; i<nbins; i++) {
          tval[i] = i * maxtime / nbins;
          ival[i] = 0.0;
        }
      }
      
      for (unsigned int j=0; j < theWaveForm->GetVectorLength(); j++) {
        G4double ampl = (*theWaveForm)[j];
        G4double decy = theWaveForm->Energy(j);
        {
          for (int i=0; i<nbins; i++) {
            ival[i] += ampl * (1.0 - exp(tval[i] / decy));
          }
        }
      }

      {
        for (int i=0; i<nbins; i++) {
          ival[i] /= ival[nbins-1];
        }
      }

      timeIntegral = new G4PhysicsOrderedFreeVector(tval, ival, nbins);
      I_own_timeIntegral = true;

      // in Geant4.0.0, G4PhysicsOrderedFreeVector makes its own copy
      // of any array passed to its constructor, so ...
      delete[] tval;
      delete[] ival;
    }
  }
  else {
    // Use default integral (possibly null)
    timeIntegral = MyPhysicsTable::GetDefault()->GetEntry(i)->timeIntegral;
    I_own_timeIntegral = false;
    
  }

  // Retrieve vector of scintillation "modifications"
  // for the material from the material's optical
  // properties table ("SCINTMOD")    
  property_string.str("");
  property_string << "SCINTMOD" << name;
  G4MaterialPropertyVector* theScintModVector = 
    aMaterialPropertiesTable->GetProperty(property_string.str().c_str());

  if (theScintModVector == NULL) {
    // Use default if not particle-specific value given
    theScintModVector = 
      aMaterialPropertiesTable->GetProperty("SCINTMOD");
  }
  
  if (theScintModVector) {
    // Parse the entries in ScintMod
    // ResolutionScale= ScintMod(0);
    // BirksConstant= ScintMod(1);
    // Ref_dE_dx= ScintMod(2);
    for (unsigned int i=0; i < theScintModVector->GetVectorLength(); i++) {
      G4double key = theScintModVector->Energy(i);
      G4double value = (*theScintModVector)[i];

      if (key == 0) {
        resolutionScale = value;
      }
      else if (key == 1) {
        birksConstant = value;
      }
      else if (key == 2) {
        ref_dE_dx = value;
      }
      else {
        G4cerr << "GLG4Scint::MyPhysicsTable::Entry::Build"
               << ":  Warning, unknown key " << key
               << "in SCINTMOD" << name << G4endl;
      }
    }
  }

  property_string.str("");
  property_string << "QF" << name;
  QuenchingArray = aMaterialPropertiesTable->GetProperty(property_string.str().c_str());
}


void GLG4Scint::SetNewValue(G4UIcommand* command, G4String newValues) {
  G4String commandName= command -> GetCommandName();
  if (commandName == "on") {
    doScintillation= true;
  }
  else if (commandName == "off") {
    doScintillation= false;
  }
  else if (commandName == "reemission") {
    char* endptr;
    G4int i = strtol((const char*)newValues, &endptr, 0);
    if (*endptr != '\0') { // non-numerical argument
      if (!(i = strcmp((const char*)newValues, "on"))) {
        doReemission = true;
      }
      else if (!(i = strcmp((const char*)newValues, "off"))) {
        doReemission = false;
      }
      else {
        G4cerr << "Command /glg4scint/reemission given unknown parameter "
               << '\"' << newValues << '\"' << G4endl
               << "  old value unchanged: "
               << ( doReemission ? "on" : "off" ) << G4endl;
      }
    }
    else {
      doReemission= (i != 0);
    }
  }
  else if (commandName == "maxTracksPerStep") {
    G4int i = strtol((const char*)newValues, NULL, 0);
    if (i > 0) {
      maxTracksPerStep = i;
    }
    else {
      G4cerr << "Value must be greater than 0, old value unchanged" << G4endl;
    }
  }
  else if (commandName == "meanPhotonsPerSecondary") {
    G4double d = strtod((const char*)newValues, NULL);
    if (d >= 1.0) {
      meanPhotonsPerSecondary = d;
    }
    else {
      G4cerr << "Value must be >= 1.0, old value unchanged" << G4endl;
    }
  }
  else if (commandName == "verbose") {
    // Sets same verbosity for all tables
    for (unsigned int i=0; i<masterVectorOfGLG4Scint.size(); i++)
      masterVectorOfGLG4Scint[i]->SetVerboseLevel(strtol((const char *)newValues, NULL, 0));
  }
  else if (commandName == "dump") {
    G4std::vector<GLG4Scint *>::iterator it = masterVectorOfGLG4Scint.begin();
    for (; it != masterVectorOfGLG4Scint.end(); it++) {
      (*it)->DumpInfo();
    }
  }
  else if (commandName == "setQF") {
    G4double d= strtod((const char *)newValues, NULL);
    if (d <= 1.0) {
      SetQuenchingFactor(d);
      UserQF = true;
    }
    else {
      G4cerr << "The quenching factor is <= 1.0, old value unchanged" << G4endl;
    }
  }
  else {
    G4cerr << "No GLG4Scint command named " << commandName << G4endl;
  }
}

G4String GLG4Scint::GetCurrentValue(G4UIcommand* command) {
  G4String commandName = command->GetCommandName();
  if (commandName == "on" || commandName == "off") {
    return doScintillation ? "on" : "off";
  }
  else if (commandName == "reemission") {
    return doReemission ? "1" : "0";
  }
  else if (commandName == "maxTracksPerStep") {
    char outbuff[64];
    sprintf(outbuff, "%d", maxTracksPerStep);
    return G4String(outbuff);
  }
  else if (commandName == "meanPhotonsPerSecondary") {
    char outbuff[64];
    sprintf(outbuff, "%g", meanPhotonsPerSecondary);
    return G4String(outbuff);
  }
  else if (commandName == "verbose") {
    char outbuff[64];
    sprintf(outbuff, "%d", verboseLevel);
    return G4String(outbuff);
  }
  else if (commandName == "dump") {
    return "?/glg4scint/dump not supported";
  }
  else if(commandName=="setQF"){
    char outbuff[64];
    sprintf(outbuff, "%g", GetQuenchingFactor());
    return G4String(outbuff);
  }
  else {
    return (commandName + " is not a valid GLG4Scint command");
  }
}

