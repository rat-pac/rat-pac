#include "kpfit.hh"
#include <assert.h>

#include "TChain.h"
#include "TMinuitMinimizer.h"
#include "RAT/DS/MCPMT.hh"
#include "RAT/DS/MCPhoton.hh"

KPFit::KPFit( std::string pmtinfofile ) : ROOT::Math::IMultiGenFunction() {
  // setup minimizer
  minuit = new TMinuitMinimizer( ROOT::Minuit::kCombined, NDim() );
  minuit->SetFunction( *this );
  minuit->SetLimitedVariable( 0, "x", 0, 10.0, -150.0, 150.0 );
  minuit->SetLimitedVariable( 1, "y", 0, 10.0, -150.0, 150.0 );
  minuit->SetLimitedVariable( 2, "z", 0, 100.0, -5000.0, 5000.0 );
  //minuit->SetLimitedVariable( 3, "evis", 100, 1.0, 0, 2000.0 );
  
  // setup PMT positon info
  fpmtinfofile = pmtinfofile;
  pmtinfo = new TChain("pmtinfo");
  pmtinfo->Add(fpmtinfofile.c_str());
  pmtinfo->SetBranchAddress("x",&fpmtpos[0]);
  pmtinfo->SetBranchAddress("y",&fpmtpos[1]);
  pmtinfo->SetBranchAddress("z",&fpmtpos[2]);
  storePMTInfo();
  
  // initialize variables
  fMCdata = NULL;
  fSigTime = 5.0; // ns timing width
  fNPMTS = 100000;
  fNhoops = 3;
  fDarkRate = 10e3; // Hz
  fTimeWindow = 10e3; // 10 microseconds
  fAbsLength = 1000; // 10 m
  fLightYield = 9800.0; // photons/MeV
  fPromptCut = 1000.0; // prompt cut
  wasrun = false;
}

KPFit::~KPFit() {
  delete minuit;
  delete pmtinfo;
  deletePMTInfo();
}

ROOT::Math::IBaseFunctionMultiDim* KPFit::Clone() const {
  KPFit* copy = new KPFit( this->fpmtinfofile  );
  for (int i=0; i<4; i++) {
    copy->fSeedPos[i] = this->fSeedPos[i];
    copy->fSeedPos[i] = this->fPos[i];
    copy->fSeedPos[i] = this->finalpos[i];
  }
  copy->fMCdata = this->fMCdata;
  return copy;
}

double KPFit::DoEval( const double* x) const {
  if (!fMCdata)
    assert(false);

  // likelihood
  // timing: Gaus( (pmt_tmin - |pmtpos-pos|/c), tsig )
  // flux: Gaus(  solid_angle

  int npmts = fMCdata->GetMCPMTCount();
  double npe  = (double)fMCdata->GetNumPE();
  assert( npe!=0.0 );

//   std::cout << "DoEval: x=(" << x[0] << ", " << x[1] << ", " << x[2] << ", " << x[3] << ") ";
//   std::cout << " npe= " << npe << " npmts=" << npmts << " ";

  std::map< int, double > fracQ;   // fractional charge per PMT
  std::map< int, double > fracQ_prompt;   // fractional charge per PMT
  std::map< int, double > obsHits;
  double ll_t = 0.0;
  double total_prompt = 0.0;
  int brightest_pmt = -1;
  double maxq = -1.0;
  float pmtpos[3];
  for (int ipmt=0; ipmt<npmts; ipmt++) {
    RAT::DS::MCPMT* pmt = fMCdata->GetMCPMT( ipmt );
    int pmtid = pmt->GetID();
    //pmtinfo->GetEntry( pmtid );
    getPMTInfo( pmtid, pmtpos );
    
    // loop through hits to find first time
    // and total hits
    double tmin = 1.0e9; // ns
    double frac_totQ = 0.0;
    double frac_promptQ = 0.0;
    for (int ihit=0; ihit<pmt->GetMCPhotonCount(); ihit++) {
      RAT::DS::MCPhoton* hit = pmt->GetMCPhoton(ihit);
      if ( tmin> hit->GetHitTime() )
	tmin = hit->GetHitTime();
      if ( hit->GetHitTime()<fPromptCut ) {
	frac_promptQ += 1.0;
	total_prompt += 1.0;
      }
      
      frac_totQ += 1.0; // later replace with pe distribution      
    }
    // save in map
    fracQ[pmtid] = frac_totQ/npe;
    //obsHits[pmtid] = frac_totQ;
    obsHits[pmtid] = frac_promptQ;
    fracQ_prompt[pmtid] = frac_promptQ;

    if ( maxq<frac_promptQ ) {
      maxq = frac_promptQ;
      brightest_pmt = pmtid;
    }
    
    // expected time
    double n_scint = 1.45; // index of refraction
    double dist_to_pmt = 0.0; // cm
    double C = 3.0e8*1.0e-9*1.0e2; // m per s * ( 1.0e-9 s/ns ) * ( 1.0e2 cm/m )
    for (int i=0; i<3; i++)
      dist_to_pmt += (pmtpos[i]-x[i])*(pmtpos[i]-x[i]);
    dist_to_pmt = sqrt( dist_to_pmt );
    double t_expected = dist_to_pmt/( C/n_scint ); // ns

    //std::cout << std::endl;
    //std::cout << " pmt " << pmtid << ": tobs=" << tmin << " texp=" << t_expected << " pos=" << fpmtpos[0] << ", " << fpmtpos[1] << ", " << fpmtpos[2] << std::endl;
    //std::cin.get();

    // time neg. log-likelihood
    ll_t += 0.5*( t_expected-tmin )*( t_expected-tmin )/(fSigTime*fSigTime);    
   
  }

  //std::cout << "qprompt=" << total_prompt << " brightest=" << brightest_pmt << std::endl;
  
  // calculate expected dark rate in pmts
  double darkcounts = fDarkRate*1.0e-9*fTimeWindow;

  // loop over all PMTs!
  // calc neg. log likelihood ratio: E-O + (O/E)*log(
  double ll_q = 0.; 
  double sa_tot = 0.0;
  float brightest_pos[3];
  getPMTInfo( brightest_pmt, brightest_pos );
  double sa_brightest =  getSA( x, brightest_pos );
  double dist_brightest = 0.;
  for (int i=0; i<3; i++)
    dist_brightest += (brightest_pos[i]-x[i])*(brightest_pos[i]-x[i]);
  dist_brightest = sqrt( dist_brightest );
  double nexp_brightest = sa_brightest*exp( -dist_brightest/fAbsLength );

  std::map< int, double >::iterator it;
  int ipmt_start = brightest_pmt - brightest_pmt%100 - fNhoops*100; // 10 hoops below
  int ipmt_end = brightest_pmt - brightest_pmt%100 + (fNhoops+1)*100;   // 10 hoops above
  if ( ipmt_start<0 )
   ipmt_start = 0;
  if ( ipmt_end>100000 )
   ipmt_end = 100000;
  //double totexp = fLightYield*x[3]; // x[3] = MeV
  double totdark = darkcounts*(ipmt_end-ipmt_start);
  for (int ipmt=ipmt_start; ipmt<ipmt_end; ipmt++) {
    getPMTInfo( ipmt, pmtpos );
    //pmtinfo->GetEntry( ipmt );
    it = obsHits.find(ipmt);
    //double sa2 = getApproxSA( x, pmtpos );
    double sa = getSA( x, pmtpos );
    sa_tot += sa;
    
    double dist = 0.;
    for (int i=0; i<3; i++) {
      dist += ( pmtpos[i]-x[i] )*( pmtpos[i]-x[i] );
    }
    dist = sqrt(dist); // cm
    double absorb = exp( -dist/fAbsLength );

    double nexp = sa*absorb/nexp_brightest;
    double nobs = 0.0;
    if ( it!=obsHits.end() ) {
      nobs += it->second;
    }
    nobs /= maxq;
    double ll  = 0.0;
    if ( nobs>0 )
      ll += 2.0*( nobs*( log(nobs) - log(nexp) ) );
    ll += 2.0*( nexp-nobs );
//     if ( nobs*total_prompt>darkcounts ) {
//       std::cout << "  sipm=" << ipmt << " pos=" << fpmtpos[0] << ", " << fpmtpos[1] << ", " << fpmtpos[2]  << " "
// 		<< " SA=" << sa 
// 	//<< " SA(2)=" << sa2 << " "
// 		<< " nexp=" << nexp << " nobs=" << nobs << "  abs=" << absorb << std::endl;
//       std::cout << " pos=" << x[0] << ", " << x[1] << ", " << x[2] 
// 	//<< ", mev=" << x[3] 
// 		<< std::endl;
//     }
    ll_q += ll;
  }

//   std::cout << " LL(t)=" << ll_t << " LL(q)=" << ll_q << " total=" << ll_t+ll_q << std::endl;
//   std::cout << " totsa = " << sa_tot << " : " << sa_tot/(4*3.14159) << std::endl;
//   std::cin.get();

  return ll_q;
}

void KPFit::calcSeedFromWeightedMean() {
  // lazy
  for (int i=0; i<4; i++)
    fSeedPos[i] = 0.0;

  // get the z position using this. It's pretty good!
  int npmts = fMCdata->GetMCPMTCount();
  double ntothits = 0.0;
  int brightest = 0;
  double maxhits = 0;
  float pmtpos[3];
  for (int ipmt=0; ipmt<npmts; ipmt++) {
    RAT::DS::MCPMT* pmt = fMCdata->GetMCPMT( ipmt );
    int pmtid = pmt->GetID();
    double pmt_pe = 0.0;
    getPMTInfo( pmtid, pmtpos );
    for ( int ihit=0; ihit<pmt->GetMCPhotonCount(); ihit++ ) {
      if ( pmt->GetMCPhoton( ihit )->GetHitTime()<fPromptCut ) {
	pmt_pe += 1.0;
        for (int i=0; i<3; i++) {
          fSeedPos[i] += pmtpos[i];
        }
      }
    }
    ntothits += pmt_pe;
    if ( pmt_pe>maxhits ) {
      maxhits = pmt_pe;
      brightest = pmtid;
    }
  }
  for (int i=0; i<3; i++) {
    fSeedPos[i] /= ntothits;
    minuit->SetVariableValue(i,fSeedPos[i]);
  }

  // (cheating by seeding with truth)
  double posv[3];
  posv[0] = fMCdata->GetMCParticle(0)->GetPosition().X()/10.0; //change to cm
  posv[1] = fMCdata->GetMCParticle(0)->GetPosition().Y()/10.0; //change to cm
  posv[2] = fMCdata->GetMCParticle(0)->GetPosition().Z()/10.0; //change to cm

  // use the brightest pmt hits to set the visible energy scale
  float brightest_pos[3];
  double brightest_sa = getSA( fSeedPos, brightest_pos );
  //double seed_mev = maxhits/fLightYield/(brightest_sa/(4*3.14159));
  //minuit->SetVariableValue(3, seed_mev ); 
  minuit->FixVariable(2);

//   minuit->SetVariableValue(0, posv[0] );
//   minuit->SetVariableValue(1, posv[1] );
//   minuit->SetVariableValue(2, posv[2] );
  //minuit->SetVariableValue(3, 100.0 );

}

bool KPFit::fit( RAT::DS::MC* _mcdata, double* fitted_pos ) {
  fMCdata = _mcdata;

  int npmts = fMCdata->GetMCPMTCount();
  if ( npmts==0 ) {
    wasrun = false;
    return false;
  }
  wasrun = true;

  calcSeedFromWeightedMean();
  bool converged = minuit->Minimize();
  minuit->PrintResults();
  fitted_pos[0] = minuit->X()[0];
  fitted_pos[1] = minuit->X()[1];
  fitted_pos[2] = minuit->X()[2];

  double posv[3];
  posv[0] = fMCdata->GetMCParticle(0)->GetPosition().X()/10.0; //change to cm
  posv[1] = fMCdata->GetMCParticle(0)->GetPosition().Y()/10.0; //change to cm
  posv[2] = fMCdata->GetMCParticle(0)->GetPosition().Z()/10.0; //change to cm

  double err = 0.;
  for (int i=0; i<3; i++) {
    err += (fitted_pos[i]-posv[i])*(fitted_pos[i]-posv[i]);
  }
  err = sqrt(err);
  double rv = sqrt( posv[0]*posv[0] + posv[1]*posv[1] );
  double r  = sqrt( fitted_pos[0]*fitted_pos[0] + fitted_pos[1]*fitted_pos[1] );

  std::cout << "FIT: " << fitted_pos[0] << ", " << fitted_pos[1] << ", " << fitted_pos[2] << std::endl; //<< " evis=" << minuit->X()[3] << std::endl;
  std::cout << "TRUTH: " << posv[0] << ", " << posv[1] << ", " << posv[2] << " MeV=" << fMCdata->GetMCParticle(1)->GetKE() << std::endl;
  std::cout << "Error: " << err << " cm. dr = " << r-rv << " cm " << std::endl;
  return converged;
}

void KPFit::getCorners( const float* sipmpos, std::vector< std::vector< float > >& corners ) const {
  // PMTPOS: center of PMT.  We use our knowledge that the
  //         detector s axial to infer the norm of the SiPM
  double norm[3] = { -sipmpos[0], -sipmpos[1], 0 };
  double nn = 0.0;
  for (int i=0; i<3; i++)
    nn += norm[i]*norm[i];
  nn = sqrt(nn);
  for (int i=0; i<3; i++)
    norm[i] /= nn;
  
  double crossz[3]; // cross with z: (z x norm) = crossz ( 0,1 = 2; -2,1 = 0; 2,0 = 1 )
  crossz[0] = (-1)*norm[1];
  crossz[1] = (1)*norm[0];
  crossz[2] = 0.0;
  double nc = 0.0;
  for (int i=0; i<3; i++)
    nc += crossz[i]*crossz[i];
  nc = sqrt(nc);
  for (int i=0; i<3; i++)
    crossz[i] /= nc;

  double sipmR = 3.0; // half width (3 mm)
  double _corners[4][3] = { 0 };
  for (int a=0; a<2; a++) {
    for (int i=0; i<3; i++) {
      _corners[ 2*a + 0 ][i] = sipmpos[i] + sipmR*crossz[i];
      _corners[ 2*a + 1 ][i] = sipmpos[i] - sipmR*crossz[i];
    }
    if ( a==0 ) {
      _corners[ 2*a + 0 ][2] += sipmR;
      _corners[ 2*a + 1 ][2] += sipmR;
    }
    else {
      _corners[ 2*a + 0 ][2] -= sipmR;
      _corners[ 2*a + 1 ][2] -= sipmR;
    }
  }
  
  for (int a=0; a<4; a++) {
    std::vector< float > c;
    for (int i=0; i<3; i++) {
      c.push_back( _corners[a][i] );
    }
    corners.push_back( c );
  }
  
}

void KPFit::getApproxBounds( const float* sipmpos, const double* pos, double* cozbound, double* phibound ) const {
  // PMTPOS: center of PMT.  We use our knowledge that the
  //         detector s axial to infer the norm of the SiPM
  // POS: hypothesized location of interaction
  // Returns:
  //   cozbound: lower and upper bounds of cos(theta) angle
  //   phibound: lower and upper bound of phi angle


  std::vector< std::vector<float> > corners;
  getCorners( sipmpos, corners );  

  double corner_coz[4];
  double corner_phi[4];
  for (int a=0; a<4; a++) {
    // cosz
    double diff[3] = {0.0};
    double nd = 0.0;
    for (int i=0; i<3; i++) {
      diff[i] = corners[a][i] - pos[i];
      nd += diff[i]*diff[i];
    }
    nd = sqrt(nd);
    for (int i=0; i<3; i++)
      diff[i] /= nd;
    corner_coz[a] = diff[2];
    double sinz = sqrt(1-corner_coz[a]*corner_coz[a]);
    corner_phi[a] = atan2( diff[1], diff[0] );
    if ( corner_phi[a]<0 )
      corner_phi[a] += 2*3.14159;
  }
  
  double minphi = 100.0;
  double mincz = 100.0;
  double maxcz = -100.0;
  double maxphi = -100.0;
  for (int a=0; a<4; a++) {
    if ( corner_coz[a]<mincz )
      mincz = corner_coz[a];
    if ( corner_coz[a]>maxcz )
      maxcz = corner_coz[a];
    
    if ( corner_phi[a]<minphi )
      minphi = corner_phi[a];
    if ( corner_phi[a]>maxphi )
      maxphi = corner_phi[a];
  }
  
  cozbound[0] = mincz;
  cozbound[1] = maxcz;
  phibound[0] = minphi;
  phibound[1] = maxphi;

}

double KPFit::getApproxSA( const double* pos, const float* sipmpos ) const {
  double cosbound[2], phibound[2];
  getApproxBounds( sipmpos, pos, cosbound, phibound );
  return ( cosbound[1]-cosbound[0] )*( phibound[1]-phibound[0] );
}

double KPFit::saOfCenteredRectangle( double xwidth, double yheight, double zdist ) const {
  double alpha = xwidth/(2*zdist);
  double beta  = yheight/(2*zdist);
  return 4*acos( sqrt( (1+alpha*alpha + beta*beta)/( (1+alpha*alpha)*(1 + beta*beta ) ) ) );
}

double KPFit::getSA( const double* _pos, const float* _sipmpos ) const {
  // we calculate the solid angle of an arbitrary rectangle by combining
  // solid angle calculations for boxes whose center is normal to pos
  // to do this, we need to rotate the SiPM so that center is on x axis.


  // rotate the frame along the z-axis so that the sipm is sitting at (R,0,0)
  double shift_z = -_sipmpos[2];
  double rot_phi = -atan2( _sipmpos[1], _sipmpos[0] );
//   if ( rot_phi<0 )
//     rot_phi += 2.0*3.14159;
  double sipmpos[3];
  double pos[3];
  sipmpos[0] = cos(rot_phi)*_sipmpos[0] - sin(rot_phi)*_sipmpos[1];
  sipmpos[1] = sin(rot_phi)*_sipmpos[0] + cos(rot_phi)*_sipmpos[1];
  sipmpos[2] = _sipmpos[2] + shift_z;
  pos[0] = cos(rot_phi)*_pos[0] - sin(rot_phi)*_pos[1];
  pos[1] = sin(rot_phi)*_pos[0] + cos(rot_phi)*_pos[1];
  pos[2] = _pos[2] + shift_z;
//   std::cout << "getsa" << std::endl;
//   std::cout << "  _pos=" << _pos[0] << ", " << _pos[1] << ", " << _pos[2] << std::endl;
//   std::cout << "  _sipm=" << _sipmpos[0] << ", " << _sipmpos[1]  << ", " << _sipmpos[2] << std::endl;
//   std::cout << "  rot pos="  << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
//   std::cout << "  rot sipm=" << sipmpos[0] << ", " << sipmpos[1]  << ", " << sipmpos[2] << std::endl;

  assert( fabs(sipmpos[1])<1.0e-5 );

  // now transform system so that pos is at origin
  for (int i=0; i<3; i++) {
    sipmpos[i] -= pos[i];
    pos[i] -= pos[i];
  }

//   std::cout << "  final sipm=" << sipmpos[0] << ", " << sipmpos[1]  << ", " << sipmpos[2]
// 	    << " / " << pos[0] << ", " << pos[1]  << ", " << pos[2] << std::endl;

  // ---------------------
  // |   b,d    |  x,d   |
  // ---------------------
  // | a,b,c,d  |  c,d   |
  // ---------------------
  // x = d + [b-a] + [c-a] + a
  double sipmW = 0.6; // cm

  // box a
  double a_halfw = fabs( sipmpos[1] )-0.5*sipmW;
  double a_halfh = fabs( sipmpos[2] )-0.5*sipmW;
  double a_dist  = fabs(sipmpos[0]);
  double sa_a = saOfCenteredRectangle( 2*a_halfw, 2*a_halfh, a_dist );

  // box b
  double b_halfw = fabs( sipmpos[1] )-0.5*sipmW;
  double b_halfh = fabs( sipmpos[2] )+0.5*sipmW;
  double b_dist  = fabs(sipmpos[0]);
  double sa_b = saOfCenteredRectangle( 2*b_halfw, 2*b_halfh, b_dist );

  // box c
  double c_halfw = fabs( sipmpos[1] )+0.5*sipmW;
  double c_halfh = fabs( sipmpos[2] )-0.5*sipmW;
  double c_dist  = fabs(sipmpos[0]);
  double sa_c = saOfCenteredRectangle( 2*c_halfw, 2*c_halfh, c_dist );

  // box d
  double d_halfw = fabs( sipmpos[1] )+0.5*sipmW;
  double d_halfh = fabs( sipmpos[2] )+0.5*sipmW;
  double d_dist  = fabs(sipmpos[0]);
  double sa_d = saOfCenteredRectangle( 2*d_halfw, 2*d_halfh, d_dist );

  double sa_total = 0.25*( sa_d - sa_b - sa_c + sa_a );
  //std::cout << "   " << sa_total << " = 0.25*( " << sa_d << " - " << sa_b << " - " << sa_c << " + " << sa_a << ")" << std::endl;

  return 0.25*( sa_d - sa_b - sa_c + sa_a );
  
}

void KPFit::storePMTInfo() {
  fpmtposdata = new float[ 3*pmtinfo->GetEntries() ];
  for (int i=0; i<pmtinfo->GetEntries(); i++) {
    pmtinfo->GetEntry(i);
    // save in map
    std::vector< float >* pmtpos = new std::vector< float >;
    pmtpos->push_back( fpmtpos[0] );
    pmtpos->push_back( fpmtpos[1] );
    pmtpos->push_back( fpmtpos[2] );
    fpmtposmap[i] = pmtpos;
    // save in array
    memcpy( fpmtposdata + 3*i, fpmtpos, 3*sizeof(float) );
  }
}

void KPFit::getPMTInfo( int pmtid, float* pmtpos ) const {
  // use map
//   std::map< int, std::vector< float >* >::iterator it = fpmtposmap.find( pmtid );
//   for (int i=0; i<3; i++) {
//     pmtpos[i] =it->second->at(i);
//   }
  // use array
  if ( pmtid>=0 && pmtid<100000 )
    memcpy( pmtpos, fpmtposdata + 3*pmtid, 3*sizeof(float) );
  else {
    std::cerr << "seeking position for pmtid=" << pmtid << std::endl;
    memset( pmtpos, 0, 3*sizeof(float) );
  }
}

void KPFit::deletePMTInfo() {
  std::map< int, std::vector< float >* >::iterator it;
  for (it=fpmtposmap.begin(); it!=fpmtposmap.end(); it++) {
    delete it->second;
  }
  fpmtposmap.clear();
  delete [] fpmtposdata;
}
