// Created by Daniel Hellfeld (07/2015)
// Directly follows from ESgen.hh

#ifndef __RAT_ReactorESgen__
#define __RAT_ReactorESgen__

#include <RAT/LinearInterp.hh>
#include <G4ThreeVector.hh>
#include <CLHEP/Vector/LorentzVector.h>

namespace RAT {

  class ReactorESgenMessenger;

  class ReactorESgen {
	  
  public:
    ReactorESgen();
    ~ReactorESgen();

    CLHEP::HepLorentzVector GenerateEvent(const G4ThreeVector& nu_dir);
  
    void SetDefaultFissionFractions();
	void CheckFissionFractions();
	void SetU235FissionFrac(G4double);
	void SetU238FissionFrac(G4double);
	void SetPu239FissionFrac(G4double);
	void SetPu241FissionFrac(G4double);
	G4double GetU235FissionFrac();
	G4double GetU238FissionFrac();
	G4double GetPu239FissionFrac();
	G4double GetPu241FissionFrac();

  protected:
      G4double GetAntiNuEnergy();
      G4double GetElectronEnergy(G4double);
      CLHEP::HepLorentzVector GetEmomentum(G4double, G4double, G4ThreeVector);
      
      double m_e;
      double sigma_0;
      double g_1;
      double pi;
      double g_2;
      double U235fraction;
      double U238fraction;
      double Pu239fraction;
      double Pu241fraction;
    
      ReactorESgenMessenger* messenger;
  };

} // namespace RAT

#endif
