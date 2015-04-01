#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <ctime>

#include "TFile.h"
#include "TChain.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TRandom3.h"

#include "tree3.h"
#include "GenGeomDAR.h"


int main(int nargs, char** argv ) {
  
  if ( nargs<3 || nargs>4 ) {
    std::cout << "usage: gen_kdar_event [source tree file path] [nevents] [offset]" << std::endl;
    return -1;
  }
  
  std::string inputfile = argv[1];
  if ( inputfile=="default" )
    inputfile = "/net/nudsk0001/d00/scratch/spitzj/eventsout_nuwroxsec_numu_kpipe_3_25_2015.root";
  int nevents = std::atoi( argv[2] );
  int offset = 0;
  if (nargs==4)
    offset = std::atoi(argv[3]);

  TFile* finput = new TFile( inputfile.c_str(), "open");
  TTree* tevents = (TTree*)finput->Get( "treeout" );
//   if ( tevents==NULL ) {
//     return -1;
//   }

  // geometry parameters (move to config file at some point)
  double baseline = 20000.0; // cm [200 m]. distance from center of tank to source
  double source_pos[3] = { 0, 0, -baseline };
  double half_z_outerpipe = 10104.0/2.0; // cm
  double r_outerpipe = 192.0; // cm
  double half_z_dirt = 0.5*20000.0;
  double dist_src_2_frontface = fabs(-half_z_outerpipe+baseline);
  if ( dist_src_2_frontface<0 )
    return -1;

  // Vertex generator
  std::string gdml_filename = "../data/kpipe/kpipe.gdml";
  //std::string gdml_filename = "/net/hisrv0001/home/taritree/kpipe/ratpac-kpipe/data/kpipe/kpipe.gdml"
  GenGeomDAR vtxgen( gdml_filename, source_pos, 0.8*dist_src_2_frontface, 1.1*(half_z_outerpipe-source_pos[2]), -1.1*half_z_outerpipe, 200.0, 200.0 );
  vtxgen.MakeDiagnosticPlots();
  double vertex[3];
  double weight = 0;
  for (int i=0; i<nevents; i++) {
    if (i%1000==0)
      std::cout << "vertex " << i << std::endl;
    vtxgen.GetPosition( vertex, weight );
  }
  
  return -1;


  // setup tree
  tree data( tevents );

  int ngen = 0;
  int ievent = offset;
  long bytes = data.GetEntry(ievent);

  while ( bytes!=0 && ngen<nevents ) {

    // first need to generate point in detector

    // out to RAT
    //the zeroth entry in the array always refers to the outgoing lepton
    std::cout << "npost: " << data.post_ << " particles" << std::endl;
    
    //output_event_to_rat( tevents );
    ievent++;
    bytes = data.GetEntry(ievent);
    if ( bytes==0 ) {
      // loop to beginning
      ievent = 0;
      bytes = data.GetEntry(ievent);
    }
    ngen++;
  }
  

  
  

  return 0;
};
