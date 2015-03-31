#include <iostream>
#include <string>
#include <cstdlib>

#include "TFile.h"
#include "TChain.h"
#include "TGeoManager.h"

// void output_event_to_rat( TTree* aStep ) {
  
//   double Eproton, Elepton, Ehadron, anglelep, Eall, Eleptrue, Elepkin, Elep_mev, Elepton_nosmear, Plep_mev;
//   double MNminusV=939.56536-34.;
//   double Mmuon=105.658;
//   double bindingE=34.;
//   double deltaM2=pow(939.56536,2)-pow(938.27203,2);

  

// };


int main(int nargs, char** argv ) {
  
  if ( nargs!=3 || nargs!=4 ) {
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
  if ( tevents==NULL ) {
    return -1;
  }

  // geometry parameters (move to config file at some point)
  double baseline = 20000.0; // cm [200 m]. distance from center of tank to source
  double half_z_outerpipe = 10104.0/2.0; // cm
  double r_outerpipe = 192.0; // cm
  double dist_src_2_frontface = baseline-half_z_outerpipe;
  if ( dist_src_2_frontface<0 )
    return -1;

  // gdml data
  TGeoManager* gdml = TGeoManager::Import("~taritree/kpipe/ratpac-kpipe/kdar_muons/../data/kpipe/kpipe.gdml");

  // setup tree
  const Int_t kMaxin = 2;
  const Int_t kMaxtemp = 2;
  const Int_t kMaxout = 3;
  const Int_t kMaxpost = 8;
  const Int_t kMaxall = 6;
  
  Bool_t          flag_cc;
  Int_t           post_;
  Double_t        post_t[kMaxpost];   //[post_]
  tevents->SetBranchAddress( "flag_cc", &flag_cc );
  tevents->SetBranchAddress( "post", &post_ );
  tevents->SetBranchAddress( "post.t", post_t );

  int ngen = 0;
  int ievent = offset;
  long bytes = tevents->GetEntry(ievent);

  while ( bytes!=0 && ngen<nevents ) {

    // first need to generate 

    // out to RAT
    for (int out = 0; out<post_; out++) {

    }
    //the zeroth entry in the array always refers to the outgoing lepton

    //output_event_to_rat( tevents );
    ievent++;
    bytes = tevents->GetEntry(ievent);
    if ( bytes==0 ) {
      // loop to beginning
      ievent = 0;
      bytes = tevents->GetEntry(ievent);
    }
    ngen++;
  }
  

  
  

  return 0;
};
