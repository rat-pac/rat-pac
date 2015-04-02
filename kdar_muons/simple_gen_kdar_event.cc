#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <ctime>
#include <assert.h>
#include <cmath>

#include "TFile.h"
#include "TChain.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TRandom3.h"
#include "TROOT.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"

#include "tree3.h"
#include "GenGeomDAR.h"

int main(int nargs, char** argv ) {

  gROOT->ProcessLine("gErrorIgnoreLevel=kError;");
  
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

  // Vertex generator: uniform box
  double source_pos[3] = { 0, 0, -20000.0 }; // cm
  double xrange[2] = { -300, 300 };
  double yrange[2] = { -300, 300 };
  double zrange[2] = { -6000, 6000 };
  double rnorm = fabs( zrange[0] - source_pos[2] ); // event normalized to 1 at this radius
  bool save_diagnostic_plots = true;

  std::string gdml_file = "/net/hisrv0001/home/taritree/kpipe/ratpac-kpipe/data/kpipe/kpipe.gdml";
  //std::string gdml_file = "../data/kpipe/kpipe.gdml";
  TGeoManager* gdml = TGeoManager::Import( gdml_file.c_str() );
  TRandom3 rand( time(NULL) );

  // setup tree
  tree data( tevents );

  int ngen = 0;
  int ievent = offset;
  long bytes = data.GetEntry(ievent);

  double dir[3], pos[3];
  int pdg[10] = {0};
  double pmom[10] = {0.0};
  double pdir[10][3] = {0.0};
  double pdir_pre[10][3] = {0.0};
  double t[10] = {0.0};
  int npars;

  TFile* out = NULL;
  TH1D* hRdist;
  TH1D* hR2dist;
  TH1D* hCosz;
  TH1D* hPhi;
  TH2D* hXY;
  TH2D* hZY;
  TH2D* hZX;
  
  if ( save_diagnostic_plots ) {
    out = new TFile( "output_simple_gen_plots.root", "recreate" );
    hRdist = new TH1D("hRdist", "", 1000, 0, 1.1*zrange[1] );
    hR2dist = new TH1D("hR2dist", "", 1000, 0, 1.1*zrange[2] );
    hCosz = new TH1D("hCosz", "", 1000, 0.95, 1.0 );
    hPhi = new TH1D("hPhi", "", 1000, -2.0*TMath::Pi(), 2.0*TMath::Pi() );
    hXY = new TH2D("hXY","",100, -1000, 1000, 100, -1000, 1000 );
    hZY = new TH2D("hZY","",100, -10000, 10000, 100, -1000, 1000 );
    hZX = new TH2D("hZX","",100, -10000, 10000, 100, -1000, 1000 );
  }

  while ( bytes!=0 && ngen<nevents ) {

    // ----------------------------------------
    // first need to generate point in detector
    double radius, density;
    double rweight;
    double denweight;
    bool valid = false;
    int nthrows = 0;
    while ( !valid ) {
      pos[0] = xrange[0] + (xrange[1]-xrange[0])*rand.Uniform();
      pos[1] = yrange[0] + (yrange[1]-yrange[0])*rand.Uniform();
      pos[2] = zrange[0] + (zrange[1]-zrange[0])*rand.Uniform();
      density = gdml->FindNode( pos[0], pos[1], pos[2] )->GetMedium()->GetMaterial()->GetDensity();
      for (int i=0; i<3; i++)
	radius += ( pos[i] - source_pos[i] )*( pos[i] - source_pos[i] );
      radius = sqrt( radius );
      rweight = (rnorm*rnorm)/(radius*radius);
      if ( rweight>1.0 ) {
	std::cout << "oops" << std::endl;
	assert(false);
      }
      denweight = density/7.8;
      if ( rand.Uniform()<rweight*denweight ) // weight 1/r^2 and by density
	valid = true;
    }

    double dirnorm = 0;
    for (int i=0; i<3; i++) {
      dir[i] = pos[i] - source_pos[i];
      dirnorm += dir[i]*dir[i];
    }
    dirnorm = sqrt(dirnorm);
    for (int i=0; i<3; i++)
      dir[i] /= dirnorm;

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

    // ======================================================================
    // PRINT OUT KINEMATICS FOR RAT

//     NHEP
//       ISTHEP IDHEP JDAHEP1 JDAHEP2 PHEP1 PHEP2 PHEP3 PHEP5 DT X Y Z PLX PLY PLZ
//       ISTHEP IDHEP JDAHEP1 JDAHEP2 PHEP1 PHEP2 PHEP3 PHEP5 DT X Y Z PLX PLY PLZ
//       ISTHEP IDHEP JDAHEP1 JDAHEP2 PHEP1 PHEP2 PHEP3 PHEP5 DT X Y Z PLX PLY PLZ
//       ... [NHEP times]

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

    // NEVENTS
    std::cout << npars << std::endl;

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

      //       ISTHEP IDHEP JDAHEP1 JDAHEP2 PHEP1 PHEP2 PHEP3 PHEP5 DT X Y Z PLX PLY PLZ 
      std::cout << "    ";
      std::cout << 1 << " " << pdg[out] << " " << 0 << " " << 0 // ISTHEP IDHEP JDAHEP1 JDAHEP2
		<< pmom[out]*pdir[out][0] << " " // PHEP1
		<< pmom[out]*pdir[out][1] << " " // PHEP2
		<< pmom[out]*pdir[out][2] << " " // PHEP3
		<< data.post__mass[out] << " " // PHEP5
		<< 0.0 << " " // DT
		<< pos[0] << " " // X
		<< pos[1] << " "  // Y
		<< pos[2] << " " // Z
		<< std::endl;

    }//end of out loop


    if ( save_diagnostic_plots ) {
      double final_radius = 0.;
      double relative_weight = 1.0;
      for (int i=0; i<3; i++)
	final_radius += ( source_pos[i]-pos[i] )*( source_pos[i]-pos[i] );
      final_radius = sqrt(final_radius);
      hRdist->Fill( final_radius, relative_weight );
      hR2dist->Fill( final_radius, (final_radius/rnorm)*(final_radius/rnorm)*relative_weight );
      hCosz->Fill( dir[2], relative_weight );
      hPhi->Fill( atan2( dir[1], dir[0] ), relative_weight );
      hXY->Fill( pos[0], pos[1], relative_weight );
      hZY->Fill( pos[2], pos[1], relative_weight );
      hZX->Fill( pos[2], pos[0], relative_weight );
    }

    // ======================================================================
    
    ievent++;
    bytes = data.GetEntry(ievent);
    if ( bytes==0 ) {
      // loop to beginning
      ievent = 0;
      bytes = data.GetEntry(ievent);
    }
    ngen++;
  }
  
  if ( save_diagnostic_plots )
    out->Write();

  return 0;
};
