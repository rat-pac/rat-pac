#ifndef __RAT_FitPathProc__
#define __RAT_FitPathProc__

#include <string>
#include <vector>
#include <RAT/SimulatedAnnealing.hh>
#include <RAT/Processor.hh>

namespace RAT {

namespace DS {
  class Root;
  class EV;
}

typedef struct {
    double x,y,z,px,py,pz,t;
} hit;
  
class FitPathProc : public Processor, public Minimizable {
public:
  FitPathProc();
  virtual ~FitPathProc() {}

  virtual Processor::Result Event(DS::Root* ds, DS::EV* ev);
  
  double operator()(double *params);

protected:
    //per-event hit data
    std::vector<hit> fHits;
    
    std::vector<double> fSeed;
    
    //whole-run constants
    double fDirectProb, fOtherProb, fPhotocathodeArea;
    double fLightSpeed, fCherenkovMultiplier;
    double fDirectTime0, fDirectTimeStep, fOtherTime0, fOtherTimeStep;
    std::vector<double> fDirectTimeProb, fOtherTimeProb;
    double fCosAlpha0, fCosAlphaStep;
    std::vector<double> fCosAlphaProb;
    
    //minimization parameters
    int fStage;
    size_t fNumCycles, fNumEvals;
    double fPosSigma0, fPosSigma1, fThetaSigma, fPhiSigma, fTimeSigma0, fTimeSigma1;
    double fTemp0, fTemp1, fAlpha;
    
    inline double PDFDirectTime(double tresid) {
        const int i = (int)((tresid - fDirectTime0)/fDirectTimeStep);
        if (i < 0 || i >= (int)fDirectTimeProb.size()) return 0.0;
        return fDirectTimeProb[i];
    }

    inline double PDFOtherTime(double tresid) {
        const int i = (int)((tresid - fOtherTime0)/fOtherTimeStep);
        if (i < 0 || i >= (int)fOtherTimeProb.size()) return 0.0;
        return fOtherTimeProb[i];
    }

    inline double PDFCherenkovAngle(double cosalpha) {
        const int i = (int)((cosalpha - fCosAlpha0)/fCosAlphaStep);
        if (i < 0 || i >= (int)fCosAlphaProb.size()) return 0.0;
        return fCosAlphaProb[i];
    }
    
    double FTPProbability(double x, double y, double z, double dx, double dy, double dz, double t);
    
    double AvgSquareTimeResid(double x, double y, double z, double t);
    
};

} // namespace RAT

#endif

