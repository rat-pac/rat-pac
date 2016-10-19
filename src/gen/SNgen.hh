#ifndef __RAT_SNgen__
#define __RAT_SNgen__

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
#include "TF1.h"
#include "TGraph.h"
#include "RAT/DB.hh"


namespace RAT {
    
    // Forward delcarations within the namespace
    class SNgenMessenger;
    
    class SNgen {
    public:
        SNgen();
        ~SNgen();
        
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
        
        inline double GetIBDAmplitude()   {return IBDAmp;} ;
        inline double GetESAmplitude()   {return ESAmp;} ;
        inline double GetCCAmplitude()  {return CCAmp;} ;
        inline double GetICCAmplitude()  {return ICCAmp;} ;
        inline double GetNCAmplitude()  {return NCAmp;} ;
        inline double GetINCAmplitude()  {return INCAmp;} ;
        inline int GetModel()  {return ModelTmp;} ;
        
        void SetIBDAmplitude(    double IBDAm =  IBDDEFAULT);
        void SetESAmplitude (    double ESAm = ESDEFAULT);
        void SetCCAmplitude (    double CCAm = CCDEFAULT);
        void SetICCAmplitude(double ICCAm = ICCDEFAULT);
        void SetNCAmplitude ( double NCAm = NCDEFAULT);
        void SetINCAmplitude ( double INCAm = INCDEFAULT);
        void SetModel ( double ModelTm= MODELDEFAULT);
        
        void LoadSpectra();
        

        
        double GetIBDRandomEnergy();
        double GetESRandomEnergy();
        double GetCCRandomEnergy();
        double GetICCRandomEnergy();
        double GetNCRandomEnergy();
        double GetNCRandomNuEnergy();
        double GetINCRandomEnergy();
        

    protected:
//        double IBDTGraph2TF1(Double_t *x, Double_t *);
//        double ESTGraph2TF1(Double_t *x, Double_t *);
//        double CCTGraph2TF1(Double_t *x, Double_t *);
//        double ICCTGraph2TF1(Double_t *x, Double_t *);
//        double NCTGraph2TF1(Double_t *x, Double_t *);
//        
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
        
        double IBDAmp;
        double ESAmp;
        double CCAmp;
        double ICCAmp;
        double NCAmp;
        double INCAmp;
        double ModelTmp;
        
        static const double IBDDEFAULT;
        static const double ESDEFAULT;
        static const double CCDEFAULT;
        static const double ICCDEFAULT;
        static const double NCDEFAULT;
        static const double INCDEFAULT;
        static const int    MODELDEFAULT;
        
//        TGraph *graphIBD;
//        TGraph *graphES;
//        TGraph *graphCC;
//        TGraph *graphICC;
//        TGraph *graphNC;
//        
        TF1 *funcIBD;
        TF1 *funcES;
        TF1 *funcCC;
        TF1 *funcICC;
        TF1 *funcNC;
        TF1 *funcNCNU;
        TF1 *funcINC;
        
        std::vector<double> spec_E;		// spectrum energy values
        std::vector<double> spec_mag;
        DBLinkPtr	_lspec;				// link to spectrum entry in database

        //
        
        // Allows the user to change parameters via the command line.
        SNgenMessenger* messenger;
    };
    
} // namespace RAT

#endif
