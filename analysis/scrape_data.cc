#include <iostream>

#include "TFile.h"
#include "TTree.h"

#include "RAT/DSReader.hh"
#include "RAT/DS/MC.hh"

#include "kptrigger.h"

int main( int nargs, char** argv ) {

  if ( nargs<3 ) {
    std::cout << "usage: scrape_data <input RAT root file> <output rootfile> [optional: pmt info file. default: ]" << std::endl;
    return 0;
  }

  double prompt_cut = 500.0; // ns

  //std::string inputfile = "../kpipeout_test.root";
  //std::string inputfile = "/net/nudsk0001/d00/scratch/taritree/kpipe_out/first_set/run1_kpipe_output.root";
  //std::string inputfile = "/net/nudsk0001/d00/scratch/taritree/kpipe_out/run100_kpipe.root";
  //std::string inputfile = "/net/nudsk0001/d00/scratch/taritree/kpipe_out/kpipe_run00_partial.root";
  std::string inputfile = argv[1];
  std::string outfile = argv[2];

  std::cout << "analyze: " << inputfile << std::endl;

  std::string pmtinfofile = "/net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/data/kpipe/PMTINFO.root";
  //std::string pmtinfofile = "../data/kpipe/PMTINFO.root";
  if (nargs==4) 
    pmtinfofile = argv[3];

  RAT::DSReader* ds = new RAT::DSReader( inputfile.c_str() ); 
  TFile* tf_pmtinfo = new TFile( pmtinfofile.c_str(), "open" );

  TTree* pmtinfo = (TTree*)tf_pmtinfo->Get("pmtinfo");
  float pmtpos[3];
  pmtinfo->SetBranchAddress("x",&pmtpos[0]);
  pmtinfo->SetBranchAddress("y",&pmtpos[1]);
  pmtinfo->SetBranchAddress("z",&pmtpos[2]);

  TFile* out = new TFile( outfile.c_str(), "RECREATE" );
  
  // variables we want
  int npe;
  int npe_prompt[4];
  int npe_late[4];
  int npe_prompt_tot;
  int npe_late_tot;
  int npmts;
  int nhoops;
  double pos[3];
  double r,z;
  double pos_prompt[3];  // recon position
  double r_prompt,z_prompt;     // recon
  double pos_late[3];  // recon position
  double r_late,z_late;     // recon
  double qhpos[3]; // recon using quarter hoop
  double qhr,qhz;  // recon using quarter hoop
  double posv[3]; // true vertex
  double mudirv[3]; // true direction
  double mumomv;  // truth momentum (0 if no muon! NC event)
  double rv, zv;  // from truth
  double tstart, tstart_dcye;
  double tave, tave_dcye;
  // trigger info
//   int npulses = 0;
// //   std::vector<double> ttrig(4);
// //   std::vector<double> tpeak(4);
// //   std::vector<double> peakamp(4);
// //   std::vector<double> tend(4);
// //   std::vector<double> pulsepe(4);
//   double ttrig[10];
//   double tpeak[10];
//   double peakamp[10];
//   double tend[10];
//   double pulsepe[10];
  
  TTree* tree = new TTree( "mcdata", "MC Data" );
  // recon
  tree->Branch( "npe", &npe, "npe/I" );
  tree->Branch( "npe_prompt", npe_prompt, "npe_prompt[4]/I" );
  tree->Branch( "npe_late", npe_late, "npe_late[4]/I" );
  tree->Branch( "npe_prompt_tot", npe_prompt_tot, "npe_prompt_tot/I" );
  tree->Branch( "npe_late_tot", npe_late_tot, "npe_late_tot/I" );
  tree->Branch( "npmts", &npmts, "npmts/I" );
  tree->Branch( "pos", pos, "pos[3]/D" );
  tree->Branch( "r", &r, "r/D" );
  tree->Branch( "z", &z, "z/D" );
  tree->Branch( "pos_prompt", pos_prompt, "pos_prompt[3]/D" );
  tree->Branch( "r_prompt", &r_prompt, "r_prompt/D" );
  tree->Branch( "z_prompt", &z_prompt, "z_prompt/D" );
  tree->Branch( "pos_late", pos_late, "pos_late[3]/D" );
  tree->Branch( "r_late", &r_late, "r_late/D" );
  tree->Branch( "z_late", &z_late, "z_late/D" );
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
  // trigger vars
//   tree->Branch( "npulses", &npulses, "npulses/I" );
//   tree->Branch( "ttrig",  ttrig, "ttrig[10]/D" );
//   tree->Branch( "tpeak",  tpeak, "tpeak[10]/D" );
//   tree->Branch( "tend",  tend, "tend[10]/D" );
//   tree->Branch( "peakamp",  peakamp, "peakamp[10]/D" );
//   tree->Branch( "pulsepe",  pulsepe, "pulsepe[10]/D" );

  int ievent = 0;
  int nevents = ds->GetTotal();

  //KPPulseList pulselist;

  std::cout << "Number of events: " << nevents << std::endl;
  nevents = 10;
  
  while (ievent<nevents) {
    RAT::DS::Root* root = ds->NextEvent();

//     npulses = 0;
//     for (int i=0; i<10; i++){
//       ttrig[i] = 0;
//       tpeak[i] = 0;
//       tend[i] = 0;
//       peakamp[i] = 0;
//       pulsepe[i] = 0;
//     }

    //if ( ievent%1000==0 )
    std::cout << "Event " << ievent << std::endl;

    RAT::DS::MC* mc = root->GetMC();
    if ( mc==NULL )
      break;
    npe = mc->GetNumPE();
    npmts = mc->GetMCPMTCount();

//     if ( mc->GetMCParticleCount()==0 )
//       continue;

    // true vertex
    posv[0] = mc->GetMCParticle(0)->GetPosition().X()/10.0; //change to cm
    posv[1] = mc->GetMCParticle(0)->GetPosition().Y()/10.0; //change to cm
    posv[2] = mc->GetMCParticle(0)->GetPosition().Z()/10.0; //change to cm
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
    for (int i=0; i<4; i++)
      npe_prompt[i] = npe_late[i] = 0;
    npe_prompt_tot = npe_late_tot = 0;
    tstart = tave = 0.0;
    r = z = qhr = qhz = 0.0;
    r_prompt = z_prompt = r_late = z_late = 0.0;
    pos[0] = pos[1] = pos[2] = 0.0;
    pos_late[0] = pos_late[1] = pos_late[2] = 0.0;
    pos_prompt[0] = pos_prompt[1] = pos_prompt[2] = 0.0;
    qhpos[0] = qhpos[1] = qhpos[2] = 0.0;
    tave = tstart = tave_dcye = tstart_dcye = 0.0;
    
    if (npe>0) {

      for (int ipmt=0; ipmt<npmts; ipmt++) {
	RAT::DS::MCPMT* pmt = mc->GetMCPMT( ipmt );
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
	  RAT::DS::MCPhoton* hit = pmt->GetMCPhoton(ihit);
	  // earliest time
	  double t = hit->GetHitTime();
	  int origin = hit->GetOriginFlag();
	  if ( t<prompt_cut ) {
	    if ( origin>=0 && origin<=2 )
	      npe_prompt[origin] += 1;
	    else
	      npe_prompt[3] += 1;
	    npe_prompt_tot += 1;
	    tave += t;
	    if ( tstart>t ) 
	      tstart = t;
	    for (int v=0; v<3; v++)
	      pos_prompt[v] += pmtpos[v];
	  }
	  else {
	    if ( origin>=0 && origin<=2 )
	      npe_late[origin] += 1;
	    else
	      npe_late[3] += 1;
	    npe_late_tot += 1;
	    tave_dcye += t;
	    if ( tstart_dcye>t )
	      tstart_dcye = t;
	    for (int v=0; v<3; v++)
	      pos_late[v] += pmtpos[v];
	  }
	}//end of hit loop
      
      }//end of hit PMT loop

      // take the averages
      for (int v=0; v<3; v++) {
	pos[v] /= double(npe);
	qhpos[v] /= double(npe);
	pos_prompt[v] /= double(npe_prompt_tot);
	pos_late[v] /= double(npe_late_tot);
      }
      tave /= double(npe_prompt_tot);
      tave_dcye /= double(npe_late_tot);

      // r,z
      r = sqrt(pos[0]*pos[0]+pos[1]*pos[1]);
      z = pos[2];
      r_prompt = sqrt(pos_prompt[0]*pos_prompt[0]+pos_prompt[1]*pos_prompt[1]);
      z_prompt = pos_prompt[2];
      r_late = sqrt(pos_late[0]*pos_late[0]+pos_late[1]*pos_late[1]);
      z_late = pos_late[2];
      qhr = sqrt(qhpos[0]*qhpos[0]+qhpos[1]*qhpos[1]);
      qhz = qhpos[2];

      nhoops = 0;
      for (int i=0; i<1000; i++)
	nhoops += hoop_hit[i];

      // TRIGGER
//       find_trigger( mc, 5.0, 5.0, 10.0, 45.0, pulselist, 90000, false );
//       npulses = 0;
//       for ( KPPulseListIter it=pulselist.begin(); it!=pulselist.end(); it++ ) {
// // 	ttrig.push_back( (*it)->tstart );
// // 	tpeak.push_back( (*it)->tpeak );
// // 	tend.push_back( (*it)->tend );
// // 	peakamp.push_back( (*it)->peakamp );
// // 	pulsepe.push_back( 0 ); // not yet calculated
// // 	ttrig[npulses] = (*it)->tstart;
// // 	tpeak[npulses] = (*it)->tpeak;
// // 	tend[npulses] = (*it)->tend;
// // 	peakamp[npulses] = (*it)->peakamp;
// // 	pulsepe[npulses] = 0.0;
// 	npulses++;
// 	delete *it;
// 	*it = NULL;
// 	if ( npulses==10 )
// 	  break;
//       }

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
