#ifndef __GenGeomDAR__
#define __GenGeomDAR__

#include <string>

class TH1D;
class TH2D;
class TGeoManager;
class TFile;
class TRandom3;

class GenGeomDAR {

 public:
  GenGeomDAR( std::string gdml_file, double* source_pos, double rmin, double rmax, double planez, double facex, double facey, int seed=-1 );
  ~GenGeomDAR();
  void GetPosition( double pos[], double& relative_weight );
  void MakeDiagnosticPlots( std::string outfile="output_gengeomdarplots.root" );
  
 protected:
  
  void GetDirection( double dir[] );
  void GenPosition( double dir[], double final_pos[], double& path_rate );

  TGeoManager* gdml;
  double fSourcePos[3];
  double fRmin;
  double fRmax;
  double fPlaneZ;
  double fFaceX;
  double fFaceY;
  double fPathRateNorm;
  
  int fVerbose;
  bool fMakeDiagnosticPlots;
  TFile* fDiagFile;
  TH1D* hRdist;
  TH1D* hR2dist;
  TH1D* hCosz;
  TH1D* hPhi;
  TH2D* hXY;
  TH2D* hZY;
  TH2D* hZX;
  TRandom3* rand;

};

#endif
