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
#include "TROOT.h"

#include "tree3.h"
#include "GenGeomDAR.h"



int main(int nargs, char** argv ) {

  gROOT->ProcessLine("gErrorIgnoreLevel=kError;");
  
  if ( nargs<4 || nargs>5 ) {
    std::cout << "usage: gen_kdar_event [source tree file path] [output event file] [nevents] [offset]" << std::endl;
    return -1;
  }
  
  std::string inputfile = argv[1];
  std::string outputfile = argv[2];
  if ( inputfile=="default" )
    inputfile = "/net/nudsk0001/d00/scratch/spitzj/eventsout_nuwroxsec_numu_kpipe_3_25_2015.root";
  int nevents = std::atoi( argv[3] );
  int offset = 0;
  if (nargs==5)
    offset = std::atoi(argv[4]);

  TFile* finput = new TFile( inputfile.c_str(), "open");
  TTree* tevents = (TTree*)finput->Get( "treeout" );
//   if ( tevents==NULL ) {
//     return -1;
//   }

  TFile* foutput = new TFile( outputfile.c_str(), "recreate" );
  double dir[3];
  double vertex[3];
  double weight;
  int pdg[10] = {0};
  double pmom[10] = {0.0};
  double pdir[10][3] = {0.0};
  double pdir_pre[10][3] = {0.0};
  double t[10] = {0.0};
  int npars;
  TTree* tVertex = new TTree("events", "Event Vertex");
  tVertex->Branch( "nudir", dir, "nudir[3]/D" );
  tVertex->Branch( "vertex_cm", vertex, "vertex_cm[3]/D" );
  tVertex->Branch( "weight", &weight, "weight/D" );
  tVertex->Branch( "npars", &npars, "npars/I" );
  tVertex->Branch( "pdg", pdg, "pdg[10]/I" );
  tVertex->Branch( "pmom_gev", pmom, "pmom_gev[10]/D" );
  tVertex->Branch( "pdir", pdir, "pdir[10][3]/D" );
  tVertex->Branch( "pdir_pre", pdir_pre, "pdir_pre[10][3]/D" );
  tVertex->Branch( "t", t, "t[10]/D" );


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

  // setup tree
  tree data( tevents );

  int ngen = 0;
  int ievent = offset;
  long bytes = data.GetEntry(ievent);

  while ( bytes!=0 && ngen<nevents ) {
    if ( ngen%100==0 )
      std::cout << "Event " << ngen << " of " << nevents << std::endl;

    // ----------------------------------------
    // first need to generate point in detector
    vtxgen.GetPosition( dir, vertex, weight ); // vertex gives position, dir is direction of neutrino
    
    // ----------------------------------------
    // get event from data

    // euler angles: we use Z-X-Z extrinsic. Last Z is set to 0 for all rotations
    double alpha = acos( -dir[1]/sqrt(1-dir[2]*dir[2]) );
    double beta  = acos( dir[2] );
    double gamma = 0.0;
    double angles[3] = { alpha, beta, gamma };
    double c[4], s[4]; // index 1-indexed to more easily copy wikipedia matrix
    for (int i=0; i<3; i++) {
      c[i+1] = cos( angles[i] );
      s[i+1] = sin( angles[i] );
    }
    
    // matrix: wikipedia: Euler angles
    double rot[3][3] = { 
      { c[1]*c[3]-c[2]*s[1]*s[3], -c[1]*s[3]-c[2]*c[3]*s[1], s[1]*s[2] },
      { c[3]*s[1]+c[1]*c[2]*s[3], c[1]*c[2]*c[3]-s[1]*s[3], -c[1]*s[2] },
      { s[2]*s[3], c[3]*s[2], c[2] }, 
    };

//     // check
//     double nurot[3];
//     double nurotnorm = 0.;
//     for (int i=0; i<3; i++) {
//       nurot[i] = 0.;
//       for (int j=0; j<3; j++)
// 	nurot[i] += rot[i][j]*dnu_nuwro[j];
//       nurotnorm += nurot[i]*nurot[i];
//     }
//     nurotnorm = sqrt(nurotnorm);
//     for (int i=0; i<3; i++)
//       nurot[i] /= nurotnorm;
//     std::cout << "rot check: " 
// 	      << fabs(nurot[0]-dnu_nuwro[0]) << ",  "
// 	      << fabs(nurot[1]-dnu_nuwro[1]) << ",  "
// 	      << fabs(nurot[2]-dnu_nuwro[2]) << std::endl;

    //the zeroth entry in the array always refers to the outgoing lepton (note from josh)
    //std::cout << "npost: " << data.post_ << " particles" << std::endl;
    double dnu_nuwro[3] = { data.in_x[0], data.in_y[0], data.in_z[0] };
    double pnu = 0.;
    for (int i=0; i<3; i++)
      pnu += dnu_nuwro[i]*dnu_nuwro[i];
    pnu = sqrt(pnu);
    for (int i=0; i<3; i++)
      dnu_nuwro[i] /= pnu;
    //std::cout << "in-coming nu dir: " << dnu_nuwro[0] << ", " << dnu_nuwro[1]  << ", " << dnu_nuwro[2] << std::endl;

//     for (int in=0; in<data.in_; in++) {
//       std::cout << " in " << in << ") pgg=" << data.in_pdg[in] << std::endl;
//     }

    int npost = data.post_;
    if ( npost>10)
      npost = 10;
    npars = npost;
    for (int out=0; out<npost; out++) {
      // set t, pdg, pmom, pdir for each out

      double outdir[3] = { data.post_x[out], data.post_y[out], data.post_z[out] };
      pmom[out] = 0.;
      for (int i=0; i<3; i++)
	pmom[out] += outdir[i]*outdir[i];
      pmom[out] = sqrt( pmom[out] );
      for (int i=0; i<3; i++) {
	outdir[i] /= pmom[out];
	pdir_pre[out][i] = outdir[i];
      }

      pmom[out] *= 0.001; // change to GeV

      //std::cout << " out " << out << ") pdg=" << data.post_pdg[out] << std::endl;
      pdg[out] = data.post_pdg[out];

      t[out] = 0.0;

      for (int i=0; i<3; i++) {
	pdir[out][i] = 0.0;
	for (int j=0; j<3; j++)
	  pdir[out][i] += rot[i][j]*outdir[j];
      }
      double norm =0.;
      for (int i=0; i<3; i++)
	norm += pdir[out][i]*pdir[out][i];
      for (int i=0; i<3; i++)
	pdir[out][i] /= norm;
    }
    tVertex->Fill();
    
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
  

  foutput->cd();
  tVertex->Write();
  

  return 0;
};
