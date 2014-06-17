#ifndef __RAT_ESgen__
#define __RAT_ESgen__

// Generates an neutrino-elastic scattering event, based on the
// cross-section as function of neutrino energy and the electron's
// recoil energy.  Allow for variations in the weak mixing angle and
// the possibility of a neutrino magnetic moment
//
// J. Formaggio (UW) -02/09/2005

// Converted to Geant4+GLG4Sim+RAT by Bill Seligman (07-Feb-2006).
// I'm following the code structure of the IBD classes:
// RATVertexGen_ES handles the G4-related tasks of constructing an
// event, while this class deals with the physics of the
// cross-section.  Some of the code (the flux in particular) is copied
// from IBDgen.

#include <RAT/LinearInterp.hh>
#include <G4ThreeVector.hh>
#include <CLHEP/Vector/LorentzVector.h>

namespace RAT {

  // Forward delcarations within the namespace
  class ESgenMessenger;

  class ESgen {
  public:
    ESgen();
    ~ESgen();
  
    // Generate random event vectors
    //    Pass in the neutrino direction (unit vector)
    //    Returns 4-momentum vectors for resulting electron.
    CLHEP::HepLorentzVector GenerateEvent(const G4ThreeVector& nu_dir);
  
    // Flux as a function of energy.  Interpolated from table in ES RATDB table
    double Flux(float E) const { return rmpflux(E); };

    void Reset();
    void Show();

    void SetMixingAngle(double sin2thw = WEAKANGLE);
    void SetNeutrinoMoment(double vMu = 0.0);
  
    double GetXSec(double Enu, double T);

    void SetXSecMax(int ntry = NTRIAL);

    double GetRandomNumber(double rmin = 0., double rmax = 1.);

    inline bool GetNormFlag() {return FlagNorm;};
    inline void SetNormFlag(bool iFlag) {FlagNorm = iFlag;};

    inline double GetMixingAngle()   {return SinSqThetaW;} ;
    inline double GetMagneticMoment(){return MagneticMoment;} ;
    inline double GetXSecMax() {return XSecMax;} ;

  protected:
    LinearInterp<double> rmpflux;
    double Emax;
    double Emin;
    double FluxMax;

    bool   FlagNorm;

    double SinSqThetaW;
    double MagneticMoment;
    double XSecMax;

    double massElectron;
    static const double WEAKANGLE;
    static const int NTRIAL;

    // Allows the user to change parameters via the command line.
    ESgenMessenger* messenger;
  };

} // namespace RAT

#endif
