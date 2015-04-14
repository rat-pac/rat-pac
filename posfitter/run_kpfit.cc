#include <iostream>
#include "kpfit.hh"
#include "RAT/DSReader.hh"
#include "RAT/DS/Root.hh"

#include <gperftools/profiler.h>

int main( int narg, char** argv ) {


  std::cout << "KPFIT" << std::endl;

  RAT::DSReader ds( "../analysis/output_kpipe_303.root" );
  RAT::DS::Root* root = ds.NextEvent();
  
  //ProfilerStart("kpfit.prof");

  int event = 0;
  while ( root!=NULL ) {
    std::cout << "EVENT " << event << std::endl;
    KPFit fitter;
    double pos[3];
    fitter.fit( root->GetMC(), pos );
    root = ds.NextEvent();
    event++;
    if ( event==100 )
      break;
  }
  //ProfilerStop();

  return 0;
}
