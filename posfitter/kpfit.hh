#ifndef __KPFIT__
#define __KPFIT__

/**
   -------------------------------------------------------------
   KPFit: vertex reconstruction

   Minimizes a log likelihood using expected charge and time.
   Class inherits from interface class
   ROOT::Math::IMultiGenFunction in order to pass itself to
   the minimizer class TMinuitMinimizer.
   -------------------------------------------------------------
*/
#include <map>
#include <vector>
#include <string>
#include "Math/IFunction.h"
#include "Math/IFunctionfwd.h"
#include "RAT/DS/MC.hh"

class TMinuitMinimizer;
class TChain;

class KPFit : public ROOT::Math::IMultiGenFunction {

public:
  KPFit( std::string pmtinfofile );
  virtual ~KPFit();

  // methods to make concrete as a derived class of IMultiGenFunction
  virtual ROOT::Math::IBaseFunctionMultiDim* Clone() const;
  virtual unsigned int NDim() const { return 3; }; // function is loglikelihood( pos )

private:
  virtual double DoEval(const double * x) const;

  // methods belonging to class
public:
  bool fit( RAT::DS::MC* mcdata, double* fitted_pos );
  bool getwasrun() { return wasrun; };
  
protected:

  // data
  RAT::DS::MC* fMCdata;
  std::string fpmtinfofile;
  TChain* pmtinfo; // tree contaning pmt position info
  float fpmtpos[3];
  mutable std::map< int, std::vector<float>* > fpmtposmap;
  float* fpmtposdata;

  // parameters
  double fSeedPos[4];   //(x,y,z,t)
  double fPos[4];       // current position
  double finalpos[4];   // final position
  double fSigTime;      // timing res. for likelihood (ns)
  double fDarkRate;     // dark rate (Hz)
  double fTimeWindow;   // ns
  double fLightYield;   // light yield
  double fPromptCut;    // light yield
  double fAbsLength;    // absorbtion length (cm)
  int fNPMTS;
  int fNhoops;
  bool wasrun;

  void calcSeedFromWeightedMean();
  void getApproxBounds( const float* sipmpos, const double* pos, double* cozbound, double* phibound ) const;
  double getApproxSA( const double* pos, const float* sipmpos ) const;
  void getCorners( const float* sipmpos, std::vector< std::vector< float > >& corners ) const;
  double saOfCenteredRectangle( double xwidth, double yheight, double xdist ) const;
  double getSA( const double* _pos, const float* _sipmpos ) const;
  void storePMTInfo();
  void deletePMTInfo();
  void getPMTInfo( int pmtid, float* pmtpos ) const;

  // Minimizer
public:
  TMinuitMinimizer* minuit;
  

};

#endif
