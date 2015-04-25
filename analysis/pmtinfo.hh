#ifndef __PMTINFO__
#define __PMTINFO__

#include <string>
class TChain;

class PMTinfo {

  PMTinfo( std::string info_file );
  ~PMTinfo();

public:
  static PMTinfo* GetPMTinfo( std::string info_file );
  static void getposition( int id, float* pos );

private:
  static PMTinfo* ginstance;
  static const int __MAX_PMT__ = 901000;
  static float* fpmtposdata;
  std::string fpmtinfofile;
  TChain* pmtinfo_chain;

};

#endif 
