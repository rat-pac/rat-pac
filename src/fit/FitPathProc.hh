#ifndef __RAT_FitPathProc__
#define __RAT_FitPathProc__

#include <string>
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
    
    //whole-run constants
    double fDirectProb, fOtherProb, fPhotocathodeArea;
    double fLightSpeed, fCherenkovMultiplier;
    
    //minimization parameters
    size_t fNumCycles, fNumEvals;
    double fPosSigma, fCosThetaSigma, fPhiSigma, fTimeSigma;
    double fTemp0, fAlpha, fFTol;
    
    inline double PDFDirectTime(double tresid) {
        return 1.0;
    }

    inline double PDFOtherTime(double tresid) {
        return 1.0;
    }

    inline double PDFCherenkovAngle(double cosalpha) {
        return 1.0;
    }
    
    double Probability(double x, double y, double z, double dx, double dy, double dz, double t);
    
};

} // namespace RAT

#endif

