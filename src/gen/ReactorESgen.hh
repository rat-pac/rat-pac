// Created by Daniel Hellfeld (07/2015)
// Follows directly from ESgen.hh

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
	void SetReactorPower(G4double);
	void SetEnergyPerFission(G4double);
	void SetDetectorStandoff(G4double);
	void SetAcquisitionTime(G4double);
	void SetWaterVolume(G4double);
	void CalculateNumEvents();
      
	G4double GetU235FissionFrac();
	G4double GetU238FissionFrac();
	G4double GetPu239FissionFrac();
	G4double GetPu241FissionFrac();
	G4double GetReactorPower();
	G4double GetEnergyPerFission();
	G4double GetDetectorStandoff();
	G4double GetAcquisitionTime();
	G4double GetWaterVolume();

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
	  double reactorpower;
	  double energyperfission;
	  double detectorstandoff;
	  double acquisitiontime;
	  double watervolume;
	  double num_events;
    
      ReactorESgenMessenger* messenger;
  };

} // namespace RAT

#endif
