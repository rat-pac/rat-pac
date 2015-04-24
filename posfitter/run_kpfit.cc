#include <iostream>
#include "kpfit.hh"
#include "RAT/DSReader.hh"
#include "RAT/DS/Root.hh"

#include "TFile.h"
#include "TTree.h"
#include "TMinuitMinimizer.h"

//#include <gperftools/profiler.h>

int main( int narg, char** argv ) {


  std::cout << "KPFIT" << std::endl;
  std::string input = argv[1];
  std::string output = argv[2];
  std::string pmtinfo = argv[3];

  //RAT::DSReader ds( "../analysis/output_kpipe_303.root" );
  RAT::DSReader ds( input.c_str() );
  RAT::DS::Root* root = ds.NextEvent();
  
  TFile* out = new TFile( output.c_str(), "recreate" );
  TTree* tree = new TTree( "vtxfit", "Vertex Fitter Results" );

  int wasrun;
  double fitpos[3];
  double evis;
  double fcn;
  int converged;
  tree->Branch( "wasrun", &wasrun, "wasrun/I" );
  tree->Branch( "fitpos", fitpos, "fitpos[3]/D" );
  tree->Branch( "evis", &evis, "evis/D" );
  tree->Branch( "converged", &converged, "converged/I" );
  tree->Branch( "fcn", &fcn, "fcn/D" );

  //ProfilerStart("kpfit.prof");
  KPFit fitter( pmtinfo );
  int event = 0;
  while ( root!=NULL ) {
    std::cout << "EVENT " << event << std::endl;
    if ( fitter.fit( root->GetMC(), fitpos ) )
      converged = 1;
    else
      converged = 0;
    if ( fitter.getwasrun() ) {
      wasrun = 1;
      fcn = fitter.minuit->MinValue();
      //evis = fitter.minuit->X()[3];
    }
    else {
      wasrun = 0;
      fcn = -1.0;
      evis = -1.0;
    }
    root = ds.NextEvent();
    event++;
    //if ( event==100 )
    //break;
    tree->Fill();
  }
  //ProfilerStop();

  tree->Write();

  return 0;
}
