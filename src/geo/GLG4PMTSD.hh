// This file is part of the GenericLAND software library.
// $Id: GLG4PMTSD.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
//  GLG4PMTSD.hh
//
//   Records total number of hits and "waveform" on each PMT,
//   with no PMT TTS or digitizer bandwidth effects included.
//   (Convolve the hit waveform with the TTS+digitizer single-hit function
//   to get the resulting digitized waveform.)
//
//   Bypasses Geant4 hit collection mechanism.
//
//   Author:  Glenn Horton-Smith, 2000/01/28 
//

#ifndef GLG4PMTSD_h
#define GLG4PMTSD_h 1

#include "G4VSensitiveDetector.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class GLG4PMTSD : public G4VSensitiveDetector
{
  protected:
      int max_pmts;
      int pmt_no_offset;
      int my_id_pmt_size;
      // enum { max_waveform_ns= 200 };
  
  public:
      G4int *hit_sum;  /* indexed by pmt number */
      //typedef G4int waveform_t[max_waveform_ns];
      //waveform_t *hit_waveform; /* indexed by pmt number */
  
      G4int n_pmt_hits;   /* # of hits,       calculated at EndOfEvent */
      G4int n_hit_pmts;   /* # of PMTs hit,   calculated at EndOfEvent */

  public:
  // member functions
      GLG4PMTSD(G4String name,
	      int max_pmts=1000000,
	      int pmt_no_offset=0,
	      int my_id_pmt_size= -1);
      virtual ~GLG4PMTSD();

      virtual void Initialize(G4HCofThisEvent*HCE);
      virtual void EndOfEvent(G4HCofThisEvent*HCE);
      virtual void clear();
      virtual void DrawAll();
      virtual void PrintAll();

      void SimpleHit( G4int ipmt,
		      G4double time,
		      G4double kineticEnergy,
		      const G4ThreeVector & position,
		      const G4ThreeVector & momentum,
		      const G4ThreeVector & polarization,
		      G4int iHitPhotonCount,
		      G4int trackID=-1,
		      G4bool prepulse=false );
  
  protected:
      virtual G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
};

#endif
