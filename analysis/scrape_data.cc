#include <iostream>

#include "TFile.h"
#include "TTree.h"

#include "RAT/DSReader.hh"
#include "RAT/DS/MC.hh"

using namespace RAT;
using namespace std;

int main( int nargs, char** argv ) {

  std::string inputfile = "../kpipeout_test.root";
  DSReader* ds = new DSReader( inputfile.c_str() ); 

  TFile* tf_pmtinfo = new TFile( "../data/kpipe/PMTINFO.root", "open" );
  TTree* pmtinfo = (TTree*)tf_pmtinfo->Get("pmtinfo");
  float pmtpos[3];
  pmtinfo->SetBranchAddress("x",&pmtpos[0]);
  pmtinfo->SetBranchAddress("y",&pmtpos[1]);
  pmtinfo->SetBranchAddress("z",&pmtpos[2]);

  TFile* out = new TFile("output_scraped.root", "RECREATE" );
  // variables we want
  int npe;
  int npe_prompt;
  int npe_late;
  int npmts;
  int nhoops;
  double pos[3];  // recon position
  double r,z;     // recon
  double qhpos[3]; // recon using quarter hoop
  double qhr,qhz;  // recon using quarter hoop
  double posv[3]; // true vertex
  double mudirv[3]; // true direction
  double mumomv;  // truth momentum (0 if no muon! NC event)
  double rv, zv;  // from truth
  double tstart, tstart_dcye;
  double tave, tave_dcye;
  
  TTree* tree = new TTree( "mcdata", "MC Data" );
  // recon
  tree->Branch( "npe", &npe, "npe/I" );
  tree->Branch( "npe_prompt", &npe_prompt, "npe_late/I" );
  tree->Branch( "npe_late", &npe_late, "npe_late/I" );
  tree->Branch( "npmts", &npmts, "npmts/I" );
  tree->Branch( "pos", pos, "pos[3]/D" );
  tree->Branch( "r", &r, "r/D" );
  tree->Branch( "z", &z, "z/D" );
  tree->Branch( "qhpos", qhpos, "qhpos[3]/D" );
  tree->Branch( "qhr", &qhr, "qhr/D" );
  tree->Branch( "qhz", &qhz, "qhz/D" );
  tree->Branch( "tstart", &tstart, "tstart/D" );
  tree->Branch( "tave", &tave, "tave/D" );
  tree->Branch( "tstart_dcye", &tstart_dcye, "tstart_dcye/D" );
  tree->Branch( "tave_dcye", &tave_dcye, "tave_dcye/D" );
  // truth
  tree->Branch( "posv", posv, "posv[3]/D" );
  tree->Branch( "mudirv", mudirv, "mudirv[3]/D" );
  tree->Branch( "mumomv", &mumomv, "mumomv/D" );
  tree->Branch( "rv", &rv, "rv/D" );  
  tree->Branch( "zv", &zv, "zv/D" );

  long ievent = 0;
  long nevents = ds->GetTotal();

  
  while (ievent<nevents) {
    DS::Root* root = ds->NextEvent();

    if ( ievent%1==0 )
      std::cout << "Event " << ievent << std::endl;

    DS::MC* mc = root->GetMC();
    npe = mc->GetNumPE();
    npmts = mc->GetMCPMTCount();

    // true vertex
    posv[0] = mc->GetMCParticle(0)->GetPosition().X();
    posv[1] = mc->GetMCParticle(0)->GetPosition().Y();
    posv[2] = mc->GetMCParticle(0)->GetPosition().Z();
    rv = sqrt(posv[0]*posv[0] + posv[1]*posv[1]);
    zv = posv[2];

    // true muon momentum
    mumomv = 0;
    mudirv[0] = mudirv[1] = mudirv[2] = 0.0;
    for (int ipart=0; ipart<mc->GetMCParticleCount(); ipart++) {
      if ( mc->GetMCParticle(ipart)->GetPDGCode()==13 ) {
	TVector3 mom( mc->GetMCParticle(ipart)->GetMomentum() );
	mumomv = sqrt( mom.X()*mom.X() + mom.Y()*mom.Y() + mom.Z()*mom.Z() );
	mudirv[0] = mom.X()/mumomv;
	mudirv[1] = mom.Y()/mumomv;
	mudirv[2] = mom.Z()/mumomv;
	break;
      }
    }

    // recon
    int hoop_hit[1000] = {0};
    nhoops = 0;
    npe_prompt = npe_late = 0;
    tstart = tave = 0.0;
    r = z = qhr = qhz = 0.0;
    pos[0] = pos[1] = pos[2] = 0.0;
    qhpos[0] = qhpos[1] = qhpos[2] = 0.0;
    tave = tstart = tave_dcye = tstart_dcye = 0.0;
    
    if (npe>0) {

      for (int ipmt=0; ipmt<npmts; ipmt++) {
	DS::MCPMT* pmt = mc->GetMCPMT( ipmt );
	int pmtid = pmt->GetID();
	pmtinfo->GetEntry( pmtid );

	// individual PMTs
	double pmt_pe = (double)pmt->GetMCPhotonCount();
	pos[0] += pmt_pe*pmtpos[0];
	pos[1] += pmt_pe*pmtpos[1];
	pos[2] += pmt_pe*pmtpos[2];
      
	// quarter hoop
	qhpos[2] += pmt_pe*pmtpos[2];
	int ihoop = pmtid/100;
	int ipmt_hoop = pmtid%100;
	int ipmt_quad = ipmt_hoop/25;
	
	hoop_hit[ihoop] = 1;
      
	double qhpe = 0.70*135.0*pmt_pe;
	double qh = 0.70*135.0;
	switch (ipmt_quad) {
	case 0:
	  qhpos[0] += qhpe;
	  qhpos[1] += qhpe;
	  break;
	case 1:
	  qhpos[0] += -qhpe;
	  qhpos[1] += qhpe;
	  break;
	case 2:
	  qhpos[0] += -qhpe;
	  qhpos[1] += -qhpe;
	  break;
	case 3:
	  qhpos[0] += qhpe;
	  qhpos[1] += -qhpe;
	  break;
	}//end of QH switch;

	// timing
	for (int ihit=0; ihit<pmt->GetMCPhotonCount(); ihit++) {
	  DS::MCPhoton* hit = pmt->GetMCPhoton(ihit);
	  // earliest time
	  double t = hit->GetHitTime();
	  if ( t<1000.0 ) {
	    npe_prompt += 1;
	    tave += t;
	    if ( tstart>t ) 
	      tstart = t;
	  }
	  else {
	    npe_late += 1;
	    tave_dcye += t;
	    if ( tstart_dcye>t )
	      tstart_dcye = t;
	  }
	}
      
      }//end of hit PMT loop

      // take the averages
      for (int v=0; v<3; v++) {
	pos[v] /= double(npe);
	qhpos[v] /= double(npe);
      }
      tave /= double(npe_prompt);
      tave_dcye /= double(npe_late);

      // r,z
      r = sqrt(pos[0]*pos[0]+pos[1]*pos[1]);
      z = pos[2];
      qhr = sqrt(qhpos[0]*qhpos[0]+qhpos[1]*qhpos[1]);
      qhz = qhpos[2];

      nhoops = 0;
      for (int i=0; i<10; i++)
	nhoops += hoop_hit[i];
    }//end of if pe
    ievent++;
    tree->Fill();
  }//end of while loop

  std::cout << "write." << std::endl;

  out->cd();
  tree->Write();

  std::cout << "finished." << std::endl;

  return 0;
}
