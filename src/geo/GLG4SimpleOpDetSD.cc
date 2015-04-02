// This file is part of the GenericLAND software library.
// $Id: GLG4SimpleOpDetSD.cc,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
//  GLG4SimpleOpDetSD.cc
//
//   Records total number of hits on each SimpleOpDet.
//   Uses Geant4-style hit collection to record hit time, place, etc.
//
//  GLG4 version by Glenn Horton-Smith December, 2004.
//  Based on earlier work by O. Tajima and G. Horton-Smith
//

#include "GLG4SimpleOpDetSD.hh"
#include "GLG4VEventAction.hh"

#include "G4Track.hh"
#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4ios.hh"
#include "G4SDManager.hh"

#include "GLG4Scint.hh"  // for doScintilllation and total energy deposition info
#include "G4VSolid.hh" // for access to solid store
#include "Randomize.hh"
#include <RAT/Log.hh>

#include <string.h>  // for memset

GLG4SimpleOpDetSD::GLG4SimpleOpDetSD(G4String name, int arg_max_opdets, int arg_opdet_no_offset )
:G4VSensitiveDetector(name)
{
  max_opdets= arg_max_opdets;
  opdet_no_offset= arg_opdet_no_offset;
  my_id_opdet_size= 0;
  
  hit_sum= new G4int[max_opdets];
}

GLG4SimpleOpDetSD::~GLG4SimpleOpDetSD()
{
  if (hit_sum)
    delete[] hit_sum;
}


void GLG4SimpleOpDetSD::Initialize(G4HCofThisEvent*)
{
  memset(hit_sum, 0, sizeof(hit_sum[0])*max_opdets);
  n_opdet_hits= n_hit_opdets= 0;
}


G4bool GLG4SimpleOpDetSD::ProcessHits(G4Step* aStep, G4TouchableHistory* hist)
{

  if (aStep->GetTrack()->GetDefinition()->GetParticleName() != "opticalphoton")
    return false;
  // things to do:
  // (1) get optical ID number (means we need system of indexing!)
  // (2) get track info
  // (3) call SimpleHit
  // (4) kill photon

  // get optical id
  G4StepPoint* prestep = aStep->GetPreStepPoint();
  G4VPhysicalVolume* pv = prestep->GetPhysicalVolume();
  int channelid = pv_to_channelid_map[pv];
  G4double time = aStep->GetTrack()->GetGlobalTime();
  G4double ke = aStep->GetTrack()->GetKineticEnergy();
  G4ThreeVector pos = aStep->GetTrack()->GetPosition();
  G4ThreeVector mom = aStep->GetTrack()->GetMomentum();
  G4ThreeVector pol = aStep->GetTrack()->GetPolarization();
  G4int N_pe = 1;
  G4int trackid = aStep->GetTrack()->GetTrackID();
  G4bool prepulse = false;
  //RAT::info << "GLG4SimpleOpDetSD detects photon in OpDet Channel " << channelid << "!" << newline;

  SimpleHit( channelid, time, ke, pos, mom, pol, N_pe, trackid, prepulse );
  
  aStep->GetTrack()->SetTrackStatus(fStopAndKill);
  return true;
}


// Here is the real "hit" routine, used by GLG4SimpleOpDetOpticalModel and by ProcessHits
// It is more efficient in some ways.
void GLG4SimpleOpDetSD::SimpleHit( G4int iopdet,
			   G4double time,
			   G4double kineticEnergy,
			   const G4ThreeVector &hit_position,
			   const G4ThreeVector &hit_momentum,
			   const G4ThreeVector &hit_polarization,
			   G4int iHitPhotonCount,
			   G4int trackID,
			   G4bool prepulse )
{
  G4int opdet_index = channelid_to_opdetindex[iopdet]-opdet_no_offset;
  if (opdet_index < 0 || opdet_index >= max_opdets)
    {
      G4cerr << "Error: GLG4SimpleOpDetSD::SimpleHit [" << GetName() << "] passed iopdet="
	     << iopdet << ", but max_opdets=" << max_opdets
	     << " and offset=" << opdet_no_offset << " !" << G4endl;
      return;
    }
  
  hit_sum[opdet_index]+= iHitPhotonCount;

  // create new GLG4HitPhoton, the way of recording photo hits on SimpleOpDets
  GLG4HitPhoton* hit_photon = new GLG4HitPhoton();
  hit_photon->SetPMTID((int)iopdet);
  hit_photon->SetTime((double) time );
  hit_photon->SetKineticEnergy((double) kineticEnergy );
  hit_photon->SetPosition( 
			  (double)hit_position.x(),
			  (double)hit_position.y(),
			  (double)hit_position.z()
			  );
  hit_photon->SetMomentum( 
			  (double)hit_momentum.x(),
			  (double)hit_momentum.y(),
			  (double)hit_momentum.z()
			  );
  hit_photon->SetPolarization( 
			      (double)hit_polarization.x(),
			      (double)hit_polarization.y(),
			      (double)hit_polarization.z()
			      );
  hit_photon->SetCount( iHitPhotonCount );
  hit_photon->SetTrackID( trackID );
  hit_photon->SetPrepulse( prepulse );
    
  //  GLG4VEventAction::GetTheHitPhotons()->AddHitPhoton(hit_photon);
  GLG4VEventAction::GetTheHitPMTCollection()->DetectPhoton(hit_photon); // we still use the PMT hit collection.
}


void GLG4SimpleOpDetSD::EndOfEvent(G4HCofThisEvent*)
{
  int iopdet;
    
  n_opdet_hits=0;
  n_hit_opdets=0;
  for (iopdet=0; iopdet<max_opdets; iopdet++) {
    if (hit_sum[iopdet]) {
      n_opdet_hits+= hit_sum[iopdet];
      n_hit_opdets++;
    }
  }

}


void GLG4SimpleOpDetSD::clear()
{} 

void GLG4SimpleOpDetSD::DrawAll()
{}

void GLG4SimpleOpDetSD::PrintAll()
{
}
