#ifndef __RAT_ESgen_2__
#define __RAT_ESgen_2__

#include <RAT/LinearInterp.hh>
#include <G4ThreeVector.hh>
#include <CLHEP/Vector/LorentzVector.h>

namespace RAT {

  class ESgenMessenger_2;

  class ESgen_2 {
  public:
    ESgen_2();
    ~ESgen_2();

    CLHEP::HepLorentzVector GenerateEvent(const G4ThreeVector& nu_dir);
  
    void SetFissionFractions(G4double, G4double, G4double, G4double);
    void DefaultValues();

  protected:
      G4double GetAntiNuEnergy();
      G4double GetElectronEnergy(G4double);
      G4ThreeVector GetScatteringAngle(G4double, G4double);
      
      double m_e;
      double sigma_0;
      double g_1;
      double pi;
      double g_2;
      double U235fraction;
      double U238fraction;
      double Pu239fraction;
      double Pu241fraction;
    
      ESgenMessenger_2* messenger;
  };

} // namespace RAT

#endif
