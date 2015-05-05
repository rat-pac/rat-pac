#ifndef __RAT_FitPathProc__
#define __RAT_FitPathProc__

#include <string>
#include <vector>
#include <RAT/SimulatedAnnealing.hh>
#include <RAT/Processor.hh>


#include <Minuit2/FCNBase.h>

namespace RAT {

namespace DS {
  class Root;
  class EV;
}

typedef struct {
    double x,y,z,px,py,pz,t;
} hit;
  
class FitPathProc : public Processor, public Minimizable, public ROOT::Minuit2::FCNBase {
public:
  FitPathProc();
  virtual ~FitPathProc() {}

  virtual Processor::Result Event(DS::Root* ds, DS::EV* ev);
  
  double operator()(double *params); //Minimizable
  double operator()(const std::vector<double>& lParams ) const; //FCNBase
  double Up() const { return 0.5; } //FCNBase

protected:
    //per-event hit data
    std::vector<hit> fHits;
    TVector3 fFitPos; double fFitTime;
    std::vector<double> fSeed;
    
    //whole-run constants
    double fDirectProb, fOtherProb, fPhotocathodeArea;
    double fLightSpeed, fCherenkovMultiplier;
    double fDirectTime0, fDirectTimeStep, fOtherTime0, fOtherTimeStep;
    std::vector<double> fDirectTimeProb, fOtherTimeProb;
    double fCosAlpha0, fCosAlphaStep;
    std::vector<double> fCosAlphaProb;
    
    //minimization parameters
    bool fMigrad;
    int fStage;
    size_t fNumCycles, fNumEvals;
    double fPosSigma0, fPosSigma1, fThetaSigma, fPhiSigma, fTimeSigma0, fTimeSigma1;
    double fTemp0, fTemp1, fAlpha;
    
    inline double PDFDirectTime(const double tresid) const {
        const int i = (int)((tresid - fDirectTime0)/fDirectTimeStep);
        if (i < 0 || i >= (int)fDirectTimeProb.size()) return 0.0;
        return fDirectTimeProb[i];
    }

    inline double PDFOtherTime(const double tresid) const {
        const int i = (int)((tresid - fOtherTime0)/fOtherTimeStep);
        if (i < 0 || i >= (int)fOtherTimeProb.size()) return 0.0;
        return fOtherTimeProb[i];
    }

    inline double PDFCherenkovAngle(const double cosalpha) const {
        const int i = (int)((cosalpha - fCosAlpha0)/fCosAlphaStep);
        if (i < 0 || i >= (int)fCosAlphaProb.size()) return 0.0;
        return fCosAlphaProb[i];
    }
    
    double FTPProbability(const double x, const double y, const double z, const double dx, const double dy, const double dz, const double t) const;
    
    double AvgSquareTimeResid(double x, double y, double z, double t);
    
};

} // namespace RAT

#endif

