#include "pmtinfo.hh"
#include <iostream>
#include "TChain.h"

PMTinfo* PMTinfo::ginstance = NULL;
float* PMTinfo::fpmtposdata = NULL;

PMTinfo::PMTinfo( std::string info_file ) {
 
  fpmtinfofile = info_file;
  pmtinfo_chain = new TChain("pmtinfo");
  pmtinfo_chain->Add(fpmtinfofile.c_str());
  float pmtpos[3];
  pmtinfo_chain->SetBranchAddress("x",&pmtpos[0]);
  pmtinfo_chain->SetBranchAddress("y",&pmtpos[1]);
  pmtinfo_chain->SetBranchAddress("z",&pmtpos[2]);

  fpmtposdata = new float[ 3*pmtinfo_chain->GetEntries() ];
  for (int i=0; i<pmtinfo_chain->GetEntries(); i++) {
    pmtinfo_chain->GetEntry(i);
    // save in array
    memcpy( fpmtposdata + 3*i, pmtpos, 3*sizeof(float) );
  }

}

PMTinfo::~PMTinfo() {
  delete pmtinfo_chain;
  delete [] fpmtposdata;
}

PMTinfo* PMTinfo::GetPMTinfo( std::string info_file ) {
  if ( ginstance==NULL )
    ginstance = new PMTinfo( info_file );

  return ginstance;
}

void PMTinfo::getposition( int pmtid, float* pmtpos ) {
  if ( pmtid>=0 && pmtid<__MAX_PMT__ )
    memcpy( pmtpos, fpmtposdata + 3*pmtid, 3*sizeof(float) );
  else {
    std::cerr << "no id: seeking position for pmtid=" << pmtid << std::endl;
    memset( pmtpos, 0, 3*sizeof(float) );
  }
}



