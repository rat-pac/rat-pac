// This file is part of the GenericLAND software library.
// $Id: GLG4BoxSD.hh,v 1.1 2005/08/30 19:55:20 volsung Exp $
//
//  GLG4BoxSD.hh
//
//   Records total number of charged and neutral crossing each plane
//   above some TOTAL energy cut (by default, 1.5 MeV).  Also records
//   the energy deposition per radiation length.  Also reports primary
//   particle's final exiting/stopping position, time, energy.
//   Granularity of histograms is 1/2 rad length.
//
//   Eschews Geant4 hit collection mechanism.
//
//   Author:  Glenn Horton-Smith, 2000/11/20
//   Last modified: 2000/11/20
//

#ifndef GLG4BOXSD_h
#define GLG4BOXSD_h 1

#include "G4VSensitiveDetector.hh"
#include "G4Timer.hh"

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class GLG4BoxSD : public G4VSensitiveDetector
{
public:
  // member functions
  GLG4BoxSD(G4String name);
  virtual ~GLG4BoxSD();
  
  virtual void Initialize(G4HCofThisEvent*HCE);
  virtual void EndOfEvent(G4HCofThisEvent*HCE);
  virtual void clear();
  virtual void DrawAll();
  virtual void PrintAll();

  void SetZ0( G4double newZ0 ) { z0= newZ0; }
  void SetRadLength( G4double newRadLength ) { radLength= newRadLength; }
  void SetECut( G4double e ) { eCut= e; }
  void SetGCut( G4double e ) { gCut= e; }
  G4double GetZ0() { return z0; }
  G4double GetRadLength() { return radLength; }
  G4double GetECut() { return eCut; }
  G4double GetGCut() { return gCut; }
  
protected:
  G4double eCut;
  G4double gCut;
  G4double radLength;
  G4double z0;
  enum {nbin= 40};
  G4double tot_edep;
  G4double h_edep[nbin];
  G4int h_ng[nbin], h_ne[nbin];
  virtual G4bool ProcessHits(G4Step*aStep,G4TouchableHistory*ROhist);
};




#endif
