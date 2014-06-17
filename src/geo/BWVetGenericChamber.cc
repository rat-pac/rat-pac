//
// $Id: BWVetGenericChamber.cc,v 1.2 2005/11/01 04:44:21 sekula Exp $
// --------------------------------------------------------------
//
#include <RAT/BWVetGenericChamber.hh>
#include <RAT/BWVetGenericChamberHit.hh>
#include <G4RunManager.hh>
#include <G4Run.hh>
#include <G4HCofThisEvent.hh>
#include <G4TouchableHistory.hh>
#include <G4Track.hh>
#include <G4Step.hh>
#include <G4SDManager.hh>
#include <G4ios.hh>
#include <G4UImanager.hh>
#include <G4UIcommand.hh>
#include <G4UnitsTable.hh>

namespace RAT {


BWVetGenericChamber::BWVetGenericChamber(G4String name)
:G4VSensitiveDetector(name)
{
  G4String HCname;
  collectionName.insert(HCname="genericchamberColl");
  HCID = -1;
}

BWVetGenericChamber::~BWVetGenericChamber(){;}

void BWVetGenericChamber::Initialize(G4HCofThisEvent*HCE)
{  
  int deb=0; //G4int(db["veto_debugging"]);
  if(deb!=0)G4cout << "BWVetGenericChamber::Initialize start." << G4endl;
  _hitsCollection = new BWVetGenericChamberHitsCollection
                   (SensitiveDetectorName,collectionName[0]);

  if(deb!=0)
    {
      G4cout << "BWVetGenericChamber::Initialize hit collection address is "
	     << _hitsCollection
	     << G4endl;
    }
  if(HCID<0)
    { HCID = G4SDManager::GetSDMpointer()->GetCollectionID(_hitsCollection); }
 if(deb!=0)
    G4cout << "BWVetGenericChamber::Initialize hit collection ID = " << HCID << G4endl;
  HCE->AddHitsCollection(HCID,_hitsCollection);
  
  // store pointer to hit collection
  _HCE = HCE;

  //
  // Initialize the data members used to store information for the RAT Event
  //

  // Empty the hit information vectors
  _hit_x.clear();
  _hit_y.clear();
  _hit_z.clear();
  _hit_E.clear();
  _hit_time.clear();
  _hit_uid.clear();
  _hit_pdg.clear();
  _hit_volume.clear();
  fLastTrackID = fLastEventID = -1;

  if(deb!=0)G4cout << "BWVetGenericChamber::Initialize end." << G4endl;
}


G4bool BWVetGenericChamber::ProcessHits(G4Step*aStep,G4TouchableHistory* /*ROhist*/)
{
  // Don't store optical photons!
  if (aStep->GetTrack()->GetDefinition()->GetParticleName() == "opticalphoton")
    return true; // method description neglects to say what retval is for

  int deb = 0; // G4int(db["veto_debugging"]);
  
  if(deb!=0)
  {
    G4cout << "BWVetGenericChamber::ProcessHits start." << G4endl;
    G4cout << "BWVetGenericChamber::ProcessHits getting energy deposited." << G4endl;
  }
  G4double edep = aStep->GetTotalEnergyDeposit();
  G4double dl = aStep->GetStepLength();

  if (deb) G4cout << "   Energy deposited: " << edep << G4endl;

//   if(edep==0.) return true;

  G4Track* aTrack=aStep->GetTrack();
  G4ParticleDefinition* part=aTrack->GetDefinition();
  int pdg=part->GetPDGEncoding();

  if (deb) 
    G4cout << "  track information: " << G4endl
	   << "    G4Track Pointer: " << aTrack  << G4endl
	   << "    Particle Definition Pointer: " << part << G4endl
	   << "    Particle PDG Encoding: " << pdg << G4endl;

  if (deb) G4cout << "BWVetGenericChamber::ProcessHits getting global time." << G4endl;
  G4StepPoint* preStepPoint = aStep->GetPreStepPoint();
  G4Material* m=preStepPoint->GetMaterial();
  G4String mname=m->GetName();
  G4double d=m->GetDensity();
  G4String f=m->GetChemicalFormula();


  G4TouchableHistory* theTouchable
    = (G4TouchableHistory*)(preStepPoint->GetTouchable());
  // Get the copy number of this element. This may not be unique,
  // if the mother is also a replica and occurs more than once.
  G4int copyNo = theTouchable->GetVolume()->GetCopyNo();
  // Get the mother copy number, and offset it by 1000. This will help
  // form the basis of the chamber ID.
  G4int motherCopyNo = (theTouchable->GetVolume(1)->GetCopyNo()) + 1000;
  G4int uid = motherCopyNo + copyNo; // unique identifier of chamber

  // construct a unique identifier for this copy
  G4int ivol = 0;
  G4int idOffset = 1;
  uid = 0;
  
  if(deb!=0)
    G4cout << "History level: " << theTouchable->GetHistoryDepth() << G4endl;

  while( ivol < theTouchable->GetHistoryDepth() ) {
    if(deb!=0)G4cout << " * volume layer level = " << ivol << G4endl;
    uid += theTouchable->GetVolume(ivol)->GetCopyNo() * idOffset;
    idOffset *= 100;
    ivol++;
  }

  G4ThreeVector worldPos = preStepPoint->GetPosition();

  if(deb!=0)
    {
      G4cout << "Hit material name " << mname << G4endl;
      G4cout << "density           " << d << G4endl;
      G4cout << "formula           " << f << G4endl;
      G4cout << "edep " << G4BestUnit(edep,"Energy") << G4endl;
      G4cout << "dl " << G4BestUnit(dl,"Length") << G4endl;
      G4cout << "pid " << pdg << G4endl;
      G4cout << "Position " << 
	G4BestUnit(worldPos.x(),"Length") << " " <<
	G4BestUnit(worldPos.y(),"Length") << " " <<
	G4BestUnit(worldPos.z(),"Length") << " " << G4endl;
      
      G4cout << " " << G4endl;
    }
  
  G4double hitTime = preStepPoint->GetGlobalTime();

  if(deb!=0)G4cout << "BWVetGenericChamber::ProcessHits checking for an existing hit in this element." << G4endl;
  // check if this finger already has a hit
  G4int ix = -1;

  if(deb!=0)
  {
    G4cout << "BWVetGenericChamber::ProcessHits hit collection address is "
	   << _hitsCollection
	   << G4endl;
    G4cout << "BWVetGenericChamber: Hit ID = " << uid 
	   << " and position: " << worldPos << G4endl;
  }

  int eventID = G4RunManager::GetRunManager()->GetCurrentRun()->GetNumberOfEvent();
  int trackID = aStep->GetTrack()->GetTrackID();
  
  // get volume name of hit as well 
  // note that this volume name may not be unique
  G4String vol = theTouchable->GetVolume()->GetName();

  if (fLastEventID != eventID || fLastTrackID != trackID) {
      // Fill the hit information
      _hit_x.push_back( worldPos.x());
      _hit_y.push_back( worldPos.y());
      _hit_z.push_back( worldPos.z());
      _hit_E.push_back( edep );
      _hit_time.push_back( hitTime );
      _hit_uid.push_back( uid );
      _hit_pdg.push_back( pdg );
      _hit_volume.push_back( vol );
      fLastEventID = eventID;
      fLastTrackID = trackID;
  }



  /*G4int hitfreq = G4int(db["veto_hit_frequency"]);*/
  
  if ( NULL == _hitsCollection ) {
    if(deb!=0)G4cout << "BWVetGenericChamber::ProcessHits hit collection null. Reloading from HCofEThisEvent." << G4endl;
    if (_HCE) {
      _hitsCollection = (BWVetGenericChamberHitsCollection*)(_HCE->GetHC(HCID));
      if(deb!=0)
	{
	  G4cout << "BWVetGenericChamber::ProcessHits   * hit collection address is "
		 << _hitsCollection
		 << G4endl;
	}
    } else {
      if(deb!=0)G4cout << "BWVetGenericChamber::ProcessHits   (E) HCofEThisEvent pointer is NULL!" << G4endl;
    }
    
  }


  
  if (_hitsCollection) {
    for(G4int i=0;i<_hitsCollection->entries();i++)
      {
// 	G4cout << "  * BWVetGenericChamber::ProcessHits checking hit " 
// 	       << i + 1
// 	       << " of "
// 	       << _hitsCollection->entries()
// 	       << G4endl;
	if(deb!=0)G4cout << "  * this hit ID is " << (*_hitsCollection)[i]->GetID() << G4endl;
	if((*_hitsCollection)[i]->GetID()==uid)
	  {
	    ix = i;
	    break;
	  }
      }


    // if it has, then take the earlier time
    if(ix>=0)
      {
	if(deb!=0)G4cout << "BWVetGenericChamber::ProcessHits use existing earlier time for hit." << G4endl;
	if((*_hitsCollection)[ix]->GetTime()>hitTime)
	  { (*_hitsCollection)[ix]->SetTime(hitTime); }
      }
    else
      // if not, create a new hit and set it to the collection
      {
	if(deb!=0)G4cout << "BWVetGenericChamber::ProcessHits creating a new hit." << G4endl;
	BWVetGenericChamberHit* aHit = new BWVetGenericChamberHit(uid,hitTime);
	G4VPhysicalVolume* thePhysical = theTouchable->GetVolume();
	aHit->SetLogV(thePhysical->GetLogicalVolume());
	G4AffineTransform aTrans = theTouchable->GetHistory()->GetTopTransform();
	aTrans.Invert();
	aHit->SetRot(aTrans.NetRotation());
	aHit->SetPos(aTrans.NetTranslation());
	_hitsCollection->insert( aHit );
	aHit->Print();
	aHit->Draw();
	if(deb!=0)G4cout << "  * Drawing Hit " << uid << G4endl;
      }
  }
  if(deb!=0)G4cout << "BWVetGenericChamber::ProcessHits end." << G4endl;
  return true;
}

void BWVetGenericChamber::EndOfEvent(G4HCofThisEvent* /*HCE*/)
{;}


} // namespace RAT
