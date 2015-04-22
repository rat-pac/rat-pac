#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include "TFile.h"
#include "TChain.h"
#include "TBranch.h"

int main( int narg, char** argv ) {

  if ( narg!=3 ) {
    std::cout << "usage: cosmic_interface <input rootfile> <nevents>" << std::endl;
    return -1;
  }

  std::string input = argv[1];
  long nevents = (long)atoi( argv[2] );
  TChain* crytree = new TChain("crytree");
  crytree->Add( input.c_str() );
  
  long bytes = 0;
  long ievent = 0;

  int nparticles;
  std::vector< int >* status;
  std::vector< int >* pdg;
  std::vector< double >* momx_gev;
  std::vector< double >* momy_gev;
  std::vector< double >* momz_gev;
  std::vector< double >* mass_gev;
  std::vector< double >* posx_mm;
  std::vector< double >* posy_mm;
  std::vector< double >* posz_mm;
  std::vector< double >* telapsed_sec;
  std::vector< double >* delta_time_sec;

  TBranch* b_status;
  TBranch* b_pdg;
  TBranch* b_momx_gev;
  TBranch* b_momy_gev;
  TBranch* b_momz_gev;
  TBranch* b_mass_gev;
  TBranch* b_posx_mm;
  TBranch* b_posy_mm;
  TBranch* b_posz_mm;
  TBranch* b_telapsed_sec;
  TBranch* b_delta_time_sec;

  crytree->SetBranchAddress( "nparticles", &nparticles );
  crytree->SetBranchAddress( "pdg", &pdg, &b_pdg );
  crytree->SetBranchAddress( "momx_gev", &momx_gev, &b_momx_gev );
  crytree->SetBranchAddress( "momy_gev", &momy_gev, &b_momy_gev );
  crytree->SetBranchAddress( "momz_gev", &momz_gev, &b_momz_gev );
  crytree->SetBranchAddress( "mass_gev", &mass_gev, &b_mass_gev );
  crytree->SetBranchAddress( "posx_mm", &posx_mm, &b_posx_mm );
  crytree->SetBranchAddress( "posy_mm", &posy_mm, &b_posy_mm );
  crytree->SetBranchAddress( "posz_mm", &posz_mm, &b_posz_mm );
  crytree->SetBranchAddress( "telapsed_sec", &telapsed_sec, &b_telapsed_sec );
  crytree->SetBranchAddress( "delta_time_sec", &delta_time_sec, &b_delta_time_sec );

  bytes = crytree->GetEntry(ievent);
  while ( bytes>0 && ievent<nevents ) {

    std::cout << nparticles << std::endl;

    for ( int i=0; i<nparticles; i++) {
      std::cout << 1 << " " // ISTEHP: Status code
		<< pdg->at(i) << " " // IDHEP: PDG
		<< 0 << " " << 0 << " " // JDAHEP1, JDAHEP2: first, last daughter
		<< momx_gev->at(i) << " " << momy_gev->at(i) <<  " " << momz_gev->at(i) << " " //  PHEP1 PHEP2 PHEP3: momentum GeV
		<< mass_gev->at(i) << " " // PHEP5: mass GeV
		<< 0.0 << " " //delta_time_sec->at(i)*1.0e9 << " " // DT: delta time
		<< posx_mm->at(i) << " " << posy_mm->at(i) << " " << posz_mm->at(i) << " " 
		<< std::endl;
    }

    // get next entry
    ievent++;
    bytes = crytree->GetEntry(ievent);
  }

}
