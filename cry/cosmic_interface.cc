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
  
  long bytes = crytree->GetEntry(0);
  long ievent = 0;

  

  while ( bytes>0 && ievent<nevents ) {

    


    // get next entry
    ievent++;
    bytes = crytree->GetEntry(ievent);
  }

}
