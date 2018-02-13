// Decay0.cc
// See notes in Decay0.hh, COPYING.decay0

// Ignore -Wshadow for this file, to allow compilation with -Werror enabled
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#include <RAT/Decay0.hh>
#include <iostream>
#include <cstring>
#include <cmath>
#include <stdio.h>
#include <algorithm>
#include <complex>
#include <sstream>

#include "TF1.h"
#include "TF2.h"
#include "Math/WrappedMultiTF1.h"
#include "Math/AllIntegrationTypes.h"
#include <RAT/Log.hh>
#include "TMath.h"
#include "Math/WrappedTF1.h"
#include "Math/GaussIntegrator.h"
#include "Math/Integrator.h"
#include "Math/IntegratorMultiDim.h"
#include "Math/AllIntegrationTypes.h"
#include "Math/Functor.h"
#include "Math/GaussIntegrator.h"

#include "Math/IFunction.h"
#include "Math/WrappedParamFunction.h"
#include "Math/AdaptiveIntegratorMultiDim.h"
#include "Math/IFunctionfwd.h"
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include "Math/GaussLegendreIntegrator.h"

// Replaced the existing using namespace std by explicit using statements to reduce the risk of variable shadowing
using std::ostringstream;
using std::max;

using CLHEP::s;
using CLHEP::ns;
using CLHEP::pi;
using CLHEP::twopi;

namespace RAT {


///************************************************/
// Values of natural logarithm of the standard values of momentum
// (in units m_e*c) from:  H.Behrens, J.Janecke,
// "Numerical tables for beta-decay and electron capture",
// Berlin, Springer-Verlag, 1969.
// Range of momenta correspond to kinetic energy range
// from 2.55 keV to 25.0 MeV.
double plog69[]={-2.302585,  -1.609438,  -1.203973,  -0.9162907, -0.6931472, -0.5108256, -0.3566750, -0.2231435, -0.1053605,  0.0000000, 0.1823216,  0.3364722,  0.4700036,  0.5877866,  0.6931472, 0.7884574,  0.8754688,  0.9555114,  1.029619,   1.098612, 1.163151,   1.223776,   1.280934,   1.335001,   1.386294, 1.504077,   1.609438,   1.704748,   1.791759,   1.871802, 1.945910,   2.014903,   2.079442,   2.197225,   2.302585, 2.397895,   2.484907,   2.564949,   2.639057,   2.772589, 2.890372,   2.995732,   3.218876,   3.401197,  3.555348, 3.688879,   3.806663,   3.912023 };
///************************************************/
Double_t Decay0::funbeta(Double_t *x, Double_t *par)
{
  Double_t xx =x[0];
  //  par[0]=fQbeta;
  //  par[1]=GetMass(3);
  //  par[2]=fZdtr;
  Double_t funbeta=0;
  if(xx>0.) funbeta=sqrt(xx*(xx+2.*par[1]))*(xx+par[1])*pow((par[0]-xx),2)*fermi(par[2],xx);
  return funbeta;
}
///************************************************/
Double_t Decay0::funbeta1f(Double_t *x, Double_t *par)
{
  Double_t xx =x[0];
  Double_t funbeta1f=0.;

  if (xx>0){
    float all=sqrt(xx*(xx+2.*par[1]))*(xx + par[1])*pow(par[0]-xx,2)*fermi(par[2],xx);
    float w=xx/par[1]+1.;
    float cf=1.+fC1/w+ fC2*w+ fC3*w*w+ fC4*w*w*w;
    funbeta1f=all*cf;
  }
  return funbeta1f;
}
///************************************************/
Double_t Decay0::funbeta1fu(Double_t *x, Double_t *par)
{
  Double_t xx =x[0];
  Double_t funbeta1fu=0.;
  if (xx>0){ //allowed spectrum
     double all=sqrt(xx*(xx+2.*par[1]))*(xx +par[1])*pow(par[0]-xx,2)*fermi(par[2],xx);

     // correction factor 1 (theoretical)
     double w=xx/par[1]+1.;
     double pel=sqrt(w*w-1.);
     double pnu=(par[0]-xx)/par[1];

     // calculation of the screened lambda2 value by
     // interpolation of the table  with the help
     // of the divdif CERN function for logarithms of p
     double pellog=log(pel);
     double scrl2=divdif(plog69,pellog);
     double cf1=pow(pnu,2)+scrl2*pow(pel,2);
     //  correction factor 2 (empirical)
     double cf2=1.+fC1/w+ fC2*w+ fC3*w*w+ fC4*w*w*w;

     // spectrum with correction
     funbeta1fu=all*cf1*cf2;
  }
  return funbeta1fu;
}
///************************************************/
Double_t Decay0::funbeta2f(Double_t *x, Double_t *par)
{
  Double_t xx =x[0];
  Double_t funbeta2f=0.;
  if (xx>0){
     //allowed spectrum
    float all=sqrt(xx*(xx+2.*par[1]))*(xx + par[1])*pow(par[0]-xx,2)*fermi(par[2],xx);
     //correction factor
    float w=xx/par[1]+1.;
    float pel=sqrt(w*w-1.);
    float pnu=(par[0]-xx)/par[1];
    float cf=1.;
    if(fKf==1) cf=pel*pel+fC1*pnu*pnu;
    if(fKf==2) cf=pow(pel,4)+fC1*pel*pel*pnu*pnu+fC2*pow(pnu,4);
    if(fKf==3) cf=pow(pel,6)+fC1*pow(pel,4)*pnu*pnu+fC2*pel*pel*pow(pnu,4)+fC3*pow(pnu,6);
    if(fKf==4) cf=pow(pel,8)+fC1*pow(pel,6)*pnu*pnu+fC2*pow(pel,4)*pow(pnu,4)+fC3*pel*pel*pow(pnu,6)+
                  fC4*pow(pnu,8);
    //spectrum with correction
    funbeta2f=all*cf;
  }
  return funbeta2f;
}
///************************************************/
Decay0::Decay0() : fHasTimeCutoff(false), fHasAlphaCut(false),fIsotope("")
{
  // Set the default time cutoff to what was used in the past
  fCutoffWindow = 500.0*ns;

}
///************************************************/
Decay0::Decay0(const std::string isotope, const int level, const int mode, const float lE, const float hE)
{
  fLevel= level;
  fIsotope= isotope;
  fMode= mode;
  fLoE=lE;
  fHiE=hE;
  #ifdef DEBUG
   info << "In RAT::Decay0::Decay0(), isotope = "<< isotope << " level= "<<fLevel <<std::endl;
   info << "                          bb-mode= "<<fMode<<" energy range ("<<fLoE<<", "<<fHiE<<")"<<std::endl;
  #endif
  // Set the default time cutoff to what was used in the past
  fCutoffWindow = 500.0*ns;

  fHasTimeCutoff = false;
  fHasAlphaCut = false;
}
///************************************************/
Decay0::Decay0(const std::string isotope)
{
  fIsotope= isotope;
  #ifdef DEBUG
   info << "In RAT::Decay0::Decay0(), isotope = "<< isotope << std::endl;
  #endif
  // Set the default time cutoff to what was used in the past
  fCutoffWindow = 500.0*ns;
  fHasTimeCutoff = false;
  fHasAlphaCut = false;
  fCurParentIdx = 0;
}
///************************************************/
Decay0::~Decay0()
{
}
///************************************************/
void Decay0::GenBBTest()
{
  // Testing if parameters agree with the conditions for double beta decay
  fNbPart=0;
  fPparent.clear();
  fCurParentIdx = 0;
  if(fMode>=1 && fMode<=5)    fModebb=fMode;
  if(fMode==6)                fModebb=14;
  if(fMode==7)                fModebb=6;
  if(fMode==8)                fModebb=13;
  if(fMode>=9 && fMode<=14)   fModebb=fMode-2;
  if(fMode==15 || fMode==16)  fModebb=fMode;

  fEbb1=0.;
  fEbb2=4.3;

  if ((fModebb==4 || fModebb== 5 || fModebb== 6|| fModebb== 8 ||
       fModebb==10|| fModebb==13 || fModebb==14)){
    fEbb1=fLoE;
    fEbb2=fHiE;
  }

  fLdecay = DB::Get()->GetLink("Decay0", fIsotope);
  fQbb    =fLdecay->GetD("Qbb");
  fZdbb   =fLdecay->GetI("Zdbb");
  try{fEbindeK=fLdecay->GetD("EbindeK");} catch (DBNotFoundError &e){;}
  try{fEbindeL=fLdecay->GetD("EbindeL");} catch (DBNotFoundError &e){;}
  try{fEbindeM=fLdecay->GetD("EbindeM");} catch (DBNotFoundError &e){;}
  try{fEnGamma =fLdecay->GetDArray("EnGamma");}    catch (DBNotFoundError &e){;}
  fEK     =fLdecay->GetD("EK");
  fLevelE =fLdecay->GetIArray("LevelE");
  fTrans  =fLdecay->GetIArray("Trans");

  float El=fLevelE[fLevel]/1000;

  if(fLevel==8 && fIsotope=="Ru96") fEK=0.003;
  if(fLevel==9 && fIsotope=="Ru96") fEK=0.002;
  if(fLevel==9 && fIsotope=="Ce136")fEK=0.006;

  if (fZdbb>=0) fE0=fQbb;
  if (fZdbb<0)  fE0=fQbb-4.*GetMass(3);
  if (fModebb== 9 || fModebb==10) fE0=fQbb-fEK-2.*GetMass(3);
  if (fModebb==11 || fModebb==12) fE0=fQbb-2.*fEK;

  if (fE0<=El){
    Log::Die(dformat("****\nnot enough energy for transition to this level, full energy release and Elevel: %lf %lf\n****",fE0,El));
  }

  if (fZdbb>=0 && (fModebb==9 || fModebb==10 || fModebb==11 || fModebb==12)){
    Log::Die(dformat("****\ndecay mode (%d) and nuclide (%s) are inconsistent:\n this mode is only for isotopes with Zdbb<0 ! \n**** ",fMode,fIsotope.data()));
  }

  if ((fTrans[fLevel]==0)&& (fModebb==7 || fModebb==8 || fModebb==16)){
    Log::Die(dformat("****\ndecay mode (%d): spin of daughter nucleus level should be 2+: you have %d+ \n****\n", fMode,fTrans[fLevel]));
  }
  if ((fTrans[fLevel]==2) && (fModebb!=3&& fModebb!=7&& fModebb!=8&& fModebb!=9&& fModebb!=10&& fModebb!=11&& fModebb!=12&& fModebb!=16)){
     Log::Die(dformat("****\ndecay mode (%d): spin of daughter nucleus level should be 0+: you have %d+ \n****", fMode,fTrans[fLevel]));
  }
  fSpmax=-1;fFe2m=-1;
  fStartbb=0;
  bb();
  fStartbb=1;
}

///************************************************/
void Decay0::GenBackgTest()
{

  info << "Decay0::GenBackgTest : Isotope name: " << fIsotope << endl;
  info << "Decay0::GenBackgTest : Checked suffixes: " << endl;
  info << "Decay0::GenBackgTest : [PURE]    : " << ((fHasAlphaCut)?"true":"false")  << newline;
  info << "Decay0::GenBackgTest : [TIMECUT] : " << ((fHasTimeCutoff)?"true":"false")  << newline;
  if (fHasTimeCutoff) {
    info << "Decay0::GenBackgTest : Current time cutoff set to " << GetCutoffWindow()/ns << " ns." << endl;
  }
  fLdecay = DB::Get()->GetLink("Decay0Backg", fIsotope);
  try{fHLifeTime=fLdecay->GetDArray("Thnuc");} catch (DBNotFoundError &e) {Log::Die("Decay0: can't find isotope " + fIsotope);}
  info<<"Decay0::GenBackgTest : Loading data for isotope: "<<fIsotope<<endl;
  try{fEbindeK = fLdecay->GetD("EbindeK");} catch (DBNotFoundError &e){;}
  try{fEbindeK2 = fLdecay->GetD("EbindeK2");} catch (DBNotFoundError &e){;}
  try{fEbindeL = fLdecay->GetD("EbindeL");}   catch (DBNotFoundError &e){;}
  try{fEbindeL2 = fLdecay->GetD("EbindeL2");} catch (DBNotFoundError &e){;}
  try{fEbindeM = fLdecay->GetD("EbindeM");}   catch (DBNotFoundError &e){;}
  try{fEbindeM2 = fLdecay->GetD("EbindeM2");} catch (DBNotFoundError &e){;}
  try{fDaughterZ = fLdecay->GetIArray("Zdtr");} catch (DBNotFoundError &e){;}
  try{fShCorrFactor = fLdecay->GetDArray("SCorF");}catch (DBNotFoundError &e){;}
  try{fProbDecay=fLdecay->GetDArray("ProbDecay");} catch (DBNotFoundError &e){;}
  try{fProbAlpha=fLdecay->GetDArray("ProbAlpha");} catch (DBNotFoundError &e){;}

  if (fHasAlphaCut) {
    if (fProbDecay.size() != 2) {
      ostringstream msg;
      msg << "Decay0::GenBackgTest : Requested \"-pure\" suffix on a decay with " << fProbDecay.size() << " parents! (2 expected).";
      Log::Die(msg.str());
    }
    // This is exactly what the previous implementation of "-pure" did
    fProbDecay[0] = 0.0;
  }

  try{fEnAlpha=fLdecay->GetDArray("EnAlpha");}     catch (DBNotFoundError &e){;}
  try{fProbEC =fLdecay->GetDArray("ProbEC");}      catch (DBNotFoundError &e){;}
  try{fEnGamma =fLdecay->GetDArray("EnGamma");}    catch (DBNotFoundError &e){;}
  try{fProbBeta=fLdecay->GetDArray("ProbBeta");}   catch (DBNotFoundError &e){;}
  try{fEndPoint=fLdecay->GetDArray("EndPoint");}   catch (DBNotFoundError &e){;}



}
///************************************************/
void Decay0::GenEvent()
{
  fThnuc=fHLifeTime[0];
  fZdtr=fDaughterZ[0];
  fTevst=0.;
  fTclev=0.;
  fThlev=0.;
  float tcnuc=0.;
  fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
  fTevst=fTdnuc;
  fNbPart=0;
  fCurParentIdx = 0;
  fPparent.clear();
  float pdecay, pbeta, p;

  if (fIsotope=="Ac228") Ac228();
  else if (fIsotope=="Ar39"){
     // Nucl. Phys. A 633(1998)1.
     beta1fu(fEndPoint[0],tcnuc,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
  }
  else if (fIsotope=="Ar42"){
     // Nucl. Data Sheets 92(2001)1.
     beta1fu(fEndPoint[0],tcnuc,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
  }
  else if (fIsotope=="As79") As79();
  else if (fIsotope=="Be11") Be11();
  else if (fIsotope=="Bi207") Bi207();
  else if (fIsotope=="Bi208"){
     // NDS 47(1986)797 + ToI-1978.
     // VIT, 17.12.1995; 10.05.2005
     pdecay=100.*GetRandom();
     if (pdecay<=fProbDecay[0]) fEgamma=fEbindeK;            //EC-K 43.6%
     else if (pdecay>fProbDecay[0] && pdecay<=fProbDecay[1]) //EC-L 40.2%
          fEgamma=fEbindeL;
     else fEgamma=fEbindeM;                                  //EC-M 16.2%

     particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
     fThlev=32.e-12;
     nucltransK(fEnGamma[0],fEbindeK,8.5e-3,4.3e-4);
  }
  else if (fIsotope=="Bi210") Bi210();
  else if (fIsotope=="Bi212") Bi212();
  else if (fIsotope=="BiPo212") {
     Bi212();
     int fNbPart0=fNbPart;

     if (fNpGeant[1]!=47){    ///decay of Po212
        fThnuc=fHLifeTime[1];
        if (fHasTimeCutoff) {
          double time = tcnuc-fThnuc/log(2.)*log(GetRandom());
          while( time > GetCutoffWindow()/s )  // Time cut off here
            time = tcnuc-fThnuc/log(2.)*log(GetRandom());
          fTdnuc=time;
        } else {
          fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
        }
        fCurParentIdx++; // Increment the parent, since this is the decay from Po
        particle(47,fEnAlpha[5],fEnAlpha[5],0.,pi,0.,twopi,0,0);
        fPtime[fNbPart0+1]=fPtime[fNbPart0+1]+fTdnuc;
     }
  }
  else if (fIsotope=="Bi212Tl208"){
    //Only 212Bi alpha-decay to 208Tl.
    bool next400=false, next328=false;
    float palfa;

    palfa=100.*GetRandom(); //212Bi alpha-decay
    if (palfa<=fProbAlpha[0]){  // 1.10%
       particle(47,fEnAlpha[0],fEnAlpha[0],0.,pi,0.,twopi,0,0);
       p=100.*GetRandom();
       if (p<= 5.){
	  nucltransK(0.493,0.086,2.8e-2,0.);
       }
       else if (p<=99.){
	  nucltransK(0.453,0.086,0.18,0.);
	  next400=true;
       }
       else{
	  nucltransK(0.164,0.086,0.75,0.);
	  next328=true;
       }
    }
    else if (palfa<=fProbAlpha[1]){  // 0.15%
       particle(47,fEnAlpha[1],fEnAlpha[1],0.,pi,0.,twopi,0,0);
       p=100.*GetRandom();
       if (p<=68.){
	  nucltransK(0.474,0.086,0.14,0.);
       }
       else if (p<=87.){
	  nucltransK(0.434,0.086,0.14,0.);
	  next400=true;
       }
       else{
	  nucltransK(0.145,0.086,2.8,0.);
	  next328=true;
       }
    }
    else if (palfa<=fProbAlpha[2]){  // 1.67%
       particle(47,fEnAlpha[2],fEnAlpha[2],0.,pi,0.,twopi,0,0);
       next328=true;
    }
    else if (palfa<=fProbAlpha[3]){ // 69.88%
       particle(47,fEnAlpha[3],fEnAlpha[3],0.,pi,0.,twopi,0,0);
       next400=true;
    }
    else{                  // 27.20%
       particle(47,fEnAlpha[4],fEnAlpha[4],0.,pi,0.,twopi,0,0);
    }

    if (next328){
       p=100.*GetRandom();
       if (p<=29.){
	  nucltransK(0.328,0.086,0.33,0.);
       }
       else{
	  nucltransK(0.288,0.086,0.53,0.);
	  next400=true;
       }
    }
    if (next400){
       fThlev=6.e-12;
       nucltransK(0.040,0.015,22.55,0.);
    }
    int fNbPart0=fNbPart;
    //beta-decay of Tl208
    fThnuc=fHLifeTime[1];
    fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
    fCurParentIdx++; // Increment the parent, since this is now the decay from Tl
    Tl208();
    //set the time of the first 208Tl-decay emitted particle to be after T1/2
    fPtime[fNbPart0+1]=fPtime[fNbPart0+1]+fTdnuc;
    //set the time of the following 208Tl-decay emitted particles to be after the first one
    int i = 0;
    for (i=fNbPart0+2; i<=fNbPart; i++){
        fPtime[i]=fPtime[fNbPart0+1]+fPtime[i];
    }
  }
  else if (fIsotope=="Bi214") Bi214();
  else if (fIsotope=="BiPo214") {
    Bi214();
    int fNbPart0=fNbPart;

    if (fNpGeant[1]!=47){  ///decay of Po214
      fCurParentIdx++; // Increment the parent, since this is the decay from Po
      Po214();
      if (fHasTimeCutoff) {
        // There is a time cutoff
        double time = tcnuc-fThnuc/log(2.)*log(GetRandom());
        while( time > GetCutoffWindow()/s )  // 500ns cut off here
          time = tcnuc-fThnuc/log(2.)*log(GetRandom());
        fTdnuc=time;
      }
      fPtime[fNbPart0+1]=fPtime[fNbPart0+1]+fTdnuc;
    }
  }
  else if (fIsotope=="C10"){
     //information from http://www.nndc.bnl.gov/nudat2/decaysearchdirect.jsp?nuc=10C&unc=nds
     //K binding energy from: https://userweb.jlab.org/~gwyn/ebindene.html
     pdecay=100.0*GetRandom();
     if (pdecay <= fProbDecay[0]){
     // beta+ decay -> 10B
        pbeta = 100.0*GetRandom();
        if (pbeta <= fProbBeta[0]){
           beta(fEndPoint[0], 0., 0.);
           nucltransK(fEnGamma[0], fEbindeK, 0., 0.);
           nucltransK(fEnGamma[1], fEbindeK, 0., 0.);
        }
        else if(pbeta > fProbBeta[0] && pbeta <= fProbBeta[1]){
           beta(fEndPoint[1], 0., 0.);
           nucltransK(fEnGamma[1], fEbindeK, 0., 0.);
        }
     }
     else{
        //EC -> 10B
        nucltransK(fEnGamma[0], fEbindeK, 0., 0.);
        nucltransK(fEnGamma[1], fEbindeK, 0., 0.);
     }
  }
  else if (fIsotope=="C11"){
     //information from http://www.nndc.bnl.gov/nudat2/decaysearchdirect.jsp?nuc=11C&unc=nds
     beta(fEndPoint[0], 0., 0.);
  }
  else if (fIsotope=="C14"){
     // NPA 523(1991)1 and ToI'1998
     // VIT, 5.11.2006.
     // experimental corrections to the allowed beta shape from
     // F.E.Wietfeldt et al., PRC 52(1995)1028
     // cf(e)=(1+c1/w+c2*w+c3*w**2+c4*w**3), w=e/GetMass(3)+1
     beta1f(fEndPoint[0],tcnuc,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
  }
  else if (fIsotope=="Ca48"){
     // It is supposed that decay goes to excited 5+ level of
     // Sc48 (E_exc=131 keV) with T1/2=1.1e21 y calculated in
     // M.Aunola et al., Europhys. Lett. 46(1999)577
     // (transition to g.s. is suppressed by factor of ~1e-10;
     // to 4+ excited level with E_exc=252 keV - by factor of ~1e-5).
     // VIT, 07.05.1998; update of 13.08.2007.
     beta(fEndPoint[0],0.,0.);
     nucltransK(fEnGamma[0],fEbindeK,8.1e-3,0.);
     int fNbPart0=fNbPart;
     Sc48();
     fPtime[fNbPart0+1]=fPtime[fNbPart0+1]+fTdnuc;
  }
  else if (fIsotope=="Cd113"){
     // Half-life and coefficients in the shape correction factor
     // are taken from: F.A.Danevich et al., Phys. At. Nuclei 59(1996)1.
     //  Q_beta=0.320 MeV, G.Audi et al., Nucl. Phys. A 729(2003)337.
     // VIT, 31.03.2006.
     beta2f(fEndPoint[0],0.,0.,3,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
  }
  else if (fIsotope=="Co60")  Co60();
  else if (fIsotope=="Cs136") Cs136();
  else if (fIsotope=="Cs137"){
     // Nucl. Data Sheets 72(1994)355
     // really it is model for (Cs137 + Ba137m)-decay
     // but not the model of Cs137 decay alone !
     pbeta=100.*GetRandom();
     if (pbeta<=fProbBeta[0]){
        beta1fu(fEndPoint[0],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
        fThlev=153.12;
        nucltransKL(fEnGamma[0],fEbindeK,9.0e-2,fEbindeL,1.6e-2,0.);
     }
     else{
        beta1f(fEndPoint[1],0.,0.,fShCorrFactor[4],fShCorrFactor[5],fShCorrFactor[6],fShCorrFactor[7]);
     }
  }
  else if (fIsotope=="Eu147") Eu147();
  else if (fIsotope=="Eu152") Eu152();
  else if (fIsotope=="Eu154") Eu154();
  else if (fIsotope=="Gd146") Gd146();
  else if (fIsotope=="He6"){
     //information from http://www.nndc.bnl.gov/nudat2/decaysearchdirect.jsp?nuc=6HE&unc=nds
     beta(fEndPoint[0],0.,0.);
  }
  else if (fIsotope=="Hf182"){
     // Scheme of 182Hf decay("Table of Isotopes", 7th ed., 1978)
     // VIT, 5.03.1996.
     beta(fEndPoint[0],0.,0.);
     fThlev=1.2e-9;
     p=100.*GetRandom();
     if (p<=91.77){
        nucltransK(fEnGamma[0],fEbindeK,3.1e-1,0.);
     }
     else if (p<=92.00){
        nucltransK(fEnGamma[1],fEbindeK,9.5e-2,0.);
        fThlev=0.;
        nucltransK(fEnGamma[2],fEbindeK,5.0e-0,0.);
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,1.5e-1,0.);
        fThlev=0.;
        nucltransK(fEnGamma[4],fEbindeK,4.5e-0,0.);
     }
  }
  else if (fIsotope=="I126") I126();
  else if (fIsotope=="I133") I133();
  else if (fIsotope=="I134") I134();
  else if (fIsotope=="I135") I135();
  else if (fIsotope=="K40")  K40();
  else if (fIsotope=="K42")  K42();
  else if (fIsotope=="Kr81"){
     // NDS 79(1996)447 and ENSDF at NNDC site on 9.12.2007).
     // VIT, 9.12.2007.
     float pklm;
     pdecay=100.*GetRandom();

     if (pdecay<=fProbDecay[0]){
        //capture from only K shell is supposed
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
        fThlev=9.7e-12;
        nucltransKLM(fEnGamma[0],fEbindeK,7.3e-3,fEbindeL,7.8e-4,fEbindeM,2.6e-4,0.);
     }
     else{
        pklm=100.*GetRandom();
        if (pklm<=84.73) particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
        if (pklm>84.73 && pklm<=97.44) particle(1,fEbindeL,fEbindeL,0.,pi,0.,twopi,0,0);
        if (pklm>97.44) particle(1,fEbindeM,fEbindeM,0.,pi,0.,twopi,0,0);
     }
   }
   else if (fIsotope=="Kr85"){
      // NDS 62(1991)271 and ENSDF at NNDC site on 9.12.2007).
      // VIT, 9.12.2007.
      pdecay=100.*GetRandom();

      if (pdecay<=fProbDecay[0]){
         beta(fEndPoint[0],0.,0.);
         fThlev=1.015e-6;
         p=100.*GetRandom();
         if (p<=99.99947){
            nucltransKLM(fEnGamma[0],fEbindeK,6.3e-3,fEbindeL,7.1e-4,fEbindeM,2.3e-4,0.);
         }
         else{
            nucltransKLM(fEnGamma[1],fEbindeK,2.9e-2,fEbindeL,3.9e-3,fEbindeM,1.3e-3,0.);
            fThlev=0.71e-9;
            nucltransKLM(fEnGamma[2],fEbindeK,4.3e-2,fEbindeL,4.8e-3,fEbindeM,3.4e-4,0.);
         }
      }
      else beta2f(fEndPoint[1],0.,0.,1,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);// 1st forbidden unique beta decay
     }
     else if (fIsotope=="Mn54"){
        // "Table of Isotopes",8th ed.,1996 + NDS 50(1987)255).
        // Accuracy in description of:
        // decay branches - 0.001%, deexcitation process - 0.001%.
        // VIT, 16.04.1998.
        // VIT, 1.04.2007, updated to NDS 107(2006)1393.
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);//100% EC to Cr54
        fThlev=7.9e-12;
        nucltransK(fEnGamma[0],fEbindeK,2.4e-4,0.);
     }
     else if (fIsotope=="Na22"){
        // "Table of Isotopes", 7th ed., 1978).
        // Accuracy in description of
        // decay branches - 0.001%, deexcitation process - 0.001%.
        // VIT, 26.07.1993, 22.10.1995.
        // VIT, 12.11.2006 (updated to NDS 106(2005)1
        // and change to beta spectra with experimental corrections).
        // VIT, 26.08.2007 (corrected beta shapes).
        pdecay=100.*GetRandom();
        if (pdecay<=fProbDecay[1]){
           fThlev=3.63e-12;
           if (pdecay<=fProbDecay[0]) particle(1,0.001,fEbindeK,0.,pi,0.,twopi,0,0);
           else{
            fThlev=3.63e-12;
            beta1f(fEndPoint[0],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
           }
           fThlev=3.63e-12;
           nucltransK(fEnGamma[0],fEbindeK,6.8e-6,2.1e-5);
        }
        else{
           beta2f(fEndPoint[1],0.,0.,2,fShCorrFactor[4],fShCorrFactor[5],fShCorrFactor[6],fShCorrFactor[7]);
        }
     }
     else if (fIsotope=="P32"){
        // ToI'1998 and ENSDF'2004.
        // experimental corrections to the allowed beta shape from
        // H.Daniel, RMP 40(1968)659 and M.J.Canty et al., NP 85(1966)317
        // VIT, 5.11.2006.
        beta1f(fEndPoint[0],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
     }
     else if (fIsotope=="Pa234m") Pa234m();
     else if (fIsotope=="Pb210"){
        // NDS 99(2003)649 and ENSDF at the NNDC site on 6.08.2007.
        // VIT, 6.08.2007.
        pbeta=100.*GetRandom();  // beta decay to U234
        if (pbeta<=fProbBeta[0]){
           beta(fEndPoint[0],0.,0.);
           nucltransKLM(fEnGamma[0],fEbindeK,14.2,fEbindeL,3.36,fEbindeM,1.14,0.);
        }
        else beta(fEndPoint[1],0.,0.);
     }
     else if (fIsotope=="Pb211") Pb211();
     else if (fIsotope=="Pb212") Pb212();
     else if (fIsotope=="Pb214") Pb214();
     else if (fIsotope=="Po212"){
        particle(47,fEnAlpha[0],fEnAlpha[0],0.,pi,0.,twopi,0,0);
     }
     else if (fIsotope=="Po214") Po214();
     else if (fIsotope=="Ra228") Ra228();
     else if (fIsotope=="Rb87"){
        // NDS 95(2002)543 and ENSDF at NNDC site on 6.08.2007.
        // VIT, 6.08.2007.
        beta2f(fEndPoint[0],0.,0.,2,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
     }
     else if (fIsotope=="Rh106") Rh106();
     else if (fIsotope=="Sb125") Sb125();
     else if (fIsotope=="Sb126") Sb126();
     else if (fIsotope=="Sb133") Sb133();
     else if (fIsotope=="Sr90"){
        // "Table of Isotopes", 7th ed., 1978, update to NDS 82(1997)379.
        // VIT, 9.08.1993, 22.10.1995, 26.10.2006.
        // Change from the allowed shape to the 1st forbidden unique
        // with empirical correction from:
        // H.H.Hansen, Appl. Rad. Isot. 34(1983)1241
        beta1fu(fEndPoint[0],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
     }
     else if (fIsotope=="Ta182")  Ta182();
     else if (fIsotope=="Te133")  Te133();
     else if (fIsotope=="Te133m") Te133m();
     else if (fIsotope=="Te134")  Te134();
     else if (fIsotope=="Th234")  Th234();
     else if (fIsotope=="Tl207"){
        // "Table of Isotopes", 7th ed., 1978).
        // VIT, 14.08.1992, 22.10.1995;
        // VIT, 30.10.2006 (update to NDS 70(1993)315 and correction
        // to the beta shape).
        pbeta=100.*GetRandom();
        if (pbeta<= fProbBeta[0]){
           beta(fEndPoint[0],0.,0.);
           fThlev=0.115e-12;
           p=100*GetRandom();
           if (p<= 99.29){
              nucltransK(fEnGamma[0],fEbindeK,2.5e-2,0.);
           }
           else{
              nucltransK(fEnGamma[1],fEbindeK,3.5e-1,0.);
              fThlev=130.5e-12;
              nucltransK(fEnGamma[2],fEbindeK,2.2e-2,0.);
           }
        }
        else{
           // change to forbidden spectrum with experimental correction
           // from J.M.Trischuk et al., NPA 90(1967)33 and H.Daniel,
           // RMP 40(1968)659
           beta1f(fEndPoint[1],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
        }
     }
     else if (fIsotope=="Tl208") Tl208();
     else if (fIsotope=="Xe129m"){
        // NDS 77(1996)631 and ENSDF at NNDC site on 13.11.2007).
        // VIT, 13.11.2007.
        nucltransKLM(fEnGamma[0],fEbindeK,13.94,fEbindeL,5.34,fEbindeM,1.52,0.);
        fThlev=0.97e-9;
        nucltransKLM(fEnGamma[1],fEbindeK,10.49,fEbindeL,1.43,fEbindeM,0.39,0.);
     }
     else if (fIsotope=="Xe131m"){
        // NDS 107(2006)2715 and ENSDF at NNDC site on 13.11.2007).
        // VIT, 13.11.2007.
        nucltransKLM(fEnGamma[0],fEbindeK,31.60,fEbindeL,14.75,fEbindeM,4.15,0.);
     }
     else if (fIsotope=="Xe133") Xe133();
     else if (fIsotope=="Xe135") Xe135();
     else if (fIsotope=="Y88")   Y88();
     else if (fIsotope=="Y90"){
        // "Table of Isotopes",7th ed., 1978).
        // Accuracy in description of: decay branches - 0.001%,
        //                     : deexcitation process - 0.001%.
        pbeta=100.*GetRandom();
        if (pbeta<=fProbBeta[0]){
           beta1fu(fEndPoint[0],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
           fThlev=61.3e-9;
           p=100.*GetRandom();
           if (p<=27.7) {
              pair(0.739);
           }
           else{
              particle(3,1.743,1.743,0.,pi,0.,twopi,fTclev,fThlev);//electron
              particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);//gamma
           }
        }
        else  beta1fu(fEndPoint[1],0.,0.,fShCorrFactor[4],fShCorrFactor[5],fShCorrFactor[6],fShCorrFactor[7]);
     }
     else if (fIsotope=="Zn65") Zn65();
     else if (fIsotope=="Zr96") {
        // It is supposed that decay goes to the first excited level (5+) // of Nb96 (E_exc=44 keV) with T1/2=2.4e20 y in accordance with:
        // H.Heiskanen et al.,JPG 34(2007)837.
        // Transition to the ground state (6+) is suppressed by factor
        // of ~1e-9, to the excited 4+ (E_exc=146 keV) - by 1e-2.
        // VIT, 07.05.1998.
        // VIT, 18.08.2007:update to NDS 68(1993)165 and ENSDF at NNDC site.
        fZdtr=fDaughterZ[0];
        beta(fEndPoint[0],0.,0.);
        nucltransK(fEnGamma[0],fEbindeK,2.4e0,0.);
        int fNbPart0=fNbPart;
        Nb96();
        fPtime[fNbPart0+1]=fPtime[fNbPart0+1]+fTdnuc;
     }
     else{
         Log::Die("VertexGen_Decay0: Unknown isotope " +fIsotope );
     }
}
///************************************************/
void Decay0::GenBBDeex()
{
  // GenBBDeex  generates the events of various modes of double beta decay.
  // Energy release in double beta decay - in accordance with
  // G.Audi and A.H.Wapstra, Nucl. Phys. A 595(1995)409.

  fNbPart=0;
  fCurParentIdx = 0;
  fPparent.clear();
  fTevst=0.;
  bb();

  fTclev=0.;
  if (fIsotope=="Ca48"  && fLevelE[fLevel]!=0.)     Ti48low();
  else if (fIsotope=="Ni58" && fLevelE[fLevel]!=0.) Fe58low();
  else if (fIsotope=="Ge76" && fLevelE[fLevel]!=0.) Se76low();
  else if (fIsotope=="Se74" && fLevelE[fLevel]!=0.) Ge74low();
  else if (fIsotope=="Se82" && fLevelE[fLevel]!=0.) Kr82low();
  else if (fIsotope=="Sr84" && fLevelE[fLevel]!=0.) {
    fThlev=4.35e-12;
    nucltransK(fEnGamma[0],fEbindeK,6.8e-4,0.);
  }
  else if (fIsotope=="Zr94" && fLevelE[fLevel]!=0.) {
    fThlev=2.9e-12;
    nucltransK(fEnGamma[0],fEbindeK,9.5e-4,0.);
  }
  else if (fIsotope=="Zr96"  && fLevelE[fLevel]!=0.) Mo96low();
  else if (fIsotope=="Mo92"  && fLevelE[fLevel]!=0.) Zr92low();
  else if (fIsotope=="Mo100" && fLevelE[fLevel]!=0.) Ru100low();
  else if (fIsotope=="Ru96"  && fLevelE[fLevel]!=0.) Mo96low();
  else if (fIsotope=="Ru104" && fLevelE[fLevel]!=0.) {
    fThlev=9.9e-12;
    nucltransK(fEnGamma[0],fEbindeK,4.5e-3,0.);
  }
  else if (fIsotope=="Cd106" && fLevelE[fLevel]!=0.) Pd106low();
  else if (fIsotope=="Cd116" && fLevelE[fLevel]!=0.) Sn116low();
  else if (fIsotope=="Sn112" && fLevelE[fLevel]!=0.) Cd112low();
  else if (fIsotope=="Sn124" && fLevelE[fLevel]!=0.) Te124low();
  else if (fIsotope=="Te120" && fLevelE[fLevel]==1171) {
    fThlev=0.642e-12;
    nucltransK(fEnGamma[0],fEbindeK,8.0e-4,0.);
  }
  else if (fIsotope=="Te128" && fLevelE[fLevel]==443) {
    fThlev=23.8e-12;
    nucltransK(fEnGamma[0],fEbindeK,8.0e-3,0.);
  }
  else if (fIsotope=="Te130" && fLevelE[fLevel]!=0.) Xe130low();
  else if ((fIsotope=="Xe136" || fIsotope=="Ce136")  && fLevelE[fLevel]!=0.) Ba136low();
  else if (fIsotope=="Nd148" && fLevelE[fLevel]!=0.) Sm148low();
  else if (fIsotope=="Nd150" && fLevelE[fLevel]!=0.) Sm150low();
  else if (fIsotope=="W186"  && fLevelE[fLevel]==137) {
    fThlev=875.e-12;
    // KLM ratios in accordance with BrIcc calculation
    nucltransKLM(fEnGamma[0],fEbindeK,0.44,fEbindeL,0.64,fEbindeM,0.20,0.);
  }
  ///Simulation of doublebeta-like radioactive decays
  else if (fIsotope=="Bi214"){
    int npfull0=fNbPart;
    At214();
    fPtime[npfull0+1]=fPtime[npfull0+1]+fTdnuc;
  }
  else if (fIsotope=="Pb214"){
    int npfull0=fNbPart;
    Po214();
    fPtime[npfull0+1]=fPtime[npfull0+1]+fTdnuc;
  }
  else if (fIsotope=="Po218"){
    int npfull0=fNbPart;
    Rn218();
    fPtime[npfull0+1]=fPtime[npfull0+1]+fTdnuc;
    npfull0=fNbPart;
    Po214();
    fPtime[npfull0+1]=fPtime[npfull0+1]+fTdnuc;
  }
  else if (fIsotope=="Rn222"){
    int npfull0=fNbPart;
    Ra222();
    fPtime[npfull0+1]=fPtime[npfull0+1]+fTdnuc;
    npfull0=fNbPart;
    Rn218();
    fPtime[npfull0+1]=fPtime[npfull0+1]+fTdnuc;
    npfull0=fNbPart;
    Po214();
    fPtime[npfull0+1]=fPtime[npfull0+1]+fTdnuc;
  }
}
///************************************************/
void Decay0::bb()
{
  // function for sampling the energies and angles of electrons in various
  // modes of double beta decay without Primakoff-Rosen approximation.
  // fStartbb  - must be =0 for first call of bb for a given mode;

  float Edlevel=fLevelE[fLevel]/1000.;
  float phi1,phi2;

  if (fZdbb>=0) fE0=fQbb-Edlevel;
  if (fZdbb<0) fE0=fQbb-Edlevel-4.*GetMass(3);

  if (fModebb==10) fE0=fQbb-Edlevel-fEK-2.*GetMass(3);
  else if (fModebb==9) {
    fE0=fQbb-Edlevel-fEK-2.*GetMass(3);
    particle(2,fE0,fE0,0.,pi,0.,twopi,0.,0.);
    particle(1,fEK,fEK,0.,pi,0.,twopi,0.,0.);
    return;
  }
  else if (fModebb==11) {
    fE0=fQbb-Edlevel-2.*fEK;
    particle(1,fE0,fE0,0.,pi,0.,twopi,0.,0.);
    particle(1,fEK,fEK,0.,pi,0.,twopi,0.,0.);
    particle(1,fEK,fEK,0.,pi,0.,twopi,0.,0.);
    return;
  }
  else if (fModebb==12){
    fE0=fQbb-Edlevel-2.*fEK;
    particle(1,fEK,fEK,0.,pi,0.,twopi,0.,0.);
    particle(1,fEK,fEK,0.,pi,0.,twopi,0.,0.);
    return;
  }
  if (fStartbb==0){
     //calculate the theoretical energy spectrum of first particle
     //with step of 1 keV and find its maximum
     if (fEbb2>fE0) fEbb2=fE0;
     if (fEbb1<0 || fEbb1>fEbb2)   fEbb1=0;

     int maxi=(int)(fE0*1000.);
     for (int i=1;i<=maxi+1;++i){
         fE1=i/1000.;
         fSpthe1[i]=0.;
         if ((fModebb>=1 && fModebb<=3) ||fModebb==7 || fModebb==10)
            fSpthe1[i]=fe1_mod();

         float elow=max(1e-4,fEbb1-fE1+1e-4);
         float ehigh=max(1e-4,fEbb2-fE1+1e-4);

         if (((fModebb>=4 && fModebb<=6) ||fModebb==8 || (fModebb>=13 && fModebb<=16))&& fE1<fE0) {
            TF1  f1 ("f1",this,0,fE0,4,"Decay0");
            ROOT::Math::WrappedTF1 wf1(f1);
            ROOT::Math::GaussLegendreIntegrator ig;
            ig.SetFunction(wf1);
            ig.SetRelTolerance(0.001);
            ig.SetNumberPoints(40);
            fSpthe1[i]=  ig.Integral(elow,ehigh);
         }
         if(fSpthe1[i]>fSpmax) fSpmax=fSpthe1[i];
     }
     return;
  }
  //printf("starting the generation\n");
  float re2s, re2f;
  int k,ke2s,ke2f;
  do{
     if (fModebb==10) fE1=fEbb1+(fEbb2-fEbb1)*GetRandom();
     else if (fModebb!=10) fE1=fEbb2*GetRandom();
     k = (int)(round(fE1*1000.));
     if (k<1) k=1;
  } while ( (fSpmax*GetRandom()) > fSpthe1[k]);

  if ((fModebb>=1 && fModebb<=3) || fModebb==7) fE2=fE0-fE1;
  else if ((fModebb>=4 && fModebb<=6) ||fModebb==8 || (fModebb>=13 && fModebb<=16)){
     re2s=max(float(0.0),fEbb1-fE1);
     re2f=fEbb2-fE1;
     float f2max=-1.;
     ke2s=(int)(max(1.,re2s*1000.));
     ke2f=(int)(round(re2f*1000.));

     for (int ke2=ke2s; ke2<=ke2f; ke2++){
         fE2=ke2/1000.;
         fSpthe2[ke2]=fe2_mod();
         if (fSpthe2[ke2]>f2max) f2max=fSpthe2[ke2];
     }

     do{
       fE2=re2s+(re2f-re2s)*GetRandom();
       if ((fModebb>=4&& fModebb<=6) || fModebb==8 || (fModebb>=13 && fModebb<=16) )
          fFe2m=fe2_mod();
     }while (f2max*GetRandom()>fFe2m);
  }
  else if( fModebb==10){
    //energy of X-ray is fixed; no angular correlation
    particle(2,fE1,fE1,0.,pi,0.,twopi,0.,0.);
    particle(1,fEK,fEK,0.,pi,0.,twopi,0.,0.);
    return;
  }
  float p1=sqrt(fE1*(fE1+2.*GetMass(3)));
  float p2=sqrt(fE2*(fE2+2.*GetMass(3)));
  float b1=p1/(fE1+GetMass(3));
  float b2=p2/(fE2+GetMass(3));

  float a,b,c;
  a=1.;
  b=-b1*b2;
  c=0.;
  float w1, w2;
  if (fModebb==2) b=b1*b2;
  else if (fModebb==3) {
     w1=fE1+GetMass(3);
     w2=fE2+GetMass(3);
     a=3.*(w1*w2+GetMass(3)*GetMass(3))*(p1*p1+p2*p2);
     b=-p1*p2*(pow(w1+w2,2)+4.*(w1*w2+GetMass(3)*GetMass(3)));
     c=2.*p1*p1*p2*p2;
  }
  else if (fModebb==7) {
     w1=fE1+GetMass(3);
     w2=fE2+GetMass(3);
     a=5.*(w1*w2+GetMass(3)*GetMass(3))*(p1*p1+p2*p2)-p1*p1*p2*p2;
     b=-p1*p2*(10.*(w1*w2+GetMass(3)*GetMass(3))+p1*p1+p2*p2);
     c=3.*p1*p1*p2*p2;
  }
  else if (fModebb==8|| fModebb==16) b=b1*b2/3.;
  else if (fModebb==15) {
     a=9.*pow(fE0-fE1-fE2,2)+21.*pow(fE2-fE1,2);
     b=-b1*b2*(9.*pow(fE0-fE1-fE2,2)-7.*pow(fE2-fE1,2));
  }
  float ctet,stet1,stet2,ctet1,ctet2;
  float romaxt=a+abs(b)+c;

  do{
     phi1=twopi*GetRandom();
     ctet1=1.-2.*GetRandom();
     stet1=sqrt(1.-ctet1*ctet1);
     phi2=twopi*GetRandom();
     ctet2=1.-2.*GetRandom();
     stet2=sqrt(1.-ctet2*ctet2);
     ctet=ctet1*ctet2+stet1*stet2*cos(phi1-phi2);
  } while(romaxt*GetRandom()> a+b*ctet+c*ctet*ctet);

  fNbPart=fNbPart+1;
  fPparent.push_back(fCurParentIdx);
  if(fZdbb>=0) fNpGeant[fNbPart]=3;
  if(fZdbb<0) fNpGeant[fNbPart]=2;
  fPmoment[0][fNbPart]=p1*stet1*cos(phi1);
  fPmoment[1][fNbPart]=p1*stet1*sin(phi1);
  fPmoment[2][fNbPart]=p1*ctet1;
  fPtime[fNbPart]=0.;

  fNbPart=fNbPart+1;
  fPparent.push_back(fCurParentIdx);
  if(fZdbb>=0) fNpGeant[fNbPart]=3;
  if(fZdbb<0) fNpGeant[fNbPart]=2;
  fPmoment[0][fNbPart]=p2*stet2*cos(phi2);
  fPmoment[1][fNbPart]=p2*stet2*sin(phi2);
  fPmoment[2][fNbPart]=p2*ctet2;
  fPtime[fNbPart]=0.;
  return;
}
///-----------------------------------------------
///----functions describe the deexcitation process after 2b-decay
///-----------------------------------------------
void Decay0::Ti48low()
{
  // Describes the deexcitation process in Ti48 nucleus after
  // 2b-decay of Ca48 to ground and excited 0+ and 2+ levels of Ti48
  // ("Table of Isotopes", 7th ed., 1978).
  // levelE:[0,984,2421]
  float p;
  bool next=false;

  if (fLevel==2){ //2421.
    fThlev=24.e-15;
    p=100.*GetRandom();
    if (p<8) nucltransK(fEnGamma[0],fEbindeK,1.5e-5,5.0e-4);
    else {
      nucltransK(fEnGamma[1],fEbindeK,3.1e-5,1.8e-4);
      next=true;
    }
  }
  if (fLevel==1 || next){ //984.
    fThlev=4.3e-12;
    nucltransK(fEnGamma[2],fEbindeK,1.2e-4,0.);
    return;
  }
  return;
}
///-----------------------------------------------
void Decay0::Fe58low()
{
  // Describes the deexcitation process in Fe58 nucleus after
  // 2b-decay of Ni58 to ground and excited 0+ and 2+ levels of Fe58
  // ("Table of Isotopes", 7th ed., 1978).
  // levelE:[0,811,1675],
  float p;
  bool next=false;

  if (fLevel==2){// 1675
     p=100.*GetRandom();
     if (p<43.) nucltransK(fEnGamma[0],fEbindeK,1.0e-4,0.6e-4);
     else{
        nucltransK(fEnGamma[1],fEbindeK,3.0e-4,0.);
        next=true;
     }
  }
  if (fLevel==1 || next){// 811.
     fThlev=9.0e-12;
     nucltransK(fEnGamma[2],fEbindeK,5.0e-4,0.);
     return;
  }
  return;
}
///-----------------------------------------------
void Decay0::Se76low()
{
  // Describes the deexcitation process in Se76 nucleus after
  // 2b-decay of Ge76 to ground and excited 0+ and 2+ levels of Se76
  // levelE:[0,559,1122,1216],
  int npg563=0, npg559=0;
  float p;
  float cg=1.;
  float cK=2.0e-3;
  bool next=false;

  if (fLevel==3){ //1216
     fThlev=3.4e-12;
     p=100.*GetRandom();
     if (p<36.) {
        nucltransK(fEnGamma[0],fEbindeK,4.3e-4,0.1e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,2.1e-3,0.);
        next=true;
     }
  }
  if (fLevel==2){ //1122
     fThlev=11.e-12;
     fEgamma=fEnGamma[2];
     p=(cg+cK)*GetRandom();
     if (p<=cg){
        particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        npg563=fNbPart;
     }
     else{
       particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
     }
     next=true;
  }
  if (fLevel==1 || next){ //559
     fThlev=12.3e-12;
     fEgamma=fEnGamma[3];
     p=(cg+cK)*GetRandom();
     if (p<cg){
        particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        npg559=fNbPart;
     }
     else{
        particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
     }
     // Angular correlation between gammas 559 and 563 keV, L.Pandola + VIT
     if (npg559!=0 && npg563!=0){
        float p559=sqrt(pow(fPmoment[0][npg559],2)+pow(fPmoment[1][npg559],2)+
                         pow(fPmoment[2][npg559],2));
        float p563=sqrt(pow(fPmoment[0][npg563],2)+pow(fPmoment[1][npg563],2)+
                         pow(fPmoment[2][npg563],2));
        // Coefficients in formula 1+a2*ctet**2+a4*ctet**4 are from: R.D.Evans,
        //"The Atomic Nucleus", Krieger Publ. Comp., 1985, p. 240, 0(2)2(2)0 cascade.
        float ctet,stet1,stet2,ctet1,ctet2;
        float phi1,phi2;
        float a2=-3.0;
        float a4=4.0;
        do{
           phi1=twopi*GetRandom();
           ctet1=1.-2.*GetRandom();
           stet1=sqrt(1.-ctet1*ctet1);
           phi2=twopi*GetRandom();
           ctet2=1.-2.*GetRandom();
           stet2=sqrt(1.-ctet2*ctet2);
           ctet=ctet1*ctet2+stet1*stet2*cos(phi1-phi2);
         }while(GetRandom()*(1.+abs(a2)+abs(a4)) > 1.+a2*ctet*ctet+a4*pow(ctet,4));

         fPmoment[0][npg559]=p559*stet1*cos(phi1);
         fPmoment[1][npg559]=p559*stet1*sin(phi1);
         fPmoment[2][npg559]=p559*ctet1;
         fPmoment[0][npg563]=p563*stet2*cos(phi2);
         fPmoment[1][npg563]=p563*stet2*sin(phi2);
         fPmoment[2][npg563]=p563*ctet2;
         return;
      }
  }
}
///-----------------------------------------------
void Decay0::Ge74low()
{
  // Subroutine describes the deexcitation process in Ge74 nucleus after
  // 2b-decay of Se74 to ground and excited 0+ and 2+ levels of Ge74
  // levelE:[0,596,1204],
  bool next=false;

  if (fLevel==2){ //1204
     fThlev=6.0e-12;
     float p=100.*GetRandom();
     if (p<34){
        nucltransK(fEnGamma[0],fEbindeK,1.9e-4,0.1e-4);
        return;
     }
     else {
        nucltransK(fEnGamma[1],fEbindeK,1.1e-3,0.);
        next=true;
     }
  }
  if (fLevel==1 || next){ //596
     fThlev=12.0e-12;
     nucltransK(fEnGamma[2],fEbindeK,1.1e-3,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Kr82low()
{
  // Subroutine describes the deexcitation process in Kr82 nucleus after
  // 2b-decay of Se82 to ground and excited 0+ and 2+ levels of Kr82
  // levelE:[0,776,1475],
  bool next=false;
  float p;

  if (fLevel==2){ //1475
     p=100.*GetRandom();
     if (p<=36.7){
        nucltransK(fEnGamma[0],fEbindeK,2.0e-4,0.5e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,1.3e-3,0.);
        next=true;
    }
  }
  if (fLevel==1 || next){ //776
     fThlev=5.e-12;
     nucltransK(fEnGamma[2],fEbindeK,9.3e-4,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Mo96low()
{
  // Subroutine describes the deexcitation process in Mo96 nucleus after
  // 2b- decay of Zr96 and 2b+/eb+/2e decay of Ru96 to ground and
  // excited 0+ and 2+ levels of Mo96 ("Table of Isotopes", 7th ed., 1978).
  // VIT, 20.05.2009: four levels (2096, 2426, 2623 and 2700 keV) are added
  // and information is updated in accordance with NNDC site on 19.05.2009
  // and  NDS 109(2008)2501.
  // levelE:[0,778,1148,1498,1626,2096,2426,2623,2700,2713],
  bool next778=false, next1148=false, next1498=false, next1626=false;
  float p;

  if (fLevel==9){ //2713
     nucltransK(fEnGamma[0],fEbindeK,5.9e-1,0.);
     fThlev=0.208e-12;
     nucltransK(fEnGamma[1],fEbindeK,1.3e-3,0.);
     fThlev=1.2e-12;
     nucltransK(fEnGamma[2],fEbindeK,1.1e-3,0.);
     next778=true;
  }
  if (fLevel==8){ //2700
     fThlev=0.103e-12;
     p=100.*GetRandom();
     if (p<=3.04){
        nucltransK(fEnGamma[3],fEbindeK,6.9e-2,0.);
        fThlev=0.069e-12;
        p=100.*GetRandom();
        if (p<=8.17){
          nucltransK(fEnGamma[4],fEbindeK,9.7e-4,0.);
           next1626=true;
        }
        else if (p<=28.31){
          nucltransK(fEnGamma[5],fEbindeK,7.3e-4,0.);
           next1498=true;
        }
        else{
          nucltransK(fEnGamma[6],fEbindeK,2.5e-4,1.7e-4);
           next778=true;
        }
     }
     else if (p<=12.12){
        nucltransK(fEnGamma[7],fEbindeK,6.7e-4,0.);
        next1626=true;
     }
     else if (p<=53.40){
        nucltransK(fEnGamma[8],fEbindeK,5.5e-4,0.);
        next1498=true;
     }
     else if (p<=86.67){
        nucltransK(fEnGamma[9],fEbindeK,2.1e-4,2.5e-4);
        next778=true;
     }
     else{
        nucltransK(fEnGamma[10],fEbindeK,1.1e-4,6.4e-4);
        return;
     }
  }
  if (fLevel==7){ //2623
     fThlev=0.6e-12;
     nucltransK(fEnGamma[11],fEbindeK,2.2e-4,2.3e-4);
     next778=true;
  }
  if (fLevel==6){ //2426
     fThlev=0.19e-12;
     p=100.*GetRandom();
     if (p<=2.50){
        nucltransK(fEnGamma[12],fEbindeK,6.4e-3,0.);
        fThlev=2.29e-12;
        p=100.*GetRandom();
        if (p<=0.18){
           nucltransK(fEnGamma[13],fEbindeK,2.0e-1,0.);
           fThlev=6.4e-12;
           p=100.*GetRandom();
           if (p<=7.52){
              nucltransK(fEnGamma[14],fEbindeK,2.4e-2,0.);
              fThlev=1.2e-12;
              nucltransK(fEnGamma[2],fEbindeK,1.1e-3,0.);
              next778=true;
           }
           else if (p<=12.22){
              nucltransK(fEnGamma[15],fEbindeK,1.2e-2,0.);
              next1498=true;
           }
        }
        else if (p<=5.96){
           nucltransK(fEnGamma[16],fEbindeK,1.2e-2,0.);
           fThlev=1.2e-12;
           nucltransK(fEnGamma[2],fEbindeK,1.1e-3,0.);
           next778=true;
        }
        else if (p<=9.27){
           nucltransK(fEnGamma[17],fEbindeK,1.2e-2,0.);
           next1626=true;
        }
        else if (p<=30.34){
           nucltransK(fEnGamma[18],fEbindeK,4.3e-3,0.);
           next1498=true;
        }
        else{
           nucltransK(fEnGamma[19],fEbindeK,5.4e-4,6.1e-6);
           next778=true;
        }
     }
     else if (p<=38.16){
        nucltransK(fEnGamma[20],fEbindeK,1.3e-3,0.);
        next1626=true;
     }
     else if (p<=42.07){
        nucltransK(fEnGamma[21],fEbindeK,9.2e-4,0.);
        next1498=true;
     }
     else if (p<=95.22){
        nucltransK(fEnGamma[22],fEbindeK,2.9e-4,1.2e-4);
        next778=true;
     }
     else{
        nucltransK(fEnGamma[23],fEbindeK,1.4e-4,5.1e-4);
        return;
     }
  }
  if (fLevel==5){ //2096
     fThlev=0.097e-12;
     p=100.*GetRandom();
     if (p<=3.06){
        nucltransK(fEnGamma[24],fEbindeK,8.7e-4,0.);
        next1148=true;
     }
     else if (p<=98.55){
        nucltransK(fEnGamma[25],fEbindeK,4.4e-4,2.5e-5);
        next778=true;
     }
     else{
        nucltransK(fEnGamma[26],fEbindeK,1.8e-4,3.5e-4);
        return;
     }
  }
  if (fLevel==4 || next1626){ //1626
     fThlev=1.4e-12;
     p=100.*GetRandom();
     if (p<=8.47){
        nucltransK(fEnGamma[27],fEbindeK,2.8e-4,1.3e-4);
        return;
     }
     else if (p<=98.58){
        nucltransK(fEnGamma[28],fEbindeK,1.2e-3,0.);
        next778=true;
     }
     else{
        nucltransK(fEnGamma[29],fEbindeK,1.3e-1,0.);
        next1498=true;
     }
  }
  if (fLevel==3 || next1498){ //1498
     fThlev=0.78e-12;
     p=100.*GetRandom();
     if (p<=29.73){
        nucltransK(fEnGamma[30],fEbindeK,3.3e-4,8.3e-5);
        return;
     }
     else{
        nucltransK(fEnGamma[31],fEbindeK,1.7e-3,0.);
        next778=true;
     }
  }
  if (fLevel==2 || next1148){ //1148
     fThlev=61.e-12;
     nucltransK(fEnGamma[32],fEbindeK,1.2e-2,0.);
     next778=true;
  }
  if (fLevel==1 || next778){ //778
     fThlev=3.67e-12;
     nucltransK(fEnGamma[33],fEbindeK,1.4e-3,0.);
     return;
  }
  return;
}
///-----------------------------------------------
void Decay0::Zr92low()
{
  // Subroutine describes the deexcitation process in Zr92 nucleus after
  // 2b-decay of Mo92 to ground and excited 0+ and 2+ levels of Zr92
  // levelE:[0,934,1383],
  bool next=false;

  if (fLevel==2){ //1383
     fThlev=0.17e-9;
     nucltransK(fEnGamma[0],fEbindeK,5.5e-3,0.);
     next=true;
  }
  if (fLevel==1 || next){ //934
     fThlev=5.0e-12;
     nucltransK(fEnGamma[1],fEbindeK,8.0e-4,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Ru100low()
{
  // Subroutine describes the deexcitation process in Ru100 nucleus after
  // 2b-decay of Mo100 to ground and excited 0+ and 2+ levels of Ru100
  // levelE:[0,540,1130,1362,1741],
  int  npg591=0, npg540=0;
  float cg,cK,p;
  bool next540=false, next1130=false, next1362=false;
  float ctet,stet1,stet2,ctet1,ctet2;

  if (fLevel==4){ //1741
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=0.05){
        particle(3,fEnGamma[0]-fEbindeK,fEnGamma[0]-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
        return;
     }
     else if(p<=59.00){
        nucltransK(fEnGamma[1],fEbindeK,6.2e-4,0.1e-4);
        next540=true;
     }
     else if(p<=59.03){
       particle(3,fEnGamma[2]-fEbindeK,fEnGamma[2]-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
       particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
        next1130=true;
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,1.3e-2,0.);
        next1362=true;
     }
  }
  if (fLevel==3 || next1362){ //1362
     fThlev=1.2e-12;
     p=100.*GetRandom();
     if (p<=43.){
        nucltransK(fEnGamma[4],fEbindeK,4.2e-4,0.2e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,1.7e-3,0.);
        next540=true;
     }
  }
  if (fLevel==2 || next1130){ //1130
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=0.02){
       particle(3,1.130-fEbindeK,1.130-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
       particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
        return;
     }
     else{
        //nucltransK(0.591,fEbindeK,3.3e-3,0.);
        fEgamma=fEnGamma[6];
        cg=1.;
        cK=3.3e-3;
        p=GetRandom()*(cg+cK);
        if (p<=cg){
           particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
           npg591=fNbPart;
        }
        else{
           particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
        }
     }
     next540=true;
  }
  if (fLevel==1 || next540){ //540
     //nucltransK(0.540,fEbindeK,4.4e-3,0.);
     fThlev=11.e-12;
     fEgamma=fEnGamma[7];
     cg=1.;
     cK=4.4e-3;
     p=GetRandom()*(cg+cK);
     if (p<=cg){
        particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        npg540=fNbPart;
     }
     else{
        particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
     }
     if (npg591!=0 && npg540!=0){
        float p591=sqrt(pow(fPmoment[0][npg591],2)+pow(fPmoment[1][npg591],2)+
                         pow(fPmoment[2][npg591],2));
        float p540=sqrt(pow(fPmoment[0][npg540],2)+pow(fPmoment[1][npg540],2)+
                         pow(fPmoment[2][npg540],2));
        // Coefficients in formula 1+a2*ctet**2+a4*ctet**4 are from:
        // R.D.Evans, "The Atomic Nucleus", Krieger Publ. Comp.,
        // 1985, p.240, 0(2)2(2)0 cascade.
        float a2=-3.0;
        float a4=4.0;
        float phi1,phi2;
        do{
           phi1=twopi*GetRandom();
           ctet1=1.-2.*GetRandom();
           stet1=sqrt(1.-ctet1*ctet1);
           phi2=twopi*GetRandom();
           ctet2=1.-2.*GetRandom();
           stet2=sqrt(1.-ctet2*ctet2);
           ctet=ctet1*ctet2+stet1*stet2*cos(phi1-phi2);
        } while(GetRandom()*(1.+abs(a2)+abs(a4)) > 1.+a2*pow(ctet,2)+a4*pow(ctet,4));

        fPmoment[0][npg591]=p591*stet1*cos(phi1);
        fPmoment[1][npg591]=p591*stet1*sin(phi1);
        fPmoment[2][npg591]=p591*ctet1;
        fPmoment[0][npg540]=p540*stet2*cos(phi2);
        fPmoment[1][npg540]=p540*stet2*sin(phi2);
        fPmoment[2][npg540]=p540*ctet2;
        return;
     }
  }
}
///-----------------------------------------------
void Decay0::Pd106low()
{
  // Subroutine describes the deexcitation process in Pd106 nucleus after
  // 2b-decay of Cd106 to ground and excited 0+ and 2+ levels of Pd106
  // levelE:[0,512,1128,1134,1562,1706],
  bool next1128=false, next1134=false, next512=false;
  float p;

  if (fLevel==5){ //1706
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=87.2){
       nucltransK(fEnGamma[0],fEbindeK,6.9e-4,6.7e-6);
        next512=true;
     }
     else{
       nucltransK(fEnGamma[1],fEbindeK,4.0e-3,0.);
        next1128=true;
     }
  }
  if (fLevel==4){ //1562
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=9.0){
       nucltransK(fEnGamma[2],fEbindeK,3.9e-4,1.1e-4);
        return;
     }
     else if(p<=95.0){
       nucltransK(fEnGamma[3],fEbindeK,1.0e-3,0.);
        next512=true;
     }
     else if(p<=96.1){
       nucltransK(fEnGamma[4],fEbindeK,7.7e-3,0.);
        next1128=true;
     }
     else{
       nucltransK(fEnGamma[5],fEbindeK,9.5e-3,0.);
        next1134=true;
     }
  }
  if (fLevel==3 || next1134){ //1134
     fThlev=6.8e-12;
     p=100.*GetRandom();
     if (p<=5.7e-2){
        particle(3,1.110,1.110,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[6],fEbindeK,3.2e-3,0.);
        next512=true;
     }
  }
  if (fLevel==2 || next1128){ //1128
     fThlev=3.12e-12;
     p=100.*GetRandom();
     if (p<=35.0){
        nucltransK(fEnGamma[7],fEbindeK,7.7e-4,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[8],fEbindeK,3.4e-3,0.);
        next512=true;
     }
  }
  if (fLevel==1 || next512){ //512
     fThlev=12.1e-12;
     nucltransK(fEnGamma[9],fEbindeK,5.6e-3,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Sn116low()
{
  // Subroutine describes the deexcitation process in Sn116 nucleus after
  // 2b-decay of Cd116 to ground and excited 0+ and 2+ levels of Sn116
  // levelE:[0,1294,1757,2027,2112,2225],
  bool next1294=false, next1757=false;
  float p;
  fThlev=0.;

  if (fLevel==5){ //2225
     p=100.*GetRandom();
     if (p<=37.){
       nucltransK(fEnGamma[0],fEbindeK,2.7e-4,3.4e-4);
        return;
     }
     else{
       nucltransK(fEnGamma[1],fEbindeK,1.5e-3,0.);
        next1294=true;
     }
  }
  if (fLevel==4){ //2112
     p=100.*GetRandom();
     if (p<=54.9){
       nucltransK(fEnGamma[2],fEbindeK,3.1e-4,2.7e-4);
        return;
     }
     else if(p<=96.9){
       nucltransK(fEnGamma[3],fEbindeK,2.6e-3,0.);
        next1294=true;
     }
     else{
       nucltransK(fEnGamma[4],fEbindeK,1.8e-2,0.);
        next1757=true;
     }
  }
  if (fLevel==3){ //2027
     nucltransK(fEnGamma[5],fEbindeK,2.7e-3,0.);
     next1294=true;
  }
  if (fLevel==2 || next1757){ //1757
     p=100.*GetRandom();
     if (p<=0.29){
       particle(3,1.757-fEbindeK,1.757-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
       particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        return;
     }
     else{
       nucltransK(fEnGamma[6],fEbindeK,9.0e-3,0.);
        next1294=true;
     }
  }
  if (fLevel==1 || next1294){ //1294
     fThlev=0.36e-12;
     nucltransK(fEnGamma[7],fEbindeK,7.5e-4,0.5e-4);
     return;
  }
}
///-----------------------------------------------
void Decay0::Cd112low()
{
  // Subroutine describes the deexcitation process in Cd112 nucleus after
  // 2b-decay of Sn112 to ground 0+ and excited 2+ levels of Cd112
  // levelE:[0,618,1224,1312,1433,1469,1871],
  float p;
  bool next618=false, next1224=false, next1312=false, next1469=false;

  if (fLevel==6){ //1871
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=86.91){
        nucltransK(fEnGamma[0],fEbindeK,7.2e-4,1.5e-5);
        next618=true;
     }
     else if (p<=89.88){
        nucltransK(fEnGamma[1],fEbindeK,4.9e-3,0.);
        next1312=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,1.3e-2,0.);
        next1469=true;
     }
  }
  if (fLevel==5 || next1469){ //1469
     fThlev=2.7e-12;
     p=100.*GetRandom();
     if (p<=36.98){
        nucltransK(fEnGamma[3],fEbindeK,5.8e-4,7.1e-5);
        return;
     }
     else if(p<=99.14){
        nucltransK(fEnGamma[4],fEbindeK,1.8e-3,0.);
        next618=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,6.4e-2,0.);
        next1224=true;
     }
  }
  if (fLevel==4){ //1433
     fThlev=1.9e-9;
     p=100.*GetRandom();
     if (p<=0.66){
        p=100.*GetRandom();
        if (p<=3.8){
           pair(0.411);
        }
        else{
           particle(3,1.406,1.406,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        }
        return;
     }
     else if (p<=39.36){
        nucltransK(fEnGamma[6],fEbindeK,1.8e-3,0.);
        next618=true;
     }
     else if (p<=60.61){
        particle(3,0.182,0.182,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        next1224=true;
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,7.6e-1,0.);
        next1312=true;
     }
  }
  if (fLevel==3 || next1312){ //1312
     fThlev=2.0e-12;
     p=100.*GetRandom();
     if (p<=26.59){
       nucltransK(fEnGamma[8],fEbindeK,6.6e-4,2.6e-5);
        return;
     }
     else{
       nucltransK(fEnGamma[9],fEbindeK,2.8e-3,0.);
        next618=true;
     }
  }
  if (fLevel==2 || next1224){ //1224
     fThlev=4.2e-12;
     p=100.*GetRandom();
     if (p<=0.12){
        p=100.*GetRandom();
        if (p<=0.4){
           pair(0.202);
        }
        else{
           particle(3,1.197,1.197,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        }
        return;
     }
     else{
       nucltransK(fEnGamma[10],fEbindeK,3.9e-3,0.);
        next618=true;
     }
  }
  if (fLevel==1 || next618){ //618
     fThlev=6.51e-12;
     nucltransK(fEnGamma[11],fEbindeK,3.7e-3,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Te124low()
{
  // Subroutine describes the deexcitation process in Te124 nucleus after 2b-decay
  // of Sn124 to ground and excited 0+ and 2+ levels of Te124.
  // levelE:[0,603,1326,1657,1883,2039,2092,2153,2182],
  bool next603=false, next1657=false, next1326=false;
  float p;

  if (fLevel==8){ //2182
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=9.53){
        nucltransK(fEnGamma[0],fEbindeK,7.0e-4,3.9e-4);
        return;
     }
     else if (p<=92.38){
        nucltransK(fEnGamma[1],fEbindeK,7.7e-4,1.0e-4);
        next603=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,2.3e-3,0.);
        next1326=true;
     }
  }
  if (fLevel==7){ //2153
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=19.29){
        nucltransK(fEnGamma[3],fEbindeK,6.8e-4,1.0e-4);
        next603=true;
     }
     else{
        nucltransK(fEnGamma[4],fEbindeK,2.2e-3,0.);
        next1326=true;
     }
  }
  if (fLevel==6){ //2092
     fThlev=0.28e-12;
     p=100.*GetRandom();
     if (p<=97.97){
        nucltransK(fEnGamma[5],fEbindeK,8.3e-4,7.1e-5);
        next603=true;
     }
     else if (p<=98.24){
        nucltransK(fEnGamma[6],fEbindeK,2.1e-3,0.);
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,3.0e-3,0.);
        next1326=true;
     }
  }
  if (fLevel==5){ //2039
     fThlev=0.49e-12;
     p=100.*GetRandom();
     if (p<=34.26){
        nucltransK(fEnGamma[8],fEbindeK,6.7e-4,3.2e-4);
        return;
     }
     else if (p<=94.57){
        nucltransK(fEnGamma[9],fEbindeK,8.7e-4,5.5e-5);
        next603=true;
     }
     else if (p<=96.80){
        nucltransK(fEnGamma[10],fEbindeK,2.5e-3,0.);
     }
     else if (p<=99.04){
        nucltransK(fEnGamma[11],fEbindeK,4.0e-3,0.);
        next1326=true;
     }
     else{
        nucltransK(fEnGamma[12],fEbindeK,1.8e-2,0.);
        next1657=true;
     }
  }
  if (fLevel==4){ //1883
     fThlev=0.76e-12;
     p=100.*GetRandom();
     if (p<=0.31){
        p=100.*GetRandom();
        if (p<=21.89){
           pair(0.861);
        }
        else{
           particle(3,1.851,1.851,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        }
        return;
     }
     else if (p<=99.93){
        nucltransK(fEnGamma[13],fEbindeK,6.0e-3,0.);
        next1326=true;
     }
     else{
        particle(3,0.194,0.194,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        next1657=true;
     }
  }
  if (fLevel==3 || next1657){ //1657
     fThlev=0.55-12;
     p=100.*GetRandom();
     if (p<=0.02){
        p=100.*GetRandom();
        if (p<=10.68){
           pair(0.636);
        }
        else{
           particle(3,1.626,1.626,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        }
        return;
     }
     else{
       nucltransK(fEnGamma[14],fEbindeK,1.3e-3,0.);
        next603=true;
     }
  }
  if (fLevel==2 || next1326){ //1326
     fThlev=1.04e-12;
     p=100.*GetRandom();
     if (p<=13.84){
       nucltransK(fEnGamma[15],fEbindeK,8.3e-4,2.8e-5);
        return;
     }
     else{
       nucltransK(fEnGamma[16],fEbindeK,3.1e-3,0.);
        next603=true;
     }
  }
  if (fLevel==1 || next603){ //603
     fThlev=6.2e-12;
     nucltransK(fEnGamma[17],fEbindeK,4.9e-3,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Xe130low()
{
  // Subroutine describes the deexcitation process in Xe130 nucleus after 2b-decay
  // of Te130 to ground 0+ and excited 2+ levels of Xe130
  // levelE:[0,536,1122,1794],
  float p;
  bool next536=false, next1122=false;

  if (fLevel==3){ //1794
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=1.0){
        p=100.*GetRandom();
        if (p<=12.7){
           pair(0.772);
        }
        else{
           particle(3,1.759,1.759,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        }
        return;
     }
     else if (p<=86.3){
        nucltransK(fEnGamma[0],fEbindeK,1.0e-3,1.5e-5);
        next536=true;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,4.1e-3,0.);
        next1122=true;
     }
  }
  if (fLevel==2 || next1122){ //1122
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=13.3){
        nucltransK(fEnGamma[2],fEbindeK,1.3e-3,9.0e-7);
        return;
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,7.5e-3,0.);
        next536=true;
     }
  }
  if (fLevel==1 || next536){ //536
     fThlev=7.0e-12;
     nucltransK(fEnGamma[4],fEbindeK,7.4e-3,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Ba136low()
{
  // Deexcitation process in Ba136 nucleus after 2b-decay
  // of Xe136 or Ce136 to ground and excited 0+ and 2+ levels of Ba136
  // levelE:[0,819,1551,1579,2080,2129,2141,2223,2315,2400],
  bool next819=false, next1551=false;
  float p;

  if (fLevel==9){ //2400
     fThlev=0.;
     nucltransK(fEnGamma[0],fEbindeK,1.0e-3,1.1e-4);
     next819=true;
  }
  if (fLevel==8){ //2315
     fThlev=0.;
     nucltransK(fEnGamma[1],fEbindeK,8.7e-4,7.7e-5);
     next819=true;
  }
  if (fLevel==7){ //2223
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=4.3){
        nucltransK(fEnGamma[2],fEbindeK,7.8e-4,4.0e-4);
        return;
     }
     else if (p<=51.9){
        nucltransK(fEnGamma[3],fEbindeK,9.6e-4,4.8e-5);
        next819=true;
     }
     else{
        nucltransK(fEnGamma[4],fEbindeK,6.5e-3,0.);
        next1551=true;
     }
  }
  if (fLevel==6){ //2141
     fThlev=0.;
     nucltransK(fEnGamma[5],fEbindeK,1.0e-3,2.6e-5);
     next819=true;
  }
  if (fLevel==5){ //2129
     fThlev=0.051e-12;
     p=100.*GetRandom();
     if (p<=33.3){
        nucltransK(fEnGamma[6],fEbindeK,7.7e-4,3.6e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,1.4e-3,2.3e-5);
        next819=true;
     }
  }
  if (fLevel==4){ //2080
     fThlev=0.6e-12;
     p=100.*GetRandom();
     if (p<=35.4){
        nucltransK(fEnGamma[8],fEbindeK,7.6e-4,3.3e-4);
        return;
     }
     else if (p<=93.8){
        nucltransK(fEnGamma[9],fEbindeK,1.3e-3,1.4e-5);
        next819=true;
     }
     else{
        nucltransK(fEnGamma[10],fEbindeK,1.0e-2,0.);
        next1551=true;
     }
  }
  if (fLevel==3){ //1579
     fThlev=0.;
     nucltransK(fEnGamma[11],fEbindeK,3.4e-3,0.);
     next819=true;
  }
  if (fLevel==2 || next1551){ //1551
     fThlev=1.01e-12;
     p=100.*GetRandom();
     if (p<=52.1){
        nucltransK(fEnGamma[12],fEbindeK,8.4e-4,9.7e-5);
        return;
     }
     else{
        nucltransK(fEnGamma[13],fEbindeK,4.5e-3,0.);
        next819=true;
     }
  }
  if (fLevel==1 || next819){ //819
     fThlev=1.93e-12;
     nucltransK(fEnGamma[14],fEbindeK,2.9e-3,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Sm148low()
{
  // Deexcitation process in Sm148 nucleus after 2b-decay
  // of Nd148 to ground and excited 0+ and 2+ levels of Sm148
  // levelE:[0,550,1455],
  bool next550=false;
  float p;
  if (fLevel==2){ //1455
     fThlev=0.6e-12;
     p=100.*GetRandom();
     if (p<=42.){
        nucltransK(fEnGamma[0],fEbindeK,1.1e-3,0.3e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,2.8e-3,0.);
        next550=true;
     }
  }
  if (fLevel==1 || next550){ //550
     fThlev=7.3e-12;
     nucltransK(fEnGamma[2],fEbindeK,9.0e-3,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::Sm150low()
{
  // Deexcitation process in Sm150 nucleus after 2b-decay
  // of Nd150 to ground and excited 0+ and 2+ levels of Sm150
  // levelE:[0,334,740,1046,1194,1256],
  bool next334=false, next740=false, next1046=false;
  float p;

  if (fLevel==5){ //1256
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=93.){
        nucltransK(fEnGamma[0],fEbindeK,2.6e-3,0.);
        next334=true;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,1.7e-1,0.);
        next1046=true;
     }
  }
  if (fLevel==4){ //1194
     fThlev=1.3e-12;
     p=100.*GetRandom();
     if (p<=55.9){
        nucltransK(fEnGamma[2],fEbindeK,1.6e-3,0.1e-4);
        return;
     }
     else if (p<=96.9){
        nucltransK(fEnGamma[3],fEbindeK,3.2e-3,0.);
        next334=true;
     }
     else if (p<=98.7){
        nucltransK(fEnGamma[4],fEbindeK,1.5e-2,0.);
        next740=true;
    }
    else{
        nucltransK(fEnGamma[5],fEbindeK,1.9e-2,0.);
        fThlev=6.6e-12;
        nucltransK(fEnGamma[6],fEbindeK,1.7e-2,0.);
        next334=true;
    }
  }
  if (fLevel==3 || next1046){ //1046
     fThlev=0.7e-12;
     p=100.*GetRandom();
     if (p<=7.0){
       nucltransK(fEnGamma[7],fEbindeK,2.0e-3,0.);
        return;
     }
     else if (p<=94.3){
       nucltransK(fEnGamma[8],fEbindeK,7.6e-3,0.);
        next334=true;
     }
     else if (p<=97.0){
       nucltransK(fEnGamma[9],fEbindeK,4.9e-2,0.);
        next740=true;
     }
     else{
       nucltransK(fEnGamma[10],fEbindeK,7.0e-2,0.);
        fThlev=6.6e-12;
        nucltransK(fEnGamma[6],fEbindeK,1.7e-2,0.);
        next334=true;
     }
  }
  if (fLevel==2 || next740){ //740
     fThlev=20.e-12;
     p=100.*GetRandom();
     if (p<=1.33){
       particle(3,0.740-fEbindeK,0.740-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
       particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        return;
     }
     else{
       nucltransK(fEnGamma[11],fEbindeK,2.0e-2,0.);
        next334=true;
     }
  }
  if (fLevel==1 || next334){ //334
     fThlev=48.5e-12;
     nucltransK(fEnGamma[12],fEbindeK,3.7e-2,0.);
     return;
  }
}
///-----------------------------------------------
void Decay0::At214()
{
  // NDS 99(2003)649 and ENSDF at NNDC site on 9.12.2007).
  // Simulated together with decay of Bi214
  fThnuc=558.e-9;
  float tcnuc=0.;
  fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
  fTclev=0.;
  fThlev=0.;
  float palpha=100.*GetRandom();
  if (palpha<=0.32){
     particle(47,8.267,8.267,0.,pi,0.,twopi,0,0);
     nucltransK(0.563,0.091,8.6e-2,0.);
     return;
  }
  else if (palpha<=0.90){
     particle(47,8.478,8.478,0.,pi,0.,twopi,0,0);
     nucltransK(0.348,0.091,7.9e-2,0.);
     return;
  }
  else if (palpha<=1.05){
     particle(47,8.505,8.505,0.,pi,0.,twopi,0,0);
     nucltransK(0.320,0.091,3.9e-1,0.);
     return;
  }
  else{
     particle(47,8.819,8.819,0.,pi,0.,twopi,0,0);
     return;
  }
}
///-----------------------------------------------
void Decay0::Po214()
{
  // Nucl. Data Sheets 55(1988)665).
  // Alpha decay to excited level 1097.7 keV of Pb210 is neglected (6e-5%).
  float tcnuc=0.;
  fThnuc=164.3e-6;
  fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
  float palpha=100.*GetRandom();
  if (palpha<=0.0104){
     particle(47,6.902,6.902,0.,pi,0.,twopi,0,0);
     nucltransK(0.800,0.088,1.1e-2,0.);
     return;
  }
  else{
     particle(47,7.687,7.687,0.,pi,0.,twopi,0,0);
     return;
  }
}
///-----------------------------------------------
void Decay0::Rn218()
{
  // NDS 76(1995)127 and ENSDF at NNDC site on 9.12.2007.
  float tcnuc=0.;
  fThnuc=35.e-3;
  fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
  fTclev=0.;
  float palpha=100.*GetRandom();
  if (palpha<=0.127){
    particle(47,6.532,6.532,0.,pi,0.,twopi,0,0);
    fThlev=0.;
    nucltransK(0.609,0.093,2.1e-2,0.);
    return;
  }
  else{
    particle(47,7.130,7.130,0.,pi,0.,twopi,0,0);
    return;
  }
}
///-----------------------------------------------
void Decay0::Ra222()
{
  // NDS 107(2006)1027 and ENSDF at NNDC site on 9.12.2007
  float tcnuc=0.;
  fThnuc=36.17;
  fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
  fTclev=0.;
  fThlev=0.;
  bool next653=false, next324=false;
  float palpha, p;
  palpha =100.*GetRandom();
  if (palpha<=0.0042){
     particle(47,5.734,5.734,0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=66.35){
        nucltransK(0.840,0.098,2.9e-2,0.);
        return;
     }
     else{
        nucltransK(0.516,0.098,2.5e-2,0.);
        next324=true;
     }
  }
  else if (palpha<=0.0083){
     particle(47,5.776,5.776,0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=96.75){
        nucltransK(0.473,0.098,1.0e-2,0.);
        next324=true;
     }
     else{
        nucltransK(0.144,0.098,1.9e-1,0.);
        next653=true;
     }
  }
  else if (palpha<=0.0124){
     particle(47,5.917,5.917,0.,pi,0.,twopi,0,0);
     next653=true;
  }
  else if (palpha<=3.0635){
     particle(47,6.240,6.240,0.,pi,0.,twopi,0,0);
     next324=true;
  }
  else{
     particle(47,6.559,6.559,0.,pi,0.,twopi,0,0);
     return;
  }

  if (next653){
     nucltransK(0.329,0.098,1.1e-1,0.);
     next324=true;
  }
  if (next324){
     nucltransK(0.324,0.098,1.1e-1,0.);
     return;
  }
}
///-----------------------------------------------
///-------end of functions
///-----------------------------------------------


///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
///^^^^^^^  Section of radioactive decays       ^^
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Ac228()
{
  // Scheme of 228Ac decay ("Table of Isotopes", 7th ed., 1978)
  // VIT, 8.08.1992, 22.10.1995.
  // updated to http://www.nucleide.org/DDEP_WG/DDEPdata.htm (Dec 2015)
  //    (This reference was preferred over NDS as it includes
  //      the conversion electron coefficients.)
  //    Beta endpoint energies with probability > 0.05 % are included.
  //    For levels with more than 4-5 gammas, only those with
  //    intensity > 0.06 % are included.

  bool next58=false,   next129=false,  next328=false,  next191=false;
  bool next338=false,  next332=false,  next774=false,  next874=false;
  bool next944=false,  next969=false,  next1016=false, next965=false;
  bool next1033=false, next1065=false, next1153=false, next1110=false;
  bool next1175=false, next1039=false, next1286=false, next1245=false;
  bool next1451=false, next562=false,  next1143=false, next1638=false;
  bool next1588=false, next1702=false;
  float pbeta, p;

 pbeta =100.*GetRandom();
 if (pbeta<= fProbBeta[0]){   // 0.06%
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<=39.6){
        nucltransK(fEnGamma[0],fEbindeK,3.4e-3,0.);
        next58=true;
     }
     else  if (p<=51.2){
        nucltransK(fEnGamma[1],fEbindeK,1.1e-2,0.);
        next774=true;
     }
     else  if (p<=62.2){
        nucltransK(fEnGamma[2],fEbindeK,1.2e-2,0.);
        next874=true;
     }
     else  if (p<=87.6){
        nucltransK(fEnGamma[3],fEbindeK,1.5e-2,0.);
        next969=true;
     }
     else{
        nucltransK(fEnGamma[4],fEbindeK,4.5e-2,0.);
        next1638=true;
     }
  }
 else if (pbeta<= fProbBeta[1]){   // 0.023%
     beta(fEndPoint[1],0.,0.);
     p=100*GetRandom();
     if (p<=29.2){
        nucltransK(fEnGamma[5],fEbindeK,1.0e-2,0.);
        next58=true;
     }
     else  if (p<=50.1){
        nucltransK(fEnGamma[6],fEbindeK,0.,0.);
        next129=true;
     }
     else  if (p<=73.1){
        nucltransK(fEnGamma[7],fEbindeK,5.0e-2,0.);
        next969=true;
     }
     else  if (p<=86.3){
        nucltransK(fEnGamma[8],fEbindeK,0.,0.);
        next1033=true;
     }
     else{
        nucltransK(fEnGamma[9],fEbindeK,2.3e-2,0.);
        next1065=true;
     }
  }
 else if (pbeta<= fProbBeta[2]){   // 0.07%
     beta(fEndPoint[2],0.,0.);
     p=100*GetRandom();
     if (p<=31.8){
        nucltransK(fEnGamma[10],fEbindeK,1.0e-2,0.);
        next58=true;
     }
     else  if (p<=38.8){
        nucltransK(fEnGamma[11],fEbindeK,0.,0.);
        next129=true;
     }
     else  if (p<=51.1){
        nucltransK(fEnGamma[12],fEbindeK,3.2e-3,0.);
        next191=true;
     }
     else{
        nucltransK(fEnGamma[13],fEbindeK,5.0e-2,0.);
        next969=true;
     }
  }
 else if (pbeta<= fProbBeta[3]){   // 0.31%
     beta(fEndPoint[3],0.,0.);
     p=100*GetRandom();
     if (p<=42.5){
        nucltransK(fEnGamma[14],fEbindeK,3.8e-3,0.);
        next58=true;
     }
     else  if (p<=58.8){
        nucltransK(fEnGamma[15],fEbindeK,2.8e-3,0.);
        next129=true;
     }
     else  if (p<=82.4){
        nucltransK(fEnGamma[16],fEbindeK,2.8e-2,0.);
        next969=true;
     }
     else  if (p<=91.4){
        nucltransK(fEnGamma[17],fEbindeK,0.,0.);
        next1110=true;
     }
     else{
        nucltransK(fEnGamma[18],fEbindeK,5.1e-3,0.);
        next1039=true;
     }
  }
 else if (pbeta<= fProbBeta[4]){   // 0.05%
     beta(fEndPoint[4],0.,0.);
     p=100*GetRandom();
     if (p<=2.6){
        nucltransK(fEnGamma[19],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=19.5){
        nucltransK(fEnGamma[20],fEbindeK,0.,0.);
        next129=true;
     }
     else  if (p<=41.6){
        nucltransK(fEnGamma[21],fEbindeK,0.,0.);
        next874=true;
     }
     else{
        nucltransK(fEnGamma[22],fEbindeK,0.,0.);
        next1143=true;
     }
  }
 else if (pbeta<= fProbBeta[5]){   // 0.06%
     beta(fEndPoint[5],0.,0.);
     p=100*GetRandom();
     if (p<=59.2){
        nucltransK(fEnGamma[23],fEbindeK,3.8e-3,0.);
        next58=true;
     }
     else  if (p<=78.6){
        nucltransK(fEnGamma[24],fEbindeK,0.,0.);
        next129=true;
     }
     else{
        nucltransK(fEnGamma[25],fEbindeK,1.8,0.);
        next1702=true;
     }
  }
 else if (pbeta<= fProbBeta[6]){   // 0.07%
     beta(fEndPoint[6],0.,0.);
     p=100*GetRandom();
     if (p<=60.6){
        nucltransK(fEnGamma[26],fEbindeK,4.0e-3,0.);
        next58=true;
     }
     else  if (p<=69.9){
        nucltransK(fEnGamma[27],fEbindeK,1.4e-2,0.);
        next129=true;
     }
     else  if (p<=76.4){
        nucltransK(fEnGamma[28],fEbindeK,0.,0.);
        next969=true;
     }
     else{
        nucltransK(fEnGamma[29],fEbindeK,2.4e-2,0.);
        next965=true;
     }
  }
 else if (pbeta<= fProbBeta[7]){   // 0.11%
     beta(fEndPoint[7],0.,0.);
     p=100*GetRandom();
     if (p<=38.1){
        nucltransK(fEnGamma[30],fEbindeK,2.9e-3,0.);
        next58=true;
     }
     else  if (p<=47.){
        nucltransK(fEnGamma[31],fEbindeK,6.0e-3,0.);
        next129=true;
     }
     else  if (p<=93.2){
        nucltransK(fEnGamma[32],fEbindeK,3.8e-2,0.);
        next965=true;
     }
     else{
        nucltransK(fEnGamma[33],fEbindeK,5.9e-3,0.);
        next1039=true;
     }
  }
 else if (pbeta<= fProbBeta[8]){   // 0.05%
     beta(fEndPoint[8],0.,0.);
     p=100*GetRandom();
     if (p<=24.3){
        nucltransK(fEnGamma[34],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=70.8){
        nucltransK(fEnGamma[35],fEbindeK,1.2e-2,0.);
        next328=true;
     }
     else{
        nucltransK(fEnGamma[36],fEbindeK,1.3e-2,0.);
        next338=true;
     }
  }
 else if (pbeta<= fProbBeta[9]){   // 0.14%
     beta(fEndPoint[9],0.,0.);
     next1702=true;
  }
 else if (pbeta<= fProbBeta[10]){   // 0.06%
     beta(fEndPoint[10],0.,0.);
     p=100*GetRandom();
     if (p<=20.4){
        nucltransK(fEnGamma[37],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=31.9){
        nucltransK(fEnGamma[38],fEbindeK,0.,0.);
        next129=true;
     }
     else{
        nucltransK(fEnGamma[39],fEbindeK,6.0e-2,0.);
        next1245=true;
     }
  }
 else if (pbeta<= fProbBeta[11]){   // 0.38%
     beta(fEndPoint[11],0.,0.);
     p=100*GetRandom();
     if (p<=29.3){
        nucltransK(fEnGamma[40],fEbindeK,3.0e-3,0.);
        next58=true;
     }
     else  if (p<=83.3){
        nucltransK(fEnGamma[41],fEbindeK,5.5e-3,0.);
        next129=true;
     }
     else  if (p<=90.1){
        nucltransK(fEnGamma[42],fEbindeK,8.0e-3,0.);
        next328=true;
     }
     else{
        nucltransK(fEnGamma[43],fEbindeK,1.6e-2,0.);
        next1286=true;
     }
  }
 else if (pbeta<= fProbBeta[12]){   // 0.15%
     beta(fEndPoint[12],0.,0.);
     p=100*GetRandom();
     if (p<=40.7){
        nucltransK(fEnGamma[44],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=69.3){
        nucltransK(fEnGamma[45],fEbindeK,0.,0.);
        next129=true;
     }
     else  if (p<=84.3){
        nucltransK(fEnGamma[46],fEbindeK,0.,0.);
        next191=true;
     }
     else{
        nucltransK(fEnGamma[47],fEbindeK,0.,0.);
        next332=true;
     }
  }
 else if (pbeta<= fProbBeta[13]){   // 1.93%
     beta(fEndPoint[13],0.,0.);
     p=100*GetRandom();
     if (p<=10.){
        nucltransK(fEnGamma[48],fEbindeK,7.0e-3,0.);
        next58=true;
     }
     else  if (p<=12.9){
        nucltransK(fEnGamma[49],fEbindeK,1.8e-2,0.);
        next129=true;
     }
     else  if (p<=76.8){
        nucltransK(fEnGamma[50],fEbindeK,5.6e-2,0.);
        next969=true;
     }
     else  if (p<=87.2){
        nucltransK(fEnGamma[51],fEbindeK,6.8e-2,0.);
        next965=true;
     }
     else{
        nucltransK(fEnGamma[52],fEbindeK,1.2e-1,0.);
        next1153=true;
     }
  }
 else if (pbeta<= fProbBeta[14]){   // 2.5%
     beta(fEndPoint[14],0.,0.);
     p=100*GetRandom();
     if (p<=61.){
        nucltransK(fEnGamma[53],fEbindeK,5.3e-3,0.);
        next58=true;
     }
     else  if (p<=81.6){
        nucltransK(fEnGamma[54],fEbindeK,0.,0.);
        next129=true;
     }
     else  if (p<=82.7){
        nucltransK(fEnGamma[55],fEbindeK,0.,0.);
        next1016=true;
     }
     else{
        nucltransK(fEnGamma[56],fEbindeK,0.,0.);
        next1588=true;
     }
  }
 else if (pbeta<= fProbBeta[15]){   // 0.20%        1st-forbidden
     beta(fEndPoint[15],0.,0.);
     p=100*GetRandom();
     if (p<=56.){
        nucltransK(fEnGamma[57],fEbindeK,9.0e-3,0.);
        next338=true;
     }
     else  if (p<=90.4){
        nucltransK(fEnGamma[58],fEbindeK,1.2e-2,0.);
        next332=true;
     }
     else{
        nucltransK(fEnGamma[59],fEbindeK,1.2e-1,0.);
        next1039=true;
     }
  }
 else if (pbeta<= fProbBeta[16]){   // 1.21%
     beta(fEndPoint[16],0.,0.);
     p=100*GetRandom();
     if (p<=22.5){
        nucltransK(fEnGamma[60],fEbindeK,1.6e-2,0.);
        next58=true;
     }
     else  if (p<=99.5){
        nucltransK(fEnGamma[61],fEbindeK,3.7e-3,0.);
        next129=true;
     }
     else{
        nucltransK(fEnGamma[62],fEbindeK,5.9e-3,0.);
        next965=true;
     }
  }
 else if (pbeta<= fProbBeta[17]){   // 4.12%
     beta(fEndPoint[17],0.,0.);
     next1588=true;
  }
 else if (pbeta<= fProbBeta[18]){   // 0.82%      1st-forbidden
     beta(fEndPoint[18],0.,0.);
     p=100*GetRandom();
     if (p<=71.1){
        nucltransK(fEnGamma[63],fEbindeK,1.5e-2,0.);
        next338=true;
     }
     else  if (p<=76.3){
        nucltransK(fEnGamma[64],fEbindeK,1.4e-2,0.);
        next944=true;
     }
     else  if (p<=90.5){
        nucltransK(fEnGamma[65],fEbindeK,5.0e-2,0.);
        next969=true;
     }
     else{
        nucltransK(fEnGamma[66],fEbindeK,9.0e-2,0.);
        next1065=true;
     }
  }
 else if (pbeta<= fProbBeta[19]){   // 1.23%
     beta(fEndPoint[19],0.,0.);
     next1638=true;
  }
 else if (pbeta<= fProbBeta[20]){   // 0.07%
     beta(fEndPoint[20],0.,0.);
     p=100*GetRandom();
     if (p<=31.5){
        nucltransK(fEnGamma[67],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=87.1){
        nucltransK(fEnGamma[68],fEbindeK,0.,0.);
        next129=true;
     }
     else{
        nucltransK(fEnGamma[69],fEbindeK,1.7e-2,0.);
        next969=true;
     }
  }
 else if (pbeta<= fProbBeta[21]){   // 8.8%     1st-forbidden
     // This decay populates the 228Th 1588 keV level producing 138 keV and 57 keV
     // gammas, leading to a 536 keV beta energy.
     // The reference used here (http://www.nucleide.org/DDEP_WG/DDEPdata.htm)
     // diverges from the NDS for this transition.
     // In the NDS, 228Ac directly populates the 1531 keV level (producing a 592 keV
     // beta) and not the 1588 keV level, which is instead populated by 228Pa.
     // In order to include the observed gammas below, A. Pearce (author of
     // the nucleide.org reference), replaced the 1588 keV level for the 1531 keV.
     // (See note under Decay Scheme here:
     // http://www.nucleide.org/DDEP_WG/Nuclides/Ac-228_com.pdf)
     beta(fEndPoint[21],0.,0.);
     p=100*GetRandom();
     if (p<=2.9){
        nucltransK(fEnGamma[70],fEbindeK,6.0,0.);
        next1451=true;
     }
     else{
        nucltransKLM(fEnGamma[71],fEbindeK,0.,fEbindeL,2.6e+2,fEbindeM,7.0e+1,0.);
        next562=true;
     }
  }
 else if (pbeta<= fProbBeta[22]){   // 1.6%
     beta(fEndPoint[22],0.,0.);
     next1245=true;
  }
 else if (pbeta<= fProbBeta[23]){   // 0.06%
     beta(fEndPoint[23],0.,0.);
     p=100*GetRandom();
     if (p<=10.9){
        nucltransK(fEnGamma[72],fEbindeK,0.,0.);
        next129=true;
     }
     else  if (p<=19.5){
        nucltransK(fEnGamma[73],fEbindeK,0.,0.);
        next328=true;
     }
     else  if (p<=50.2){
        nucltransK(fEnGamma[74],fEbindeK,0.,0.);
        next338=true;
     }
     else{
        nucltransK(fEnGamma[75],fEbindeK,3.1e-1,0.);
        next944=true;
     }
  }
 else if (pbeta<= fProbBeta[24]){   // 0.21%   1st-forbidden
     beta(fEndPoint[24],0.,0.);
     next1286=true;
  }
 else if (pbeta<= fProbBeta[25]){   // 1.46%   1st-forbidden unique
     beta(fEndPoint[25],0.,0.);
     p=100*GetRandom();
     if (p<=61.1){
        nucltransK(fEnGamma[76],fEbindeK,2.3e-3,0.);
        next129=true;
     }
     else{
        nucltransK(fEnGamma[77],fEbindeK,9.0e-3,0.);
        next338=true;
     }
  }
 else if (pbeta<= fProbBeta[26]){   // 0.67%   1st-forbidden
     beta(fEndPoint[26],0.,0.);
     next1039=true;
  }
 else if (pbeta<= fProbBeta[27]){   // 0.17%
     beta(fEndPoint[27],0.,0.);
     next1175=true;
  }
 else if (pbeta<= fProbBeta[28]){   // 3.39%    1st-forbidden
     beta(fEndPoint[28],0.,0.);
     next1110=true;
  }
 else if (pbeta<= fProbBeta[29]){   // 6%
     beta(fEndPoint[29],0.,0.);
     next1153=true;
  }
 else if (pbeta<= fProbBeta[30]){   // 6.67%     1st-forbidden
     beta(fEndPoint[30],0.,0.);
     next1065=true;
  }
 else if (pbeta<= fProbBeta[31]){   // 0.10%     1st-forbidden
     beta(fEndPoint[31],0.,0.);
     p=100*GetRandom();
     if (p<=35.3){
        nucltransK(fEnGamma[78],fEbindeK,3.6e-3,0.);
        next129=true;
     }
     else  if (p<=67.3){
        nucltransK(fEnGamma[79],fEbindeK,5.0e-2,0.);
        next338=true;
     }
     else{
        nucltransK(fEnGamma[80],fEbindeK,8.0e-2,0.);
        next332=true;
     }
  }
 else if (pbeta<= fProbBeta[32]){   // 3%
     beta(fEndPoint[32],0.,0.);
     next965=true;
  }
 else if (pbeta<= fProbBeta[33]){   // 0.39%
     beta(fEndPoint[33],0.,0.);
     next1016=true;
  }
 else if (pbeta<= fProbBeta[34]){   // 0.24%
     beta(fEndPoint[34],0.,0.);
     p=100*GetRandom();
     if (p<=5.1){
        nucltransK(fEnGamma[81],fEbindeK,2.1e-2,0.);
        next58=true;
     }
     else  if (p<=31.4){
        nucltransK(fEnGamma[82],fEbindeK,1.1e-2,0.);
        next129=true;
     }
     else  if (p<=61.5){
        nucltransK(fEnGamma[83],fEbindeK,6.0e-3,0.);
        next328=true;
     }
     else{
        nucltransK(fEnGamma[84],fEbindeK,7.6e-3,0.);
        next338=true;
     }
  }
 else if (pbeta<= fProbBeta[35]){   // 31%
     beta(fEndPoint[35],0.,0.);
     next969=true;
  }
 else if (pbeta<= fProbBeta[36]){   // 0.18%   1st-forbidden
     beta(fEndPoint[36],0.,0.);
     p=100*GetRandom();
     if (p<=20.6){
        nucltransK(fEnGamma[85],fEbindeK,1.7e-2,0.);
        next328=true;
     }
     else  if (p<=81.1){
        nucltransK(fEnGamma[86],fEbindeK,7.0e-2,0.);
        next338=true;
     }
     else{
        nucltransK(fEnGamma[87],fEbindeK,3.3e-2,0.);
        next332=true;
     }
  }
 else if (pbeta<= fProbBeta[37]){   // 0.09%
     beta(fEndPoint[37],0.,0.);
     next944=true;
  }
 else if (pbeta<= fProbBeta[38]){   // 0.17%
     beta(fEndPoint[38],0.,0.);
     next874=true;
  }
 else if (pbeta<= fProbBeta[39]){   // 12.4%   1st-forbidden
     beta(fEndPoint[39],0.,0.);
     next338=true;
  }
 else if (pbeta<= fProbBeta[40]){   // 0.15%    2nd-forbidden unique
     beta(fEndPoint[40],0.,0.);
     next191=true;
  }
 else if (pbeta<= fProbBeta[41]){   // 0.72%    1st-forbidden unique
     beta(fEndPoint[41],0.,0.);
     next328=true;
  }
 else if (pbeta<= fProbBeta[42]){   // 0.6%
     beta(fEndPoint[42],0.,0.);
     next129=true;
  }
 else if (pbeta<= fProbBeta[43]){   // 6%
     beta(fEndPoint[43],0.,0.);
     next58=true;
  }
 if (next1702){
     p=100*GetRandom();
     if (p<=39.7){
        nucltransK(fEnGamma[88],fEbindeK,1.4e-2,0.);
        next58=true;
     }
     else  if (p<=64.3){
        nucltransK(fEnGamma[89],fEbindeK,3.4e-3,0.);
        next129=true;
     }
     else  if (p<=71.8){
        nucltransK(fEnGamma[90],fEbindeK,3.1e-2,0.);
        next969=true;
     }
     else{
        nucltransK(fEnGamma[91],fEbindeK,3.7e-2,0.);
        next965=true;
     }
  }
 if (next1588){
     p=100*GetRandom();
     if (p<=56.9){
        nucltransK(fEnGamma[92],fEbindeK,5.7e-3,0.);
        next58=true;
     }
     else  if (p<=73.4){
        nucltransK(fEnGamma[93],fEbindeK,3.9e-3,0.);
        next129=true;
     }
     else  if (p<=85.5){
        nucltransK(fEnGamma[94],fEbindeK,0.,0.);
        next338=true;
     }
     else  if (p<=97.6){
        nucltransK(fEnGamma[95],fEbindeK,5.0e-2,0.);
        next969=true;
     }
     else{
        nucltransK(fEnGamma[96],fEbindeK,9.3e-3,0.);
        next1065=true;
     }
  }
 if (next1638){
     p=100*GetRandom();
     if (p<=39.9){
        nucltransK(fEnGamma[97],fEbindeK,3.0e-3,0.);
        return;
     }
     else  if (p<=93.9){
        nucltransK(fEnGamma[98],fEbindeK,5.7e-3,0.);
        next58=true;
     }
     else  if (p<=95.6){
        nucltransK(fEnGamma[99],fEbindeK,1.6e-2,0.);
        next328=true;
     }
     else{
        nucltransK(fEnGamma[100],fEbindeK,9.7e-3,0.);
        next1065=true;
     }
  }
 if (next1143){
     p=100*GetRandom();
     if (p<=43.9){
        nucltransK(fEnGamma[101],fEbindeK,0.,0.);
        next338=true;
     }
     else{
        nucltransK(fEnGamma[102],fEbindeK,1.5e-2,0.);
        next1065=true;
     }
  }
 if (next562){
     p=100*GetRandom();
     if (p<=12.4){
        nucltransK(fEnGamma[103],fEbindeK,7.0e-2,0.);
        next969=true;
     }
     else  if (p<=19.7){
        nucltransK(fEnGamma[104],fEbindeK,8.7e-2,0.);
        next965=true;
     }
     else  if (p<=21.8){
        nucltransK(fEnGamma[105],fEbindeK,2.4e-1,0.);
        next1033=true;
     }
     else{
        nucltransKL(fEnGamma[106],fEbindeK,0.,fEbindeL,2.9,0.);
        next1245=true;
     }
  }
 if (next1451){
     p=100*GetRandom();
     if (p<=3.0){
        nucltransK(fEnGamma[107],fEbindeK,7.0e-3,0.);
        return;
     }
     else  if (p<=8.1){
        nucltransK(fEnGamma[108],fEbindeK,1.5e-2,0.);
        next338=true;
     }
     else  if (p<=46.0){
        nucltransK(fEnGamma[109],fEbindeK,4.0e-1,0.);
        next1110=true;
     }
     else  if (p<=61.6){
        nucltransK(fEnGamma[110],fEbindeK,1.5,0.);
        next1039=true;
     }
     else{
        nucltransK(fEnGamma[111],fEbindeK,0.,0.);
        next1245=true;
     }
  }
 if (next1245){
     p=100*GetRandom();
     if (p<=1.6){
        nucltransK(fEnGamma[112],fEbindeK,1.0e-2,0.);
        next129=true;
     }
     else  if (p<=65.3){
        nucltransK(fEnGamma[113],fEbindeK,3.1e-2,0.);
        next969=true;
     }
     else  if (p<=94.2){
        nucltransK(fEnGamma[114],fEbindeK,1.6e-1,0.);
        next965=true;
     }
     else{
        nucltransK(fEnGamma[115],fEbindeK,7.1e-2,0.);
        next1033=true;
     }
  }
 if (next1286){
     p=100*GetRandom();
     if (p<=22.6){
        nucltransK(fEnGamma[116],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=71.){
        nucltransK(fEnGamma[117],fEbindeK,2.0e-2,0.);
        next338=true;
     }
     else  if (p<=94.5){
        nucltransK(fEnGamma[118],fEbindeK,1.0e-2,0.);
        next332=true;
     }
     else{
        nucltransK(fEnGamma[119],fEbindeK,1.8,0.);
        next1175=true;
     }
  }
 if (next1039){
     p=100*GetRandom();
     if (p<=5.8){
        nucltransK(fEnGamma[120],fEbindeK,0.,0.);
        next129=true;
     }
     else  if (p<=68.9){
        nucltransK(fEnGamma[121],fEbindeK,1.1e-2,0.);
        next338=true;
     }
     else  if (p<=85.7){
        nucltransK(fEnGamma[122],fEbindeK,1.4e-1,0.);
        next332=true;
     }
     else  if (p<=97.5){
        nucltransK(fEnGamma[123],fEbindeK,7.3,0.);
        next965=true;
     }
     else{
        nucltransK(fEnGamma[124],fEbindeK,1.9e-1,0.);
        next1033=true;
     }
  }
 if (next1175){
     p=100*GetRandom();
     if (p<=13.3){
        nucltransK(fEnGamma[125],fEbindeK,2.1e-2,0.);
        return;
     }
     else  if (p<=44.8){
        nucltransK(fEnGamma[126],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=86.6){
        nucltransK(fEnGamma[127],fEbindeK,7.7e-3,0.);
        next129=true;
     }
     else{
        nucltransK(fEnGamma[128],fEbindeK,1.2e-1,0.);
        next944=true;
     }
  }
 if (next1110){
     p=100*GetRandom();
     if (p<=9.2){
        nucltransK(fEnGamma[129],fEbindeK,2.3e-3,0.);
        next58=true;
     }
     else  if (p<=41.1){
        nucltransK(fEnGamma[130],fEbindeK,1.0e-2,0.);
        next328=true;
     }
     else  if (p<=90.3){
        nucltransK(fEnGamma[131],fEbindeK,1.8e-2,0.);
        next338=true;
     }
     else  if (p<=90.3){
        nucltransK(fEnGamma[132],fEbindeK,7.5e-2,0.);
        next969=true;
     }
     else{
        nucltransK(fEnGamma[133],fEbindeK,1.6e-1,0.);
        next965=true;
     }
  }
 if (next1153){
     fThlev=2.9e-10;
     p=100*GetRandom();
     if (p<=2.5){
        nucltransK(fEnGamma[134],fEbindeK,2.2e-2,0.);
        return;
     }
     else  if (p<=4.7){
        nucltransK(fEnGamma[135],fEbindeK,1.4e-2,0.);
        next58=true;
     }
     else  if (p<=8.7){
        nucltransK(fEnGamma[136],fEbindeK,6.0e-2,0.);
        next774=true;
     }
     else  if (p<=14.3){
        nucltransK(fEnGamma[137],fEbindeK,5.0e-1,0.);
        next874=true;
     }
     else{
        nucltransK(fEnGamma[138],fEbindeK,8.0e+1,0.);
        next874=true;
     }
  }
 if (next1065){
     p=100*GetRandom();
     if (p<=2.3){
        nucltransK(fEnGamma[139],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=74.1){
        nucltransK(fEnGamma[140],fEbindeK,1.3e-2,0.);
        next328=true;
     }
     else  if (p<=85.5){
        nucltransK(fEnGamma[141],fEbindeK,1.3e-2,0.);
        next338=true;
     }
     else  if (p<=98.1){
        nucltransK(fEnGamma[142],fEbindeK,1.4e-1,0.);
        next969=true;
     }
     else{
        nucltransKL(fEnGamma[143],fEbindeK,8.0e+1,fEbindeL,2.3,0.);
        next965=true;
     }
  }
 if (next1033){
     p=100*GetRandom();
     if (p<=20.7){
        nucltransK(fEnGamma[144],fEbindeK,7.2e-3,0.);
        next58=true;
     }
     else{
        nucltransK(fEnGamma[145],fEbindeK,9.0e-3,0.);
        next129=true;
     }
  }
 if (next965){
     p=100*GetRandom();
     if (p<=74.6){
        nucltransK(fEnGamma[146],fEbindeK,8.5e-3,0.);
        next58=true;
     }
     else{
        nucltransK(fEnGamma[147],fEbindeK,1.0e-2,0.);
        next129=true;
     }
  }
 if (next1016){
     p=100*GetRandom();
     if (p<=4.2){
        nucltransK(fEnGamma[148],fEbindeK,1.7e-2,0.);
        return;
     }
     else  if (p<=66.8){
        nucltransK(fEnGamma[149],fEbindeK,0.,0.);
        next58=true;
     }
     else  if (p<=81.9){
        nucltransK(fEnGamma[150],fEbindeK,0.,0.);
        next328=true;
     }
     else{
        nucltransK(fEnGamma[151],fEbindeK,0.,0.);
        next338=true;
     }
  }
 if (next969){
     p=100*GetRandom();
     if (p<=37.3){
        nucltransK(fEnGamma[152],fEbindeK,8.0e-3,0.);
        return;
     }
     else  if (p<=98.8){
        nucltransK(fEnGamma[153],fEbindeK,9.0e-3,0.);
        next58=true;
     }
     else{
        nucltransK(fEnGamma[154],fEbindeK,1.1e-2,0.);
        next129=true;
     }
  }
 if (next944){
     p=100*GetRandom();
     if (p<=54.5){
        nucltransK(fEnGamma[155],fEbindeK,2.0e-2,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[156],fEbindeK,6.8e-3,0.);
        next328=true;
     }
  }
 if (next874){
     p=100*GetRandom();
     if (p<=10.){
        nucltransK(fEnGamma[157],fEbindeK,9.6e-3,0.);
        return;
     }
     else  if (p<=16.2){
        nucltransK(fEnGamma[158],fEbindeK,2.8e-2,0.);
        next58=true;
     }
     else  if (p<=55.6){
        nucltransK(fEnGamma[159],fEbindeK,8.6e-3,0.);
        next328=true;
     }
     else{
        nucltransK(fEnGamma[160],fEbindeK,1.1e-1,0.);
        next338=true;
     }
  }
 if (next774){
     p=100*GetRandom();
     if (p<=26.7){
        nucltransK(fEnGamma[161],fEbindeK,1.2e-2,0.);
        next58=true;
     }
     else{
        nucltransK(fEnGamma[162],fEbindeK,1.0e-2,0.);
        next328=true;
     }
  }
 if (next332){
     p=100*GetRandom();
     if (p<=87.4){
        nucltransK(fEnGamma[163],fEbindeK,2.4e-2,0.);
        next129=true;
     }
     else{
        nucltransK(fEnGamma[164],fEbindeK,1.7e-1,0.);
        next191=true;
     }
  }
 if (next338){
     p=100*GetRandom();
     if (p<=73.1){
        nucltransK(fEnGamma[165],fEbindeK,2.3e-2,0.);
        next58=true;
     }
     else{
        nucltransK(fEnGamma[166],fEbindeK,6.7e-2,0.);
        next129=true;
     }
  }
 if (next191){
     nucltransK(fEnGamma[167],fEbindeK,1.7e-1,0.);
     next129=true;
  }
 if (next328){
     p=100*GetRandom();
     if (p<=45.7){
        nucltransK(fEnGamma[168],fEbindeK,2.5e-2,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[169],fEbindeK,3.7e-2,0.);
        next58=true;
     }
  }
 if (next129){
     nucltransKLM(fEnGamma[170],fEbindeK,2.6e-1,fEbindeL,2.5,fEbindeM,7.0e-1,0.);
     next58=true;
  }
 if (next58){
     nucltransKLM(fEnGamma[171],fEbindeK,0.,fEbindeL,1.1e+2,fEbindeM,3.1e+1,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::As79()
{
  //  Model for scheme of As79+Se79m decay
  // ("Table of Isotopes", 8th ed.,1998 and Nucl. Data Sheets 96(2002)1).
  bool next572=false, next528=false, next365=false, next96=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<= fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<=53.70){
        nucltransK(fEnGamma[0],fEbindeK,4.0e-4,0.);
        next96=true;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,8.5e-4,0.);
        next365=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100*GetRandom();
     if (p<=68.97){
        nucltransK(fEnGamma[2],fEbindeK,8.5e-4,0.);
        next365=true;
     }
     else {
        nucltransK(fEnGamma[3],fEbindeK,1.5e-3,0.);
        next528=true;
     }
  }
  else if (pbeta<= fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     p=100*GetRandom();
     if (p<=79.37){
        nucltransK(fEnGamma[4],fEbindeK,5.5e-4,0.);
        next96=true;
     }
     else if (p<=94.45){
        nucltransK(fEnGamma[5],fEbindeK,2.5e-3,0.);
        next528=true;
     }
     else{
        nucltransK(fEnGamma[6],fEbindeK,3.2e-3,0.);
        next572=true;
     }
  }
  else if (pbeta<= fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     next572=true;
  }
  else if (pbeta<=fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     next528=true;
  }
  else if(pbeta<=fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     next365=true;
  }
  else{
     beta(fEndPoint[6],0.,0.);
     next96=true;
  }

  if (next572){
     fThlev=1.6e-11;
     p=100*GetRandom();
     if (p<=95.33){
        nucltransK(fEnGamma[7],fEbindeK,1.0e-3,0.);
        next96=true;
     }
     else{
        nucltransK(fEnGamma[8],fEbindeK,1.8e-2,0.);
        next365=true;
     }
  }
  if (next528){
     fThlev=3.1e-12;
     nucltransK(fEnGamma[9],fEbindeK,2.7e-3,0.);
     next96=true;
  }
  if (next365){
     fThlev=9.4e-11;
     nucltransK(fEnGamma[10],fEbindeK,2.0e-3,0.);
     return;
  }
  if (next96){
     fThlev=235.2;
     p=100*GetRandom();
     if (p<=99.944){
        nucltransK(fEnGamma[11],fEbindeK,9.39,0.); //IT to Se79 (gs)
        return;
     }
     else{
        fZdtr=fDaughterZ[1];
        beta(0.247,fTclev,fThlev);// beta to Br79 (gs)
        return;
     }
  }
}
void Decay0::Be11()
{
  // information from Phys. Rev. C, Vol. 26, Number 3 and JEFF 3.1.1 entry for 11Be (using JANIS)
  // alpha energies calculated by JEFF-Q-Values and given states

  float pbeta, palpha, p;
  bool next2125=false, next5020=false, next7286=false, next2895=false;
  fZdtr=fDaughterZ[0];

  pbeta = 100.*GetRandom();
  if (pbeta <= fProbBeta[0]){
      //beta- decay to 9.875 MeV state of 11B, followed by alpha to 7Li
      fZdtr = fDaughterZ[1];
      beta(fEndPoint[0], 0., 0.);

      palpha = 100.*GetRandom();
      if (palpha <= fProbAlpha[0]){
          //alpha decay to 7Li ground state
          particle(47,fEnAlpha[0],fEnAlpha[0],0.,pi,0.,twopi,0,0);
      }
      else{
          //alpha decay to 7Li first exited state + de-excitation
          particle(47,fEnAlpha[1],fEnAlpha[1],0.,pi,0.,twopi,0,0);
          nucltransK(0.478, fEbindeK, 0, 0);
      }
  }
  else if (pbeta <= fProbBeta[1]){
      //beta- decay to 7.978 MeV state of 11B
      beta(fEndPoint[1], 0., 0.);
      p = 100.*GetRandom();
      if (p <= 46.6767){
          //de-excitation direct to GS
          nucltransK(fEnGamma[0], fEbindeK, 0.0, 0.0);
      }
      else if (p <= 46.7307){
          //7.978 -> 0.692 + 7.286 MeV
          nucltransK(fEnGamma[3], fEbindeK, 0.0, 0.0);
          next7286 = true;
      }
      else{
          //7.978 -> 5.853 + 2.125 MeV
          nucltransK(fEnGamma[1], fEbindeK, 0.0, 0.0);
          next2125 = true;
      }

  }
  else if (pbeta <= fProbBeta[2]){
      //beta- decay to 6.792 MeV state of 11B
      beta(fEndPoint[2], 0., 0.);
      p = 100.*GetRandom();
      if (p <= 68.3){
          //de-excitation direct to GS (6.792 MeV)
          nucltransK(fEnGamma[5], fEbindeK, 0.0, 0.0);
      }
      else if (p <= 96.0){
          //6.792 -> 4.667 + 2.125 MeV
          nucltransK(fEnGamma[6], fEbindeK, 0.0, 0.0);
          next2125 = true;
      }
      else if (p <= 99.412){
          //6.792 -> 1.772 + 5.020 MeV
          nucltransK(fEnGamma[7], fEbindeK, 0.0, 0.0);
          next5020 = true;
      }
      else{
          //6.792 -> 1.772 + 2.895 + 2.125
          nucltransK(fEnGamma[7], fEbindeK, 0.0, 0.0);
          next2895 = true;
          next2125 = true;
      }
  }
  else if (pbeta <= fProbBeta[3]){
      //beta- decay to 5.020 MeV stste of 11B
      //first order non-unique forbidden: 1/2+ -> 3/2-
      beta1f(fEndPoint[3],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
      p = 100.0*GetRandom();
      if (p <= 85.3){
          //de-excitation direct to GS (5.020 MeV)
          nucltransK(fEnGamma[8], fEbindeK, 0.0, 0.0);
      }
      else {
          //5.020 -> 2895 + 2125
          nucltransK(fEnGamma[9], fEbindeK, 0.0, 0.0);
          next2125 = true;
      }
  }
  else if (pbeta <= fProbBeta[4]){
      //beta- decay to 4.445 MeV excited state and direct de-excitation to GS
      //first order unique forbidden: 1/2+ -> 5/2-
      beta1fu(fEndPoint[4], 0.0, 0.0, fShCorrFactor[0], fShCorrFactor[1], fShCorrFactor[2], fShCorrFactor[3]);
      nucltransK(fEnGamma[10], fEbindeK, 0.0, 0.0);
  }
  else if (pbeta <= fProbBeta[5]){
      //beta- decay to 2.125 MeV excited state and direct de-excitation to GS
      //first order non-unique forbidden: 1/2+ -> 1/2-
      beta1f(fEndPoint[5], 0.0, 0.0, fShCorrFactor[0], fShCorrFactor[1], fShCorrFactor[2], fShCorrFactor[3]);
      next2125 = true;
  }
  else {
      //beta- decay to GS
      //first order non-unique forbidden: 1/2+ -> 3/2-
      beta1f(fEndPoint[6], 0.0, 0.0, fShCorrFactor[0], fShCorrFactor[1], fShCorrFactor[2], fShCorrFactor[3]);
  }
  if (next2125 == true){ nucltransK(fEnGamma[2], fEbindeK, 0.0, 0.0);}
  if (next5020 == true){ nucltransK(fEnGamma[8], fEbindeK, 0.0, 0.0);}
  if (next7286 == true){ nucltransK(fEnGamma[4], fEbindeK, 0.0, 0.0);}
  if (next2895 == true){ nucltransK(fEnGamma[9], fEbindeK, 0.0, 0.0);}
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Bi207()
{
  // Scheme of Bi207+Pb207m decay (Nucl. Data Sheets 70(1993)315)
  // with NNDC corrections of 10.10.2000 and 20.04.2005.
  // To describe the deexcitation processes in atomic shell of Pb207,
  // the information of PC Nuclear Data File retrieval program and
  // data base (last updated on 16-Aug-1994) was used.
  // Gammas, beta+, e+e- pairs, K, L and M conversion electrons,
  // K, L and M X-rays and K and L Auger electrons are emitted.
  // VIT, 7.07.1995, 22.10.1995, 11.09.2005, 3.10.2005.
  bool next57=false, next898=false;
  float pdecay, p, cg=1.;
  int npe570=0, npe1064=0, npg570=0, npg1064=0, np570=0, np1064=0;
  double cK=0., cL=0., cM=0., cp=0., a2=0., a4=0.;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){
     bool calc=false;
     if (GetRandom()<=0.70)  PbAtShell(88);
     else PbAtShell(15);

     p=100*GetRandom();
     if (p<=98.13){
        fEgamma=fEnGamma[0];
        cK=3.51e-3; cL=0.49e-3; cM=0.13e-3; cp=2.5e-4;
        calc=true;
        next57=true;
     }
     else{
        fEgamma=fEnGamma[1];
        cK=2.7e-3; cL=0.4e-3; cM=0.1e-3; cp=0.2e-4;
        calc=true;
        next898=true;
     }
     if (calc){
        p=GetRandom()*(cg+cK+cL+cM+cp);
        if (p<=cg) particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        else if (p<=cg+cK){
           particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(88);
        }
        else if (p<=cg+cK+cL){
           particle(3,fEgamma-fEbindeL,fEgamma-fEbindeL,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(15);
        }
        else if (p<=cg+cK+cL+cM){
           particle(3,fEgamma-fEbindeM,fEgamma-fEbindeM,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(3);
        }
        else pair(fEgamma-1.022);
     }
  }
  else if (pdecay<=fProbDecay[1]){
     p=GetRandom();
     if (p<=0.733) PbAtShell(88);
     else if (p<=0.931) PbAtShell(15);
     else PbAtShell(3);

     fThlev=0.80;
     fEgamma=fEnGamma[2];
     cK=9.42e-2, cL=2.47e-2, cM=0.73e-2;
     p=GetRandom()*(cg+cK+cL+cM);
     if (p<=cg){
        particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        npg1064=fNbPart;
     }
     else if (p<=cg+cK){
        particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
        npe1064=fNbPart;
        PbAtShell(88);
     }
     else if (p<=cg+cK+cL){
        particle(3,fEgamma-fEbindeL,fEgamma-fEbindeL,0.,pi,0.,twopi,fTclev,fThlev);
        npe1064=fNbPart;
        PbAtShell(15);
     }
     else {
        particle(3,fEgamma-fEbindeM,fEgamma-fEbindeM,0.,pi,0.,twopi,fTclev,fThlev);
        npe1064=fNbPart;
        PbAtShell(3);
     }
     next57=true;
  }
  else if (pdecay<=fProbDecay[2]){
     p=GetRandom();
     if (p<=0.7965) PbAtShell(88);
     else if (p<=0.9466)  PbAtShell(15);
     else PbAtShell(3);
     next57=true;
  }
  else {
     beta(fEndPoint[0],0.,0.);
     next57=true;
  }
  if (next898){
     p=100*GetRandom();
     if (p<=99.245){
        fEgamma=fEnGamma[3];
        cK=2.01e-2; cL=0.34e-2;
        p=GetRandom()*(cg+cK+cL);
        if (p<=cg) particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        else if (p<=cg+cK){
           particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(88);
        }
        else {
           particle(3,fEgamma-fEbindeL,fEgamma-fEbindeL,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(15);
        }
        return;
     }
     else{
        fEgamma=fEnGamma[4];
        cK=0.2850; cL=0.0486; cM=0.0151;
        p=GetRandom()*(cg+cK+cL+cM);
        if (p<=cg) particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        else if (p<=cg+cK){
           particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(88);
        }
        else if (p<=cg+cK+cL){
           particle(3,fEgamma-fEbindeL,fEgamma-fEbindeL,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(15);
        }
        else{
           particle(3,fEgamma-fEbindeM,fEgamma-fEbindeM,0.,pi,0.,twopi,fTclev,fThlev);
           PbAtShell(3);
        }
        next57=true;
     }
  }
  if (next57){
     fThlev=130.5e-12;
     fEgamma=fEnGamma[5];
     cK=1.55e-2; cL=0.45e-2; cM=0.15e-2;
     p=GetRandom()*(cg+cK+cL+cM);
     if (p<=cg){
        particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        npg570=fNbPart;
     }
     else if (p<=cg+cK){
        particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
        npe570=fNbPart;
        PbAtShell(88);
     }
     else if (p<=cg+cK+cL){
        particle(3,fEgamma-fEbindeL,fEgamma-fEbindeL,0.,pi,0.,twopi,fTclev,fThlev);
        npe570=fNbPart;
        PbAtShell(15);
     }
     else {
        particle(3,fEgamma-fEbindeM,fEgamma-fEbindeM,0.,pi,0.,twopi,fTclev,fThlev);
        npe570=fNbPart;
        PbAtShell(3);
     }
  }
  if (npg1064!=0 && npg570!=0){
     a2=0.231; a4=-0.023;
     np1064=npg1064;
     np570=npg570;
  }
  else if (npe1064!=0 && npg570!=0){
     a2=0.223; a4=-0.020;
     np1064=npe1064;
     np570=npg570;
  }
  else if (npg1064!=0 && npe570!=0){
     a2=0.275; a4=-0.012;
     np1064=npg1064;
     np570=npe570;
  }
  else if (npe1064!=0 && npe570!=0){
     a2=0.271; a4=-0.010;
     np1064=npe1064;
     np570=npe570;
  }
  else return;
  float phi1,phi2;
  float p2,p4;
  float ctet,stet1,stet2,ctet1,ctet2;
  float p1064=sqrt(pow(fPmoment[0][np1064],2)+pow(fPmoment[1][np1064],2)
      +pow(fPmoment[2][np1064],2));
  float p570=sqrt(pow(fPmoment[0][np570],2)+pow(fPmoment[1][np570],2)
      +pow(fPmoment[2][np570],2));
  do{
    phi1=twopi*GetRandom();
    ctet1=1.-2.*GetRandom();
    stet1=sqrt(1.-ctet1*ctet1);
    phi2=twopi*GetRandom();
    ctet2=1.-2.*GetRandom();
    stet2=sqrt(1.-ctet2*ctet2);
    ctet=ctet1*ctet2+stet1*stet2*cos(phi1-phi2);
    p2=(3.*ctet*ctet-1.)/2.;
    p4=(35.*pow(ctet,4)-30.*ctet*ctet+3.)/8.;
  }while(GetRandom()*(1.+abs(a2)+abs(a4))>1.+a2*p2+a4*p4);

  fPmoment[0][np1064]=p1064*stet1*cos(phi1);
  fPmoment[1][np1064]=p1064*stet1*sin(phi1);
  fPmoment[2][np1064]=p1064*ctet1;
  fPmoment[0][np570]=p570*stet2*cos(phi2);
  fPmoment[1][np570]=p570*stet2*sin(phi2);
  fPmoment[2][np570]=p570*ctet2;
  return;
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Bi210()
{
  // "Table of Isotopes", 7th ed., 1978).
  // VIT, 14.08.1992, 22.10.1995; 30.10.2006.
  // Update to NDS 99(2003)949 and empirical correction
  // to the beta shape, VIT, 28.10.2006.
  float pdecay, palfa;
  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){
     palfa=100.*GetRandom();
     if (palfa<=fProbAlpha[0]){
        particle(47,fEnAlpha[0],fEnAlpha[0],0.,pi,0.,twopi,0,0);
        fThlev=4.e-12;
        nucltransK(fEnGamma[0],fEbindeK,3.9e-1,0.);
        return;
     }
     else{
        particle(47,fEnAlpha[1],fEnAlpha[1],0.,pi,0.,twopi,0,0);
        fThlev=3.e-9;
        nucltransK(fEnGamma[1],fEbindeK,1.6e-1,0.);
        return;
     }
  }
  else beta1f(fEndPoint[0],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Bi212()
{
  // "Table of Isotopes", 7th ed., 1978).
  // Beta-alfa decays to 208Pb  are not considered (p=0.014%).
  // VIT, 4.08.1992, 22.10.1995.
  bool next400=false, next328=false, next727=false;
  float pdecay,p, palfa, pbeta;

  pdecay=100*GetRandom();
  if (pdecay<=fProbDecay[0]){  // 36% alfa to 208Tl
     palfa=100.*GetRandom();
     if (palfa<=fProbAlpha[0]){  // 1.10%
        particle(47,fEnAlpha[0],fEnAlpha[0],0.,pi,0.,twopi,0,0);
        p=100.*GetRandom();
        if (p<= 5.){
           nucltransK(fEnGamma[0],fEbindeK,2.8e-2,0.);
           return;
        }
        else if (p<=99.){
           nucltransK(fEnGamma[1],fEbindeK,0.18,0.);
           next400=true;
        }
        else{
           nucltransK(fEnGamma[2],fEbindeK,0.75,0.);
           next328=true;
        }
     }
     else if (palfa<=fProbAlpha[1]){  // 0.15%
        particle(47,fEnAlpha[1],fEnAlpha[1],0.,pi,0.,twopi,0,0);
        p=100.*GetRandom();
        if (p<=68.){
           nucltransK(fEnGamma[3],fEbindeK,0.14,0.);
           return;
        }
        else if (p<=87.){
           nucltransK(fEnGamma[4],fEbindeK,0.14,0.);
           next400=true;
        }
        else{
           nucltransK(fEnGamma[5],fEbindeK,2.8,0.);
           next328=true;
        }
     }
     else if (palfa<=fProbAlpha[2]){  // 1.67%
        particle(47,fEnAlpha[2],fEnAlpha[2],0.,pi,0.,twopi,0,0);
        next328=true;
     }
     else if (palfa<=fProbAlpha[3]){ // 69.88%
        particle(47,fEnAlpha[3],fEnAlpha[3],0.,pi,0.,twopi,0,0);
        next400=true;
     }
     else{                  // 27.20%
        particle(47,fEnAlpha[4],fEnAlpha[4],0.,pi,0.,twopi,0,0);
        return;
     }

     if (next328){
        p=100.*GetRandom();
        if (p<=29.){
           nucltransK(fEnGamma[6],fEbindeK,0.33,0.);
           return;
        }
        else{
           nucltransK(fEnGamma[7],fEbindeK,0.53,0.);
           next400=true;
        }
     }
     if (next400){
        fThlev=6.e-12;
        nucltransK(fEnGamma[8],0.015,22.55,0.);
        return;
     }
  }
  else{                //64% beta to 212Po
     pbeta=64.*GetRandom();
     if (pbeta<=fProbBeta[0]){ // 0.660%
        beta(fEndPoint[0],0.,0.);
        p=100*GetRandom();
        if (p<=17.){
           nucltransK(fEnGamma[9],fEbindeK2,2.6e-2,1.7e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[10],fEbindeK2,2.0e-2,0.);
           next727=true;
        }
     }
     else if (pbeta<=fProbBeta[1]){//0.027%
        beta(fEndPoint[1],0.,0.);
        p=100*GetRandom();
        if (p<=35.){
           particle(3,1.708,1.708,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
           return;
        }
        else{
           nucltransK(fEnGamma[11],fEbindeK2,7.0e-3,0.);
           next727=true;
        }
     }
     else if (pbeta<=fProbBeta[2]){ //0.250%
        beta(fEndPoint[2],0.,0.);
        p=100*GetRandom();
        if (p<=28.){
           nucltransK(fEnGamma[12],fEbindeK2,2.8e-3,1.0e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[13],fEbindeK2,4.5e-2,0.);
           next727=true;
        }
     }
     else if (pbeta<=fProbBeta[3]){ //1.900%
        beta(fEndPoint[3],0.,0.);
        p=100*GetRandom();
        if (p<=80.){
           nucltransK(fEnGamma[14],fEbindeK2,7.0e-3,1.2e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[15],fEbindeK2,4.5e-2,0.);
           next727=true;
        }
     }
     else if (pbeta<=fProbBeta[4]){ //1.500%
        beta(fEndPoint[4],0.,0.);
        p=100*GetRandom();
        if (p<=22.){
           nucltransK(fEnGamma[16],fEbindeK2,3.5e-3,0.7e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[17],fEbindeK2,4.1e-2,0.);
           next727=true;
        }
     }
     else if (pbeta<=fProbBeta[5]){ //4.400%
        beta(fEndPoint[5],0.,0.);
        next727=true;
     }
     else{                   // 55.263%
        beta(fEndPoint[6],0.,0.);
        return;
     }

     if (next727){
        nucltransK(fEnGamma[18],fEbindeK2,1.7e-2,0.);
        return;
     }
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Bi214()
{
  // "Table of Isotopes", 7th ed., 1978.
  // Beta-alfa decays to 210Pb  are not considered (p=2.8e-3%).
  // VIT, 13.08.1992, 22.10.1995.
  // VIT, 31.05.2005, updated to NDS 99(2003)649.
  // Not well known alpha decays to 210Tl levels with E>253 keV
  // are omitted.
  bool next630=false, next609=false, next1015=false, next1275=false;
  bool next1378=false, next1415=false, next1543=false, next1661=false;
  bool next1713=false, next1730=false, next1743=false, next1764=false;
  bool next2448=false, next2266=false, next2209=false, next2204=false;
  bool next2119=false, next2088=false, next2011=false, next2017=false;
  bool next1995=false, next1847=false;
  float p, pdecay, palfa;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){       // 0.021% alfa to 210Tl
     palfa=100.*GetRandom();
     if (palfa<=fProbAlpha[0]){   // 5.86%
        particle(47,fEnAlpha[0],fEnAlpha[0],0.,pi,0.,twopi,0,0);
        nucltransK(fEnGamma[0],fEbindeK,1.3,0.);
        next630=true;
     }
     else if (palfa<=fProbAlpha[1]){ // 54.50%
        particle(47,fEnAlpha[1],fEnAlpha[1],0.,pi,0.,twopi,0,0);
        next630=true;
     }
     else{                   // 39.64%
        particle(47,fEnAlpha[2],fEnAlpha[2],0.,pi,0.,twopi,0,0);
        return;
     }
     if (next630){
        nucltransK(fEnGamma[1],0.015,6.48,0.);
        return;
     }
  }
  else{                    // 99.979% beta to 214Po
     float pbeta=100.*GetRandom();
     if (pbeta<=fProbBeta[0]){
        beta(fEndPoint[0],0.,0.);
        nucltransK(fEnGamma[2],fEbindeK2,4.0e-4,8.0e-4);
        return;
     }
     else if (pbeta<= fProbBeta[1]){ // 0.001%
        beta(fEndPoint[1],0.,0.);
        p=100.*GetRandom();
        if (p<=41.){
           nucltransK(fEnGamma[3],fEbindeK2,4.0e-4,8.0e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[4],fEbindeK2,6.0e-4,4.6e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[2]){ // 0.001%
        beta(fEndPoint[2],0.,0.);
        nucltransK(fEnGamma[5],fEbindeK2,4.0e-4,8.0e-4);
        return;
     }
     else if (pbeta<= fProbBeta[3]){ // 0.005%
        beta(fEndPoint[3],0.,0.);
        nucltransK(fEnGamma[6],fEbindeK2,4.2e-4,8.0e-4);
        return;
     }
     else if (pbeta<= fProbBeta[4]){ // 0.004%
        beta(fEndPoint[4],0.,0.);
        p=100.*GetRandom();
        if (p<=60.0){
           nucltransK(fEnGamma[7],fEbindeK2,1.0e-3,8.0e-4);
           return;
        }
        else if (p<=82.9){
           nucltransK(fEnGamma[8],fEbindeK2,1.5e-3,5.2e-4);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[9],fEbindeK2,3.5e-3,0.5e-4);
           next1415=true;
        }
     }
     else if (pbeta<= fProbBeta[5]){ // 0.005%
        beta(fEndPoint[5],0.,0.);
        p=100.*GetRandom();
        if (p<=0.9){
           nucltransK(fEnGamma[10],fEbindeK2,3.0e-3,4.1e-4);
           next609=true;
        }
        else if (p<=27.8){
           nucltransK(fEnGamma[11],fEbindeK2,7.0e-3,1.2e-4);
           next1378=true;
        }
        else if (p<=41.7){
           nucltransK(fEnGamma[12],fEbindeK2,8.0e-3,1.1e-4);
           next1415=true;
        }
        else if (p<=61.8){
           nucltransK(fEnGamma[13],fEbindeK2,9.0e-3,0.9e-4);
           next1543=true;
        }
        else{
           nucltransK(fEnGamma[14],fEbindeK2,1.2e-2,0.5e-4);
           next1730=true;
        }
     }
     else if (pbeta<= fProbBeta[6]){ // 0.007%
        beta(fEndPoint[6],0.,0.);
        nucltransK(fEnGamma[15],fEbindeK2,1.8e-2,3.0e-6);
        next1847=true;
     }
     else if (pbeta<= fProbBeta[7]){ // 0.010%
        beta(fEndPoint[7],0.,0.);
        p=100.*GetRandom();
        if (p<=84.6){
           nucltransK(fEnGamma[16],fEbindeK2,1.4e-3,6.5e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[17],fEbindeK2,2.8e-3,4.0e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[8]){ // 0.009%
        beta(fEndPoint[8],0.,0.);
        p=100.*GetRandom();
        if (p<=83.0){
           nucltransK(fEnGamma[18],fEbindeK2,2.8e-3,4.0e-4);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[19],fEbindeK2,6.0e-3,1.3e-4);
           next1275=true;
        }
     }
     else if (pbeta<= fProbBeta[9]){ // 0.017%
        beta(fEndPoint[9],0.,0.);
        p=100.*GetRandom();
        if (p<=83.6){
           nucltransK(fEnGamma[20],fEbindeK2,1.4e-3,6.5e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[21],fEbindeK2,2.8e-3,4.0e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[10]){ // 0.042%
        beta(fEndPoint[10],0.,0.);
        p=100.*GetRandom();
        if (p<=52.1){
           nucltransK(fEnGamma[22],fEbindeK2,3.0e-3,3.8e-4);
           next609=true;
        }
        else if (p<=71.7){
           nucltransK(fEnGamma[23],fEbindeK2,7.0e-3,1.1e-4);
           next1275=true;
        }
        else{
           nucltransK(fEnGamma[24],fEbindeK2,1.2e-2,1.5e-5);
           next1661=true;
        }
     }
     else if (pbeta<= fProbBeta[11]){ // 0.002%
        beta(fEndPoint[11],0.,0.);
        p=100.*GetRandom();
        if (p<=21.3){
           nucltransK(fEnGamma[25],fEbindeK2,1.5e-3,6.3e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[26],fEbindeK2,3.0e-3,3.7e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[12]){ // 0.001%
        beta(fEndPoint[12],0.,0.);
        p=100.*GetRandom();
        if (p<=73.3){
           nucltransK(fEnGamma[27],fEbindeK2,1.5e-3,6.2e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[28],fEbindeK2,3.0e-3,3.7e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[13]){ // 0.023%
        beta(fEndPoint[13],0.,0.);
        p=100.*GetRandom();
        if (p<=60.9){
           nucltransK(fEnGamma[29],fEbindeK2,4.5e-4,8.0e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[30],fEbindeK2,3.0e-3,3.7e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[14]){ // 0.001%
        beta(fEndPoint[14],0.,0.);
        nucltransK(fEnGamma[31],fEbindeK2,3.0e-3,3.7e-4);
        next609=true;
     }
     else if (pbeta<= fProbBeta[15]){ // 0.005%
        beta(fEndPoint[15],0.,0.);
        nucltransK(fEnGamma[32],fEbindeK2,3.0e-3,3.6e-4);
        next609=true;
     }
     else if (pbeta<= fProbBeta[16]){ // 0.011%
        beta(fEndPoint[16],0.,0.);
        p=100.*GetRandom();
        if (p<=26.1){
           nucltransK(fEnGamma[33],fEbindeK2,1.7e-3,6.1e-4);
           return;
        }
        else if (p<=48.3){
           nucltransK(fEnGamma[34],fEbindeK2,3.0e-3,3.6e-4);
           next609=true;
        }
        else if (p<=78.3){
           nucltransK(fEnGamma[35],fEbindeK2,9.0e-3,6.7e-5);
           next1378=true;
        }
        else{
           nucltransK(fEnGamma[36],fEbindeK2,7.5e-2,0.);
           next2266=true;
        }
     }
     else if (pbeta<= fProbBeta[17]){ // 0.011%
        beta(fEndPoint[17],0.,0.);
        p=100.*GetRandom();
        if (p<=87.6){
           nucltransK(fEnGamma[37],fEbindeK2,1.7e-3,6.0e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[38],fEbindeK2,3.0e-3,3.5e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[18]){ // 0.014%
        beta(fEndPoint[18],0.,0.);
        p=100.*GetRandom();
        if (p<=63.5){
           nucltransK(fEnGamma[39],fEbindeK2,3.0e-3,3.5e-4);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[40],fEbindeK2,8.0e-3,9.1e-5);
           next1275=true;
        }
     }
     else if (pbeta<= fProbBeta[19]){ // 0.014%
        beta(fEndPoint[19],0.,0.);
        p=100.*GetRandom();
        if (p<=2.7){
           nucltransK(fEnGamma[41],fEbindeK2,1.8e-3,6.0e-4);
           return;
        }
        else if (p<=41.5){
           nucltransK(fEnGamma[42],fEbindeK2,3.5e-3,3.4e-4);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[43],fEbindeK2,2.2e-2,0.);
           next1847=true;
        }
     }
     else if (pbeta<= fProbBeta[20]){ // 0.002%
        beta(fEndPoint[20],0.,0.);
        nucltransK(fEnGamma[44],fEbindeK2,1.8e-3,5.8e-4);
        return;
     }
     else if (pbeta<= fProbBeta[21]){ // 0.036%
        beta(fEndPoint[21],0.,0.);
        p=100.*GetRandom();
        if (p<=15.4){
           nucltransK(fEnGamma[45],fEbindeK2,1.8e-3,5.7e-4);
           return;
        }
        else if (p<=24.4){
           nucltransK(fEnGamma[46],fEbindeK2,3.5e-3,3.1e-4);
           next609=true;
        }
        else if (p<=63.6){
           nucltransK(fEnGamma[47],fEbindeK2,1.2e-2,0.);
           next1764=true;
        }
        else{
           nucltransK(fEnGamma[48],fEbindeK2,1.4e-2,0.);
           next1847=true;
        }
     }
     else if (pbeta<= fProbBeta[22]){ // 0.036%
        beta(fEndPoint[22],0.,0.);
        p=100.*GetRandom();
        if (p<=54.6){
           nucltransK(fEnGamma[49],fEbindeK2,1.8e-3,5.6e-4);
           return;
        }
        else if (p<=58.5){
           nucltransK(fEnGamma[50],fEbindeK2,3.5e-3,3.1e-4);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[51],fEbindeK2,1.0e-2,3.6e-5);
           next1378=true;
        }
     }
     else if (pbeta<= fProbBeta[23]){ // 0.561%
        beta(fEndPoint[23],0.,0.);
        p=100.*GetRandom();
        if (p<=1.2){
           nucltransK(fEnGamma[52],fEbindeK2,3.5e-3,2.9e-4);
           next609=true;
        }
        else if (p<=6.0){
           nucltransK(fEnGamma[53],fEbindeK2,2.1e-2,1.4e-7);
           next1661=true;
        }
        else if (p<=71.5){
           nucltransK(fEnGamma[54],fEbindeK2,1.4e-2,0.);
           next1764=true;
        }
        else if (p<=79.4){
           nucltransK(fEnGamma[55],fEbindeK2,2.7e-2,0.);
           next1995=true;
        }
        else if (p<=82.6){
           nucltransK(fEnGamma[56],fEbindeK2,6.0e-2,0.);
           next2204=true;
        }
        else if (p<=85.7){
           nucltransK(fEnGamma[57],fEbindeK2,7.0e-2,0.);
           next2209=true;
        }
        else{
           nucltransK(fEnGamma[58],fEbindeK2,3.3e-1,0.);
           next2448=true;
        }
     }
     else if (pbeta<= fProbBeta[24]){ // 0.278%
        beta(fEndPoint[24],0.,0.);
        p=100.*GetRandom();
        if (p<=0.7){
           nucltransK(fEnGamma[59],fEbindeK2,2.0e-3,5.4e-4);
           return;
        }
        else if (p<=33.6){
           nucltransK(fEnGamma[60],fEbindeK2,3.5e-3,2.9e-4);
           next609=true;
        }
        else if (p<=41.8){
           nucltransK(fEnGamma[61],fEbindeK2,1.0e-2,2.6e-5);
           next1378=true;
        }
        else if (p<=83.6){
           nucltransK(fEnGamma[62],fEbindeK2,1.0e-2,1.9e-5);
           next1415=true;
        }
        else if (p<=90.7){
           nucltransK(fEnGamma[63],fEbindeK2,2.6e-2,0.);
           next1743=true;
        }
        else if (p<=97.0){
           nucltransK(fEnGamma[64],fEbindeK2,2.9e-2,0.);
           next2011=true;
        }
        else{
           nucltransK(fEnGamma[65],fEbindeK2,9.0e-2,0.);
           next2119=true;
        }
     }
     else if (pbeta<= fProbBeta[25]){ // 0.054%
        beta(fEndPoint[25],0.,0.);
        p=100.*GetRandom();
        if (p<=5.3){
           nucltransK(fEnGamma[66],fEbindeK2,2.0e-3,5.3e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[67],fEbindeK2,4.0e-3,2.8e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[26]){ // 0.053%
        beta(fEndPoint[26],0.,0.);
        p=100.*GetRandom();
        if (p<=29.5){
           nucltransK(fEnGamma[68],fEbindeK2,1.6e-2,2.9e-6);
           next1543=true;
        }
        else if (p<=44.8){
           nucltransK(fEnGamma[69],fEbindeK2,2.0e-2,0.);
           next1661=true;
        }
        else if (p<=63.3){
           nucltransK(fEnGamma[70],fEbindeK2,2.8e-2,0.);
           next1764=true;
        }
        else if (p<=76.0){
           nucltransK(fEnGamma[71],fEbindeK2,6.5e-2,0.);
           next2011=true;
        }
        else{
           nucltransK(fEnGamma[72],fEbindeK2,7.0e-2,0.);
           next2204=true;
        }
     }
     else if (pbeta<= fProbBeta[27]){ // 0.263%
        beta(fEndPoint[27],0.,0.);
        p=100.*GetRandom();
        if (p<=11.9){
           nucltransK(fEnGamma[73],fEbindeK2,2.0e-3,5.3e-4);
           return;
        }
        else if (p<=15.4){
           nucltransK(fEnGamma[74],fEbindeK2,4.0e-3,2.8e-4);
           next609=true;
        }
        else if (p<=17.4){
           nucltransK(fEnGamma[75],fEbindeK2,9.5e-3,4.2e-5);
           next1275=true;
        }
        else if (p<=48.7){
           nucltransK(fEnGamma[76],fEbindeK2,1.2e-2,2.1e-5);
           next1378=true;
        }
        else if (p<=57.3){
           nucltransK(fEnGamma[77],fEbindeK2,2.2e-2,0.);
           next1661=true;
        }
        else if (p<=59.6){
           nucltransK(fEnGamma[78],fEbindeK2,2.8e-2,0.);
           next1743=true;
        }
        else if (p<=72.3){
           nucltransK(fEnGamma[79],fEbindeK2,1.5e-2,0.);
           next1764=true;
        }
        else if (p<=82.3){
           nucltransK(fEnGamma[80],fEbindeK2,1.9e-2,0.);
           next1847=true;
        }
        else if (p<=88.4){
           nucltransK(fEnGamma[81],fEbindeK2,3.0e-2,0.);
           next1995=true;
        }
        else if (p<=90.7){
           nucltransK(fEnGamma[82],fEbindeK2,7.5e-2,0.);
           next2017=true;
        }
        else{
           nucltransK(fEnGamma[83],fEbindeK2,8.0e-2,0.);
           next2209=true;
        }
     }
     else if (pbeta<= fProbBeta[28]){ // 0.127%
        beta(fEndPoint[28],0.,0.);
        p=100.*GetRandom();
        if (p<=0.2){
           nucltransK(fEnGamma[84],fEbindeK2,2.0e-3,5.1e-4);
           return;
        }
        else if (p<=55.0){
           nucltransK(fEnGamma[85],fEbindeK2,4.0e-3,2.6e-4);
           next609=true;
        }
        else if (p<=63.7){
           nucltransK(fEnGamma[86],fEbindeK2,1.3e-2,1.6e-5);
           next1378=true;
        }
        else if (p<=95.6){
           nucltransK(fEnGamma[87],fEbindeK2,1.9e-2,1.2e-6);
           next1543=true;
        }
        else{
           nucltransK(fEnGamma[88],fEbindeK2,2.8e-2,0.);
           next1713=true;
        }
     }
     else if (pbeta<= fProbBeta[29]){ // 0.021%
        beta(fEndPoint[29],0.,0.);
        p=100.*GetRandom();
        if (p<=3.8){
           nucltransK(fEnGamma[89],fEbindeK2,2.2e-3,5.0e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[90],fEbindeK2,4.0e-3,2.5e-4);
           next609=true;
        }
     }
     else if (pbeta<= fProbBeta[30]){ // 0.084%
        beta(fEndPoint[30],0.,0.);
        p=100.*GetRandom();
        if (p<=0.5){
           nucltransK(fEnGamma[91],fEbindeK2,2.2e-3,4.9e-4);
           return;
        }
        else if (p<=7.1){
           nucltransK(fEnGamma[92],fEbindeK2,4.5e-3,2.4e-4);
           next609=true;
        }
        else if (p<=21.7){
           nucltransK(fEnGamma[93],fEbindeK2,1.1e-2,2.4e-5);
           next1275=true;
        }
        else if (p<=44.2){
           nucltransK(fEnGamma[94],fEbindeK2,2.8e-2,0.);
           next1661=true;
        }
        else if (p<=56.1){
           nucltransK(fEnGamma[95],fEbindeK2,4.0e-2,0.);
           next1764=true;
        }
        else{
           nucltransK(fEnGamma[96],fEbindeK2,1.3e-1,0.);
           next2209=true;
        }
     }
     else if (pbeta<= fProbBeta[31]){ // 0.053%
        beta(fEndPoint[31],0.,0.);
        p=100.*GetRandom();
        if (p<=77.4){
           nucltransK(fEnGamma[97],fEbindeK2,3.0e-3,2.1e-4);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[98],fEbindeK2,1.6e-2,3.6e-6);
           next1378=true;
        }
     }
     else if (pbeta<= fProbBeta[32]){ // 0.140%
        beta(fEndPoint[32],0.,0.);
        p=100.*GetRandom();
        if (p<=49.8){
           nucltransK(fEnGamma[99],fEbindeK2,5.0e-3,2.0e-4);
           next609=true;
        }
        else if (p<=84.8){
           nucltransK(fEnGamma[100],fEbindeK2,1.8e-2,1.6e-6);
           next1378=true;
        }
        else if (p<=93.5){
           nucltransK(fEnGamma[101],fEbindeK2,2.6e-2,0.);
           next1543=true;
        }
        else{
           nucltransK(fEnGamma[102],fEbindeK2,7.0e-2,0.);
           next2011=true;
        }
     }
     else if (pbeta<= fProbBeta[33]){ // 0.193%
        beta(fEndPoint[33],0.,0.);
        p=100.*GetRandom();
        if (p<=3.0){
           nucltransK(fEnGamma[103],fEbindeK2,2.4e-3,4.5e-4);
           return;
        }
        else if (p<=86.0){
           nucltransK(fEnGamma[104],fEbindeK2,5.0e-3,2.0e-4);
           next609=true;
        }
        else if (p<=93.8){
           nucltransK(fEnGamma[105],fEbindeK2,1.4e-2,9.2e-6);
           next1275=true;
        }
        else{
           nucltransK(fEnGamma[106],fEbindeK2,2.6e-2,0.);
           next1543=true;
        }
     }
     else if (pbeta<= fProbBeta[34]){ // 1.332%
        beta(fEndPoint[34],0.,0.);
        p=100.*GetRandom();
        if (p<=0.1){
           nucltransK(fEnGamma[107],fEbindeK2,2.5e-3,4.4e-4);
           return;
        }
        else if (p<=16.7){
           nucltransK(fEnGamma[108],fEbindeK2,5.0e-3,1.9e-4);
           next609=true;
        }
        else if (p<=51.0){
           nucltransKLM(fEnGamma[109],fEbindeK2,1.6e-3,fEbindeL2,2.5e-4,fEbindeM2,8.0e-5,6.4e-5);
           next1275=true;
        }
        else if (p<=56.9){
           nucltransK(fEnGamma[110],fEbindeK2,1.0e-2,7.8e-7);
           next1378=true;
        }
        else if (p<=58.3){
           nucltransKLM(fEnGamma[111],fEbindeK2,1.4e-2,fEbindeL2,2.5e-3,fEbindeM2,5.0e-4,0.);
           next1543=true;
        }
        else if (p<=70.7){
           nucltransK(fEnGamma[112],fEbindeK2,3.7e-2,0.);
           next1661=true;
        }
        else if (p<=80.9){
           nucltransKLM(fEnGamma[113],fEbindeK2,2.4e-2,fEbindeL2,4.4e-3,fEbindeM2,6.0e-4,0.);
           next1730=true;
        }
        else if (p<=81.4){
           nucltransKLM(fEnGamma[114],fEbindeK2,3.6e-2,fEbindeL2,7.0e-3,fEbindeM2,7.0e-3,0.);
           next1847=true;
        }
        else if (p<=83.5){
           nucltransKLM(fEnGamma[115],fEbindeK2,8.9e-3,fEbindeL2,1.5e-3,fEbindeM2,4.5e-4,0.);
           next1995=true;
        }
        else if (p<=84.8){
           nucltransK(fEnGamma[116],fEbindeK2,1.3e-1,0.);
           next2088=true;
        }
        else{
           nucltransK(fEnGamma[117],fEbindeK2,3.6e-1,0.);
           next2209=true;
        }
     }
     else if (pbeta<= fProbBeta[35]){ // 2.815%
        beta(fEndPoint[35],0.,0.);
        next2448=true;
     }
     else if (pbeta<= fProbBeta[36]){ // 0.080%
        beta(fEndPoint[36],0.,0.);
        p=100.*GetRandom();
        if (p<=7.2){
           nucltransK(fEnGamma[118],fEbindeK2,6.5e-4,9.2e-4);
           return;
        }
        else if (p<=24.5){
           nucltransK(fEnGamma[119],fEbindeK2,1.2e-3,5.1e-4);
           next609=true;
        }
        else if (p<=65.4){
           nucltransK(fEnGamma[120],fEbindeK2,2.8e-3,1.4e-6);
           next1378=true;
        }
        else if (p<=674.8){
           nucltransK(fEnGamma[121],fEbindeK2,5.7e-3,0.);
           next1730=true;
        }
        else{
           nucltransK(fEnGamma[122],fEbindeK2,3.5e-2,0.);
           next1764=true;
        }
     }
     else if (pbeta<= fProbBeta[37]){ // 0.560%
        beta(fEndPoint[37],0.,0.);
        p=100.*GetRandom();
        if (p<=54.6){
           nucltransK(fEnGamma[123],fEbindeK2,3.0e-3,3.6e-4);
           return;
        }
        else if (p<=93.2){
           nucltransK(fEnGamma[124],fEbindeK2,7.0e-3,1.2e-4);
           next609=true;
        }
        else if (p<=97.9){
           nucltransK(fEnGamma[125],fEbindeK2,1.8e-2,0.);
           next1378=true;
        }
        else{
           nucltransK(fEnGamma[126],fEbindeK2,2.0e-2,0.);
           next1415=true;
        }
     }
     else if (pbeta<= fProbBeta[38]){ // 0.201%
        beta(fEndPoint[38],0.,0.);
        next2266=true;
     }
     else if (pbeta<=fProbBeta[39]){ // 5.729%
        beta(fEndPoint[39],0.,0.);
        next2204=true;
     }
     else if (pbeta<=fProbBeta[40]){ // 0.833%
        beta(fEndPoint[40],0.,0.);
        p=100.*GetRandom();
        if (p<=4.1){
           nucltransK(fEnGamma[127],fEbindeK2,2.0e-3,4.1e-4);
           return;
        }
        else if (p<=87.2){
           nucltransK(fEnGamma[128],fEbindeK2,5.4e-3,8.7e-5);
           next609=true;
        }
        else if (p<=87.8){
           nucltransKLM(fEnGamma[129],fEbindeK2,2.6e-3,fEbindeL2,4.1e-4,fEbindeM2,1.4e-4,0.);
           next1275=true;
        }
        else if (p<=92.5){
           nucltransKLM(fEnGamma[130],fEbindeK2,1.9e-2,fEbindeL2,3.6e-3,fEbindeM2,1.4e-3,0.);
           next1378=true;
        }
        else{
           nucltransKLM(fEnGamma[131],fEbindeK2,3.4e-2,fEbindeL2,6.0e-3,fEbindeM2,3.0e-3,0.);
           next1543=true;
        }
     }
     else if (pbeta<=fProbBeta[41]){ // 0.421%
        beta(fEndPoint[41],0.,0.);
        p=100.*GetRandom();
        if (p<=3.2){
           nucltransK(fEnGamma[132],fEbindeK2,3.5e-3,3.0e-4);
           return;
        }
        else if (p<=89.0){
           nucltransK(fEnGamma[133],fEbindeK2,8.0e-3,1.0e-4);
           next609=true;
        }
        else if (p<=93.1){
           nucltransK(fEnGamma[134],fEbindeK2,3.5e-2,0.);
           next1275=true;
        }
        else{
           nucltransK(fEnGamma[135],fEbindeK2,4.5e-2,0.);
           next1378=true;
        }
     }
     else if (pbeta<=fProbBeta[42]){ // 4.267%
        beta(fEndPoint[42],0.,0.);
        next2119=true;
     }
     else if (pbeta<=fProbBeta[43]){ // 0.096%
        beta(fEndPoint[43],0.,0.);
        next2088=true;
     }
     else if (pbeta<=fProbBeta[44]){ // 2.204%
        beta(fEndPoint[44],0.,0.);
        next2017=true;
     }
     else if (pbeta<=fProbBeta[45]){ // 1.372%
        beta(fEndPoint[45],0.,0.);
        next2011=true;
     }
     else if (pbeta<=fProbBeta[46]){ // 1.142%
        beta(fEndPoint[46],0.,0.);
        next1995=true;
     }
     else if (pbeta<=fProbBeta[47]){ // 1.593%
        beta(fEndPoint[47],0.,0.);
        p=100.*GetRandom();
        if (p<=5.0){
           nucltransK(fEnGamma[136],fEbindeK2,2.8e-3,2.6e-4);
           return;
        }
        else if (p<=96.2){
           nucltransKLM(fEnGamma[137],fEbindeK2,9.5e-3,fEbindeL2,1.6e-3,fEbindeM2,5.5e-4,1.6e-5);
           next609=true;
        }
        else{
           nucltransKLM(fEnGamma[138],fEbindeK2,5.6e-3,fEbindeL2,9.0e-4,fEbindeM2,3.0e-4,0.);
           next1275=true;
        }
     }
     else if (pbeta<=fProbBeta[48]){ // 8.193%
        beta(fEndPoint[48],0.,0.);
        next1847=true;
     }
     else if (pbeta<=fProbBeta[49]){ // 17.073%
        beta(fEndPoint[49],0.,0.);
        next1764=true;
     }
     else if (pbeta<=fProbBeta[50]){ // 0.113%
        beta(fEndPoint[50],0.,0.);
        next1743=true;
     }
     else if (pbeta<=fProbBeta[51]){ // 17.854%
        beta(fEndPoint[51],0.,0.);
        next1730=true;
     }
     else if (pbeta<=fProbBeta[52]){ // 0.150%
        beta(fEndPoint[52],0.,0.);
        next1713=true;
     }
     else if (pbeta<=fProbBeta[53]){ // 0.721%
        beta(fEndPoint[53],0.,0.);
        next1661=true;
     }
     else if (pbeta<=fProbBeta[54]){ // 2.955%
        beta(fEndPoint[54],0.,0.);
        next1543=true;
     }
     else if (pbeta<=fProbBeta[55]){ // 0.811%
        beta(fEndPoint[55],0.,0.);
        next1415=true;
     }
     else if (pbeta<=fProbBeta[56]){ // 7.442%
        beta(fEndPoint[56],0.,0.);
        next1378=true;
     }
     else if (pbeta<=fProbBeta[57]){ // 1.703%
        beta(fEndPoint[57],0.,0.);
        next609=true;
     }
     else{                   // 18.255%
        beta(fEndPoint[58],0.,0.);
        return;
     }

     if (next2266){
        p=100.*GetRandom();
        if (p<=9.0){
           nucltransK(fEnGamma[139],fEbindeK2,1.8e-3,4.4e-4);
        }
        else if (p<=31.9){
           nucltransK(fEnGamma[140],0.093,1.3e-3,3.9e-4);
           next609=true;
        }
        else if (p<=36.9){
           nucltransK(fEnGamma[141],0.093,1.3e-2,0.);
           next1275=true;
        }
        else if (p<=54.8){
           nucltransK(fEnGamma[142],0.093,2.8e-2,0.);
           next1543=true;
        }
        else if (p<=90.6){
           nucltransK(fEnGamma[143],0.093,6.0e-2,0.);
           next1730=true;
        }
        else{
           nucltransK(fEnGamma[144],0.093,7.0e-2,0.);
           next1764=true;
        }
     }
     if (next2448){
        p=100.*GetRandom();
        if (p<=54.3){
           nucltransK(fEnGamma[145],fEbindeK2,6.0e-4,9.4e-4);
           return;
        }
        else if (p<=66.8){
           nucltransK(fEnGamma[146],fEbindeK2,1.2e-3,5.3e-4);
           next609=true;
        }
        else if (p<=68.6){
           nucltransKLM(fEnGamma[147],fEbindeK2,4.4e-3,fEbindeL2,8.3e-4,fEbindeM2,2.8e-4,6.2e-6);
           next1275=true;
        }
        else if (p<=78.1){
           nucltransKLM(fEnGamma[148],fEbindeK2,2.0e-3,fEbindeL2,3.1e-4,fEbindeM2,1.0e-4,5.5e-6);
           next1378=true;
        }
        else if (p<=80.8){
           nucltransKLM(fEnGamma[149],fEbindeK2,2.1e-3,fEbindeL2,3.3e-4,fEbindeM2,1.1e-4,0.);
           next1415=true;
        }
        else if (p<=83.7){
           nucltransKLM(fEnGamma[150],fEbindeK2,2.7e-3,fEbindeL2,4.2e-4,fEbindeM2,1.4e-4,0.);
           next1543=true;
        }
        else if (p<=94.4){
           nucltransKLM(fEnGamma[151],fEbindeK2,3.5e-3,fEbindeL2,5.5e-4,fEbindeM2,1.9e-4,0.);
           next1661=true;
        }
        else if (p<=96.0){
           nucltransKLM(fEnGamma[152],fEbindeK2,4.3e-3,fEbindeL2,6.9e-4,fEbindeM2,2.2e-4,0.);
           next1743=true;
        }
        else if (p<=98.8){
           nucltransKLM(fEnGamma[153],fEbindeK2,4.6e-3,fEbindeL2,7.3e-4,fEbindeM2,2.4e-4,0.);
           next1764=true;
        }
        else{
           nucltransKLM(fEnGamma[154],fEbindeK2,8.0e-2,fEbindeL2,1.7e-2,fEbindeM2,5.6e-3,0.);
           next1995=true;
        }
     }
     if (next2209){
        p=100.*GetRandom();
        if (p<=82.1){
           nucltransK(fEnGamma[155],fEbindeK2,8.0e-3,9.2e-5);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[156],fEbindeK2,2.8e-2,0.);
           next1275=true;
        }
     }
     if (next2204){
        p=100.*GetRandom();
        if (p<=87.9){
           nucltransK(fEnGamma[157],fEbindeK2,3.5e-3,3.2e-4);
           return;
        }
        else if (p<=92.2){
           nucltransK(fEnGamma[158],fEbindeK2,5.5e-3,9.1e-5);
           next609=true;
        }
        else if (p<=94.2){
           nucltransKLM(fEnGamma[159],fEbindeK2,2.9e-2,fEbindeL2,5.0e-3,fEbindeM2,2.0e-3,0.);
           next1378=true;
        }
        else if (p<=94.5){
           nucltransKLM(fEnGamma[160],fEbindeK2,3.3e-2,fEbindeL2,5.7e-3,fEbindeM2,1.9e-3,0.);
           next1415=true;
        }
        else if (p<=95.3){
           nucltransKLM(fEnGamma[161],fEbindeK2,3.3e-2,fEbindeL2,6.0e-3,fEbindeM2,2.0e-3,0.);
           next1543=true;
        }
        else if (p<=96.9){
           nucltransKLM(fEnGamma[162],fEbindeK2,5.0e-2,fEbindeL2,1.1e-2,fEbindeM2,9.0e-3,0.);
           next1713=true;
        }
        else if (p<=99.0){
           nucltransKLM(fEnGamma[163],fEbindeK2,7.0e-2,fEbindeL2,1.5e-2,fEbindeM2,4.9e-3,0.);
           next1730=true;
        }
        else{
           nucltransKLM(fEnGamma[164],fEbindeK2,1.4e-1,fEbindeL2,2.3e-2,fEbindeM2,7.4e-3,0.);
           next1743=true;
        }
     }
     if (next2119){
        p=100.*GetRandom();
        if (p<=26.7){
           nucltransK(fEnGamma[165],fEbindeK2,3.5e-3,2.9e-4);
           return;
        }
        else if (p<=76.4){
           nucltransK(fEnGamma[166],fEbindeK2,6.3e-3,6.5e-5);
           next609=true;
        }
        else if (p<=77.3){
           nucltransK(fEnGamma[167],fEbindeK2,3.1e-2,0.);
           next1378=true;
        }
        else if (p<=89.0){
           nucltransKLM(fEnGamma[168],fEbindeK2,4.5e-2,fEbindeL2,7.6e-3,fEbindeM2,2.6e-3,0.);
           next1415=true;
        }
        else{
           nucltransKLM(fEnGamma[169],fEbindeK2,2.1e-1,fEbindeL2,3.7e-2,fEbindeM2,1.2e-2,0.);
           next1730=true;
        }
     }
     if (next2088){
        p=100.*GetRandom();
        if (p<=40.5){
           nucltransK(fEnGamma[170],fEbindeK2,9.0e-3,5.7e-5);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[171],fEbindeK2,6.0e-2,0.);
           next1378=true;
        }
     }
     if (next2017){
        p=100.*GetRandom();
        if (p<=0.27){
           p=100.*GetRandom();
           if (p<=95.08){
              particle(3,1.923,1.923,0.,pi,0.,twopi,fTclev,fThlev);
              particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
           }
           else pair(0.995);
           return;
        }
        else if (p<=98.01){
           nucltransKLM(fEnGamma[172],fEbindeK2,3.1e-3,fEbindeL2,5.7e-4,fEbindeM2,1.9e-4,5.7e-5);
           next609=true;
        }
        else if (p<=99.41){
           nucltransKLM(fEnGamma[173],fEbindeK2,1.4e-2,fEbindeL2,3.7e-3,fEbindeM2,1.2e-3,0.);
           next1378=true;
        }
        else if (p<=99.75){
           nucltransKLM(fEnGamma[174],fEbindeK2,4.6e-2,fEbindeL2,2.4e-2,fEbindeM2,8.1e-3,0.);
           next1661=true;
        }
        else{
           nucltransKLM(fEnGamma[175],fEbindeK2,6.9e-1,fEbindeL2,1.2e-1,fEbindeM2,3.8e-2,0.);
           next1764=true;
        }
     }
     if (next2011){
        p=100.*GetRandom();
        if (p<=3.4){
           nucltransK(fEnGamma[176],fEbindeK2,2.2e-3,3.2e-4);
           return;
        }
        else if (p<=94.7){
           nucltransKLM(fEnGamma[177],fEbindeK2,4.4e-3,fEbindeL2,7.7e-4,fEbindeM2,2.3e-4,3.8e-5);
           next609=true;
        }
        else if (p<=98.7){
           nucltransK(fEnGamma[178],fEbindeK2,4.5e-2,0.);
           next1378=true;
        }
        else{
           nucltransK(fEnGamma[179],fEbindeK2,5.3e-2,0.);
           next1415=true;
        }
     }
     if (next1995){
        p=100.*GetRandom();
        if (p<=60.3){
           nucltransKLM(fEnGamma[180],fEbindeK2,1.3e-3,fEbindeL2,2.0e-4,fEbindeM2,6.4e-5,1.8e-4);
           next609=true;
        }
        else if (p<=90.8){
           nucltransKLM(fEnGamma[181],fEbindeK2,1.1e-2,fEbindeL2,2.7e-3,fEbindeM2,9.3e-4,0.);
           next1275=true;
        }
        else if (p<=93.5){
           nucltransK(fEnGamma[182],fEbindeK2,6.8e-3,0.);
           next1378=true;
        }
        else{
           nucltransKLM(fEnGamma[183],fEbindeK2,2.0e-2,fEbindeL2,3.5e-3,fEbindeM2,1.1e-3,0.);
           next1661=true;
        }
     }
     if (next1847){
        p=100.*GetRandom();
        if (p<=25.7){
           nucltransK(fEnGamma[184],fEbindeK2,2.5e-3,2.4e-4);
           return;
        }
        else if (p<=97.0){
           nucltransKLM(fEnGamma[185],fEbindeK2,1.0e-2,fEbindeL2,1.8e-3,fEbindeM2,5.5e-4,1.0e-5);
           next609=true;
        }
        else if (p<=97.3){
           nucltransKLM(fEnGamma[186],fEbindeK2,8.2e-3,fEbindeL2,1.8e-3,fEbindeM2,6.4e-4,0.);
           next1015=true;
        }
        else if (p<=98.2){
           nucltransKLM(fEnGamma[187],fEbindeK2,6.4e-3,fEbindeL2,1.1e-3,fEbindeM2,3.4e-4,0.);
           next1275=true;
        }
        else{
           nucltransKLM(fEnGamma[188],fEbindeK2,8.0e-2,fEbindeL2,1.6e-2,fEbindeM2,5.1e-3,0.);
           next1378=true;
        }
     }
     if (next1764){
        p=100.*GetRandom();
        if (p<=87.61){
           nucltransK(fEnGamma[189],fEbindeK2,6.0e-3,1.5e-4);
           return;
        }
        else if (p<=97.01){
           nucltransKLM(fEnGamma[190],fEbindeK2,1.2e-2,fEbindeL2,2.0e-3,fEbindeM2,6.3e-4,2.8e-6);
           next609=true;
        }
        else if (p<=99.06){
           nucltransKLM(fEnGamma[191],fEbindeK2,1.3e-1,fEbindeL2,2.8e-2,fEbindeM2,8.9e-3,0.);
           next1378=true;
        }
        else if (p<=99.97){
           nucltransKLM(fEnGamma[192],fEbindeK2,2.9e-1,fEbindeL2,5.0e-2,fEbindeM2,1.6e-2,0.);
           next1415=true;
        }
        else{
           nucltransKLM(fEnGamma[193],fEbindeK2,5.6e-1,fEbindeL2,1.6e-1,fEbindeM2,5.4e-2,0.);
           next1543=true;
        }
     }
     if (next1743){
        nucltransKLM(fEnGamma[194],fEbindeK2,4.7e-3,fEbindeL2,9.0e-4,fEbindeM2,2.9e-4,2.8e-6);
        next609=true;
     }
     if (next1730){
        p=100.*GetRandom();
        if (p<=15.66){
           nucltransK(fEnGamma[195],fEbindeK2,2.7e-3,1.9e-4);
           return;
        }
        else if (p<=97.92){
           nucltransKLM(fEnGamma[196],fEbindeK2,1.3e-2,fEbindeL2,2.2e-3,fEbindeM2,6.7e-4,1.2e-6);
           next609=true;
        }
        else if (p<=99.55){
           nucltransKLM(fEnGamma[197],fEbindeK2,1.0e-2,fEbindeL2,1.7e-3,fEbindeM2,5.3e-4,0.);
           next1275=true;
        }
        else{
           nucltransKLM(fEnGamma[198],fEbindeK2,1.6e-1,fEbindeL2,3.7e-2,fEbindeM2,1.2e-2,0.);
           next1378=true;
        }
     }
     if (next1713){
        p=100.*GetRandom();
        if (p<=65.36){
           nucltransK(fEnGamma[199],fEbindeK2,1.2e-2,7.6e-7);
           next609=true;
        }
        else{
           nucltransK(fEnGamma[200],fEbindeK2,6.0e-2,0.);
           next1015=true;
        }
     }
     if (next1661){
        p=100.*GetRandom();
        if (p<=78.23){
           nucltransK(fEnGamma[201],fEbindeK2,3.0e-3,1.5e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[202],fEbindeK2,1.5e-2,0.);
           next609=true;
        }
     }
     if (next1543){
        p=100.*GetRandom();
        if (p<=6.00){
           nucltransK(fEnGamma[203],fEbindeK2,2.7e-3,1.1e-4);
           return;
        }
        else if (p<=99.26){
           nucltransKLM(fEnGamma[204],fEbindeK2,2.0e-2,fEbindeL2,3.5e-3,fEbindeM2,1.2e-3,0.);
           next609=true;
        }
        else if (p<=99.38){
           nucltransKLM(fEnGamma[205],fEbindeK2,2.0e-2,fEbindeL2,6.4e-3,fEbindeM2,2.1e-3,0.);
           next1015=true;
        }
        else{
           nucltransKLM(fEnGamma[206],fEbindeK2,3.3e-2,fEbindeL2,5.8e-3,fEbindeM2,1.8e-3,0.);
           next1275=true;
        }
     }
     if (next1415){
        fThlev=99.e-12;
        p=100.*GetRandom();
        if (p<=28.05){
           p=100.*GetRandom();
           if (p<=99.63){
              particle(3,1.322,1.322,0.,pi,0.,twopi,fTclev,fThlev);
              particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0.,0.);
           }
           else  pair(0.393);

           return;
        }
        else{
           nucltransKLM(fEnGamma[207],fEbindeK2,8.7e-3,fEbindeL2,2.0e-3,fEbindeM2,6.6e-4,0.);
           next609=true;
        }
     }
     if (next1378){
        p=100.*GetRandom();
        if (p<=44.47){
           nucltransKLM(fEnGamma[208],fEbindeK2,3.3e-3,fEbindeL2,5.9e-4,fEbindeM2,2.0e-4,4.8e-5);
           return;
        }
        else{
           nucltransKLM(fEnGamma[209],fEbindeK2,1.3e-2,fEbindeL2,2.7e-3,fEbindeM2,9.0e-4,0.);
           next609=true;
        }
     }
     if (next1275){
        fThlev=0.;
        nucltransKLM(fEnGamma[210],fEbindeK2,4.8e-3,fEbindeL2,7.7e-4,fEbindeM2,2.5e-4,0.);
        next609=true;
     }
     if (next1015){
        fThlev=0.;
        nucltransKLM(fEnGamma[211],fEbindeK2,3.5e-2,fEbindeL2,1.5e-2,fEbindeM2,5.1e-3,0.);
        next609=true;
     }
     if (next609){
        fThlev=0.;
        nucltransKLM(fEnGamma[212],fEbindeK2,1.5e-2,fEbindeL2,4.2e-3,fEbindeM2,1.4e-3,0.);
        return;
     }
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Sc48()
{
  // "Table of Isotopes", 8 ed.,1996 + NDS 68(1993)1.
  // VIT, 7.05.1998; 13.08.2007 update to NDS 107(2006)1747.
  float tcnuc=0.;
  fThnuc=1.57212e5;
  fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
  bool next=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  fZdtr=fDaughterZ[1];
  if (pbeta<=10.02){
    beta(0.483,0.,0.);
    fThlev=1.4e-12;
    p=100.*GetRandom();
    if (p<=24.14){
      nucltransK(1.213,0.005,8.8e-5,0.1e-4);
      next=true;
    }
    else{
      nucltransK(0.175,0.005,4.5e-3,0.);
      fThlev=221.e-15;
      nucltransK(1.038,0.005,1.1e-4,0.);
      next=true;
    }
  }
  else{
    beta(0.659,0.,0.);
    fThlev=221.e-15;
    nucltransK(1.038,0.005,1.1e-4,0.);
    next=true;
  }
  if (next){
    fThlev=0.762e-12;
    nucltransK(1.312,0.005,9.7e-5,0.1e-4);
    fThlev=4.04e-12;
    nucltransK(0.984,0.005,1.3e-4,0.);
    return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Co60()
{
  // "Table of Isotopes", 7th ed., 1978).
  // VIT, 3.08.1992, 22.10.1995.
  // Updated to NDS 100(2003)347, VIT, 16.10.2006;
  // angular correlation of 1173 and 1333 keV gammas,
  // L.Pandola + VIT, 18.10.2006;
  // 2nd forbidden unique shape for beta decay to 1333 keV level,
  // VIT, 27.10.2006.
  float phi1,phi2, pbeta, p;
  int npg1173=0, npg1333=0;
  bool next1333=false;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     fThlev=0.3e-12;
     p=100.*GetRandom();
     if (p<=0.000002){
        nucltransK(fEnGamma[0],fEbindeK,8.6e-5,0.);
        return;
     }
     else if (p<=99.992449){
        fEgamma=fEnGamma[1];
        float cg=1., cK=1.7e-4, cp=6.2e-6;
        p=GetRandom()*(cg+cK+cp);
        if (p<=cg){
           particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
           npg1173=fNbPart;
        }
        else if (p<=cg+cK){
           particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        }
        else{
           pair(fEgamma-1.022);
        }
        next1333=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,5.5e-3,0.);
        fThlev=0.;
        p=GetRandom()*100;
        if (p<=13.64){
           nucltransK(fEnGamma[3],fEbindeK,4.9e-5,3.9e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[4],fEbindeK,3.3e-4,0.);
           next1333=true;
        }
     }
  }
  else{
     beta2f(fEndPoint[1],0.,0.,2,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
     next1333=true;
  }
  if (next1333){
     fThlev=0.9e-12;
     fEgamma=fEnGamma[5];
     float cg=1., cK=1.3e-4, cp=3.4e-5;
     p=GetRandom()*(cg+cK+cp);
     if (p<=cg){
        particle(1,fEgamma,fEgamma,0.,pi,0.,twopi,fTclev,fThlev);
        npg1333=fNbPart;
     }
     else if (p<=cg+cK){
        particle(3,fEgamma-fEbindeK,fEgamma-fEbindeK,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
     }
     else{
        pair(fEgamma-1.022);
     }
  }
  if (npg1333!=0 && npg1173!=0){
     float p1333=sqrt(pow(fPmoment[0][npg1333],2)+pow(fPmoment[1][npg1333],2)
                     +pow(fPmoment[2][npg1333],2));
     float p1173=sqrt(pow(fPmoment[0][npg1173],2)+pow(fPmoment[1][npg1173],2)
                     +pow(fPmoment[2][npg1173],2));
     // Coefficients in formula 1+a2*ctet**2+a4*ctet**4 are from:
     // R.D.Evans, "The Atomic Nucleus", Krieger Publ. Comp., 1985,
     // p. 240 (4(2)2(2)0 cascade).
     // They correspond to coefficients in formula
     // 1+a2*p2+a4*p4, a2=0.1020, a4=0.0091
     // in K.Siegbahn, "Alpha-, Beta- and Gamma-Ray Spectroscopy",
     // North-Holland Publ. Comp., 1968, p. 1033.
     float ctet,stet1,stet2,ctet1,ctet2;
     float a2=1./8.;
     float a4=1./24.;
     do{
       phi1=twopi*GetRandom();
       ctet1=1.-2.*GetRandom();
       stet1=sqrt(1.-ctet1*ctet1);
       phi2=twopi*GetRandom();
       ctet2=1.-2.*GetRandom();
       stet2=sqrt(1.-ctet2*ctet2);
       ctet=ctet1*ctet2+stet1*stet2*cos(phi1-phi2);
     } while(GetRandom()*(1.+abs(a2)+abs(a4)) > 1.+a2*ctet*ctet+a4*pow(ctet,4));
     fPmoment[0][npg1333]=p1333*stet1*cos(phi1);
     fPmoment[1][npg1333]=p1333*stet1*sin(phi1);
     fPmoment[2][npg1333]=p1333*ctet1;
     fPmoment[0][npg1173]=p1173*stet2*cos(phi2);
     fPmoment[1][npg1173]=p1173*stet2*sin(phi2);
     fPmoment[2][npg1173]=p1173*ctet2;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Cs136()
{
  // "Table of Isotopes", 8th ed.,1996 and Nucl. Data Sheets 95(2002)837).
  // VIT, 11.09.2002.
  float pbeta, p;
  bool next1867=false, next2054=false, next2140=false, next2207=false;
  bool next2031=false, next819=false;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){       // 2.025%
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<=48.02){
        nucltransK(fEnGamma[0],fEbindeK,1.1e-2,0.);
        next1867=true;
     }
     else if (p<=73.36){
        nucltransK(fEnGamma[1],fEbindeK,3.5e-2,0.);
        next2054=true;
    }
    else if (p<=77.36){
        nucltransK(fEnGamma[2],fEbindeK,2.0e-2,0.);
        next2140=true;
    }
    else{
        nucltransK(fEnGamma[3],fEbindeK,2.5e-1,0.);
        next2207=true;
    }
  }
  else if (pbeta<= fProbBeta[1]){ // 0.208%
     beta(fEndPoint[1],0.,0.);
     p=100.*GetRandom();
     if (p<=47.12){
        nucltransK(fEnGamma[4],fEbindeK,6.0e-4,0.2e-4);
        next819=true;
     }
     else if (p<=85.24){
        nucltransK(fEnGamma[5],fEbindeK,1.2e-2,0.);
        next1867=true;
     }
     else{
        nucltransK(fEnGamma[6],fEbindeK,4.5e-2,0.);
        next2054=true;
     }
  }
  else if (pbeta<=fProbBeta[2]){ // 69.776%
     beta(fEndPoint[2],0.,0.);
     next2207=true;
  }
  else if (pbeta<=fProbBeta[3]){ // 10.422%
     beta(fEndPoint[3],0.,0.);
     next2140=true;
  }
  else if (pbeta<=fProbBeta[4]){ // 4.666%
     beta(fEndPoint[4],0.,0.);
     next2054=true;
  }
  else{                    // 12.903%
     beta(fEndPoint[5],0.,0.);
     next1867=true;
  }
  if (next2207){
     fThlev=3.11e-9;
     p=100.*GetRandom();
     if (p<=61.84){
        nucltransK(fEnGamma[7],fEbindeK,3.0e-2,0.);
        next1867=true;
     }
     else if (p<=76.78){
        nucltransK(fEnGamma[8],fEbindeK,5.0e-2,0.);
        next2031=true;
    }
    else if (p<=88.48){
        nucltransK(fEnGamma[9],fEbindeK,4.3e-1,0.);
        next2054=true;
    }
    else{
        nucltransK(fEnGamma[10],fEbindeK,6.9e-1,0.);
        next2140=true;
    }
  }
  if (next2140){
     fThlev=1.6e-9;
     p=100.*GetRandom();
     if (p<=0.27){
        nucltransK(fEnGamma[11],fEbindeK,1.7e-3,0.1e-4);
        next819=true;
     }
     else if (p<=60.13){
        nucltransK(fEnGamma[12],fEbindeK,1.6e-2,0.);
        next1867=true;
     }
     else if (p<=62.88){
        nucltransK(fEnGamma[13],fEbindeK,1.47,0.);
        next2031=true;
     }
     else{
        nucltransK(fEnGamma[14],fEbindeK,3.5e-1,0.);
        next2054=true;
     }
  }
  if (next2054){
     fThlev=0.;
     p=100.*GetRandom();
     if (p<=97.90){
        nucltransK(fEnGamma[15],fEbindeK,1.0e-3,0.1e-4);
        next819=true;
     }
     else{
        nucltransK(fEnGamma[16],fEbindeK,1.9e-1,0.);
        next1867=true;
     }
  }
  if (next2031){
     fThlev=0.3084;
     nucltransK(fEnGamma[17],fEbindeK,2.26,0.);
     next1867=true;
  }
  if (next1867){
     fThlev=0.;
     nucltransK(fEnGamma[18],fEbindeK,1.5e-3,0.);
     next819=true;
  }
  if (next819){
     fThlev=1.930e-12;
     nucltransK(fEnGamma[19],fEbindeK,2.7e-3,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Eu147()
{
  // "Table of Isotopes", 7th ed., 1978.
  // VIT, 10.03.1996.
  float pdecay,p;
  bool next799=false, next197=false, next121=false;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){
     particle(47,fEnAlpha[0],fEnAlpha[0],0.,pi,0.,twopi,0,0);
     return;
  }
  else if (pdecay<= fProbDecay[1]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     nucltransK(fEnGamma[1],fEbindeK,5.0e-4,0.8e-4);
     next197=true;
  }
  else if (pdecay<= fProbDecay[2]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=31.){
        nucltransK(fEnGamma[2],fEbindeK,5.0e-4,0.7e-4);
        next121=true;
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,1.8e-3,0.);
        next799=true;
     }
  }
  else if (pdecay<= fProbDecay[3]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     nucltransK(fEnGamma[4],fEbindeK,5.5e-4,0.6e-4);
     next121=true;
  }
  else if (pdecay<= fProbDecay[4]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=24.){
        nucltransK(fEnGamma[5],fEbindeK,5.5e-4,0.5e-4);
        next121=true;
     }
     else{
        nucltransK(fEnGamma[6],fEbindeK,6.0e-4,0.4e-4);
        next197=true;
     }
  }
  else if (pdecay<= fProbDecay[5]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=22.8){
        nucltransK(fEnGamma[7],fEbindeK,5.5e-4,0.5e-4);
       return;
     }
     else if (p<=68.3){
       nucltransK(fEnGamma[8],fEbindeK,6.5e-4,0.3e-4);
       next121=true;
     }
     else{
       nucltransK(fEnGamma[9],fEbindeK,7.5e-4,0.2e-4);
       next197=true;
     }
  }
  else if (pdecay<= fProbDecay[6]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=15.){
        nucltransK(fEnGamma[10],fEbindeK,7.5e-4,0.1e-4);
        next197=true;
     }
     else{
        nucltransK(fEnGamma[11],fEbindeK,4.0e-3,0.);
        next799=true;
     }
  }
  else if (pdecay<=fProbDecay[7]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=62.5){
        nucltransK(fEnGamma[12],fEbindeK,4.3e-3,0.1e-4);
        return;
     }
     else if (p<=98.5){
        nucltransK(fEnGamma[13],fEbindeK,5.5e-3,0.);
        next121=true;
     }
     else{
        nucltransK(fEnGamma[14],fEbindeK,9.5e-3,0.);
        next197=true;
     }
  }
  else if (pdecay<=fProbDecay[8]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=40.){
        nucltransK(fEnGamma[15],fEbindeK,1.1e-3,0.1e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[16],fEbindeK,1.3e-3,0.);
        next121=true;
     }
  }
  else if (pdecay<=fProbDecay[9]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=57.){
        nucltransK(fEnGamma[17],fEbindeK,1.3e-3,0.);
        next121=true;
     }
     else{
        nucltransK(fEnGamma[18],fEbindeK,1.6e-3,0.);
        next197=true;
     }
  }
  else if (pdecay<=fProbDecay[10]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=45.){
        nucltransK(fEnGamma[19],fEbindeK,1.5e-3,0.);
        next121=true;
     }
     else{
        nucltransK(fEnGamma[20],fEbindeK,9.0e-3,0.);
        next197=true;
     }
  }
  else if (pdecay<=fProbDecay[11]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     next799=true;
  }
  else if (pdecay<=fProbDecay[12]){
     p=100.*GetRandom();
     if (p<=99.4) particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     else beta(fEndPoint[0],0.,0.);
     next197=true;
  }
  else if (pdecay<=fProbDecay[13]){
     p=100.*GetRandom();
     if (p<=99.5) particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     else beta(fEndPoint[1],0.,0.);
     next121=true;
  }
  else{
     p=100.*GetRandom();
     if (p<=99.3) particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     else beta(fEndPoint[2],0.,0.);
  }
  if (next799){
     p=100.*GetRandom();
     if (p<=24.2){
         nucltransK(fEnGamma[21],fEbindeK,4.5e-3,0.);
         return;
     }
     else if (p<=71.7){
         nucltransK(fEnGamma[22],fEbindeK,1.4e-2,0.);
         next121=true;
     }
     else{
         nucltransK(fEnGamma[23],fEbindeK,1.8e-2,0.);
         next197=true;
     }
  }
  if (next197){
     fThlev=1.3e-9;
     p=100.*GetRandom();
     if (p<=97.2){
        nucltransK(fEnGamma[24],fEbindeK,2.2e-1,0.);
        return;
     }
     else {
        nucltransK(fEnGamma[25],fEbindeK,3.5e-0,0.);
        next121=true;
     }
  }
  if (next121){
     fThlev=0.77e-9;
     nucltransK(fEnGamma[26],fEbindeK,1.1e-0,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Eu152()
{
  // "Table of Isotopes", 8th ed., 1996 and NDS 79(1996)1).
  // VIT, 16.11.1996.
  // VIT, approximate electron capture from K, L or M shells, 27.12.2006.
  // VIT, correction to the 1fnu shape for Qbeta=1.474, 13.11.2007.
  bool next1530=false, next1434=false, next1372=false, next1318=false;
  bool next1293=false, next1234=false, next1222=false, next1123=false;
  bool next1109=false, next1086=false, next1041=false, next1023=false;
  bool next1048=false,  next963=false,  next931=false,  next810=false;
  bool  next755=false,  next707=false,  next685=false,  next615=false;
  bool  next366=false,  next344=false,  next122=false;
  float pdecay, pbeta, pKLM, pec, p;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){  //  EC to 152Sm
     //approximate electron capture from K (82%), L (14%) or M (4%) shell
     pKLM=100.*GetRandom();
     if (pKLM<=82.) particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
     if (pKLM>82.&& pKLM<=96.)
        particle(1,fEbindeL,fEbindeL,0.,pi,0.,twopi,0,0);
     if (pKLM>96.)  particle(1,fEbindeM,fEbindeM,0.,pi,0.,twopi,0,0);

     pec=100.*GetRandom();
     if (pec<=fProbEC[0]){
        p=100.*GetRandom();
        if (p<=15.21){
           nucltransK(fEnGamma[0],fEbindeK,8.0e-4,0.9e-4);
           return;
        }
        else if (p<=25.62){
           nucltransK(fEnGamma[1],fEbindeK,4.0e-4,0.5e-4);
           next122=true;
        }
        else if (p<=65.65){
           nucltransK(fEnGamma[2],fEbindeK,2.5e-3,0.);
           next810=true;
        }
        else if (p<=88.07){
           nucltransK(fEnGamma[3],fEbindeK,1.4e-3,0.);
           next963=true;
        }
        else if (p<=99.28){
           nucltransK(fEnGamma[4],fEbindeK,1.0e-2,0.);
           next1234=true;
        }
        else{
           nucltransK(fEnGamma[5],fEbindeK,1.0e-1,0.);
           next1530=true;
        }
     }
     else if (pec<= fProbEC[1]){
        p=100.*GetRandom();
        if (p<=0.43){
           nucltransK(fEnGamma[6],fEbindeK,3.5e-4,0.5e-4);
           next122=true;
        }
        else if (p<=14.38){
           nucltransK(fEnGamma[7],fEbindeK,5.0e-4,0.2e-4);
           next366=true;
        }
        else if (p<=81.12){
           nucltransK(fEnGamma[8],fEbindeK,1.1e-2,0.);
           next1086=true;
        }
        else if (p<=85.46){
           nucltransK(fEnGamma[9],fEbindeK,3.5e-3,0.);
           next1234=true;
        }
        else{
           nucltransK(fEnGamma[10],fEbindeK,7.0e-3,0.);
           next1372=true;
        }
     }
     else if (pec<= fProbEC[2]){
        p=100.*GetRandom();
        if (p<=11.26){
           nucltransK(fEnGamma[11],fEbindeK,4.0e-4,0.8e-4);
           next122=true;
        }
        else if (p<=66.17){
           nucltransK(fEnGamma[12],fEbindeK,5.3e-4,0.3e-4);
           next366=true;
        }
        else if (p<=79.35){
           nucltransK(fEnGamma[13],fEbindeK,2.2e-3,0.);
           next1086=true;
        }
        else{
           nucltransK(fEnGamma[14],fEbindeK,4.0e-3,0.);
           next1234=true;
        }
     }
     else if (pec<= fProbEC[3]){
        p=100.*GetRandom();
        if (p<=30.00){
           nucltransK(fEnGamma[15],fEbindeK,8.0e-3,0.);
           next122=true;
        }
        else if (p<=31.85){
           nucltransK(fEnGamma[16],fEbindeK,8.0e-3,0.);
           next810=true;
        }
        else if (p<=34.02){
           nucltransK(fEnGamma[17],fEbindeK,8.0e-3,0.);
           next963=true;
        }
        else if (p<=86.93){
           nucltransK(fEnGamma[18],fEbindeK,8.0e-3,0.);
           next1086=true;
        }
        else if (p<=99.47){
           nucltransK(fEnGamma[19],fEbindeK,8.0e-3,0.);
           next1234=true;
        }
        else{
           nucltransK(fEnGamma[20],fEbindeK,8.0e-3,0.);
           next1293=true;
        }
     }
     else if (pec<= fProbEC[4]){
        p=100.*GetRandom();
        if (p<=71.28){
           nucltransK(fEnGamma[21],fEbindeK,2.8e-3,0.);
           next707=true;
        }
        else if (p<=94.09){
           nucltransK(fEnGamma[22],fEbindeK,2.8e-3,0.);
           next1041=true;
        }
        else{
           nucltransK(fEnGamma[23],fEbindeK,7.0e-3,0.);
           next1222=true;
        }
     }
     else if (pec<= fProbEC[5]){
        p=100.*GetRandom();
        if (p<=24.04){
           nucltransK(fEnGamma[24],fEbindeK,4.5e-4,0.4e-4);
           next122=true;
        }
        else if (p<=92.32){
           nucltransK(fEnGamma[25],fEbindeK,6.2e-4,0.1e-4);
           next366=true;
        }
        else if (p<=96.69){
           nucltransK(fEnGamma[26],fEbindeK,1.5e-3,0.);
           next810=true;
        }
        else if (p<=97.13){
           nucltransK(fEnGamma[27],fEbindeK,7.0e-3,0.);
           next963=true;
        }
        else if (p<=98.06){
           nucltransK(fEnGamma[28],fEbindeK,3.0e-3,0.);
           next1023=true;
        }
        else if (p<=98.26){
           nucltransK(fEnGamma[29],fEbindeK,9.5e-3,0.);
           next1041=true;
        }
        else if (p<=99.79){
           nucltransK(fEnGamma[30],fEbindeK,4.0e-3,0.);
           next1086=true;
        }
        else{
           nucltransK(fEnGamma[31],fEbindeK,3.5e-2,0.);
           next1372=true;
        }
     }
     else if (pec<=fProbEC[6]){
        next1530=true;
     }
     else if (pec<=fProbEC[7]){
        next1372=true;
     }
     else if (pec<=fProbEC[8]){
        next1293=true;
     }
     else if (pec<=fProbEC[9]){
        next1234=true;
     }
     else if (pec<=fProbEC[10]){
        next1086=true;
     }
     else if (pec<=fProbEC[11]){
        next1041=true;
     }
     else if (pec<=fProbEC[12]){
        next1023=true;
     }
     else if (pec<=fProbEC[13]){
        next810=true;
     }
     else if (pec<=fProbEC[14]){
        next366=true;
     }
     else{
       next122=true;
     }
     if (next1530){
        p=100.*GetRandom();
        if (p<=84.56){
           nucltransK(fEnGamma[32],fEbindeK,5.0e-4,0.3e-4);
           next122=true;
        }
        else if (p<=84.80){
           nucltransK(fEnGamma[33],fEbindeK,2.0e-3,0.);
           next810=true;
        }
        else if (p<=85.32){
           nucltransK(fEnGamma[34],fEbindeK,1.4e-2,0.);
           next963=true;
        }
        else if (p<=86.97){
           nucltransK(fEnGamma[35],fEbindeK,1.4e-2,0.);
           next1041=true;
        }
        else if (p<=98.22){
           nucltransK(fEnGamma[36],fEbindeK,5.7e-3,0.);
           next1086=true;
        }
        else{
           nucltransK(fEnGamma[37],fEbindeK,1.5e-2,0.);
           next1234=true;
        }
     }
     if (next1372){
        p=100.*GetRandom();
        if (p<=19.62){
           nucltransK(fEnGamma[38],fEbindeK,1.6e-3,0.1e-4);
           next122=true;
        }
        else if (p<=88.70){
           nucltransK(fEnGamma[39],fEbindeK,2.6e-3,0.);
           next366=true;
        }
        else if (p<=90.70){
           nucltransK(fEnGamma[40],fEbindeK,6.3e-3,0.);
           next707=true;
        }
        else if (p<=90.81){
           nucltransK(fEnGamma[41],fEbindeK,3.5e-3,0.);
           next810=true;
        }
        else if (p<=98.82){
           nucltransK(fEnGamma[42],fEbindeK,1.2e-2,0.);
           next1041=true;
        }
        else{
           nucltransK(fEnGamma[43],fEbindeK,6.6e-2,0.);
           next1086=true;
        }
     }
     if (next1293){
        p=100.*GetRandom();
        if (p<=16.39){
           nucltransK(fEnGamma[44],fEbindeK,1.3e-3,0.1e-4);
           return;
        }
        else if (p<=22.12){
           nucltransK(fEnGamma[45],fEbindeK,1.6e-3,0.);
           next122=true;
        }
        else if (p<=64.58){
           nucltransK(fEnGamma[46],fEbindeK,3.0e-3,0.);
           next366=true;
        }
        else if (p<=68.91){
           nucltransK(fEnGamma[47],fEbindeK,2.0e-2,0.);
           next810=true;
        }
        else if (p<=88.66){
           nucltransK(fEnGamma[48],fEbindeK,1.2e-2,0.);
           next963=true;
        }
        else if (p<=89.98){
           nucltransK(fEnGamma[49],fEbindeK,7.9e-2,0.);
           next1023=true;
        }
        else{
           nucltransK(fEnGamma[50],fEbindeK,2.3e-2,0.);
           next1041=true;
        }
     }
     if (next1234){
        p=100.*GetRandom();
        if (p<=76.33){
           nucltransK(fEnGamma[51],fEbindeK,2.0e-3,0.2e-4);
           next122=true;
        }
        else if (p<=99.76){
           nucltransK(fEnGamma[52],fEbindeK,3.5e-3,0.);
           next366=true;
        }
        else if (p<=99.78){
           nucltransK(fEnGamma[53],fEbindeK,2.7e-2,0.);
           next810=true;
        }
        else{
           nucltransK(fEnGamma[54],fEbindeK,5.8e-1,0.);
           next1086=true;
        }
     }
     if (next1222){
        p=100.*GetRandom();
        if (p<=80.65){
           nucltransK(fEnGamma[55],fEbindeK,1.6e-3,0.);
           next366=true;
        }
        else{
           nucltransK(fEnGamma[56],fEbindeK,3.7e-3,0.);
           next707=true;
        }
     }
     if (next1086){
        fThlev=0.85e-12;
        p=100.*GetRandom();
        if (p<=40.36){
           nucltransK(fEnGamma[57],fEbindeK,2.1e-3,0.);
           return;
        }
        else if (p<=98.77){
           nucltransK(fEnGamma[58],fEbindeK,2.7e-3,0.);
           next122=true;
        }
        else if (p<=99.86){
           nucltransK(fEnGamma[59],fEbindeK,5.2e-3,0.);
           next366=true;
        }
        else{
           nucltransK(fEnGamma[60],fEbindeK,1.0e-1,0.);
           next810=true;
        }
     }
     if (next1041){
        fThlev=0;
        p=100.*GetRandom();
        if (p<=71.23){
           nucltransK(fEnGamma[61],fEbindeK,1.2e-3,0.);
           next122=true;
        }
        else{
           nucltransK(fEnGamma[62],fEbindeK,2.3e-3,0.);
           next366=true;
        }
     }
     if (next1023){
        fThlev=6.7e-12;
        p=100.*GetRandom();
        if (p<=35.73){
           nucltransK(fEnGamma[63],fEbindeK,3.1e-3,0.);
           next122=true;
        }
        else if (p<=91.56){
           nucltransK(fEnGamma[64],fEbindeK,5.7e-2,0.);
           next366=true;
        }
        else if (p<=92.40){
           nucltransK(fEnGamma[65],fEbindeK,4.8e-2,0.);
           next707=true;
        }
        else{
           nucltransK(fEnGamma[66],fEbindeK,1.7e-1,0.);
           next810=true;
        }
     }
     if (next963){
        fThlev=28.2e-15;
        p=100.*GetRandom();
        if (p<=45.143){
           nucltransK(fEnGamma[67],fEbindeK,1.1e-3,0.);
           return;
        }
        else if (p<=99.994){
           nucltransK(fEnGamma[68],fEbindeK,1.4e-3,0.);
           next122=true;
        }
        else if (p<=99.995){
           nucltransK(fEnGamma[69],fEbindeK,1.8e-2,0.);
           next685=true;
        }
        else{
           nucltransK(fEnGamma[70],fEbindeK,8.7e-2,0.);
           next810=true;
        }
     }
     if (next810){
        fThlev=7.2e-12;
        p=100.*GetRandom();
        if (p<=21.66){
           nucltransK(fEnGamma[71],fEbindeK,4.0e-3,0.);
           return;
        }
        else if (p<=78.06){
           nucltransK(fEnGamma[72],fEbindeK,4.3e-2,0.);
           next122=true;
        }
        else if (p<=99.21){
           nucltransK(fEnGamma[73],fEbindeK,1.8e-2,0.);
           next366=true;
        }
        else{
           nucltransK(fEnGamma[74],fEbindeK,1.0e-0,0.);
           next685=true;
        }
     }
     if (next707){
        fThlev=10.1e-12;
        nucltransK(fEnGamma[75],fEbindeK,3.8e-4,0.);
        next366=true;
     }
     if (next685){
        fThlev=6.2e-12;
        p=100.*GetRandom();
        if (p<= 1.43){
           particle(3,0.638,0.638,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
           return;
        }
        else{
           nucltransK(fEnGamma[76],fEbindeK,9.5e-3,0.);
           next122=true;
        }
     }
     if (next366){
        fThlev=57.7e-12;
        nucltransK(fEnGamma[77],fEbindeK,1.1e-1,0.);
        next122=true;
     }
     if (next122){
        fThlev=1.428e-9;
        nucltransK(fEnGamma[78],fEbindeK,1.2e-0,0.);
        return;
     }
  }
  else{               //   b- to 152Gd
      pbeta=100.*GetRandom();
      if (pbeta<=fProbBeta[0]){
         beta(fEndPoint[0],0.,0.);
         p=100.*GetRandom();
         if (p<=84.03){
            nucltransK(fEnGamma[79],fEbindeK2,1.5e-3,0.2e-4);
            next344=true;
         }
         else{
            nucltransK(fEnGamma[80],fEbindeK2,3.2e-3,0.);
            next755=true;
         }
      }
      else if (pbeta<= fProbBeta[1]){
         beta(fEndPoint[1],0.,0.);
         p=100.*GetRandom();
         if (p<=89.21){
            nucltransK(fEnGamma[81],fEbindeK2,7.5e-4,0.3e-4);
            next344=true;
         }
         else if (p<=94.47){
            nucltransK(fEnGamma[82],fEbindeK2,2.2e-3,0.);
            next931=true;
         }
         else if (p<=96.83){
            nucltransK(fEnGamma[83],fEbindeK2,4.5e-3,0.);
            next1109=true;
         }
         else if (p<=99.75){
            nucltransK(fEnGamma[84],fEbindeK2,1.8e-2,0.);
            next1123=true;
         }
         else if (p<=99.76){
            nucltransK(fEnGamma[85],fEbindeK2,1.3e-2,0.);
            next1318=true;
         }
         else{
            nucltransK(fEnGamma[86],fEbindeK2,4.0e-2,0.);
            next1434=true;
         }
      }
      else if (pbeta<= fProbBeta[2]){
         beta(fEndPoint[2],0.,0.);
         p=100.*GetRandom();
         if (p<=7.64){
            nucltransK(fEnGamma[87],fEbindeK2,1.2e-3,0.5e-4);
            return;
         }
         else if (p<=40.86){
            nucltransK(fEnGamma[88],fEbindeK2,2.7e-3,0.5e-4);
            next344=true;
         }
         else if (p<=72.09){
            nucltransK(fEnGamma[89],fEbindeK2,3.0e-3,0.);
            next615=true;
         }
         else if (p<=91.36){
            nucltransK(fEnGamma[90],fEbindeK2,7.6e-3,0.);
            next931=true;
         }
         else if (p<=95.35){
            nucltransK(fEnGamma[91],fEbindeK2,1.1e-2,0.);
            next1048=true;
         }
         else if (p<=99.99){
            nucltransK(fEnGamma[92],fEbindeK2,9.7e-2,0.);
            next1109=true;
         }
         else{
            nucltransK(fEnGamma[93],fEbindeK2,5.0e-3,0.);
            next1123=true;
         }
      }
      else if (pbeta<=fProbBeta[3]){
         beta(fEndPoint[3],0.,0.);
         next1434=true;
      }
      else if (pbeta<=fProbBeta[4]){
         beta(fEndPoint[4],0.,0.);
         next1318=true;
      }
      else if (pbeta<=fProbBeta[5]){
         beta(fEndPoint[5],0.,0.);
         p=100.*GetRandom();
         if (p<=58.41){
            nucltransK(fEnGamma[94],fEbindeK2,9.4e-2,0.);
            next755=true;
         }
         else if (p<=98.13){
            nucltransK(fEnGamma[95],fEbindeK2,3.8e-2,0.);
            next931=true;
         }
         else{
            nucltransK(fEnGamma[96],fEbindeK2,3.7e-1,0.);
            next1109=true;
         }
      }
      else if (pbeta<=fProbBeta[6]){
         beta(fEndPoint[6],0.,0.);
         next1123=true;
      }
      else if (pbeta<=fProbBeta[7]){
         beta(fEndPoint[7],0.,0.);
         next1109=true;
      }
      else if (pbeta<=fProbBeta[8]){
         beta(fEndPoint[8],0.,0.);
         next931=true;
      }
      else if (pbeta<=fProbBeta[9]){
         beta(fEndPoint[9],0.,0.);
         next755=true;
      }
      else{
         beta1f(fEndPoint[10],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
         next344=true;
      }

      if (next1434){
         p=100.*GetRandom();
         if (p<=71.07){
            nucltransK(fEnGamma[97],fEbindeK2,2.3e-3,0.);
            next344=true;
         }
         else if (p<=90.54){
            nucltransK(fEnGamma[98],fEbindeK2,6.9e-3,0.);
            next755=true;
         }
         else if (p<=96.87){
            nucltransK(fEnGamma[99],fEbindeK2,1.4e-2,0.);
            next931=true;
         }
         else if (p<=99.99){
            nucltransK(fEnGamma[100],fEbindeK2,6.3e-2,0.);
            next1109=true;
         }
         else{
            nucltransK(fEnGamma[101],fEbindeK2,1.4e-0,0.);
            next1318=true;
         }
      }
      if (next1318){
         p=100.*GetRandom();
         if (p<=6.87){
            nucltransK(fEnGamma[102],fEbindeK2,1.6e-3,0.1e-4);
            return;
         }
         else if (p<=62.27){
            nucltransK(fEnGamma[103],fEbindeK2,5.6e-3,0.);
            next344=true;
         }
         else if (p<=76.12){
            nucltransK(fEnGamma[104],fEbindeK2,6.0e-3,0.);
            next615=true;
         }
         else if (p<=77.39){
            nucltransK(fEnGamma[105],fEbindeK2,1.0e-2,0.);
            next755=true;
         }
         else if (p<=84.76){
            nucltransK(fEnGamma[106],fEbindeK2,4.5e-1,0.);
            next931=true;
         }
         else if (p<=92.35){
            nucltransK(fEnGamma[107],fEbindeK2,8.4e-2,0.);
            next1048=true;
         }
         else if (p<=92.79){
            nucltransK(fEnGamma[108],fEbindeK2,5.0e-1,0.);
            next1109=true;
         }
         else{
            nucltransK(fEnGamma[109],fEbindeK2,4.9e-2,0.);
            next1123=true;
         }
      }
      if (next1123){
         fThlev=7.3e-12;
         p=100.*GetRandom();
         if (p<=93.11){
            nucltransK(fEnGamma[110],fEbindeK2,1.9e-3,0.);
            next344=true;
         }
         else if (p<=99.26){
            nucltransK(fEnGamma[111],fEbindeK2,9.7e-3,0.);
            next755=true;
         }
         else{
            nucltransK(fEnGamma[112],fEbindeK2,5.0e-2,0.);
            next931=true;
         }
      }
      if (next1109){
         fThlev=7.3e-12;
         p=100.*GetRandom();
         if (p<=50.00){
            nucltransK(fEnGamma[113],fEbindeK2,2.2e-3,0.);
            return;
         }
         else if (p<=97.50){
            nucltransK(fEnGamma[114],fEbindeK2,5.2e-3,0.);
            next344=true;
         }
         else{
            nucltransK(fEnGamma[115],fEbindeK2,1.5e-2,0.);
            next615=true;
         }
      }
      if (next1048){
         fThlev=0.;
         p=100.*GetRandom();
         if (p<=0.88){
            particle(3,0.998,0.998,0.,pi,0.,twopi,fTclev,fThlev);
            particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
            return;
         }
         else if (p<=65.79){
            nucltransK(fEnGamma[116],fEbindeK2,6.0e-3,0.);
            next344=true;
         }
         else if (p<=83.77){
            particle(3,0.383,0.383,0.,pi,0.,twopi,fTclev,fThlev);
            particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
            next615=true;
         }
         else{
            nucltransK(fEnGamma[117],fEbindeK2,1.4e-0,0.);
            next931=true;
         }
      }
      if (next931){
         fThlev=7.3e-12;
         p=100.*GetRandom();
         if (p<=12.40){
            nucltransK(fEnGamma[118],fEbindeK2,3.2e-3,0.);
            return;
         }
         else if (p<=90.91){
            nucltransK(fEnGamma[119],fEbindeK2,2.4e-2,0.);
            next344=true;
         }
         else if (p<=99.55){
            nucltransK(fEnGamma[120],fEbindeK2,5.2e-2,0.);
            next615=true;
         }
         else{
            nucltransK(fEnGamma[121],fEbindeK2,3.5e-1,0.);
            next755=true;
         }
      }
      if (next755){
         fThlev=7.3e-12;
         nucltransK(fEnGamma[122],fEbindeK2,2.4e-2,0.);
         next344=true;
      }
      if (next615){
         fThlev=37.e-12;
         p=100.*GetRandom();
         if (p<=11.35){
            particle(3,0.565,0.565,0.,pi,0.,twopi,fTclev,fThlev);
            particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
            return;
         }
         else{
            nucltransK(fEnGamma[123],fEbindeK2,8.3e-2,0.);
            next344=true;
         }
      }
      if (next344){
         fThlev=31.9e-12;
         nucltransK(fEnGamma[124],fEbindeK2,4.0e-2,0.);
         return;
      }
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Eu154()
{
  // "Table of Isotopes", 8th ed., 1996 and NDS 69(1993)507).
  // VIT, 13.11.1996.
  // VIT, correction to the 1fnu shape for Qbeta=1.846, 13.11.2007.
  bool next1775=false, next1661=false, next1646=false, next1560=false;
  bool next1531=false, next1510=false, next1418=false, next1414=false;
  bool next1404=false, next1398=false, next1296=false, next1294=false;
  bool next1277=false, next1264=false, next1252=false, next1241=false;
  bool next1182=false, next1136=false, next1128=false, next1048=false;
  bool  next996=false,  next815=false,  next718=false,  next681=false;
  bool  next371=false,  next123=false;
  float pdecay,pbeta,p;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){     // 0.020% EC to 154Sm
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     if (pdecay<=0.005){
        fThlev=172.e-12;
        nucltransK(fEnGamma[1],fEbindeK,2.7e-1,0.);
        fThlev=3.02e-9;
        nucltransK(fEnGamma[2],fEbindeK,4.9+0,0.);
        return;
     }
     else{
        fThlev=3.02e-9;
        nucltransK(fEnGamma[2],fEbindeK,4.9+0,0.);
        return;
     }
  }
  else{                   // 99.980% b- to 154Gd
     pbeta=100.*GetRandom();
     if (pbeta<=fProbBeta[0]){
        beta(fEndPoint[0],0.,0.);
        p=100.*GetRandom();
        if (p<=4.0){
           nucltransK(fEnGamma[3],fEbindeK2,1.0e-3,1.0e-4);
           return;
        }
        else if (p<=6.8){
           nucltransK(fEnGamma[4],fEbindeK2,1.1e-3,0.7e-4);
           next123=true;
        }
        else if (p<=38.1){
           nucltransK(fEnGamma[5],fEbindeK2,3.8e-3,0.);
           next815=true;
        }
        else if (p<=87.8){
           nucltransK(fEnGamma[6],fEbindeK2,5.0e-3,0.);
           next1048=true;
        }
        else if (p<=94.0){
           nucltransK(fEnGamma[7],fEbindeK2,2.8e-1,0.);
           next1646=true;
        }
        else{
           nucltransK(fEnGamma[8],fEbindeK2,1.4e+1,0.);
           next1775=true;
        }
     }
     else if (pbeta<= fProbBeta[1]){
        beta(fEndPoint[1],0.,0.);
        p=100.*GetRandom();
        if (p<=2.4){
           nucltransK(fEnGamma[9],fEbindeK2,5.0e-4,0.9e-4);
           next123=true;
        }
        else if (p<=4.0){
           nucltransK(fEnGamma[10],fEbindeK2,6.0e-4,0.4e-4);
           next371=true;
        }
        else if (p<=15.3){
           nucltransK(fEnGamma[11],fEbindeK2,1.2e-3,0.);
           next815=true;
        }
        else if (p<=58.7){
           nucltransK(fEnGamma[12],fEbindeK2,1.8e-3,0.);
           next996=true;
        }
        else if (p<=77.0){
           nucltransK(fEnGamma[13],fEbindeK2,2.5e-3,0.);
           next1128=true;
        }
        else if (p<=82.0){
           nucltransK(fEnGamma[14],fEbindeK2,1.1e-2,0.);
           next1241=true;
        }
        else if (p<=91.6){
           nucltransK(fEnGamma[15],fEbindeK2,4.0e-3,0.);
           next1264=true;
        }
        else if (p<=99.9){
           nucltransK(fEnGamma[16],fEbindeK2,3.0e-2,0.);
           next1404=true;
        }
        else{
           nucltransK(fEnGamma[17],fEbindeK2,2.0e-2,0.);
           next1531=true;
        }
     }
     else if (pbeta<=fProbBeta[2]){
        beta(fEndPoint[2],0.,0.);
        p=100.*GetRandom();
        if (p<=6.272){
           nucltransK(fEnGamma[18],fEbindeK2,5.0e-4,0.7e-4);
           next123=true;
        }
        else if (p<=9.404){
           nucltransK(fEnGamma[19],fEbindeK2,1.4e-3,0.);
           next815=true;
        }
        else if (p<=79.950){
           nucltransK(fEnGamma[20],fEbindeK2,2.2e-3,0.);
           next996=true;
        }
        else if (p<=97.361){
           nucltransK(fEnGamma[21],fEbindeK2,3.3e-3,0.);
           next1128=true;
        }
        else if (p<=98.150){
           nucltransK(fEnGamma[22],fEbindeK2,1.6e-2,0.);
           next1241=true;
        }
        else if (p<=98.362){
           nucltransK(fEnGamma[23],fEbindeK2,3.5e-2,0.);
           next1252=true;
        }
        else if (p<=98.594){
           nucltransK(fEnGamma[24],fEbindeK2,9.0e-2,0.);
           next1398=true;
        }
        else if (p<=98.629){
           nucltransK(fEnGamma[25],fEbindeK2,9.5e-2,0.);
           next1414=true;
        }
        else if (p<=98.665){
           nucltransK(fEnGamma[26],fEbindeK2,1.6e-2,0.);
           next1418=true;
        }
        else if (p<=98.674){
           nucltransK(fEnGamma[27],fEbindeK2,3.0e-1,0.);
           next1510=true;
        }
        else if (p<=99.513){
           nucltransK(fEnGamma[28],fEbindeK2,5.5e-2,0.);
           next1531=true;
        }
        else if (p<=99.986){
           nucltransK(fEnGamma[29],fEbindeK2,4.5e-1,0.);
           next1560=true;
        }
        else{
           nucltransK(fEnGamma[30],fEbindeK2,1.2e-0,0.);
           next1661=true;
        }
     }
     else if (pbeta<=fProbBeta[3]){
        beta(fEndPoint[3],0.,0.);
        next1661=true;
     }
     else if (pbeta<=fProbBeta[4]){
        beta(fEndPoint[4],0.,0.);
        next1646=true;
     }
     else if (pbeta<=fProbBeta[5]){
        beta(fEndPoint[5],0.,0.);
        p=100.*GetRandom();
        if (p<=43.76){
           nucltransK(fEnGamma[31],fEbindeK2,5.6e-4,0.5e-4);
           next123=true;
        }
        else if (p<=97.79){
           nucltransK(fEnGamma[32],fEbindeK2,7.6e-4,0.2e-4);
           next371=true;
        }
        else if (p<=97.80){
           nucltransK(fEnGamma[33],fEbindeK2,1.8e-3,0.);
           next815=true;
        }
        else if (p<=98.37){
           nucltransK(fEnGamma[34],fEbindeK2,2.8e-3,0.);
           next996=true;
        }
        else if (p<=99.00){
           nucltransK(fEnGamma[35],fEbindeK2,3.5e-3,0.);
           next1048=true;
        }
        else if (p<=99.43){
           nucltransK(fEnGamma[36],fEbindeK2,5.0e-3,0.);
           next1128=true;
        }
        else if (p<=99.73){
           nucltransK(fEnGamma[37],fEbindeK2,5.0e-3,0.);
           next1136=true;
        }
        else if (p<=99.85){
           nucltransK(fEnGamma[38],fEbindeK2,4.0e-2,0.);
           next1241=true;
        }
        else{
           nucltransK(fEnGamma[39],fEbindeK2,2.0e-1,0.);
           next1398=true;
        }
     }
     else if (pbeta<=fProbBeta[6]){
        beta(fEndPoint[6],0.,0.);
        next1560=true;
     }
     else if (pbeta<=fProbBeta[7]){
        beta(fEndPoint[7],0.,0.);
        next1531=true;
     }
     else if (pbeta<=fProbBeta[8]){
        beta(fEndPoint[8],0.,0.);
        next1418=true;
     }
     else if (pbeta<=fProbBeta[9]){
        beta(fEndPoint[9],0.,0.);
        next1398=true;
     }
     else if (pbeta<=fProbBeta[10]){
        beta(fEndPoint[10],0.,0.);
        next1264=true;
     }
     else if (pbeta<=fProbBeta[11]){
        beta(fEndPoint[11],0.,0.);
        next1252=true;
     }
     else if (pbeta<=fProbBeta[12]){
        beta(fEndPoint[12],0.,0.);
        next1128=true;
     }
     else if (pbeta<=fProbBeta[13]){
        beta(fEndPoint[13],0.,0.);
        next996=true;
     }
     else if (pbeta<=fProbBeta[14]){
        beta(fEndPoint[14],0.,0.);
        next815=true;
     }
     else if (pbeta<=fProbBeta[15]){
        beta(fEndPoint[15],0.,0.);
        next371=true;
     }
     else{
        beta1f(fEndPoint[16],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
        next123=true;
     }

     if (next1775){
        p=100.*GetRandom();
        if (p<=19.6){
           nucltransK(fEnGamma[40],fEbindeK2,1.0e-3,0.8e-4);
           return;
        }
        else if (p<=62.3){
           nucltransK(fEnGamma[41],fEbindeK2,1.6e-3,0.6e-4);
           next123=true;
        }
        else if (p<=79.4){
           nucltransK(fEnGamma[42],fEbindeK2,1.4e-3,0.2e-4);
           next371=true;
        }
        else if (p<=88.4){
           nucltransK(fEnGamma[43],fEbindeK2,2.3e-3,0.);
           next681=true;
        }
        else if (p<=92.2){
           nucltransK(fEnGamma[44],fEbindeK2,3.0e-3,0.);
           next815=true;
        }
        else if (p<=98.7){
           nucltransK(fEnGamma[45],fEbindeK2,6.0e-3,0.);
           next1048=true;
        }
        else{
           nucltransK(fEnGamma[46],fEbindeK2,1.4e-2,0.);
           next1128=true;
        }
     }
     if (next1661){
        p=100.*GetRandom();
        if (p<=6.03){
           nucltransK(fEnGamma[47],fEbindeK2,1.2e-3,0.4e-4);
           next123=true;
        }
        else if (p<=8.84){
           nucltransK(fEnGamma[48],fEbindeK2,1.8e-3,0.1e-4);
           next371=true;
        }
        else if (p<=75.84){
           nucltransK(fEnGamma[49],fEbindeK2,4.0e-3,0.);
           next815=true;
        }
        else if (p<=79.11){
           nucltransK(fEnGamma[50],fEbindeK2,7.0e-3,0.);
           next996=true;
        }
        else if (p<=89.76){
           nucltransK(fEnGamma[51],fEbindeK2,1.2e-2,0.);
           next1048=true;
        }
        else if (p<=94.78){
           nucltransK(fEnGamma[52],fEbindeK2,1.2e-2,0.);
           next1128=true;
        }
        else if (p<=95.18){
           nucltransK(fEnGamma[53],fEbindeK2,1.4e-1,0.);
           next1241=true;
        }
        else if (p<=98.45){
           nucltransK(fEnGamma[54],fEbindeK2,3.5e-2,0.);
           next1264=true;
        }
        else{
           nucltransK(fEnGamma[55],fEbindeK2,1.0e-0,0.);
           next1531=true;
        }
     }
     if (next1646){
        p=100.*GetRandom();
        if (p<=1.26){
           nucltransK(fEnGamma[56],fEbindeK2,1.2e-3,0.4e-4);
           next123=true;
        }
        else if (p<=2.57){
           nucltransK(fEnGamma[57],fEbindeK2,1.8e-3,0.1e-4);
           next371=true;
        }
        else if (p<=3.97){
           nucltransK(fEnGamma[58],fEbindeK2,3.5e-3,0.);
           next718=true;
        }
        else if (p<=6.77){
           nucltransK(fEnGamma[59],fEbindeK2,5.7e-3,0.);
           next815=true;
        }
        else if (p<=51.79){
           nucltransK(fEnGamma[60],fEbindeK2,7.3e-3,0.);
           next996=true;
        }
        else if (p<=56.52){
           nucltransK(fEnGamma[61],fEbindeK2,1.4e-2,0.);
           next1048=true;
        }
        else if (p<=82.63){
           nucltransK(fEnGamma[62],fEbindeK2,1.3e-2,0.);
           next1128=true;
        }
        else if (p<=84.79){
           nucltransK(fEnGamma[63],fEbindeK2,8.5e-3,0.);
           next1252=true;
        }
        else if (p<=89.70){
           nucltransK(fEnGamma[64],fEbindeK2,3.4e-2,0.);
           next1264=true;
        }
        else if (p<=91.41){
           nucltransK(fEnGamma[65],fEbindeK2,9.0e-3,0.);
           next1277=true;
        }
        else if (p<=95.37){
           nucltransK(fEnGamma[66],fEbindeK2,3.5e-2,0.);
           next1294=true;
        }
        else if (p<=99.02){
           nucltransK(fEnGamma[67],fEbindeK2,3.5e-2,0.);
           next1404=true;
        }
        else{
           nucltransK(fEnGamma[68],fEbindeK2,1.4e-1,0.);
           next1418=true;
        }
     }
     if (next1560){
        p=100.*GetRandom();
        if (p<=90.61){
           nucltransK(fEnGamma[69],fEbindeK2,1.1e-3,0.1e-4);
           next371=true;
        }
        else if (p<=90.62){
           nucltransK(fEnGamma[70],fEbindeK2,6.0e-2,0.);
           next996=true;
        }
        else if (p<=92.94){
           nucltransK(fEnGamma[71],fEbindeK2,1.6e-2,0.);
           next1264=true;
        }
        else if (p<=98.83){
           nucltransK(fEnGamma[72],fEbindeK2,1.7e-2,0.);
           next1277=true;
        }
        else{
           nucltransK(fEnGamma[73],fEbindeK2,5.0e-1,0.);
           next1398=true;
        }
     }
     if (next1531){
        p=100.*GetRandom();
        if (p<=0.99){
           nucltransK(fEnGamma[74],fEbindeK2,1.1e-3,0.4e-4);
           return;
        }
        else if (p<=4.80){
           nucltransK(fEnGamma[75],fEbindeK2,3.7e-3,0.2e-4);
           next123=true;
        }
        else if (p<=12.06){
           nucltransK(fEnGamma[76],fEbindeK2,2.2e-3,0.1e-4);
           next371=true;
        }
        else if (p<=52.17){
           nucltransK(fEnGamma[77],fEbindeK2,3.9e-3,0.);
           next681=true;
        }
        else if (p<=82.25){
           nucltransK(fEnGamma[78],fEbindeK2,1.3e-2,0.);
           next815=true;
        }
        else if (p<=90.87){
           nucltransK(fEnGamma[79],fEbindeK2,2.5e-2,0.);
           next996=true;
        }
        else if (p<=91.69){
           nucltransK(fEnGamma[80],fEbindeK2,1.6e-2,0.);
           next1048=true;
        }
        else if (p<=95.70){
           nucltransK(fEnGamma[81],fEbindeK2,2.8e-2,0.);
           next1128=true;
        }
        else if (p<=96.26){
           nucltransK(fEnGamma[82],fEbindeK2,1.8e-2,0.);
           next1241=true;
        }
        else if (p<=96.75){
           nucltransK(fEnGamma[83],fEbindeK2,1.9e-2,0.);
           next1252=true;
        }
        else if (p<=98.55){
           nucltransK(fEnGamma[84],fEbindeK2,8.0e-2,0.);
           next1264=true;
        }
        else if (p<=99.59){
           nucltransK(fEnGamma[85],fEbindeK2,1.2e-1,0.);
           next1294=true;
        }
        else{
           nucltransK(fEnGamma[86],fEbindeK2,2.0e-1,0.);
           next1414=true;
        }
     }
     if (next1510){
        p=100.*GetRandom();
        if (p<=20.){
           nucltransK(fEnGamma[87],fEbindeK2,6.0e-4,0.5e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[88],fEbindeK2,6.5e-4,0.3e-4);
           next123=true;
        }
     }
     if (next1418){
        p=100.*GetRandom();
        if (p<=11.51){
           nucltransK(fEnGamma[89],fEbindeK2,1.4e-3,0.2e-4);
           return;
        }
        else if (p<=20.34){
           nucltransK(fEnGamma[90],fEbindeK2,1.6e-3,0.1e-4);
           next123=true;
        }
        else if (p<=68.31){
           nucltransK(fEnGamma[91],fEbindeK2,2.5e-3,0.);
           next371=true;
        }
        else if (p<=69.27){
           nucltransK(fEnGamma[92],fEbindeK2,5.5e-3,0.);
           next681=true;
        }
        else if (p<=90.86){
           nucltransK(fEnGamma[93],fEbindeK2,3.8e-2,0.);
           next815=true;
        }
        else if (p<=91.87){
           nucltransK(fEnGamma[94],fEbindeK2,1.4e-1,0.);
           next996=true;
        }
        else if (p<=95.71){
           nucltransK(fEnGamma[95],fEbindeK2,3.2e-2,0.);
           next1048=true;
        }
        else if (p<=96.56){
           nucltransK(fEnGamma[96],fEbindeK2,6.5e-2,0.);
           next1128=true;
        }
        else if (p<=97.71){
           nucltransK(fEnGamma[97],fEbindeK2,1.2e-1,0.);
           next1182=true;
        }
        else if (p<=98.30){
           nucltransK(fEnGamma[98],fEbindeK2,6.0e-2,0.);
           next1241=true;
        }
        else if (p<=98.94){
           nucltransK(fEnGamma[99],fEbindeK2,7.0e-2,0.);
           next1252=true;
        }
        else if (p<=99.45){
           nucltransK(fEnGamma[100],fEbindeK2,1.2e-0,0.);
           next1294=true;
        }
        else{
           nucltransK(fEnGamma[101],fEbindeK2,1.2e-0,0.);
           next1296=true;
        }
     }
     if (next1414){
        p=100.*GetRandom();
        if (p<=21.22){
           nucltransK(fEnGamma[102],fEbindeK2,6.1e-4,0.4e-4);
           return;
        }
        else if (p<=97.01){
           nucltransK(fEnGamma[103],fEbindeK2,7.2e-4,0.2e-4);
           next123=true;
        }
        else if (p<=99.68){
           nucltransK(fEnGamma[104],fEbindeK2,3.5e-3,0.);
           next815=true;
        }
        else if (p<=99.79){
           nucltransK(fEnGamma[105],fEbindeK2,4.0e-1,0.);
           next1252=true;
        }
        else{
           nucltransK(fEnGamma[106],fEbindeK2,1.8e-1,0.);
           next1294=true;
        }
     }
     if (next1404){
        nucltransK(fEnGamma[107],fEbindeK2,1.1e-3,0.);
        next371=true;
     }
     if (next1398){
        p=100.*GetRandom();
        if (p<=96.877){
           nucltransK(fEnGamma[108],fEbindeK2,7.4e-4,0.2e-4);
           next123=true;
        }
        else if (p<=99.338){
           nucltransK(fEnGamma[109],fEbindeK2,3.4e-3,0.);
           next815=true;
        }
        else if (p<=99.876){
           nucltransK(fEnGamma[110],fEbindeK2,7.0e-2,0.);
           next996=true;
        }
        else if (p<=99.896){
           nucltransK(fEnGamma[111],fEbindeK2,2.2e-2,0.);
           next1128=true;
        }
        else if (p<=99.902){
           nucltransK(fEnGamma[112],fEbindeK2,2.3e-2,0.);
           next1136=true;
        }
        else if (p<=99.929){
           nucltransK(fEnGamma[113],fEbindeK2,9.0e-2,0.);
           next1241=true;
        }
        else{
           nucltransK(fEnGamma[114],fEbindeK2,9.5e-2,0.);
           next1252=true;
        }
     }
     if (next1296){
        p=100.*GetRandom();
        if (p<=0.73){
           particle(3,1.245,1.245,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
           return;
        }
        else if (p<=73.72){
           nucltransK(fEnGamma[115],fEbindeK2,2.2e-3,0.);
           next123=true;
        }
        else if (p<=74.45){
           particle(3,0.565,0.565,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
           next681=true;
        }
        else if (p<=78.83){
           nucltransK(fEnGamma[116],fEbindeK2,1.6e-2,0.);
           next815=true;
        }
        else{
           nucltransK(fEnGamma[117],fEbindeK2,6.0e-2,0.);
           next996=true;
        }
     }
     if (next1294){
        p=100.*GetRandom();
        if (p<=0.51){
           nucltransK(fEnGamma[118],fEbindeK2,1.8e-3,0.1e-4);
           return;
        }
        else if (p<=47.79){
           nucltransK(fEnGamma[119],fEbindeK2,2.2e-3,0.1e-4);
           next123=true;
        }
        else if (p<=83.25){
           nucltransK(fEnGamma[120],fEbindeK2,3.5e-3,0.);
           next371=true;
        }
        else{
           nucltransK(fEnGamma[121],fEbindeK2,1.7e-0,0.);
           next1182=true;
        }
     }
     if (next1277){
        nucltransK(fEnGamma[122],fEbindeK2,1.4e-3,0.);
        next371=true;
     }
     if (next1264){
        p=100.*GetRandom();
        if (p<=30.09){
           nucltransK(fEnGamma[123],fEbindeK2,2.1e-3,0.);
           next123=true;
        }
        else if (p<=95.92){
           nucltransK(fEnGamma[124],fEbindeK2,3.7e-3,0.);
           next371=true;
        }
        else if (p<=97.76){
           nucltransK(fEnGamma[125],fEbindeK2,1.2e-2,0.);
           next718=true;
        }
        else{
           nucltransK(fEnGamma[126],fEbindeK2,9.5e-2,0.);
           next996=true;
        }
     }
     if (next1252){
        p=100.*GetRandom();
        if (p<=3.76){
           nucltransK(fEnGamma[127],fEbindeK2,3.5e-3,0.);
           return;
        }
        else if (p<=80.51){
           nucltransK(fEnGamma[128],fEbindeK2,9.1e-4,0.1e-4);
           next123=true;
        }
        else{
           nucltransK(fEnGamma[129],fEbindeK2,1.5e-3,0.);
           next371=true;
        }
     }
     if (next1241){
        p=100.*GetRandom();
        if (p<=54.62){
           nucltransK(fEnGamma[130],fEbindeK2,7.7e-4,0.2e-4);
           return;
        }
        else if (p<=99.02){
           nucltransK(fEnGamma[131],fEbindeK2,9.3e-4,0.1e-4);
           next123=true;
        }
        else if (p<=99.68){
           nucltransK(fEnGamma[132],fEbindeK2,4.0e-3,0.);
           next681=true;
        }
        else{
           nucltransK(fEnGamma[133],fEbindeK2,7.0e-3,0.);
           next815=true;
        }
     }
     if (next1182){
        p=100.*GetRandom();
        if (p<=0.21){
           particle(3,1.132,1.132,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
           return;
        }
        else if (p<=84.00){
           nucltransK(fEnGamma[134],fEbindeK2,2.5e-3,0.);
           next123=true;
        }
        else if (p<=84.84){
           particle(3,0.451,0.451,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
           next681=true;
        }
        else{
           nucltransK(fEnGamma[135],fEbindeK2,3.3e-2,0.);
           next815=true;
        }
     }
     if (next1136){
        p=100.*GetRandom();
        if (p<=71.94){
           nucltransK(fEnGamma[136],fEbindeK2,2.2e-3,0.);
           return;
        }
        else{
           nucltransK(fEnGamma[137],fEbindeK2,2.8e-3,0.);
           next123=true;
        }
     }
     if (next1128){
        p=100.*GetRandom();
        if (p<=79.629){
           nucltransK(fEnGamma[138],fEbindeK2,2.8e-3,0.);
           next123=true;
        }
        else if (p<=99.855){
           nucltransK(fEnGamma[139],fEbindeK2,5.2e-3,0.);
           next371=true;
        }
        else if (p<=99.938){
           nucltransK(fEnGamma[140],fEbindeK2,5.5e-2,0.);
           next815=true;
        }
        else if (p<=99.987){
           nucltransK(fEnGamma[141],fEbindeK2,9.5e-1,0.);
           next996=true;
        }
        else{
           nucltransK(fEnGamma[142],fEbindeK2,6.0e-0,0.);
           next1048=true;
        }
     }
     if (next1048){
        p=100.*GetRandom();
        if (p<=23.98){
           nucltransK(fEnGamma[143],fEbindeK2,3.3e-3,0.);
           next123=true;
        }
        else if (p<=86.75){
           nucltransK(fEnGamma[144],fEbindeK2,5.1e-2,0.);
           next371=true;
        }
        else if (p<=90.58){
           nucltransK(fEnGamma[145],fEbindeK2,4.5e-2,0.);
           next718=true;
        }
        else{
           nucltransK(fEnGamma[146],fEbindeK2,1.4e-1,0.);
           next815=true;
        }
     }
     if (next996){
        p=100.*GetRandom();
        if (p<=46.43){
           nucltransK(fEnGamma[147],fEbindeK2,2.8e-3,0.);
           return;
        }
        else if (p<=98.59){
           nucltransK(fEnGamma[148],fEbindeK2,3.7e-3,0.);
           next123=true;
        }
        else if (p<=99.95){
           nucltransK(fEnGamma[149],fEbindeK2,8.0e-3,0.);
           next371=true;
        }
        else if (p<=99.98){
           nucltransK(fEnGamma[150],fEbindeK2,5.2e-2,0.);
           next681=true;
        }
        else{
           nucltransK(fEnGamma[151],fEbindeK2,3.5e-1,0.);
           next815=true;
        }
     }
     if (next815){
        fThlev=6.9e-12;
        p=100.*GetRandom();
        if (p<=17.86){
           nucltransK(fEnGamma[152],fEbindeK2,4.3e-3,0.);
           return;
         }
         else if (p<=80.18){
           nucltransK(fEnGamma[153],fEbindeK2,4.6e-2,0.);
           next123=true;
         }
         else if (p<=99.75){
           nucltransK(fEnGamma[154],fEbindeK2,1.9e-2,0.);
           next371=true;
         }
         else{
           nucltransK(fEnGamma[155],fEbindeK2,8.7e-1,0.);
           next681=true;
         }
     }
     if (next718){
        fThlev=7.8e-12;
        nucltransK(fEnGamma[156],fEbindeK2,3.9e-2,0.);
        next371=true;
     }
     if (next681){
        fThlev=4.0e-12;
        p=100.*GetRandom();
        if (p<=2.06){
           particle(3,0.631,0.631,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
           return;
        }
        else{
           nucltransK(fEnGamma[157],fEbindeK2,1.1e-2,0.);
           next123=true;
        }
     }
     if (next371){
        fThlev=45.2e-12;
        nucltransK(fEnGamma[158],fEbindeK2,1.1e-1,0.);
        next123=true;
     }
     if (next123){
        fThlev=1.186e-9;
        nucltransK(fEnGamma[159],fEbindeK2,1.2e-0,0.);
        return;
     }
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Gd146()
{
  // "Table of Isotopes", 7th ed., 1978.
  // VIT, 10.03.1996.
  float pdecay,p;
  bool next385=false, next230=false, next115=false;

  pdecay=100.*GetRandom();
  if (pdecay<= fProbDecay[0]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     p=100.*GetRandom();
     if (p<=36.){
        nucltransK(fEnGamma[1],fEbindeK,1.8e-2,0.);
        next230=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,4.5e-2,0.);
        next385=true;
     }
  }
  else if (pdecay<= fProbDecay[1]){
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     nucltransK(fEnGamma[3],fEbindeK,1.0e-2,0.);
     return;
  }
  else if (pdecay<= fProbDecay[2]){
     p=100.*GetRandom();
     if (p<=99.91) particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     else beta(fEndPoint[0],0.,0.);
     next385=true;
  }
  else{
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     next230=true;
  }
  if (next385){
     p=100.*GetRandom();
     if (p<=0.4){
        nucltransK(fEnGamma[4],fEbindeK,8.0e-2,0.);
        next115=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,6.5e-1,0.);
        next230=true;
     }
  }
  if (next230){
     p=100.*GetRandom();
     if (p<=0.2){
        nucltransK(fEnGamma[6],fEbindeK,1.4e-1,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,1.5e-0,0.);
        next115=true;
     }
  }
  if (next115){
     nucltransK(fEnGamma[8],fEbindeK,1.5e-0,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::I126()
{
  // J.Katakura et al.,Nucl.Data Sheets 97(2002)765).
  // VIT, 25.11.2003.
  float pdecay,pbeta,p,pec;
  bool next666=false, next389=false;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){     // 47.300% beta- to 126Xe
     fZdtr=fDaughterZ[0];
     pbeta=100.*GetRandom();
     if (pbeta<=fProbBeta[0]){     // 7.65%
        beta(fEndPoint[0],0.,0.);
        p=100.*GetRandom();
        if (p<=20.51){
           nucltransK(fEnGamma[0],fEbindeK,2.2e-3,0.);
           return;
        }
        else{
           nucltransK(fEnGamma[1],fEbindeK,9.5e-3,0.);
           next389=true;
        }
     }
     else if (pbeta<=fProbBeta[1]){// 70.57%
        beta(fEndPoint[1],0.,0.);
        next389=true;
     }
     else{
        beta(fEndPoint[2],0.,0.);
        return;
     }
     if (next389){
        fThlev=41.3e-12;
        nucltransK(fEnGamma[2],fEbindeK,1.9e-2,0.);
        return;
     }
  }
  else if(pdecay<=fProbDecay[1]){ // 51.692% EC    to 126Te
     particle(1,fEbindeK2,fEbindeK2,0.,pi,0.,twopi,0,0);
     pec=100.*GetRandom();
     if (pec<=fProbEC[0]){      // 0.014%
        p=100.*GetRandom();
        if (p<=66.00){
           nucltransK(fEnGamma[3],fEbindeK2,3.0e-4,2.3e-4);
           return;
        }
        else{
          nucltransK(fEnGamma[4],fEbindeK2,1.2e-3,0.7e-4);
          next666=true;
        }
     }
     else if (pec<= fProbEC[1]){//  0.001%
        nucltransK(fEnGamma[5],fEbindeK2,1.1e-3,0.1e-4);
        next666=true;
     }
     else if (pec<= fProbEC[2]){//  8.615%
        p=100.*GetRandom();
        if (p<=6.83){
           nucltransK(fEnGamma[6],fEbindeK2,7.0e-4,0.2e-4);
           return;
        }
        else{
           nucltransK(fEnGamma[7],fEbindeK2,2.8e-3,0.);
           next666=true;
        }
     }
     else if (pec<=fProbEC[3]){// 55.170%
        next666=true;
     }
     else return;          // 36.200%

     if (next666){
        nucltransK(fEnGamma[8],fEbindeK2,3.8e-3,0.);
        return;
     }
  }
  else{                    // 1.008% beta+ to 126Te
     pbeta=100.*GetRandom();
     fZdtr=fDaughterZ[1];
     if (pbeta<=fProbBeta[3]){
        beta(fEndPoint[3],0.,0.);
        nucltransK(fEnGamma[8],fEbindeK2,3.8e-3,0.);
        return;
     }
     else{
        beta(fEndPoint[4],0.,0.);
        return;
     }
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::I133()
{
  // S.Rab, NDS 75(1995)491.
  // VIT, 13.12.2003.
  bool next1386=false, next1052=false, next911=false, next875=false;
  bool  next680=false,  next530=false, next263=false, next233=false;
  float pbeta,p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<=0.72){
        nucltransK(fEnGamma[0],fEbindeK,1.0e-3,1.1e-4);
        return;
     }
     else if (p<=33.78){
        nucltransK(fEnGamma[1],fEbindeK,1.7e-3,0.);
        next530=true;
     }
     else if (p<=85.06){
        nucltransK(fEnGamma[2],fEbindeK,2.0e-2,0.);
        next680=true;
     }
     else if (p<=90.33){
        nucltransK(fEnGamma[3],fEbindeK,5.5e-3,0.);
        next911=true;
     }
     else if (p<=98.95){
        nucltransK(fEnGamma[4],fEbindeK,8.5e-3,0.);
        next1052=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,1.5e-1,0.);
        next1386=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     next1386=true;
  }
  else if (pbeta<= fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     p=100.*GetRandom();
     if (p<=37.48){
        nucltransK(fEnGamma[6],fEbindeK,9.9e-4,0.6e-4);
        return;
     }
     else if (p<=40.53){
        nucltransK(fEnGamma[7],fEbindeK,1.3e-3,0.);
        next263=true;
     }
     else if (p<=79.26){
        nucltransK(fEnGamma[8],fEbindeK,3.1e-3,0.);
        next530=true;
     }
     else if (p<=90.00){
        nucltransK(fEnGamma[9],fEbindeK,5.0e-3,0.);
        next680=true;
     }
     else{
        nucltransK(fEnGamma[10],fEbindeK,1.4e-2,0.);
        next911=true;
     }
  }
  else if (pbeta<= fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     p=100.*GetRandom();
     if (p<=62.29){
        nucltransK(fEnGamma[11],fEbindeK,9.6e-4,0.5e-4);
        return;
     }
     else if (p<=62.52){
        nucltransK(fEnGamma[12],fEbindeK,1.5e-3,0.);
        next263=true;
     }
     else if (p<=74.71){
        nucltransK(fEnGamma[13],fEbindeK,3.5e-3,0.);
        next530=true;
     }
     else if (p<=89.13){
        nucltransK(fEnGamma[14],fEbindeK,5.9e-3,0.);
        next680=true;
     }
     else if (p<=97.51){
        nucltransK(fEnGamma[15],fEbindeK,1.5e-2,0.);
        next875=true;
     }
     else if (p<=99.07){
        nucltransK(fEnGamma[16],fEbindeK,2.0e-2,0.);
        next911=true;
     }
     else{
        nucltransK(fEnGamma[17],fEbindeK,8.0e-2,0.);
        next1052=true;
     }
  }
  else if (pbeta<= fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     p=100.*GetRandom();
     if (p<=47.94){
        nucltransK(fEnGamma[18],fEbindeK,1.0e-3,0.1e-4);
        return;
     }
     else if (p<=95.88){
        nucltransK(fEnGamma[19],fEbindeK,4.3e-3,0.);
        next530=true;
     }
     else if (p<=96.51){
        nucltransK(fEnGamma[20],fEbindeK,7.0e-3,0.);
        next680=true;
     }
     else{
        nucltransK(fEnGamma[21],fEbindeK,3.0e-2,0.);
        next875=true;
     }
  }
  else if (pbeta<= fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     next1052=true;
  }
  else if (pbeta<=fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     next875=true;
  }
  else if (pbeta<=fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     nucltransK(fEnGamma[22],fEbindeK,9.0e-3,0.);
     next233=true;
  }
  else if (pbeta<=fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     p=100.*GetRandom();
     if (p<=99.87){
        nucltransK(fEnGamma[23],fEbindeK,8.2e-3,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[24],fEbindeK,6.0e-2,0.);
        next263=true;
     }
  }
  else{
      beta(fEndPoint[9],0.,0.);
      next233=true;
  }
  if (next1386){
     p=100.*GetRandom();
     if (p<=0.72){
        nucltransK(fEnGamma[25],fEbindeK,3.5e-3,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[26],fEbindeK,2.6e-3,0.);
        next530=true;
     }
  }
  if (next1052){
     p=100.*GetRandom();
     if (p<=90.26){
        nucltransK(fEnGamma[27],fEbindeK,2.0e-3,0.);
        return;
     }
     else if (p<=98.37){
        nucltransK(fEnGamma[28],fEbindeK,3.0e-3,0.);
        next263=true;
     }
     else{
        nucltransK(fEnGamma[29],fEbindeK,2.5e-2,0.);
        next680=true;
     }
  }
  if (next911){
     p=100.*GetRandom();
     if (p<=31.08){
        nucltransK(fEnGamma[30],fEbindeK,3.0e-3,0.);
        return;
     }
     else if (p<=69.59){
        nucltransK(fEnGamma[31],fEbindeK,6.0e-3,0.);
        next263=true;
     }
     else{
        nucltransK(fEnGamma[32],fEbindeK,2.0e-2,0.);
        next530=true;
     }
  }
  if (next875){
     p=100.*GetRandom();
     if (p<=97.75){
        nucltransK(fEnGamma[33],fEbindeK,2.2e-3,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[34],fEbindeK,3.0e-2,0.);
        next530=true;
     }
  }
  if (next680){
     p=100.*GetRandom();
     if (p<=77.66){
        nucltransK(fEnGamma[35],fEbindeK,5.4e-3,0.);
        return;
     }
     else if (p<=96.42){
        nucltransK(fEnGamma[36],fEbindeK,1.6e-2,0.);
        next263=true;
     }
     else{
        nucltransK(fEnGamma[37],fEbindeK,3.0e-1,0.);
        next530=true;
     }
  }
  if (next530){
     p=100.*GetRandom();
     if (p<=99.87){
        nucltransK(fEnGamma[38],fEbindeK,8.2e-3,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[39],fEbindeK,6.0e-2,0.);
        next263=true;
     }
  }
  if (next263){
     nucltransK(fEnGamma[40],fEbindeK,5.8e-2,0.);
     return;
  }
  if (next233){
      fThlev=189216.;
      nucltransK(fEnGamma[41],fEbindeK,8.8,0.);
      return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::I134()
{
  // Yu.V.Sergeenkov, NDS 71(1994)557.
  // VIT, 8.10.2002.
  bool next3314=false, next2654=false, next2588=false, next2548=false;
  bool next2408=false, next2353=false, next2302=false, next2272=false;
  bool next2137=false, next1920=false, next1731=false, next1614=false, next847=false;
  float pbeta,p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<=5.17){
        nucltransK(fEnGamma[0],fEbindeK,1.5e-4,5.2e-4);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,2.0e-3,0.2e-4);
        next2302=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100.*GetRandom();
     if (p<=18.77){
        nucltransK(fEnGamma[2],fEbindeK,1.5e-4,5.1e-4);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,2.5e-1,0.);
        next3314=true;
     }
  }
  else if (pbeta<= fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     p=100.*GetRandom();
     if (p<=21.79){
        nucltransK(fEnGamma[4],fEbindeK,1.5e-3,1.2e-4);
        next1731=true;
     }
     else if (p<=33.52){
        nucltransK(fEnGamma[5],fEbindeK,1.7e-3,0.4e-4);
        next2137=true;
     }
     else if (p<=78.21){
        nucltransK(fEnGamma[6],fEbindeK,2.1e-3,0.1e-4);
        next2272=true;
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,2.5e-3,0.);
        next2408=true;
     }
  }
  else if (pbeta<= fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     p=100.*GetRandom();
     if (p<=6.16){
        nucltransK(fEnGamma[8],fEbindeK,2.0e-4,4.6e-4);
        next847=true;
     }
     else if (p<=23.64){
        nucltransK(fEnGamma[9],fEbindeK,9.0e-4,1.1e-4);
        next1731=true;
     }
     else{
        nucltransK(fEnGamma[10],fEbindeK,5.0e-3,0.);
        next2654=true;
     }
  }
  else if (pbeta<= fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     next3314=true;
  }
  else if (pbeta<= fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     nucltransK(fEnGamma[11],fEbindeK,2.0e-4,4.4e-4);
     next847=true;
  }
  else if (pbeta<= fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     p=100.*GetRandom();
     if (p<=35.78){
        nucltransK(fEnGamma[12],fEbindeK,2.1e-4,4.1e-4);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[13],fEbindeK,1.2e-3,0.6e-4);
        next1920=true;
     }
  }
  else if (pbeta<= fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     p=100.*GetRandom();
     if (p<=3.95){
        nucltransK(fEnGamma[14],fEbindeK,3.0e-4,3.2e-4);
        next847=true;
     }
     else if (p<=59.80){
        nucltransK(fEnGamma[15],fEbindeK,1.0e-3,0.9e-4);
        next1614=true;
     }
     else if (p<=90.32){
        nucltransK(fEnGamma[16],fEbindeK,1.2e-3,0.6e-4);
        next1731=true;
     }
     else{
        nucltransK(fEnGamma[17],fEbindeK,1.6e-3,0.2e-4);
        next1920=true;
     }
  }
  else if (pbeta<=fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     p=100.*GetRandom();
     if (p<=0.62){
        nucltransK(fEnGamma[18],fEbindeK,3.0e-4,2.3e-4);
        next847=true;
     }
     else if (p<=30.54){
        nucltransK(fEnGamma[19],fEbindeK,1.6e-3,0.3e-4);
        next1731=true;
     }
     else if (p<=43.73){
        nucltransK(fEnGamma[20],fEbindeK,2.4e-3,0.);
        next1920=true;
     }
     else if (p<=49.75){
        nucltransK(fEnGamma[21],fEbindeK,3.4e-3,0.);
        next2137=true;
     }
     else if (p<=86.25){
        nucltransK(fEnGamma[22],fEbindeK,7.2e-3,0.);
        next2272=true;
     }
     else if (p<=93.65){
        nucltransK(fEnGamma[23],fEbindeK,8.0e-3,0.);
        next2353=true;
     }
     else if (p<=97.99){
        nucltransK(fEnGamma[24],fEbindeK,1.1e-2,0.);
        next2408=true;
     }
     else if (p<=99.50){
        nucltransK(fEnGamma[25],fEbindeK,3.7e-2,0.);
        next2548=true;
     }
     else{
        nucltransK(fEnGamma[26],fEbindeK,5.2e-2,0.);
        next2588=true;
     }
  }
  else if (pbeta<=fProbBeta[9]){
     beta(fEndPoint[9],0.,0.);
     p=100.*GetRandom();
     if (p<=34.62){
        nucltransK(fEnGamma[27],fEbindeK,4.0e-4,2.0e-4);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[28],fEbindeK,7.0e-4,0.3e-4);
        next1614=true;
     }
  }
  else if (pbeta<=fProbBeta[10]){
     beta(fEndPoint[10],0.,0.);
     next2654=true;
  }
  else if (pbeta<=fProbBeta[11]){
     beta(fEndPoint[11],0.,0.);
     next2588=true;
  }
  else if (pbeta<=fProbBeta[12]){
     beta(fEndPoint[12],0.,0.);
     next2548=true;
  }
  else if (pbeta<=fProbBeta[13]){
     beta(fEndPoint[13],0.,0.);
     next2408=true;
  }
  else if (pbeta<=fProbBeta[14]){
     beta(fEndPoint[14],0.,0.);
     next2353=true;
  }
  else if (pbeta<=fProbBeta[15]){
     beta(fEndPoint[15],0.,0.);
     next2302=true;
  }
  else if (pbeta<=fProbBeta[16]){
     beta(fEndPoint[16],0.,0.);
     next2272=true;
  }
  else if (pbeta<=fProbBeta[17]){
     beta(fEndPoint[17],0.,0.);
     next1920=true;
  }
  else{
     beta(fEndPoint[18],0.,0.);
     next1731=true;
  }

  if (next3314){
     p=100.*GetRandom();
     if (p<=63.81){
        nucltransK(fEnGamma[29],fEbindeK,2.0e-4,4.5e-4);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[30],fEbindeK,1.2e-3,0.7e-4);
        next1920=true;
     }
  }
  if (next2654){
     p=100.*GetRandom();
     if (p<=71.82){
        nucltransK(fEnGamma[31],fEbindeK,6.0e-4,1.7e-4);
        next847=true;
     }
     else if (p<=98.18){
        nucltransK(fEnGamma[32],fEbindeK,2.0e-3,0.);
        next1614=true;
     }
     else{
        nucltransK(fEnGamma[33],fEbindeK,2.1e-3,0.);
        next1731=true;
     }
  }
  if (next2588){
     p=100.*GetRandom();
     if (p<=15.66){
        nucltransK(fEnGamma[34],fEbindeK,6.0e-4,0.8e-4);
        next847=true;
     }
     else if (p<=44.90){
        nucltransK(fEnGamma[35],fEbindeK,1.7e-3,0.);
        next1614=true;
     }
     else if (p<=85.87){
        nucltransK(fEnGamma[36],fEbindeK,2.9e-3,0.);
        next1731=true;
     }
     else{
        nucltransK(fEnGamma[37],fEbindeK,8.5e-2,0.);
        next2353=true;
     }
  }
  if (next2548){
     p=100.*GetRandom();
     if (p<=13.78){
        nucltransK(fEnGamma[38],fEbindeK,4.0e-3,0.);
        next1731=true;
     }
     else if (p<=63.11){
        nucltransK(fEnGamma[39],fEbindeK,4.9e-3,0.);
        next1920=true;
     }
     else if (p<=75.78){
        nucltransK(fEnGamma[40],fEbindeK,2.0e-2,0.);
        next2137=true;
     }
     else{
        nucltransK(fEnGamma[41],fEbindeK,4.5e-1,0.);
        next2408=true;
     }
  }
  if (next2408){
     p=100.*GetRandom();
     if (p<=84.57){
        nucltransK(fEnGamma[42],fEbindeK,5.3e-3,0.);
        next1731=true;
     }
     else{
        nucltransK(fEnGamma[43],fEbindeK,9.6e-3,0.);
        next1920=true;
     }
  }
  if (next2353){
     p=100.*GetRandom();
     if (p<=4.42){
        nucltransK(fEnGamma[44],fEbindeK,3.3e-3,0.);
        next1614=true;
     }
     else if (p<=73.01){
        nucltransK(fEnGamma[45],fEbindeK,6.1e-3,0.);
        next1731=true;
     }
     else{
        nucltransK(fEnGamma[46],fEbindeK,1.6e-2,0.);
        next1920=true;
     }
  }
  if (next2302){
     p=100.*GetRandom();
     if (p<=88.08){
        nucltransK(fEnGamma[47],fEbindeK,1.1e-3,0.8e-4);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[48],fEbindeK,9.0e-3,0.);
        next1731=true;
     }
  }
  if (next2272){
     p=100.*GetRandom();
     if (p<=57.01){
        nucltransK(fEnGamma[49],fEbindeK,7.7e-3,0.);
        next1731=true;
     }
     else{
        nucltransK(fEnGamma[50],fEbindeK,3.5e-1,0.);
        next2137=true;
     }
  }
  if (next2137){
     p=100.*GetRandom();
     if (p<=96.64){
        nucltransK(fEnGamma[51],fEbindeK,1.8e-2,0.);
        next1731=true;
     }
     else{
        nucltransK(fEnGamma[52],fEbindeK,1.2e-1,0.);
        next1920=true;
     }
  }
  if (next1920){
     p=100.*GetRandom();
     if (p<=94.36){
        nucltransK(fEnGamma[53],fEbindeK,1.9e-3,0.1e-4);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[54],fEbindeK,1.7e-1,0.);
        next1731=true;
     }
  }
  if (next1731){
     nucltransK(fEnGamma[55],fEbindeK,2.2e-3,0.);
     next847=true;
  }
  if (next1614){
     p=100.*GetRandom();
     if (p<=50.83){
        nucltransK(fEnGamma[56],fEbindeK,3.0e-4,0.5e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[57],fEbindeK,3.2e-3,0.);
        next847=true;
     }
  }
  if (next847){
     fThlev=1.9e-12;
     nucltransK(fEnGamma[58],fEbindeK,2.4e-3,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::I135()
{
  // Yu.V.Sergeenkov et al., Nucl. Data Sheets 84 (1998) 115.
 // VIT, 12.10.2002.
  bool next2093=false, next2049=false, next1927=false, next1791=false;
  bool next1781=false,  next1678=false, next1565=false, next1544=false;
  bool next1458=false, next1448=false, next1260=false, next1132=false;
  bool next527=false,  next288=false;
  float pbeta,p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<=9.72){
        nucltransK(fEnGamma[0],fEbindeK,2.0e-4,4.5e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[1],fEbindeK,3.0e-4,3.1e-4);
        next288=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
      beta(fEndPoint[1],0.,0.);
      p=100.*GetRandom();
      if (p<=45.00){
         nucltransK(fEnGamma[2],fEbindeK,2.4e-4,2.1e-4);
         next527=true;
      }
      else{
         nucltransK(fEnGamma[3],fEbindeK,4.0e-4,0.7e-4);
         next1132=true;
      }
  }
  else if (pbeta<= fProbBeta[2]){
      beta(fEndPoint[2],0.,0.);
      p=100.*GetRandom();
      if (p<=56.69){
         nucltransK(fEnGamma[4],fEbindeK,4.0e-4,5.3e-4);
         return;
      }
      else if (p<=81.89){
         nucltransK(fEnGamma[5],fEbindeK,1.2e-3,0.6e-4);
         next1132=true;
      }
      else{
         nucltransK(fEnGamma[6],fEbindeK,4.0e-3,0.);
         next1781=true;
      }
  }
  else if (pbeta<= fProbBeta[3]){
      beta(fEndPoint[3],0.,0.);
      p=100.*GetRandom();
      if (p<=46.81){
         nucltransK(fEnGamma[7],fEbindeK,4.5e-4,0.5e-4);
         next1132=true;
      }
      else{
         nucltransK(fEnGamma[8],fEbindeK,1.6e-3,0.);
         next1791=true;
      }
  }
  else if (pbeta<= fProbBeta[4]){
      beta(fEndPoint[4],0.,0.);
      p=100.*GetRandom();
      if (p<=93.48){
         nucltransK(fEnGamma[9],fEbindeK,3.5e-4,4.1e-4);
         return;
      }
      else if (p<=96.40){
         nucltransK(fEnGamma[10],fEbindeK,2.6e-3,0.);
         next1448=true;
      }
      else{
         nucltransK(fEnGamma[11],fEbindeK,7.0e-3,0.);
         next1791=true;
      }
  }
  else if (pbeta<= fProbBeta[5]){
      beta(fEndPoint[5],0.,0.);
      p=100.*GetRandom();
      if (p<=0.66){
         nucltransK(fEnGamma[12],fEbindeK,2.6e-4,1.8e-4);
         next527=true;
      }
      else{
         nucltransK(fEnGamma[13],fEbindeK,1.4e-3,0.5e-4);
         next1132=true;
      }
  }
  else if (pbeta<= fProbBeta[6]){
      beta(fEndPoint[6],0.,0.);
      p=100.*GetRandom();
      if (p<=41.91){
         nucltransK(fEnGamma[14],fEbindeK,2.6e-4,1.6e-4);
         next527=true;
      }
      else if (p<=45.02){
         nucltransK(fEnGamma[15],fEbindeK,1.4e-3,0.4e-4);
         next1132=true;
      }
      else if (p<=51.45){
         nucltransK(fEnGamma[16],fEbindeK,1.4e-3,0.);
         next1260=true;
      }
      else if (p<=55.42){
         nucltransK(fEnGamma[17],fEbindeK,6.0e-3,0.);
         next1678=true;
      }
      else if (p<=64.74){
         nucltransK(fEnGamma[18],fEbindeK,8.5e-3,0.);
         next1781=true;
      }
      else if (p<=86.41){
         nucltransK(fEnGamma[19],fEbindeK,1.8e-2,0.);
         next1927=true;
      }
      else{
         nucltransK(fEnGamma[20],fEbindeK,6.5e-2,0.);
         next2093=true;
      }
  }
  else if (pbeta<= fProbBeta[7]){
      beta(fEndPoint[7],0.,0.);
      p=100.*GetRandom();
      if (p<=12.88){
         nucltransK(fEnGamma[21],fEbindeK,3.7e-4,3.8e-4);
         return;
      }
      else if (p<=89.34){
         nucltransK(fEnGamma[22],fEbindeK,1.7e-3,0.2e-4);
         next1132=true;
      }
      else if (p<=92.51){
         nucltransK(fEnGamma[23],fEbindeK,1.9e-3,0.);
         next1260=true;
      }
      else if (p<=93.48){
         nucltransK(fEnGamma[24],fEbindeK,2.6e-3,0.);
         next1448=true;
      }
      else if (p<=97.07){
         nucltransK(fEnGamma[25],fEbindeK,4.0e-3,0.);
         next1458=true;
      }
      else if (p<=99.79){
         nucltransK(fEnGamma[26],fEbindeK,5.5e-3,0.);
         next1565=true;
      }
      else{
         nucltransK(fEnGamma[27],fEbindeK,2.2e-1,0.);
         next2093=true;
      }
  }
  else if (pbeta<=fProbBeta[8]){
      beta(fEndPoint[8],0.,0.);
      p=100.*GetRandom();
      if (p<=55.75){
         nucltransK(fEnGamma[28],fEbindeK,2.8e-4,1.0e-4);
         next527=true;
      }
      else if (p<=77.64){
         nucltransK(fEnGamma[29],fEbindeK,1.4e-3,0.1e-4);
         next1132=true;
      }
      else if (p<=94.09){
         nucltransK(fEnGamma[30],fEbindeK,1.8e-3,0.);
         next1260=true;
      }
      else if (p<=98.39){
         nucltransK(fEnGamma[31],fEbindeK,1.6e-2,0.);
         next1781=true;
      }
      else if (p<=99.68){
         nucltransK(fEnGamma[32],fEbindeK,4.0e-2,0.);
         next1927=true;
      }
      else{
         nucltransK(fEnGamma[33],fEbindeK,4.0e-2,0.);
         next2049=true;
      }
  }
  else if (pbeta<=fProbBeta[9]){
      beta(fEndPoint[9],0.,0.);
      nucltransK(fEnGamma[34],fEbindeK,2.0e-4,2.9e-4);
      return;
  }
  else if (pbeta<=fProbBeta[10]){
      beta(fEndPoint[10],0.,0.);
      nucltransK(fEnGamma[35],fEbindeK,2.0e-4,2.7e-4);
      return;
  }
  else if (pbeta<=fProbBeta[11]){
      beta(fEndPoint[11],0.,0.);
      next2093=true;
  }
  else if (pbeta<=fProbBeta[12]){
      beta(fEndPoint[12],0.,0.);
      p=100.*GetRandom();
      if (p<=79.31){
         nucltransK(fEnGamma[36],fEbindeK,4.5e-4,2.3e-4);
         return;
      }
      else if (p<=93.16){
         nucltransK(fEnGamma[37],fEbindeK,3.6e-3,0.);
         next1260=true;
      }
      else if (p<=97.90){
         nucltransK(fEnGamma[38],fEbindeK,8.5e-3,0.);
         next1458=true;
      }
      else{
         nucltransK(fEnGamma[39],fEbindeK,6.5e-2,0.);
         next1791=true;
      }
  }
  else if (pbeta<=fProbBeta[13]){
      beta(fEndPoint[13],0.,0.);
      p=100.*GetRandom();
      if (p<=0.21){
         nucltransK(fEnGamma[40],fEbindeK,0.8e-4,1.0e-3);
         next527=true;
      }
      else if (p<=84.72){
         nucltransK(fEnGamma[41],fEbindeK,2.5e-3,0.);
         next1132=true;
      }
      else if (p<=93.05){
         nucltransK(fEnGamma[42],fEbindeK,5.0e-3,0.);
         next1260=true;
      }
      else if (p<=95.98){
         nucltransK(fEnGamma[43],fEbindeK,1.8e-2,0.);
         next1565=true;
      }
      else{
         nucltransK(fEnGamma[44],fEbindeK,4.6e-2,0.);
         next1678=true;
      }
  }
  else if (pbeta<=fProbBeta[14]){
      beta(fEndPoint[14],0.,0.);
      next1927=true;
  }
  else if (pbeta<=fProbBeta[15]){
      beta(fEndPoint[15],0.,0.);
      nucltransK(fEnGamma[45],fEbindeK,9.0e-4,0.2e-4);
      next527=true;
  }
  else if (pbeta<=fProbBeta[16]){
      beta(fEndPoint[16],0.,0.);
      next1791=true;
  }
  else if (pbeta<=fProbBeta[17]){
     beta(fEndPoint[17],0.,0.);
     next1678=true;
  }
  else if (pbeta<=fProbBeta[18]){
     beta(fEndPoint[18],0.,0.);
     next1565=true;
  }
  else if (pbeta<=fProbBeta[19]){
     beta(fEndPoint[19],0.,0.);
     next1458=true;
  }
  else if (pbeta<=fProbBeta[20]){
     beta(fEndPoint[20],0.,0.);
     next1260=true;
  }
  else if (pbeta<=fProbBeta[21]){
     beta(fEndPoint[21],0.,0.);
     next1132=true;
  }
  else{
     beta(fEndPoint[22],0.,0.);
     next527=true;
  }

  if (next2093){
      p=100.*GetRandom();
      if (p<=72.79){
         nucltransK(fEnGamma[46],fEbindeK,2.8e-4,0.7e-4);
         next527=true;
      }
      else if (p<=81.26){
         nucltransK(fEnGamma[47],fEbindeK,2.6e-3,0.);
         next1132=true;
      }
      else if (p<=98.25){
         nucltransK(fEnGamma[48],fEbindeK,2.0e-2,0.);
         next1678=true;
      }
      else{
         nucltransK(fEnGamma[49],fEbindeK,2.2e-1,0.);
         next1927=true;
      }
  }
  if (next2049){
     nucltransK(fEnGamma[50],fEbindeK,9.0e-4,0.9e-4);
     next527=true;
  }
  if (next1927){
      p=100.*GetRandom();
      if (p<= 58.50){
         nucltransK(fEnGamma[51],fEbindeK,5.0e-4,1.4e-4);
         return;
      }
      else if (p<=63.05){
         nucltransK(fEnGamma[52],fEbindeK,4.0e-3,0.);
         next1132=true;
      }
      else{
         nucltransK(fEnGamma[53],fEbindeK,2.8e-2,0.);
         next1565=true;
      }
  }
  if (next1791){
      p=100.*GetRandom();
      if (p<=86.68){
        nucltransK(fEnGamma[54],fEbindeK,6.5e-4,1.6e-4);
        return;
     }
     else if (p<=98.81){
        nucltransK(fEnGamma[55],fEbindeK,7.5e-4,0.3e-4);
        next288=true;
     }
     else if (p<=99.17){
        nucltransK(fEnGamma[56],fEbindeK,9.3e-3,0.);
        next1260=true;
     }
     else if (p<=99.18){
        nucltransK(fEnGamma[57],fEbindeK,3.0e-2,0.);
        next1448=true;
     }
     else if (p<=99.60){
        nucltransK(fEnGamma[58],fEbindeK,3.5e-2,0.);
        next1458=true;
     }
     else if (p<=99.92){
        nucltransK(fEnGamma[59],fEbindeK,8.0e-3,0.);
        next1544=true;
     }
     else{
        nucltransK(fEnGamma[60],fEbindeK,6.0e-1,0.);
        next1678=true;
     }
  }
  if (next1781){
     p=100.*GetRandom();
      if (p<=1.29){
         nucltransK(fEnGamma[61],fEbindeK,4.7e-4,0.3e-4);
         next527=true;
      }
      else{
         nucltransK(fEnGamma[62],fEbindeK,4.5e-3,0.);
         next1132=true;
      }
  }
  if (next1678){
     p=100.*GetRandom();
     if (p<=42.57){
        nucltransK(fEnGamma[63],fEbindeK,5.6e-4,0.7e-4);
        return;
     }
     else if (p<=74.58){
        nucltransK(fEnGamma[64],fEbindeK,9.1e-3,0.);
        next1132=true;
     }
     else if (p<=90.20){
        nucltransK(fEnGamma[65],fEbindeK,1.6e-2,0.);
        next1260=true;
     }
     else if (p<=91.38){
        nucltransK(fEnGamma[66],fEbindeK,1.0e-1,0.);
        next1448=true;
     }
     else if (p<=99.94){
        nucltransK(fEnGamma[67],fEbindeK,1.0e-1,0.);
        next1458=true;
     }
     else{
        nucltransK(fEnGamma[68],fEbindeK,6.0e-1,0.);
        next1565=true;
     }
  }
  if (next1565){
     p=100.*GetRandom();
     if (p<=93.00){
        nucltransK(fEnGamma[69],fEbindeK,6.4e-4,0.);
        next527=true;
     }
     else if (p<=99.62){
        nucltransK(fEnGamma[70],fEbindeK,1.6e-2,0.);
        next1132=true;
     }
     else{
        nucltransK(fEnGamma[71],fEbindeK,4.0e-2,0.);
        next1260=true;
     }
  }
  if (next1544){
     p=100.*GetRandom();
     if (p<=81.25){
        nucltransK(fEnGamma[72],fEbindeK,9.0e-4,1.0e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[73],fEbindeK,1.2e-3,0.5e-4);
        next288=true;
     }
  }
  if (next1458){
     p=100.*GetRandom();
     if (p<=90.44){
        nucltransK(fEnGamma[74],fEbindeK,7.0e-4,0.9e-4);
        return;
     }
     else if (p<=99.59){
        nucltransK(fEnGamma[75],fEbindeK,1.3e-3,0.);
        next288=true;
     }
     else if (p<=99.61){
        nucltransK(fEnGamma[76],fEbindeK,3.5e-2,0.);
        next1132=true;
     }
     else{
        nucltransK(fEnGamma[77],fEbindeK,1.5e-1,0.);
        next1260=true;
     }
  }
  if (next1448){
     p=100.*GetRandom();
     if (p<=75.65){
        nucltransK(fEnGamma[78],fEbindeK,7.0e-4,0.9e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[79],fEbindeK,1.3e-3,0.);
        next288=true;
     }
  }
  if (next1260){
     p=100.*GetRandom();
     if (p<=96.99){
        nucltransK(fEnGamma[80],fEbindeK,1.2e-3,0.1e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[81],fEbindeK,1.8e-3,0.);
        next288=true;
     }
  }
  if (next1132){
     nucltransK(fEnGamma[82],fEbindeK,1.3e-3,0.);
     return;
  }
  if (next527){
     fThlev=917.4;
     nucltransK(fEnGamma[83],fEbindeK,2.4e-1,0.);
     return;
  }
  if (next288){
     nucltransK(fEnGamma[84],fEbindeK,4.7e-2,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::K40()
{
  // J.A.Cameron et al., ENSDF 29.10.2002.
  // 3rd forbidden unique shape for beta decay, VIT, 27.10.2006
  // in accordance with H.Daniel, RMP 40(1968)659 and
  // W.H.Kelly et al., Nucl. Phys. 11(1959)492

  float pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){
     beta2f(fEndPoint[0],0.,0.,3,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
  }
  else if (pdecay<=fProbDecay[1]){ // 10.660% ec 40Ar(1461)
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
     fThlev=1.12e-12;
     nucltransK(fEnGamma[1],fEbindeK,3.0e-5,7.3e-5);
  }
  else if (pdecay<=fProbDecay[2]){        // 0.199% ec 40Ar(gs)
     particle(1,fEnGamma[0],fEnGamma[0],0.,pi,0.,twopi,0,0);
  }
  else{                            // 0.001% b+ 40Ar(gs)
     fZdtr=fDaughterZ[1];
     beta(fEndPoint[1],0.,0.);
  }
  return;
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::K42()
{
  // B.Singh et al., NDS 92(2001)1
  // with additional information from ToI-1978 and ToI-1998.
  // VIT, 31.03.2006; 29.10.2006.
  bool next2753=false, next2424=false, next1525=false;
  float pbeta,p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     fThlev=250.e-15;  // ToI-1998
     p=100.*GetRandom();
     if (p<=63.66){
        nucltransK(fEnGamma[0],fEbindeK,1.2e-5,5.8e-4);
        next1525=true;
     }
     else if (p<=94.88){
        nucltransK(fEnGamma[1],fEbindeK,3.9e-5,0.);
        next2424=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,8.7e-5,0.);
        next2753=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     next2424=true;
  }
  else if (pbeta<= fProbBeta[2]){
     beta1fu(fEndPoint[2],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
     fThlev=0.33e-9;
     p=100.*GetRandom();
     if (p<=2.1){  // ToI-1978
        p=100.*GetRandom();
        if (p<=90.){
           pair(0.815);
        }
        else{
           particle(3,1.833,1.833,0.,pi,0.,twopi,fTclev,fThlev);
           particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
        }
        return;
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,3.4e-3,0.);
        next1525=true;
     }
  }
  else if (pbeta<=fProbBeta[3]){
     beta1f(fEndPoint[3],0.,0.,fShCorrFactor[4],fShCorrFactor[5],fShCorrFactor[6],fShCorrFactor[7]);
     next1525=true;
  }
  else{
     beta1fu(fEndPoint[4],0.,0.,fShCorrFactor[8],fShCorrFactor[9],fShCorrFactor[10],fShCorrFactor[11]);
     return;
  }
  if (next2753){
     fThlev=3.0e-12; // ToI-1998
     nucltransK(fEnGamma[4],fEbindeK,5.6e-5,1.4e-5);
     next1525=true;
  }
  if (next2424){
     fThlev=140.e-15; // ToI-1998
     p=100.*GetRandom();
     if (p<=27.78){
        nucltransK(fEnGamma[5],fEbindeK,1.5e-5,5.2e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[6],fEbindeK,8.3e-5,0.);
        next1525=true;
     }
  }
  if (next1525){
     fThlev=0.82e-12;  // ToI-1998
     nucltransK(fEnGamma[7],fEbindeK,3.6e-5,9.8e-5);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Pa234m()
{
  // Model (not the exact) scheme of Pa234m decay
  // ("Table of Isotopes",7th ed., 1978): decays of Pa234m
  // to excited levels of U234 with energies greater than
  // 1.045 MeV are not considered (p=0.20%).
  // VIT, 18.08.1992, 22.10.1995;
  // VIT, 3.11.2006 (update to NDS 71(1994)181 -however, levels
  // with E_exc>1.045 MeV are still not considered;
  // change of beta shape of the gs-to-gs beta decay
  // to the 1st forbidden with exp. corr.).
  bool next927=false, next852=false, next849=false, next810=false;
  bool next786=false, next143=false, next43=false;
  float pdecay, pbeta, p;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){  // IT to Pa234
     nucltransK(fEnGamma[0],0.021,1.1e+1,0.);
     return;
  }
  pbeta=100.*GetRandom();      // beta decay to U234
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<=83.7){
        nucltransK(fEnGamma[1],fEbindeK,1.1e-2,0.);
        next43=true;
     }
     else if (p<=91.3){
        nucltransK(fEnGamma[2],fEbindeK,5.5e-2,0.);
        next786=true;
     }
     else if (p<=99.9){
        particle(3,0.120,0.120,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        next810=true;
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,8.8e-1,0.);
        next852=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100.*GetRandom();
     if (p<=44.8){
        nucltransK(fEnGamma[4],fEbindeK,4.1e-3,0.);
        next43=true;
     }
     else if (p<=56.1){
        nucltransK(fEnGamma[5],fEbindeK,1.5e0,0.);
        next786=true;
     }
     else if (p<=92.3){
        nucltransK(fEnGamma[6],fEbindeK,5.5e0,0.);
        next849=true;
     }
     else{
        nucltransK(fEnGamma[7],0.022,4.3e-1,0.);
        next927=true;
     }
  }
  else if (pbeta<=fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     next852=true;
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     next810=true;
  }
  else if (pbeta<=fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     next786=true;
  }
  else{
     beta1f(fEndPoint[5],0.,0.,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
     return;
  }

  if (next927){
     p=100.*GetRandom();
     if (p<=40.4){
        nucltransK(fEnGamma[8],fEbindeK,1.3e-2,0.);
        return;
     }
     else if (p<=98.7){
        nucltransK(fEnGamma[9],fEbindeK,1.4e-2,0.);
        next43=true;
     }
     else{
        nucltransK(fEnGamma[10],fEbindeK,1.8e-2,0.);
        next143=true;
     }
  }
  if (next852){
     p=100.*GetRandom();
     if (p<=18.6){
        nucltransK(fEnGamma[11],fEbindeK,1.5e-2,0.);
        return;
     }
     else if (p<=64.6){
        nucltransK(fEnGamma[12],fEbindeK,4.2e0,0.);
        next43=true;
     }
     else{
        nucltransK(fEnGamma[13],0.022,1.0e+1,0.);
        next810=true;
     }
  }
  if (next849){
     p=100.*GetRandom();
     if (p<=51.8){
        nucltransK(fEnGamma[14],fEbindeK,5.5e-3,0.);
        next43=true;
     }
     else{
        nucltransK(fEnGamma[15],fEbindeK,7.0e-3,0.);
        next143=true;
     }
  }
  if (next810){
     p=100.*GetRandom();
     if (p<=63.0){
        particle(3,0.694,0.694,0.,pi,0.,twopi,fTclev,fThlev);
        particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[16],fEbindeK,1.9e-2,0.);
        next43=true;
     }
  }
  if (next786){
     p=100.*GetRandom();
     if (p<=37.6){
        nucltransK(fEnGamma[17],fEbindeK,5.8e-3,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[18],fEbindeK,6.4e-3,0.);
        next43=true;
     }
  }
  if (next143){
     nucltransK(fEnGamma[19],0.022,1.4e+1,0.);
     next43=true;
  }
  if (next43){
     fThlev=0.25e-9;
     nucltransK(fEnGamma[20],0.022,7.2e+2,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Pb211()
{
  // "Table of Isotopes", 7th ed., 1978.
  // VIT, 14.08.1992, 22.10.1995;
  // VIT, 31.10.2006 (updated to NDS 103(2004)183)
  bool next1014=false, next767=false, next405=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<=36.76){
        nucltransK(fEnGamma[0],fEbindeK,1.0e-2,0.2e-4);
        return;
     }
     else if (p<=68.65){
        nucltransK(fEnGamma[1],fEbindeK,2.8e-2,0.);
        next405=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,1.1e-1,0.);
        next767=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     nucltransK(fEnGamma[3],fEbindeK,1.1e-2,0.1e-4);
     return;
  }
  else if (pbeta<=fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     p=100.*GetRandom();
     if (p<=62.96){
        nucltransK(fEnGamma[4],fEbindeK,1.2e-2,0.1e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,1.8e-1,0.);
        next767=true;
     }
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     p=100.*GetRandom();
     if (p<=13.9){
        nucltransK(fEnGamma[6],fEbindeK,1.5e-2,0.4e-6);
        return;
     }
     else if (p<=72.8){
        nucltransK(fEnGamma[7],fEbindeK,5.0e-2,0.);
        next405=true;
     }
     else if (p<=77.0){
        nucltransK(fEnGamma[8],fEbindeK,3.2e-1,0.);
        next767=true;
     }
     else{
        nucltransK(fEnGamma[9],0.016,9.6e+0,0.);
        next1014=true;
     }
  }
  else if (pbeta<=fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     nucltransK(fEnGamma[10],fEbindeK,1.5e-2,0.3e-6);
     return;
  }
  else if (pbeta<=fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     p=100.*GetRandom();
     if (p<=21.8){
        nucltransK(fEnGamma[11],fEbindeK,1.6e-2,0.1e-4);
        return;
     }
     else if (p<=44.9){
        nucltransK(fEnGamma[12],fEbindeK,5.3e-2,0.);
        next405=true;
     }
     else{
        nucltransK(fEnGamma[13],fEbindeK,4.1e-1,0.);
        next767=true;
     }
  }
  else if (pbeta<=fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     nucltransK(fEnGamma[14],fEbindeK,2.2e-2,0.);
     return;
  }
  else if (pbeta<=fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     p=100.*GetRandom();
     if (p<=57.4){
        nucltransK(fEnGamma[15],fEbindeK,2.9e-2,0.);
        return;
     }
     else if (p<=90.3){
        nucltransK(fEnGamma[16],fEbindeK,1.9e-1,0.);
        next405=true;
     }
     else{
        nucltransK(fEnGamma[17],0.016,6.9e+0,0.);
        next767=true;
     }
  }
  else if (pbeta<=fProbBeta[8]){
      beta(fEndPoint[8],0.,0.);
      next405=true;
  }
  else{
     beta(fEndPoint[9],0.,0.);
     return;
  }

  if (next1014){
     p=100.*GetRandom();
     if (p<=28.7){
        nucltransK(fEnGamma[18],fEbindeK,1.9e-2,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[19],fEbindeK,7.0e-2,0.);
        next405=true;
     }
  }
  if (next767){
     p=100.*GetRandom();
     if (p<=57.4){
        nucltransK(fEnGamma[20],fEbindeK,4.0e-2,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[21],fEbindeK,2.8e-1,0.);
        next405=true;
     }
  }
  if (next405){
      fThlev=0.317e-9;
      nucltransK(fEnGamma[22],fEbindeK,1.3e-1,0.);
      return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Pb212()
{
  // "Table of Isotopes", 7th ed., 1978.
  // VIT, 5.08.1992, 22.10.1995.
  bool next239=false, next115=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<=0.5){
        nucltransK(fEnGamma[0],fEbindeK,0.24,0.);
        return;
     }
     else if (p<=98.5){
        nucltransK(fEnGamma[1],fEbindeK,0.55,0.);
        next115=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,2.4,0.);
        next239=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
      beta(fEndPoint[1],0.,0.);
      next239=true;
  }
  else{
     beta(fEndPoint[2],0.,0.);
     return;
  }

  if (next239){
     fThlev=1.e-12;
     nucltransK(fEnGamma[3],fEbindeK,1.1,0.);
     return;
  }
  if (next115){
     fThlev=8.e-12;
     nucltransK(fEnGamma[4],fEbindeK,8.0,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Pb214()
{
  // "Table of Isotopes", 7th ed., 1978.
  // VIT, 11.08.1992, 22.10.1995.
  // VIT, 4.01.2007: updated to NDS 76(1995)127,
  // conversion from K, L, M shells is introduced.
  bool next534=false, next377=false, next352=false, next295=false;
  bool next259=false, next63=false, next53=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=0.034){
     beta(0.135,0.,0.);
     nucltransK(fEnGamma[0],fEbindeK,6.0e-2,0.);
     next377=true;
  }
  else if (pbeta<= 2.766){
     beta(0.184,0.,0.);
     p=100.*GetRandom();
     if (p<=21.3){
        nucltransKLM(fEnGamma[1],fEbindeK,2.99e-3,fEbindeL,4.7e-4,fEbindeM,1.5e-4,0.);
        return;
     }
     else if (p<=60.1){
        nucltransKLM(fEnGamma[2],fEbindeK,3.38e-3,fEbindeL,5.3e-4,fEbindeM,1.7e-4,0.);
        next53=true;
     }
     else if (p<=72.9){
        nucltransKLM(fEnGamma[3],fEbindeK,6.06e-3,fEbindeL,9.7e-4,fEbindeM,3.2e-4,0.);
        next259=true;
     }
     else if (p<=75.4){
        nucltransKLM(fEnGamma[4],fEbindeK,6.90e-3,fEbindeL,1.11e-3,fEbindeM,3.7e-4,0.);
        next295=true;
     }
     else if (p<=90.8){
        nucltransKLM(fEnGamma[5],fEbindeK,8.65e-3,fEbindeL,1.41e-3,fEbindeM,4.4e-4,0.);
        next352=true;
     }
     else if (p<=98.8){
        nucltransKLM(fEnGamma[6],fEbindeK,9.64e-3,fEbindeL,1.58e-3,fEbindeM,4.8e-4,0.);
        next377=true;
     }
     else{
        nucltransKLM(fEnGamma[7],fEbindeK,2.40e-2,fEbindeL,4.1e-3,fEbindeM,1.3e-3,0.);
        next534=true;
     }
  }
  else if (pbeta<= 2.787){
     beta(0.226,0.,0.);
     nucltransK(fEnGamma[8],fEbindeK,8.5e-3,0.);
     next259=true;
  }
  else if (pbeta<= 3.951){
     beta(0.489,0.,0.);
     next534=true;
  }
  else if (pbeta<=52.172){
     beta(0.671,0.,0.);
     next352=true;
  }
  else if (pbeta<=93.787){
     beta(0.728,0.,0.);
     next295=true;
  }
  else{
     beta(1.023,0.,0.);
     return;
  }

  if (next534){
     p=100.*GetRandom();
     if (p<=16.3){
        nucltransKL(fEnGamma[9],fEbindeK,5.0e-2,fEbindeL,1.0e-2,0.);
        return;
     }
     else if (p<=46.0){
        nucltransKLM(fEnGamma[10],fEbindeK,1.22e-1,fEbindeL,1.9e-2,fEbindeM,6.0e-3,0.);
        next53=true;
     }
     else{
        nucltransKLM(fEnGamma[11],fEbindeK,2.9e-1,fEbindeL,7.3e-2,fEbindeM,2.4e-2,0.);
        next259=true;
     }
  }
  if (next377){
     p=100.*GetRandom();
     if (p<=26.2){
        nucltransK(fEnGamma[12],fEbindeK,2.1e-1,0.);
        next53=true;
     }
     else{
        nucltransK(fEnGamma[13],fEbindeK,2.3e-1,0.);
        next63=true;
     }
  }
  if (next352){
     nucltransKLM(fEnGamma[14],fEbindeK,2.55e-1,fEbindeL,4.41e-2,fEbindeM,1.38e-2,0.);
     return;
  }
  if (next295){
     p=100.*GetRandom();
     if (p<=67.10){
        nucltransKLM(fEnGamma[15],fEbindeK,3.8e-1,fEbindeL,6.9e-2,fEbindeM,2.2e-2,0.);
        return;
     }
     else{
        nucltransKLM(fEnGamma[16],fEbindeK,7.13e-1,fEbindeL,1.23e-1,fEbindeM,3.88e-2,0.);
        next53=true;
     }
  }
  if (next259){
     p=100.*GetRandom();
     if (p<=81.4){
        nucltransKLM(fEnGamma[17],fEbindeK,5.92e-1,fEbindeL,1.03e-1,fEbindeM,3.2e-2,0.);
        return;
     }
     else if (p<=83.8){
        nucltransKLM(fEnGamma[18],fEbindeK,1.12e+0,fEbindeL,1.95e-1,fEbindeM,6.1e-2,0.);
        next53=true;
     }
     else{
       nucltransKLM(fEnGamma[19],fEbindeK,1.28e+0,fEbindeL,2.23e-1,fEbindeM,7.0e-2,0.);
       next63=true;
     }
  }
  if (next63){
     nucltransK(fEnGamma[20],fEbindeL,1.0e+1,0.);
     return;
  }
  if (next53){
     nucltransKL(fEnGamma[21],fEbindeL,9.69e+0,fEbindeM,3.05e+0,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Ra228()
{
  // NDS 80(1997)723 and ENSDF database at NNDC site on 8.08.2007.
  float pbeta, p;
  bool next202=false, next67=false;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<=50.){
        nucltransK(fEnGamma[0],0.0198,2.1e2,0.);
        next67=true;
     }
     else{
        nucltransK(fEnGamma[1],0.0050,8.7e0,0.);
        next202=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
      beta(fEndPoint[1],0.,0.);
      next202=true;
  }
  else if (pbeta<=fProbBeta[2]){
      beta(fEndPoint[2],0.,0.);
      next67=true;
  }
  else{
     beta(fEndPoint[3],0.,0.);
     nucltransK(fEnGamma[2],0.0050,7.1e6,0.);
     return;
  }
  if (next202){
      nucltransK(fEnGamma[3],0.0050,6.1e0,0.);
      next67=true;
  }
  if (next67){
     nucltransK(fEnGamma[4],0.0050,1.6e6,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Rh106()
{
  // Approximate scheme of 106Rh decay ("Table of Isotopes", 7th ed., 1978)
  // beta decays to excited levels of 106Pd
  // not higher 2.002 MeV, 99.32% of decay
  // VIT, 17.12.1995.
  bool next1562=false, next1134=false, next1128=false, next512=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<= fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<=0.4){
        nucltransK(fEnGamma[0],fEbindeK,4.0e-4,0.3e-4);
        next512=true;
     }
     else if (p<=98.6){
        nucltransK(fEnGamma[1],fEbindeK,1.3e-3,0.);
        next1128=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,8.5e-3,0.);
        next1562=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
      beta(fEndPoint[1],0.,0.);
      next1562=true;
  }
  else if (pbeta<=fProbBeta[2]){
      beta(fEndPoint[2],0.,0.);
      next1134=true;
  }
  else if (pbeta<=fProbBeta[3]){
      beta(fEndPoint[3],0.,0.);
      next512=true;
  }
  else{
      beta(fEndPoint[4],0.,0.);
      return;
  }

  if (next1562){
      p=100*GetRandom();
      if (p<=9.1){
         nucltransK(fEnGamma[3],fEbindeK,3.5e-4,0.4e-4);
         return;
      }
      else if (p<=95.6){
         nucltransK(fEnGamma[4],fEbindeK,8.5e-4,0.);
         next512=true;
      }
      else if (p<=96.8){
         nucltransK(fEnGamma[5],fEbindeK,8.5e-3,0.);
         next1128=true;
      }
      else{
         nucltransK(fEnGamma[6],fEbindeK,8.5e-3,0.);
         next1134=true;
      }
  }
  if (next1134){
      fThlev=7.0e-12;
      nucltransK(fEnGamma[7],fEbindeK,3.3e-3,0.);
      next512=true;
  }
  if (next1128){
     fThlev=3.2e-12;
     p=100*GetRandom();
     if (p<=34.){
        nucltransK(fEnGamma[8],fEbindeK,7.0e-4,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[9],fEbindeK,3.0e-3,0.);
        next512=true;
     }
  }
  if (next512){
      fThlev=11.0e-12;
      nucltransK(fEnGamma[10],fEbindeK,5.5e-3,0.);
      return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Sb125()
{
  // NDS 86(1999)955 + NNDC on 7.02.2010.
  // VIT, 13.02.2010.
  bool next539=false, next525=false, next463=false, next444=false;
  bool next321=false, next145=false, next35=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     fThlev=1.26e-12;
     p=100*GetRandom();
     if (p<=13.347){
        nucltransK(fEnGamma[0],fEbindeK,3.8e-3,0.);
        return;
     }
     else if (p<=96.921){
        nucltransK(fEnGamma[1],fEbindeK,5.3e-3,0.);
        next35=true;
     }
     else if (p<=97.980){
        nucltransK(fEnGamma[2],fEbindeK,8.4e-2,0.);
        next444=true;
     }
     else if (p<=99.993){
        nucltransK(fEnGamma[3],fEbindeK,9.1e-2,0.);
        next463=true;
     }
     else{
        nucltransK(fEnGamma[4],fEbindeK,3.1e-1,0.);
        next539=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100*GetRandom();
     if (p<=4.85){
        nucltransK(fEnGamma[5],fEbindeK,4.0e-3,0.);
        return;
     }
     else if (p<=14.56){
        nucltransK(fEnGamma[6],fEbindeK,5.6e-3,0.);
        next35=true;
     }
     else if (p<=19.11){
        nucltransK(fEnGamma[7],fEbindeK,2.8e-2,0.);
        next321=true;
     }
     else{
        nucltransK(fEnGamma[8],fEbindeK,8.9e-2,0.);
        next444=true;
     }
  }
  else if (pbeta<=fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     fThlev=70e-12;
     p=100*GetRandom();
     if (p<=86.63){
        nucltransK(fEnGamma[9],fEbindeK,4.9e-3,0.);
        next35=true;
     }
     else if (p<=86.70){
        nucltransK(fEnGamma[10],fEbindeK,3.2e-2,0.);
        next145=true;
     }
     else if (p<=93.91){
        nucltransK(fEnGamma[11],fEbindeK,7.9e-3,0.);
        next321=true;
     }
     else if (p<=94.17){
        nucltransK(fEnGamma[12],fEbindeK,1.5e-1,0.);
        next444=true;
     }
     else if (p<=94.86){
        nucltransK(fEnGamma[13],fEbindeK,1.8e-1,0.);
        next463=true;
     }
     else{
        nucltransK(fEnGamma[14],fEbindeK,1.3e-1,0.);
        next525=true;
     }
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     fThlev=40e-12;
     p=100*GetRandom();
     if (p<=98.716){
        nucltransK(fEnGamma[15],fEbindeK,5.0e-3,0.);
        next35=true;
     }
     else if (p<=98.743){
        nucltransK(fEnGamma[16],fEbindeK,3.2e-2,0.);
        next145=true;
     }
     else if (p<=98.766){
        nucltransK(fEnGamma[17],fEbindeK,8.3e-3,0.);
        next321=true;
     }
     else if (p<=99.994){
        nucltransK(fEnGamma[18],fEbindeK,1.5e-1,0.);
        next463=true;
     }
     else{
        nucltransK(fEnGamma[19],fEbindeK,1.5e-1,0.);
        next525=true;
     }
  }
  else if (pbeta<=fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     next525=true;
  }
  else if (pbeta<=fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     next463=true;
  }
  else if (pbeta<=fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     next444=true;
  }
  else if (pbeta<=fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     p=100*GetRandom();
     if (p<=29.72){
        nucltransK(fEnGamma[20],fEbindeK,1.9e-1,0.);
        return;
     }
     else if (p<=67.45){
        nucltransK(fEnGamma[21],fEbindeK,2.0e-2,0.);
        next35=true;
    }
    else{
       nucltransK(fEnGamma[22],fEbindeK,3.6e-1,0.);
       next321=true;
    }
  }
  else if (pbeta<=fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     next321=true;
  }
  else {
     beta(fEndPoint[9],0.,0.);
     next145=true;
  }

  if (next539){
     fThlev=0.;
     p=100*GetRandom();
     if (p<=26.42){
        nucltransK(fEnGamma[23],fEbindeK,7.8e-3,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[24],fEbindeK,9.3e-3,0.);
        next35=true;
     }
  }
  if (next525){
     fThlev=160e-12;
     p=100*GetRandom();
     if (p<=0.07){
        nucltransK(fEnGamma[25],fEbindeK,3.3e-2,0.);
        next35=true;
     }
     else if (p<=81.12){
        nucltransK(fEnGamma[26],fEbindeK,1.8e-2,0.);
        next145=true;
     }
     else if (p<=99.89){
        nucltransK(fEnGamma[27],fEbindeK,1.3e-1,0.);
        next321=true;
     }
     else{
        nucltransK(fEnGamma[28],fEbindeK,7.4e-1,0.);
        next463=true;
     }
  }
  if (next463){
     fThlev=13.2e-12;
     p=100*GetRandom();
     if (p<=26.08){
        nucltransK(fEnGamma[29],fEbindeK,1.0e-2,0.);
        return;
     }
     else if (p<=99.39){
        nucltransK(fEnGamma[30],fEbindeK,1.4e-2,0.);
        next35=true;
     }
     else{
        nucltransK(fEnGamma[31],0.005,11.1,0.);
        next444=true;
     }
  }
  if (next444){
     fThlev=19.1e-12;
     p=100*GetRandom();
     if (p<=62.40){
        nucltransK(fEnGamma[32],fEbindeK,1.2e-2,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[33],fEbindeK,1.5e-2,0.);
        next35=true;
     }
  }
  if (next321){
     fThlev=0.673e-9;
     nucltransK(fEnGamma[34],fEbindeK,1.7e-1,0.);
     next145=true;
  }
  if (next145){
     fThlev=4.959e6;
     nucltransK(fEnGamma[35],fEbindeK,355.,0.);
     next35=true;
  }
  if (next35){
     fThlev=1.48e-9;
     nucltransK(fEnGamma[36],fEbindeK,14.,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Sb126()
{
  // J.Katakura et al., NDS 97(2002)765.
  // VIT, 2.12.2003
  bool next2840=false, next2812=false, next2766=false, next2515=false;
  bool next2497=false, next2396=false, next2218=false, next1776=false;
  bool next1362=false, next667=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     nucltransK(fEnGamma[0],fEbindeK,1.5e-3,0.);
     next2515=true;
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100*GetRandom();
     if (p<=57.14){
        nucltransK(fEnGamma[1],fEbindeK,1.5e-3,0.);
        next2497=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,3.6e-3,0.);
        next2812=true;
     }
  }
  else if (pbeta<=fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     nucltransK(fEnGamma[3],fEbindeK,3.0e-3,0.);
     next2497=true;
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     p=100*GetRandom();
     if (p<=62.82){
        nucltransK(fEnGamma[4],fEbindeK,3.2e-3,0.);
        next2497=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,3.4e-3,0.);
        next2515=true;
     }
  }
  else if (pbeta<=fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     p=100*GetRandom();
     if (p<=79.86){
        nucltransK(fEnGamma[6],fEbindeK,5.0e-3,0.);
        next2497=true;
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,5.0e-3,0.);
        next2515=true;
     }
  }
  else if (pbeta<=fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     p=100*GetRandom();
     if (p<=57.18){
        nucltransK(fEnGamma[8],fEbindeK,1.0e-3,0.1e-4);
        next1776=true;
     }
     else if (p<=90.43){
        nucltransK(fEnGamma[9],fEbindeK,9.0e-2,0.);
        next2766=true;
     }
     else{
        nucltransK(fEnGamma[10],fEbindeK,4.0e-1,0.);
        next2840=true;
     }
  }
  else if (pbeta<=fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     nucltransK(fEnGamma[11],fEbindeK,1.3e-1,0.);
     next2766=true;
  }
  else if (pbeta<=fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     next2840=true;
  }
  else if (pbeta<=fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     next2812=true;
  }
  else if (pbeta<=fProbBeta[9]){
     beta(fEndPoint[9],0.,0.);
     next2766=true;
  }
  else if (pbeta<=fProbBeta[10]){
     beta(fEndPoint[10],0.,0.);
     next2497=true;
  }
  else if (pbeta<=fProbBeta[11]){
     beta(fEndPoint[11],0.,0.);
     next2396=true;
  }
  else if (pbeta<=fProbBeta[12]){
     beta(fEndPoint[12],0.,0.);
     next2218=true;
  }
  else{
     beta(fEndPoint[13],0.,0.);
     next1776=true;
  }

  if (next2840){
     p=100*GetRandom();
     if (p<=23.73){
        nucltransK(fEnGamma[12],fEbindeK,4.0e-4,0.3e-4);
        next1362=true;
     }
     else{
        nucltransK(fEnGamma[13],fEbindeK,1.2e-3,0.);
        next1776=true;
     }
  }
  if (next2812){
     p=100*GetRandom();
     if (p<=83.33){
        nucltransK(fEnGamma[14],fEbindeK,5.0e-3,0.);
        next2218=true;
     }
     else if (p<=94.44){
        nucltransK(fEnGamma[15],fEbindeK,1.2e-2,0.);
        next2396=true;
     }
     else{
        nucltransK(fEnGamma[16],fEbindeK,3.0e-2,0.);
        next2515=true;
     }
  }
  if (next2766){
     nucltransK(fEnGamma[17],fEbindeK,1.5e-3,0.);
     next1776=true;
  }
  if (next2515){
     nucltransK(fEnGamma[18],fEbindeK,4.0e-2,0.);
     next2218=true;
  }
  if (next2497){
     fThlev=0.152e-9;
     p=100*GetRandom();
     if (p<=95.56){
        nucltransK(fEnGamma[19],fEbindeK,1.0e-2,0.);
        next1776=true;
     }
     else{
        nucltransK(fEnGamma[20],fEbindeK,4.9e-2,0.);
        next2218=true;
     }
  }
  if (next2396){
     p=100*GetRandom();
     if (p<=52.63){
        nucltransK(fEnGamma[21],fEbindeK,1.3e-3,0.);
        next1362=true;
     }
     else{
        nucltransK(fEnGamma[22],fEbindeK,5.0e-3,0.);
        next1776=true;
     }
  }
  if (next2218){
     fThlev=0.;
     nucltransK(fEnGamma[23],fEbindeK,8.4e-4,0.);
     next1362=true;
  }
  if (next1776){
     fThlev=68.e-12;
     nucltransK(fEnGamma[24],fEbindeK,1.4e-2,0.);
     next1362=true;
  }
  if (next1362){
     fThlev=0.;
     nucltransK(fEnGamma[25],fEbindeK,3.4e-3,0.);
     next667=true;
  }
  if (next667){
     fThlev=0.;
     nucltransK(fEnGamma[26],fEbindeK,3.8e-3,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Sb133()
{
  // S.Rab, NDS 75(1995)491
  // VIT, 11.12.2003.
  bool next2332=false, next2211=false, next2024=false, next1913=false;
  bool next1729=false, next1640=false, next1501=false, next1265=false;
  bool next1096=false, next334=false, next308=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<=45.15){
        nucltransK(fEnGamma[0],fEbindeK,1.5e-4,5.7e-4);
        return;
     }
     else if (p<=49.81){
        nucltransK(fEnGamma[1],fEbindeK,1.5e-4,5.2e-4);
        next308=true;
     }
     else if (p<=57.90){
        nucltransK(fEnGamma[2],fEbindeK,4.0e-4,1.3e-4);
        next1096=true;
     }
     else if (p<=67.22){
        nucltransK(fEnGamma[3],fEbindeK,5.0e-4,0.9e-4);
        next1265=true;
     }
     else if (p<=87.10){
        nucltransK(fEnGamma[4],fEbindeK,1.5e-3,0.);
        next1729=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,1.2e-2,0.);
        next2332=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100*GetRandom();
     if (p<=32.79){
        nucltransK(fEnGamma[6],fEbindeK,8.0e-5,5.0e-4);
        next334=true;
     }
     else if (p<=39.00){
        nucltransK(fEnGamma[7],fEbindeK,4.0e-4,1.3e-4);
        next1096=true;
     }
     else if (p<=42.36){
        nucltransK(fEnGamma[8],fEbindeK,1.0e-3,0.4e-4);
        next1501=true;
     }
     else if (p<=49.24){
        nucltransK(fEnGamma[9],fEbindeK,1.5e-3,0.1e-4);
        next1640=true;
     }
     else if (p<=92.58){
        nucltransK(fEnGamma[10],fEbindeK,8.0e-4,0.);
        next1913=true;
     }
     else{
        nucltransK(fEnGamma[11],fEbindeK,7.0e-3,0.);
        next2211=true;
     }
  }
  else if (pbeta<=fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     next2211=true;
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     next2024=true;
  }
  else if (pbeta<=fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     nucltransK(fEnGamma[12],fEbindeK,3.0e-4,3.2e-4);
     return;
  }
  else if (pbeta<=fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     next1913=true;
  }
  else if (pbeta<=fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     next1729=true;
  }
  else if (pbeta<=fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     nucltransK(fEnGamma[13],fEbindeK,4.5e-4,1.5e-4);
     return;
  }
  else if (pbeta<=fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     nucltransK(fEnGamma[14],fEbindeK,5.0e-4,1.2e-4);
     return;
  }
  else if (pbeta<=fProbBeta[9]){
     beta(fEndPoint[9],0.,0.);
     next1640=true;
  }
  else if (pbeta<=fProbBeta[10]){
     beta(fEndPoint[10],0.,0.);
     nucltransK(fEnGamma[15],fEbindeK,5.0e-4,1.0e-4);
     return;
  }
  else if (pbeta<=fProbBeta[11]){
     beta(fEndPoint[11],0.,0.);
     nucltransK(fEnGamma[16],fEbindeK,1.3e-2,0.);
     next1096=true;
  }
  else if (pbeta<=fProbBeta[12]){
     beta(fEndPoint[12],0.,0.);
     next1501=true;
  }
  else if (pbeta<=fProbBeta[13]){
     beta(fEndPoint[13],0.,0.);
     next1265=true;
  }
  else{
     beta(fEndPoint[14],0.,0.);
     next1096=true;
  }

  if (next2332){
     p=100*GetRandom();
     if (p<=65.82){
        nucltransK(fEnGamma[17],fEbindeK,1.0e-3,0.4e-4);
        next1096=true;
     }
     else{
        nucltransK(fEnGamma[18],fEbindeK,2.5e-2,0.);
        next2024=true;
     }
  }
  if (next2211){
     p=100*GetRandom();
     if (p<=23.75){
        nucltransK(fEnGamma[19],fEbindeK,3.5e-4,1.9e-4);
        next334=true;
     }
     else if (p<=97.38){
        nucltransK(fEnGamma[20],fEbindeK,1.5e-3,0.1e-4);
        next1096=true;
     }
     else{
        nucltransK(fEnGamma[21],fEbindeK,5.5e-3,0.);
        next1640=true;
     }
  }
  if (next2024){
     p=100*GetRandom();
     if (p<=81.61){
        nucltransK(fEnGamma[22],fEbindeK,1.8e-3,0.);
        next1096=true;
     }
     else{
        nucltransK(fEnGamma[23],fEbindeK,7.0e-3,0.);
        next1501=true;
     }
  }
  if (next1913){
     p=100*GetRandom();
     if (p<=7.81){
        nucltransK(fEnGamma[24],fEbindeK,4.5e-4,0.4e-4);
        next334=true;
     }
     else if (p<=98.08){
        nucltransK(fEnGamma[25],fEbindeK,8.0e-4,0.);
        next1096=true;
     }
     else{
        nucltransK(fEnGamma[26],fEbindeK,1.2e-2,0.);
        next1501=true;
     }
  }
  if (next1729){
     p=100*GetRandom();
     if (p<=68.47){
        nucltransK(fEnGamma[27],fEbindeK,4.5e-4,1.5e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[28],fEbindeK,4.5e-3,0.);
        next1096=true;
     }
  }
  if (next1640){
     nucltransK(fEnGamma[29],fEbindeK,9.0e-4,0.5e-4);
     next334=true;
  }
  if (next1501){
     p=100*GetRandom();
     if (p<=13.44){
        nucltransK(fEnGamma[30],fEbindeK,6.5e-4,0.7e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[31],fEbindeK,1.3e-3,0.);
        next308=true;
     }
  }
  if (next1265){
     nucltransK(fEnGamma[32],fEbindeK,9.0e-4,0.5e-4);
     return;
  }
  if (next1096){
     nucltransK(fEnGamma[33],fEbindeK,1.1e-3,0.);
     return;
  }
  if (next334){
     return; // creation of isomeric 133mTe with E_exc=334 keV
             // and T1/2=55.4 m
  }
  if (next308){
     nucltransK(fEnGamma[34],fEbindeK,2.5e-2,0.);
     return; // creation of 133Te in g.s. (T1/2=12.5 m)
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Ta182()
{
  // "Table of Isotopes", 7th ed., 1978.
  // VIT, 6.03.1996.
  bool next1488=false, next1443=false, next1374=false, next1331=false;
  bool next1289=false, next1257=false, next1221=false, next329=false;
  bool next100=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     fThlev=1.23e-9;
     p=100*GetRandom();
     if (p<=0.25){
        nucltransK(fEnGamma[0],fEbindeK,4.5e-3,0.1e-4);
        next100=true;
     }
     else if (p<=1.45){
        nucltransK(fEnGamma[1],fEbindeK,2.5e-3,0.1e-4);
        next329=true;
     }
     else if (p<=22.47){
        nucltransK(fEnGamma[2],fEbindeK,1.4e-1,0.);
        next1289=true;
     }
     else if (p<=65.50){
        nucltransK(fEnGamma[3],fEbindeK,5.0e-2,0.);
        next1331=true;
     }
     else if (p<=83.50){
        nucltransK(fEnGamma[4],fEbindeK,7.5e-1,0.);
        next1374=true;
     }
     else if (p<=84.00){
        nucltransK(fEnGamma[5],fEbindeK,3.0e-1,0.);
        next1443=true;
     }
     else{
        nucltransK(fEnGamma[6],0.012,3.0e-0,0.);
        next1488=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     fThlev=0.;
     p=100*GetRandom();
     if (p<=34.){
        nucltransK(fEnGamma[7],fEbindeK,2.4e-3,0.1e-4);
        next100=true;
     }
     else{
        nucltransK(fEnGamma[8],fEbindeK,1.4e-3,0.1e-4);
        next329=true;
     }
  }
  else if (pbeta<=fEndPoint[2]){
     beta(fEndPoint[2],0.,0.);
     next1488=true;
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     next1443=true;
  }
  else if (pbeta<=fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     next1374=true;
  }
  else if (pbeta<=fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     next1331=true;
  }
  else if (pbeta<=fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     next1289=true;
  }
  else if (pbeta<=fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     next1257=true;
  }
  else if (pbeta<=fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     next1221=true;
  }
  else if (pbeta<=fProbBeta[9]){
     beta(fEndPoint[9],0.,0.);
     next329=true;
  }
  else{
     beta(fEndPoint[10],0.,0.);
     next100=true;
  }

  if (next1488){
     fThlev=0.;
     p=100*GetRandom();
     if (p<=1.1){
        nucltransK(fEnGamma[9],fEbindeK,5.0e-3,0.5e-5);
        next100=true;
     }
     else if (p<=6.1){
        nucltransK(fEnGamma[10],fEbindeK,1.5e-3,0.5e-5);
        next329=true;
     }
     else if (p<=29.3){
        nucltransK(fEnGamma[11],fEbindeK,3.2e-1,0.);
        next1289=true;
     }
     else if (p<=70.8){
        nucltransK(fEnGamma[12],fEbindeK,1.2e-1,0.);
        next1331=true;
     }
     else{
        nucltransK(fEnGamma[13],fEbindeK,3.8e-0,0.);
        next1374=true;
     }
  }
  if (next1443){
     fThlev=0.;
     p=100*GetRandom();
     if (p<=40.){
        nucltransK(fEnGamma[14],fEbindeK,2.8e-3,0.1e-4);
        next100=true;
     }
     else{
        nucltransK(fEnGamma[15],fEbindeK,6.0e-3,0.);
        next329=true;
     }
  }
  if (next1374){
     fThlev=0.08e-9;
     p=100*GetRandom();
     if (p<=2.0){
        nucltransK(fEnGamma[16],fEbindeK,5.5e-3,0.5e-5);
        return;
     }
     else if (p<=7.7){
        nucltransK(fEnGamma[17],fEbindeK,3.0e-3,0.1e-4);
        next100=true;
     }
     else if (p<=9.8){
        nucltransK(fEnGamma[18],fEbindeK,6.6e-3,0.);
        next329=true;
     }
     else if (p<=71.3){
        nucltransK(fEnGamma[19],fEbindeK,1.2e-1,0.);
        next1221=true;
     }
     else if (p<=75.1){
        nucltransK(fEnGamma[20],fEbindeK,2.6e-1,0.);
        next1257=true;
     }
     else if (p<=97.9){
        nucltransK(fEnGamma[21],fEbindeK,8.5e-0,0.);
        next1289=true;
     }
     else{
        nucltransK(fEnGamma[22],0.012,7.0e-1,0.);
        next1331=true;
     }
  }
  if (next1331){
     fThlev=0.;
     p=100*GetRandom();
     if (p<=85.){
        nucltransK(fEnGamma[23],fEbindeK,3.0e-3,0.1e-4);
        next100=true;
     }
     else{
        nucltransK(fEnGamma[24],fEbindeK,4.7e-3,0.);
        next329=true;
     }
  }
  if (next1289){
     fThlev=1.12e-9;
     p=100*GetRandom();
     if (p<=2.35){
        nucltransK(fEnGamma[25],fEbindeK,1.3e-2,0.3e-4);
        return;
     }
     else if (p<=29.75){
        nucltransK(fEnGamma[26],fEbindeK,5.3e-3,0.1e-4);
        next100=true;
     }
     else if (p<=30.34){
        nucltransK(fEnGamma[27],fEbindeK,1.3e-2,0.);
        next329=true;
     }
     else if (p<=99.00){
        nucltransK(fEnGamma[28],0.012,2.0e-1,0.);
        next1221=true;
     }
     else{
        nucltransK(fEnGamma[29],0.012,1.6e-0,0.);
        next1257=true;
     }
  }
  if (next1257){
     fThlev=1.7e-12;
     p=100*GetRandom();
     if (p<=54.5){
        nucltransK(fEnGamma[30],fEbindeK,3.0e-3,0.1e-4);
        return;
     }
     else if (p<=77.8){
        nucltransK(fEnGamma[31],fEbindeK,5.3e-3,0.);
        next100=true;
     }
     else{
        nucltransK(fEnGamma[32],fEbindeK,5.5e-3,0.);
        next329=true;
     }
  }
  if (next1221){
     fThlev=0.37e-12;
     p=100*GetRandom();
     if (p<=44.00){
        nucltransK(fEnGamma[33],fEbindeK,3.0e-3,0.1e-4);
        return;
     }
     else if (p<=99.92){
        nucltransK(fEnGamma[34],fEbindeK,3.5e-3,0.1e-5);
        next100=true;
     }
     else{
        nucltransK(fEnGamma[35],fEbindeK,6.0e-3,0.);
        next329=true;
     }
  }
  if (next329){
     fThlev=64.e-12;
     nucltransK(fEnGamma[36],fEbindeK,2.4e-1,0.);
     next100=true;
  }
  if (next100){
     fThlev=1.38e-9;
     nucltransK(fEnGamma[37],0.012,4.0e-0,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Te133()
{
  // S.Rab, Nucl.Data Sheets 75 (1995) 491.
  // VIT, 14.12.2003
  bool next2597=false, next2467=false, next2284=false, next2225=false;
  bool next2210=false, next2054=false, next2040=false, next2025=false;
  bool next1718=false, next1671=false, next1564=false, next1374=false;
  bool next1333=false, next1313=false, next1307=false, next1240=false;
  bool  next915=false,  next913=false,  next787=false,  next720=false;
  bool  next312=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<=72.87){
        nucltransK(fEnGamma[0],fEbindeK,1.5e-4,5.1e-4);
        next312=true;
     }
     else if (p<=92.25){
        nucltransK(fEnGamma[1],fEbindeK,3.0e-4,2.9e-4);
        next787=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,1.0e-3,0.7e-4);
        next1564=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100*GetRandom();
     if (p<=74.63){
        nucltransK(fEnGamma[3],fEbindeK,1.8e-4,4.9e-4);
        next312=true;
     }
     else if (p<=94.67){
        nucltransK(fEnGamma[4],fEbindeK,3.2e-4,2.7e-4);
        next787=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,7.5e-4,0.9e-4);
        next1374=true;
     }
  }
  else if (pbeta<= fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     p=100*GetRandom();
     if (p<=60.94){
        nucltransK(fEnGamma[6],fEbindeK,1.3e-4,5.9e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,3.2e-4,2.7e-4);
        next720=true;
     }
  }
  else if (pbeta<= fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     p=100*GetRandom();
     if (p<=46.23){
        nucltransK(fEnGamma[8],fEbindeK,2.0e-4,4.7e-4);
        next312=true;
     }
     else if (p<=63.26){
        nucltransK(fEnGamma[9],fEbindeK,1.1e-3,0.4e-4);
        next1564=true;
     }
     else if (p<=92.46){
        nucltransK(fEnGamma[10],fEbindeK,1.5e-3,0.2e-4);
        next1671=true;
     }
     else{
        nucltransK(fEnGamma[11],fEbindeK,3.0e-2,0.);
        next2467=true;
     }
  }
  else if (pbeta<= fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     p=100*GetRandom();
     if (p<=24.57){
        nucltransK(fEnGamma[12],fEbindeK,2.0e-4,4.4e-4);
        next312=true;
     }
     else if (p<=28.07){
        nucltransK(fEnGamma[13],fEbindeK,1.2e-3,2.5e-4);
        next720=true;
     }
     else if (p<=42.25){
        nucltransK(fEnGamma[14],fEbindeK,1.0e-3,0.8e-4);
        next1313=true;
     }
     else if (p<=71.55){
        nucltransK(fEnGamma[15],fEbindeK,5.0e-3,0.);
        next2025=true;
     }
     else if (p<=82.89){
        nucltransK(fEnGamma[16],fEbindeK,2.0e-4,0.);
        next2225=true;
     }
     else if (p<=88.56){
        nucltransK(fEnGamma[17],fEbindeK,1.0e-2,0.);
        next2284=true;
     }
     else if (p<=91.49){
        nucltransK(fEnGamma[18],fEbindeK,4.5e-2,0.);
        next2467=true;
     }
     else{
        nucltransK(fEnGamma[19],fEbindeK,2.0e-1,0.);
        next2597=true;
     }
  }
  else if (pbeta<= fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     p=100*GetRandom();
     if (p<=43.10){
        nucltransK(fEnGamma[20],fEbindeK,1.5e-4,5.2e-4);
        return;
     }
     else if (p<=47.70){
        nucltransK(fEnGamma[21],fEbindeK,2.2e-4,3.9e-4);
        next312=true;
     }
     else if (p<=82.18){
        nucltransK(fEnGamma[22],fEbindeK,2.5e-3,0.);
        next1718=true;
     }
     else{
        nucltransK(fEnGamma[23],fEbindeK,6.5e-3,0.);
        next2040=true;
     }
  }
  else if (pbeta<= fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     next2597=true;
  }
  else if (pbeta<= fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     p=100*GetRandom();
     if (p<=23.06){
        nucltransK(fEnGamma[24],fEbindeK,1.6e-4,4.8e-4);
        return;
     }
     else if (p<=63.19){
        nucltransK(fEnGamma[25],fEbindeK,2.5e-4,3.2e-4);
        next312=true;
     }
     else if (p<=73.34){
        nucltransK(fEnGamma[26],fEbindeK,5.0e-4,1.8e-4);
        next720=true;
     }
     else if (p<=75.37){
        nucltransK(fEnGamma[27],fEbindeK,5.5e-4,1.6e-4);
        next787=true;
     }
     else if (p<=77.40){
        nucltransK(fEnGamma[28],fEbindeK,1.0e-3,0.5e-4);
        next1240=true;
     }
     else if (p<=84.78){
        nucltransK(fEnGamma[29],fEbindeK,1.4e-3,0.3e-4);
        next1333=true;
     }
     else if (p<=90.32){
        nucltransK(fEnGamma[30],fEbindeK,2.5e-3,0.);
        next1564=true;
     }
     else if (p<=94.47){
        nucltransK(fEnGamma[31],fEbindeK,1.2e-2,0.);
        next2054=true;
     }
     else{
        nucltransK(fEnGamma[32],fEbindeK,3.5e-2,0.);
        next2210=true;
     }
  }
  else if (pbeta<= fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     p=100*GetRandom();
     if (p<=3.97){
        nucltransK(fEnGamma[33],fEbindeK,1.7e-4,5.7e-4);
        return;
     }
     else if (p<=37.30){
        nucltransK(fEnGamma[34],fEbindeK,2.5e-4,3.1e-4);
        next312=true;
     }
     else if (p<=78.57){
        nucltransK(fEnGamma[35],fEbindeK,5.0e-4,1.8e-4);
        next720=true;
     }
     else if (p<=86.51){
        nucltransK(fEnGamma[36],fEbindeK,5.5e-4,1.5e-4);
        next787=true;
     }
     else if (p<=90.48){
        nucltransK(fEnGamma[37],fEbindeK,1.0e-3,0.5e-4);
        next1240=true;
     }
     else{
        nucltransK(fEnGamma[38],fEbindeK,3.5e-3,0.);
        next1671=true;
     }
  }
  else if (pbeta<= fProbBeta[9]){
     beta(fEndPoint[9],0.,0.);
     p=100*GetRandom();
     if (p<=8.93){
        nucltransK(fEnGamma[39],fEbindeK,2.5e-4,3.4e-4);
        next312=true;
     }
     else if (p<=48.72){
        nucltransK(fEnGamma[40],fEbindeK,5.0e-4,0.9e-4);
        next720=true;
     }
     else if (p<=65.81){
        nucltransK(fEnGamma[41],fEbindeK,6.0e-4,1.3e-4);
        next787=true;
     }
     else{
        nucltransK(fEnGamma[42],fEbindeK,2.8e-3,0.);
        next1564=true;
     }
  }
  else if (pbeta<= fProbBeta[10]){
     beta(fEndPoint[10],0.,0.);
     next2467=true;
  }
  else if (pbeta<= fProbBeta[11]){
     beta(fEndPoint[11],0.,0.);
     p=100*GetRandom();
     if (p<=34.48){
        nucltransK(fEnGamma[43],fEbindeK,2.0e-4,4.2e-4);
        return;
     }
     else if (p<=52.63){
        nucltransK(fEnGamma[44],fEbindeK,3.0e-4,2.7e-4);
        next312=true;
     }
     else if (p<=63.52){
        nucltransK(fEnGamma[45],fEbindeK,5.0e-4,1.3e-4);
        next720=true;
     }
     else if (p<=68.06){
        nucltransK(fEnGamma[46],fEbindeK,5.2e-4,1.2e-4);
        next787=true;
     }
     else if (p<=73.69){
        nucltransK(fEnGamma[47],fEbindeK,7.0e-4,0.3e-4);
        next915=true;
     }
     else if (p<=95.46){
        nucltransK(fEnGamma[48],fEbindeK,1.8e-3,0.1e-4);
        next1307=true;
     }
     else{
        nucltransK(fEnGamma[49],fEbindeK,1.2e-1,0.);
        next2210=true;
     }
  }
  else if (pbeta<= fProbBeta[12]){
     beta(fEndPoint[12],0.,0.);
     p=100*GetRandom();
     if (p<=11.32){
        nucltransK(fEnGamma[50],fEbindeK,2.0e-4,4.1e-4);
        return;
     }
     else if (p<=64.16){
        nucltransK(fEnGamma[51],fEbindeK,3.0e-4,2.6e-4);
        next312=true;
     }
     else if (p<=82.08){
        nucltransK(fEnGamma[52],fEbindeK,5.0e-3,0.);
        next1671=true;
     }
     else{
        nucltransK(fEnGamma[53],fEbindeK,1.7e-1,0.);
        next2210=true;
     }
  }
  else if (pbeta<= fProbBeta[13]){
     beta(fEndPoint[13],0.,0.);
     p=100*GetRandom();
     if (p<=18.52){
        nucltransK(fEnGamma[54],fEbindeK,2.0e-4,4.0e-4);
        return;
     }
     else if (p<=62.96){
        nucltransK(fEnGamma[55],fEbindeK,1.8e-3,0.3e-4);
        next1240=true;
     }
     else{
        nucltransK(fEnGamma[56],fEbindeK,2.0e-3,0.);
        next1313=true;
     }
  }
  else if (pbeta<= fProbBeta[14]){
     beta(fEndPoint[14],0.,0.);
     next2284=true;
  }
  else if (pbeta<= fProbBeta[15]){
     beta(fEndPoint[15],0.,0.);
     p=100*GetRandom();
     if (p<=52.17){
        nucltransK(fEnGamma[57],fEbindeK,2.5e-4,3.5e-4);
        return;
     }
     else if (p<=64.34){
        nucltransK(fEnGamma[58],fEbindeK,2.2e-3,0.);
        next1240=true;
     }
     else if (p<=90.43){
        nucltransK(fEnGamma[59],fEbindeK,2.8e-3,0.);
        next1333=true;
     }
     else{
        nucltransK(fEnGamma[60],fEbindeK,5.5e-3,0.);
        next1564=true;
     }
  }
  else if (pbeta<= fProbBeta[16]){
     beta(fEndPoint[16],0.,0.);
     p=100*GetRandom();
     if (p<=16.92){
        nucltransK(fEnGamma[61],fEbindeK,3.0e-4,3.8e-4);
        return;
     }
     else if (p<=23.45){
        nucltransK(fEnGamma[62],fEbindeK,3.5e-4,2.1e-4);
        next312=true;
     }
     else if (p<=40.37){
        nucltransK(fEnGamma[63],fEbindeK,6.0e-4,1.0e-4);
        next720=true;
     }
     else if (p<=44.40){
        nucltransK(fEnGamma[64],fEbindeK,6.5e-4,0.9e-4);
        next787=true;
     }
     else if (p<=54.07){
        nucltransK(fEnGamma[65],fEbindeK,2.3e-3,0.);
        next1240=true;
     }
     else if (p<=79.05){
        nucltransK(fEnGamma[66],fEbindeK,2.7e-3,0.);
        next1313=true;
     }
     else if (p<=88.72){
        nucltransK(fEnGamma[67],fEbindeK,2.8e-3,0.);
        next1333=true;
     }
     else{
        nucltransK(fEnGamma[68],fEbindeK,5.5e-3,0.);
        next1564=true;
     }
  }
  else if (pbeta<= fProbBeta[17]){
     beta(fEndPoint[17],0.,0.);
     next2225=true;
  }
  else if (pbeta<=fProbBeta[18]){
     beta(fEndPoint[18],0.,0.);
     next2210=true;
  }
  else if (pbeta<=fProbBeta[19]){
     beta(fEndPoint[19],0.,0.);
     p=100*GetRandom();
     if (p<=25.07){
        nucltransK(fEnGamma[69],fEbindeK,3.0e-4,3.1e-4);
        return;
     }
     else if (p<=78.72){
        nucltransK(fEnGamma[70],fEbindeK,4.5e-4,1.9e-4);
        next312=true;
     }
     else if (p<=92.79){
        nucltransK(fEnGamma[71],fEbindeK,7.5e-4,0.9e-4);
        next720=true;
     }
     else if (p<=94.72){
        nucltransK(fEnGamma[72],fEbindeK,3.0e-3,0.);
        next1307=true;
     }
     else if (p<=97.36){
        nucltransK(fEnGamma[73],fEbindeK,3.0e-3,0.);
        next1313=true;
     }
     else{
        nucltransK(fEnGamma[74],fEbindeK,3.4e-3,0.);
        next1333=true;
     }
  }
  else if (pbeta<=fProbBeta[20]){
     beta(fEndPoint[20],0.,0.);
     p=100*GetRandom();
     if (p<=57.73){
        nucltransK(fEnGamma[75],fEbindeK,3.0e-4,2.9e-4);
        return;
     }
     else if (p<=69.61){
        nucltransK(fEnGamma[76],fEbindeK,5.0e-4,1.7e-4);
        next312=true;
     }
     else if (p<=75.10){
        nucltransK(fEnGamma[77],fEbindeK,1.0e-3,0.8e-4);
        next720=true;
     }
     else if (p<=79.32){
        nucltransK(fEnGamma[78],fEbindeK,1.2e-3,0.7e-4);
        next787=true;
     }
     else if (p<=80.12){
        nucltransK(fEnGamma[79],fEbindeK,1.0e-3,0.1e-4);
        next915=true;
     }
     else if (p<=82.23){
        nucltransK(fEnGamma[80],fEbindeK,3.0e-3,0.);
        next1240=true;
     }
     else if (p<=86.03){
        nucltransK(fEnGamma[81],fEbindeK,3.7e-3,0.);
        next1307=true;
     }
     else if (p<=89.41){
        nucltransK(fEnGamma[82],fEbindeK,3.7e-3,0.);
        next1313=true;
     }
     else if (p<=94.94){
        nucltransK(fEnGamma[83],fEbindeK,4.0e-3,0.);
        next1333=true;
     }
     else{
        nucltransK(fEnGamma[84],fEbindeK,3.2e-3,0.);
        next1374=true;
     }
  }
  else if (pbeta<=fProbBeta[21]){
     beta(fEndPoint[21],0.,0.);
     next2054=true;
  }
  else if (pbeta<=fProbBeta[22]){
     beta(fEndPoint[22],0.,0.);
     next2040=true;
  }
  else if (pbeta<=fProbBeta[23]){
     beta(fEndPoint[23],0.,0.);
     next2025=true;
  }
  else if (pbeta<=fProbBeta[24]){
     beta(fEndPoint[24],0.,0.);
     next1718=true;
  }
  else if (pbeta<=fProbBeta[25]){
     beta(fEndPoint[25],0.,0.);
     next1671=true;
  }
  else if (pbeta<=fProbBeta[26]){
     beta(fEndPoint[26],0.,0.);
     next1564=true;
  }
  else if (pbeta<=fProbBeta[27]){
     beta(fEndPoint[27],0.,0.);
     next1374=true;
  }
  else if (pbeta<=fProbBeta[28]){
     beta(fEndPoint[28],0.,0.);
     next1333=true;
  }
  else if (pbeta<=fProbBeta[29]){
     beta(fEndPoint[29],0.,0.);
     next1313=true;
  }
  else if (pbeta<=fProbBeta[30]){
     beta(fEndPoint[30],0.,0.);
     next1240=true;
  }
  else if (pbeta<=fProbBeta[31]){
     beta(fEndPoint[31],0.,0.);
     next720=true;
  }
  else{
     beta(fEndPoint[32],0.,0.);
     next312=true;
  }

  if (next2597){
     p=100*GetRandom();
     if (p<=20.00){
        nucltransK(fEnGamma[85],fEbindeK,1.6e-4,5.1e-4);
        return;
     }
     else if (p<=23.21){
        nucltransK(fEnGamma[86],fEbindeK,2.3e-4,3.5e-4);
        next312=true;
     }
     else if (p<=73.21){
        nucltransK(fEnGamma[87],fEbindeK,5.0e-4,0.6e-4);
        next915=true;
     }
     else if (p<=80.00){
        nucltransK(fEnGamma[88],fEbindeK,1.0e-3,0.5e-4);
        next1307=true;
     }
     else if (p<=88.93){
        nucltransK(fEnGamma[89],fEbindeK,1.0e-3,0.5e-4);
        next1313=true;
     }
     else if (p<=91.07){
        nucltransK(fEnGamma[90],fEbindeK,1.0e-3,0.1e-4);
        next1374=true;
     }
     else{
        nucltransK(fEnGamma[91],fEbindeK,9.0e-3,0.);
        next2025=true;
     }
  }
  if (next2467){
     p=100*GetRandom();
     if (p<=61.38){
        nucltransK(fEnGamma[92],fEbindeK,1.8e-4,5.2e-4);
        return;
     }
     else if (p<=65.12){
        nucltransK(fEnGamma[93],fEbindeK,3.0e-4,3.0e-4);
        next312=true;
     }
     else if (p<=78.59){
        nucltransK(fEnGamma[94],fEbindeK,5.0e-4,1.3e-4);
        next787=true;
     }
     else if (p<=95.36){
        nucltransK(fEnGamma[95],fEbindeK,1.2e-3,0.4e-4);
        next1240=true;
     }
     else{
        nucltransK(fEnGamma[96],fEbindeK,8.0e-2,0.);
        next2225=true;
     }
  }
  if (next2284){
     p=100*GetRandom();
     if (p<=5.77){
        nucltransK(fEnGamma[97],fEbindeK,3.5e-4,1.7e-4);
        next312=true;
     }
     else if (p<=34.62){
        nucltransK(fEnGamma[98],fEbindeK,6.0e-4,0.4e-4);
        next720=true;
     }
     else if (p<=53.85){
        nucltransK(fEnGamma[99],fEbindeK,2.5e-3,0.);
        next1313=true;
     }
     else{
        nucltransK(fEnGamma[100],fEbindeK,3.0e-3,0.);
        next1374=true;
     }
  }
  if (next2225){
     p=100*GetRandom();
     if (p<=19.00){
        nucltransK(fEnGamma[101],fEbindeK,3.0e-4,3.2e-4);
        return;
     }
     else if (p<=29.28){
        nucltransK(fEnGamma[102],fEbindeK,4.5e-4,2.0e-4);
        next312=true;
     }
     else if (p<=35.32){
        nucltransK(fEnGamma[103],fEbindeK,7.5e-4,0.9e-4);
        next720=true;
     }
     else if (p<=35.84){
        nucltransK(fEnGamma[104],fEbindeK,8.5e-4,0.8e-4);
        next787=true;
     }
     else if (p<=47.67){
        nucltransK(fEnGamma[105],fEbindeK,9.0e-4,0.1e-4);
        next915=true;
     }
     else if (p<=53.02){
        nucltransK(fEnGamma[106],fEbindeK,3.0e-3,0.);
        next1313=true;
     }
     else if (p<=79.79){
        nucltransK(fEnGamma[107],fEbindeK,2.5e-3,0.);
        next1374=true;
     }
     else if (p<=84.97){
        nucltransK(fEnGamma[108],fEbindeK,9.5e-3,0.);
        next1671=true;
     }
     else if (p<=96.80){
        nucltransK(fEnGamma[109],fEbindeK,1.2e-2,0.);
        next1718=true;
     }
     else{
        nucltransK(fEnGamma[110],fEbindeK,1.4e-1,0.);
        next2025=true;
     }
  }
  if (next2210){
     p=100*GetRandom();
     if (p<=46.12){
        nucltransK(fEnGamma[111],fEbindeK,2.9e-4,3.6e-4);
        return;
     }
     else if (p<=53.21){
        nucltransK(fEnGamma[112],fEbindeK,4.5e-4,2.0e-4);
        next312=true;
     }
     else if (p<=61.23){
        nucltransK(fEnGamma[113],fEbindeK,7.5e-4,0.9e-4);
        next720=true;
     }
     else if (p<=73.93){
        nucltransK(fEnGamma[114],fEbindeK,3.0e-3,0.);
        next1307=true;
     }
     else{
        nucltransK(fEnGamma[115],fEbindeK,6.5e-3,0.);
        next1564=true;
     }
  }
  if (next2054){
     p=100*GetRandom();
     if (p<=12.21){
        nucltransK(fEnGamma[116],fEbindeK,4.0e-4,2.5e-4);
        return;
     }
     else if (p<=24.15){
        nucltransK(fEnGamma[117],fEbindeK,5.0e-4,1.5e-4);
        next312=true;
     }
     else if (p<=31.12){
        nucltransK(fEnGamma[118],fEbindeK,1.2e-3,0.6e-4);
        next720=true;
     }
     else if (p<=47.69){
        nucltransK(fEnGamma[119],fEbindeK,1.3e-3,0.5e-4);
        next787=true;
     }
     else if (p<=58.15){
        nucltransK(fEnGamma[120],fEbindeK,3.8e-3,0.);
        next1240=true;
     }
     else if (p<=67.74){
        nucltransK(fEnGamma[121],fEbindeK,4.5e-3,0.);
        next1307=true;
     }
     else if (p<=84.31){
        nucltransK(fEnGamma[122],fEbindeK,4.5e-3,0.);
        next1313=true;
     }
     else if (p<=94.77){
        nucltransK(fEnGamma[123],fEbindeK,5.0e-3,0.);
        next1333=true;
     }
     else{
        nucltransK(fEnGamma[124],fEbindeK,4.0e-3,0.);
        next1374=true;
     }
  }
  if (next2040){
     p=100*GetRandom();
     if (p<=6.87){
        nucltransK(fEnGamma[125],fEbindeK,9.5e-4,0.1e-4);
        next720=true;
     }
     else if (p<=12.09){
        nucltransK(fEnGamma[126],fEbindeK,1.2e-3,0.5e-4);
        next787=true;
     }
     else if (p<=23.08){
        nucltransK(fEnGamma[127],fEbindeK,5.0e-3,0.);
        next1313=true;
     }
     else if (p<=75.28){
        nucltransK(fEnGamma[128],fEbindeK,6.0e-3,0.);
        next1374=true;
     }
     else{
        nucltransK(fEnGamma[129],fEbindeK,2.5e-2,0.);
        next1671=true;
     }
  }
  if (next2025){
     p=100*GetRandom();
     if (p<=4.60){
        nucltransK(fEnGamma[130],fEbindeK,3.5e-4,2.2e-4);
        return;
     }
     else if (p<=25.61){
        nucltransK(fEnGamma[131],fEbindeK,5.0e-4,1.4e-4);
        next312=true;
     }
     else if (p<=40.94){
        nucltransK(fEnGamma[132],fEbindeK,1.2e-3,0.6e-4);
        next720=true;
     }
     else if (p<=47.75){
        nucltransK(fEnGamma[133],fEbindeK,1.3e-3,0.5e-4);
        next787=true;
     }
     else if (p<=54.00){
        nucltransK(fEnGamma[134],fEbindeK,1.4e-3,0.);
        next1307=true;
     }
     else if (p<=64.79){
        nucltransK(fEnGamma[135],fEbindeK,5.0e-3,0.);
        next1313=true;
     }
     else{
        nucltransK(fEnGamma[136],fEbindeK,1.5e-2,0.);
        next1564=true;
     }
  }
  if (next1718){
     p=100*GetRandom();
     if (p<=30.02){
        nucltransK(fEnGamma[137],fEbindeK,6.0e-4,1.4e-4);
        return;
     }
     else if (p<=35.59){
        nucltransK(fEnGamma[138],fEbindeK,1.2e-3,0.7e-4);
        next312=true;
     }
     else if (p<=45.41){
        nucltransK(fEnGamma[139],fEbindeK,2.4e-3,0.);
        next720=true;
     }
     else if (p<=81.39){
        nucltransK(fEnGamma[140],fEbindeK,2.7e-3,0.);
        next787=true;
     }
     else if (p<=81.97){
        nucltransK(fEnGamma[141],fEbindeK,1.1e-3,0.);
        next915=true;
     }
     else if (p<=85.56){
        nucltransK(fEnGamma[142],fEbindeK,1.3e-2,0.);
        next1240=true;
     }
     else if (p<=94.43){
        nucltransK(fEnGamma[143],fEbindeK,2.0e-2,0.);
        next1307=true;
     }
     else if (p<=96.88){
        nucltransK(fEnGamma[144],fEbindeK,2.0e-2,0.);
        next1313=true;
     }
     else if (p<=99.43){
        nucltransK(fEnGamma[145],fEbindeK,2.5e-2,0.);
        next1333=true;
     }
     else{
        nucltransK(fEnGamma[146],fEbindeK,3.0e-2,0.);
        next1374=true;
     }
  }
  if (next1671){
     p=100*GetRandom();
     if (p<= 9.59){
        nucltransK(fEnGamma[147],fEbindeK,5.0e-4,0.7e-4);
        return;
     }
     else if (p<=15.22){
        nucltransK(fEnGamma[148],fEbindeK,1.2e-3,0.7e-4);
        next312=true;
     }
     else if (p<=28.40){
        nucltransK(fEnGamma[149],fEbindeK,2.6e-3,0.);
        next720=true;
     }
     else if (p<=71.54){
        nucltransK(fEnGamma[150],fEbindeK,3.2e-3,0.);
        next787=true;
     }
     else if (p<=78.73){
        nucltransK(fEnGamma[151],fEbindeK,1.8e-2,0.);
        next1240=true;
     }
     else if (p<=83.94){
        nucltransK(fEnGamma[152],fEbindeK,3.0e-2,0.);
        next1313=true;
     }
     else{
        nucltransK(fEnGamma[153],fEbindeK,3.5e-2,0.);
        next1333=true;
     }
  }
  if (next1564){
     p=100*GetRandom();
     if (p<=28.18){
        nucltransK(fEnGamma[154],fEbindeK,1.3e-3,0.5e-4);
        next312=true;
     }
     else if (p<=92.95){
        nucltransK(fEnGamma[155],fEbindeK,3.5e-3,0.);
        next720=true;
     }
     else if (p<=96.86){
        nucltransK(fEnGamma[156],fEbindeK,4.2e-3,0.);
        next787=true;
     }
     else if (p<=97.85){
        nucltransK(fEnGamma[157],fEbindeK,3.7e-2,0.);
        next1240=true;
     }
     else if (p<=98.46){
        nucltransK(fEnGamma[158],fEbindeK,7.5e-2,0.);
        next1313=true;
     }
     else if (p<=98.83){
        nucltransK(fEnGamma[159],fEbindeK,8.5e-2,0.);
        next1333=true;
     }
     else{
        nucltransK(fEnGamma[160],fEbindeK,1.5e-1,0.);
        next1374=true;
     }
  }
  if (next1374){
     p=100*GetRandom();
     if (p<=56.13){
        nucltransK(fEnGamma[161],fEbindeK,1.5e-3,0.);
        next312=true;
     }
     else if (p<=14.62){
        nucltransK(fEnGamma[162],fEbindeK,4.5e-3,0.);
        next720=true;
     }
     else{
        nucltransK(fEnGamma[163],fEbindeK,8.5e-3,0.);
        next787=true;
     }
  }
  if (next1333){
     p=100*GetRandom();
     if (p<=74.49){
        nucltransK(fEnGamma[164],fEbindeK,1.2e-3,0.6e-4);
        return;
     }
     else if (p<=94.05){
        nucltransK(fEnGamma[165],fEbindeK,2.3e-3,0.);
        next312=true;
     }
     else if (p<=96.28){
        nucltransK(fEnGamma[166],fEbindeK,7.0e-3,0.);
        next720=true;
     }
     else if (p<=99.83){
        nucltransK(fEnGamma[167],fEbindeK,9.5e-3,0.);
        next787=true;
     }
     else{
        nucltransK(fEnGamma[168],fEbindeK,1.5e-2,0.);
        next915=true;
     }
  }
  if (next1313){
     p=100*GetRandom();
     if (p<=17.45){
        nucltransK(fEnGamma[169],fEbindeK,9.0e-4,0.);
        return;
     }
     else if (p<=91.78){
        nucltransK(fEnGamma[170],fEbindeK,2.5e-3,0.);
        next312=true;
     }
     else if (p<=95.27){
        nucltransK(fEnGamma[171],fEbindeK,8.5e-3,0.);
        next720=true;
     }
     else{
        nucltransK(fEnGamma[172],fEbindeK,1.2e-2,0.);
        next787=true;
     }
  }
  if (next1307){
     p=100*GetRandom();
     if (p<=33.13){
        nucltransK(fEnGamma[173],fEbindeK,1.2e-3,0.5e-4);
        return;
     }
     else if (p<=75.46){
        nucltransK(fEnGamma[174],fEbindeK,2.5e-3,0.);
        next312=true;
     }
     else if (p<=81.59){
        nucltransK(fEnGamma[175],fEbindeK,8.5e-3,0.);
        next720=true;
     }
     else if (p<=82.76){
        nucltransK(fEnGamma[176],fEbindeK,8.5e-3,0.);
        next787=true;
     }
     else if (p<=84.66){
        nucltransK(fEnGamma[177],fEbindeK,2.0e-2,0.);
        next913=true;
     }
     else{
        nucltransK(fEnGamma[178],fEbindeK,2.2e-2,0.);
        next915=true;
     }
  }
  if (next1240){
     p=100*GetRandom();
     if (p<=24.03){
        nucltransK(fEnGamma[179],fEbindeK,1.2e-3,0.);
        return;
     }
     else if (p<=81.24){
        nucltransK(fEnGamma[180],fEbindeK,2.7e-3,0.);
        next312=true;
     }
     else if (p<=86.27){
        nucltransK(fEnGamma[181],fEbindeK,1.2e-2,0.);
        next720=true;
     }
     else{
        nucltransK(fEnGamma[182],fEbindeK,1.5e-2,0.);
        next787=true;
     }
  }
  if (next915){
     nucltransK(fEnGamma[183],fEbindeK,2.8e-3,0.);
     return;
  }
  if (next913){
     nucltransK(fEnGamma[184],fEbindeK,2.0e-3,0.);
     return;
  }
  if (next787){
     p=100*GetRandom();
     if (p<=78.49){
        nucltransK(fEnGamma[185],fEbindeK,3.0e-3,0.);
        return;
     }
     else if (p<=91.28){
        nucltransK(fEnGamma[186],fEbindeK,1.2e-2,0.);
        next312=true;
     }
     else{
        nucltransK(fEnGamma[187],fEbindeK,4.9,0.);
        next720=true;
     }
  }
  if (next720){
     p=100*GetRandom();
     if (p<=24.72){
        nucltransK(fEnGamma[188],fEbindeK,5.0e-3,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[189],fEbindeK,2.0e-2,0.);
        next312=true;
     }
  }
  if (next312){
     nucltransK(fEnGamma[190],fEbindeK,3.5e-2,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Te133m()
{
  // Model for scheme of Te133m decay(S.Rab,Nucl.Data
  // Sheets 75(1995)491; E_exc=334 keV).
  //  VIT, 18.12.2003.
  bool next2596=false, next2556=false, next2516=false, next2500=false;
  bool next2445=false, next2372=false, next2262=false, next2249=false;
  bool next2212=false, next2049=false, next2005=false, next1991=false;
  bool next1975=false, next1943=false, next1893=false, next1886=false;
  bool next1817=false, next1799=false, next1797=false, next1777=false;
  bool next1729=false, next1707=false, next1704=false, next1647=false;
  bool next1634=false, next1560=false, next1516=false, next1455=false;
  bool next1307=false,  next915=false,  next913=false,  next312=false;
  float pdecay, pbeta, p;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){   // 17.5% IT to 133Te(g.s.)
     nucltransK(fEnGamma[0],fEbindeK,1.431,0.);
     return;
  }
  else{
     pbeta=100.*GetRandom();    // 82.5% beta decay to 133I
     if (pbeta<=fProbBeta[0]){
        beta(fEndPoint[0],0.,0.);
        p=100*GetRandom();
        if (p<=21.62){
           nucltransK(fEnGamma[1],fEbindeK,2.0e-5,18.1e-4);
           return;
        }
        else if (p<=29.05){
           nucltransK(fEnGamma[2],fEbindeK,3.5e-4,0.3e-4);
           next1647=true;
        }
        else{
           nucltransK(fEnGamma[3],fEbindeK,8.0e-3,0.);
           next2516=true;
        }
     }
     else if (pbeta<= fProbBeta[1]){
        beta(fEndPoint[1],0.,0.);
        p=100*GetRandom();
        if (p<=35.90){
            nucltransK(fEnGamma[4],fEbindeK,2.6e-4,0.7e-4);
            next1455=true;
         }
         else if (p<=78.21){
            nucltransK(fEnGamma[5],fEbindeK,4.0e-4,0.2e-4);
            next1777=true;
         }
         else{
            nucltransK(fEnGamma[6],fEbindeK,7.0e-4,0.);
            next1975=true;
         }
     }
     else if (pbeta<= fProbBeta[2]){
        beta(fEndPoint[2],0.,0.);
        p=100*GetRandom();
        if (p<=31.25){
           nucltransK(fEnGamma[7],fEbindeK,1.4e-4,4.8e-4);
           next913=true;
        }
        else{
           nucltransK(fEnGamma[8],fEbindeK,5.0e-4,0.1e-4);
           next1777=true;
        }
     }
     else if (pbeta<= fProbBeta[3]){
        beta(fEndPoint[3],0.,0.);
        nucltransK(fEnGamma[9],fEbindeK,8.0e-5,15.1e-4);
        return;
     }
     else if (pbeta<= fProbBeta[4]){
        beta(fEndPoint[4],0.,0.);
        p=100*GetRandom();
        if (p<=17.89){
           nucltransK(fEnGamma[10],fEbindeK,1.7e-4,3.2e-4);
           next913=true;
        }
        else if (p<=47.36){
           nucltransK(fEnGamma[11],fEbindeK,3.1e-3,0.);
           next1991=true;
        }
        else if (p<=76.83){
           nucltransK(fEnGamma[12],fEbindeK,7.0e-3,0.);
           next2249=true;
        }
        else{
           nucltransK(fEnGamma[13],fEbindeK,5.0e-2,0.);
           next2596=true;
        }
     }
     else if (pbeta<= fProbBeta[5]){
        beta(fEndPoint[5],0.,0.);
        p=100*GetRandom();
        if (p<=16.10){
           nucltransK(fEnGamma[14],fEbindeK,9.0e-5,13.7e-4);
           return;
        }
        else if (p<=23.00){
           nucltransK(fEnGamma[15],fEbindeK,1.8e-4,2.7e-4);
           next913=true;
        }
        else if (p<=55.18){
           nucltransK(fEnGamma[16],fEbindeK,3.5e-4,0.3e-4);
           next1455=true;
        }
        else if (p<=67.82){
           nucltransK(fEnGamma[17],fEbindeK,1.0e-3,0.);
           next1975=true;
        }
        else{
           nucltransK(fEnGamma[18],fEbindeK,3.6e-2,0.);
           next2500=true;
        }
     }
     else if (pbeta<= fProbBeta[6]){
        beta(fEndPoint[6],0.,0.);
        p=100*GetRandom();
        if (p<=16.42){
           nucltransK(fEnGamma[19],fEbindeK,6.0e-4,0.);
           next1704=true;
        }
        else if (p<=58.21){
           nucltransK(fEnGamma[20],fEbindeK,1.0e-2,0.);
           next2500=true;
        }
        else{
           nucltransK(fEnGamma[21],fEbindeK,7.5e-2,0.);
           next2556=true;
        }
     }
     else if (pbeta<= fProbBeta[7]){
        beta(fEndPoint[7],0.,0.);
        p=100*GetRandom();
        if (p<=42.97){
           nucltransK(fEnGamma[22],fEbindeK,1.8e-4,2.2e-4);
           next913=true;
        }
        else if (p<=94.53){
           nucltransK(fEnGamma[23],fEbindeK,7.0e-4,0.);
           next1777=true;
        }
        else{
           nucltransK(fEnGamma[24],fEbindeK,1.4e-3,0.);
           next2049=true;
        }
     }
     else if (pbeta<=fProbBeta[8]){
        beta(fEndPoint[8],0.,0.);
        p=100*GetRandom();
        if (p<=23.49){
           nucltransK(fEnGamma[25],fEbindeK,1.9e-4,1.4e-4);
           next913=true;
        }
        else if (p<=62.64){
           nucltransK(fEnGamma[26],fEbindeK,1.1e-3,0.);
           next1886=true;
        }
        else if (p<=70.47){
           nucltransK(fEnGamma[27],fEbindeK,1.7e-3,0.);
           next2049=true;
        }
        else if (p<=74.38){
           nucltransK(fEnGamma[28],fEbindeK,3.5e-3,0.);
           next2212=true;
        }
        else if (p<=88.26){
           nucltransK(fEnGamma[29],fEbindeK,1.0e-2,0.);
           next2372=true;
        }
        else{
           nucltransK(fEnGamma[30],fEbindeK,2.0e-2,0.);
           next2445=true;
        }
     }
     else if (pbeta<=fProbBeta[9]){
        beta(fEndPoint[9],0.,0.);
        next2596=true;
     }
     else if (pbeta<=fProbBeta[10]){
        beta(fEndPoint[10],0.,0.);
        next2556=true;
     }
     else if (pbeta<=fProbBeta[11]){
        beta(fEndPoint[11],0.,0.);
        next2516=true;
     }
     else if (pbeta<=fProbBeta[12]){
        beta(fEndPoint[12],0.,0.);
        p=100*GetRandom();
        if (p<=64.89){
           nucltransK(fEnGamma[31],fEbindeK,2.6e-3,0.);
           next1560=true;
        }
        else{
           nucltransK(fEnGamma[32],fEbindeK,7.5e-2,0.);
           next2262=true;
        }
     }
     else if (pbeta<=fProbBeta[13]){
        beta(fEndPoint[13],0.,0.);
        next2500=true;
     }
     else if (pbeta<=fProbBeta[14]){
        beta(fEndPoint[14],0.,0.);
        p=100*GetRandom();
        if (p<=11.66){
           nucltransK(fEnGamma[33],fEbindeK,2.2e-4,4.5e-4);
           return;
        }
        else if (p<=31.09){
           nucltransK(fEnGamma[34],fEbindeK,7.0e-4,1.1e-4);
           next913=true;
        }
        else{
           nucltransK(fEnGamma[35],fEbindeK,1.5e-3,0.3e-4);
           next1307=true;
        }
     }
     else if (pbeta<=fProbBeta[15]){
        beta(fEndPoint[15],0.,0.);
        p=100*GetRandom();
        if (p<=5.30){
           nucltransK(fEnGamma[36],fEbindeK,7.0e-4,1.1e-4);
           next915=true;
        }
        else if (p<=27.73){
           nucltransK(fEnGamma[37],fEbindeK,8.5e-3,0.);
           next1893=true;
        }
        else if (p<=51.72){
           nucltransK(fEnGamma[38],fEbindeK,1.2e-2,0.);
           next1975=true;
        }
        else{
           nucltransK(fEnGamma[39],fEbindeK,1.4e-2,0.);
           next2005=true;
        }
     }
     else if (pbeta<=fProbBeta[16]){
        beta(fEndPoint[16],0.,0.);
        next2445=true;
     }
     else if (pbeta<=fProbBeta[17]){
        beta(fEndPoint[17],0.,0.);
        nucltransK(fEnGamma[40],fEbindeK,1.7e-1,0.);
        next2249=true;
     }
     else if (pbeta<=fProbBeta[18]){
        beta(fEndPoint[18],0.,0.);
        p=100*GetRandom();
        if (p<= 45.91){
           nucltransK(fEnGamma[41],fEbindeK,9.0e-4,0.9e-4);
           next913=true;
        }
        else if (p<=63.94){
           nucltransK(fEnGamma[42],fEbindeK,2.9e-3,0.);
           next1560=true;
        }
        else if (p<=81.97){
           nucltransK(fEnGamma[43],fEbindeK,2.0e-2,0.);
           next2005=true;
        }
        else{
           nucltransK(fEnGamma[44],fEbindeK,2.5e-1,0.);
           next2262=true;
        }
     }
     else if (pbeta<=fProbBeta[19]){
        beta(fEndPoint[19],0.,0.);
        next2372=true;
     }
     else if (pbeta<=fProbBeta[20]){
        beta(fEndPoint[20],0.,0.);
        next2262=true;
     }
     else if (pbeta<=fProbBeta[21]){
        beta(fEndPoint[21],0.,0.);
        next2249=true;
     }
     else if (pbeta<=fProbBeta[22]){
        beta(fEndPoint[22],0.,0.);
        next2212=true;
     }
     else if (pbeta<=fProbBeta[23]){
        beta(fEndPoint[23],0.,0.);
        p=100*GetRandom();
        if (p<=12.99){
           nucltransK(fEnGamma[45],fEbindeK,1.5e-3,0.4e-4);
           next913=true;
        }
        else if (p<=23.03){
           nucltransK(fEnGamma[46],fEbindeK,1.5e-3,0.4e-4);
           next915=true;
        }
        else if (p<=24.98){
           nucltransK(fEnGamma[47],fEbindeK,7.5e-2,0.);
           next1893=true;
        }
        else if (p<=63.96){
           nucltransK(fEnGamma[48],fEbindeK,1.8e-2,0.);
           next1991=true;
        }
        else if (p<=72.82){
           nucltransK(fEnGamma[49],fEbindeK,4.0e-1,0.);
           next2005=true;
        }
        else{
           nucltransK(fEnGamma[50],fEbindeK,1.3,0.);
           next2049=true;
        }
     }
     else if (pbeta<=fProbBeta[24]){
        beta(fEndPoint[24],0.,0.);
        next2049=true;
     }
     else if (pbeta<=fProbBeta[25]){
        beta(fEndPoint[25],0.,0.);
        next2005=true;
     }
     else if (pbeta<=fProbBeta[26]){
        beta(fEndPoint[26],0.,0.);
        next1991=true;
     }
     else if (pbeta<=fProbBeta[27]){
        beta(fEndPoint[27],0.,0.);
        next1975=true;
     }
     else if (pbeta<=fProbBeta[28]){
        beta(fEndPoint[28],0.,0.);
        next1943=true;
     }
     else if (pbeta<=fProbBeta[29]){
        beta(fEndPoint[29],0.,0.);
        next1886=true;
     }
     else if (pbeta<=fProbBeta[30]){
        beta(fEndPoint[30],0.,0.);
        next1777=true;
     }
     else if (pbeta<=fProbBeta[31]){
        beta(fEndPoint[31],0.,0.);
        next1647=true;
     }
     else if (pbeta<=fProbBeta[32]){
        beta(fEndPoint[32],0.,0.);
        next1560=true;
     }
     else if (pbeta<=fProbBeta[33]){
        beta(fEndPoint[33],0.,0.);
        next1516=true;
     }
     else if (pbeta<=fProbBeta[34]){
        beta(fEndPoint[34],0.,0.);
        next1455=true;
     }
     else if (pbeta<=fProbBeta[35]){
        beta(fEndPoint[35],0.,0.);
        next915=true;
     }
     else if (pbeta<=fProbBeta[36]){
        beta(fEndPoint[36],0.,0.);
        next913=true;
     }
     else{
        beta(fEndPoint[37],0.,0.);
        return;
     }

     if (next2596){
        p=100*GetRandom();
        if (p<=24.04){
           nucltransK(fEnGamma[51],fEbindeK,2.5e-4,1.0e-4);
           next913=true;
        }
        else if (p<=27.23){
           nucltransK(fEnGamma[52],fEbindeK,6.0e-4,0.);
           next1516=true;
        }
        else if (p<=31.05){
           nucltransK(fEnGamma[53],fEbindeK,8.0e-4,0.);
           next1647=true;
        }
        else if (p<=37.13){
           nucltransK(fEnGamma[54],fEbindeK,9.0e-4,0.);
           next1704=true;
        }
        else if (p<=41.94){
           nucltransK(fEnGamma[55],fEbindeK,9.0e-4,0.);
           next1707=true;
        }
        else if (p<=42.92){
           nucltransK(fEnGamma[56],fEbindeK,3.7e-3,0.);
           next1777=true;
        }
        else if (p<=47.09){
           nucltransK(fEnGamma[57],fEbindeK,1.4e-3,0.);
           next1886=true;
        }
        else if (p<=61.18){
           nucltransK(fEnGamma[58],fEbindeK,1.4e-3,0.);
           next1893=true;
        }
        else if (p<=64.71){
           nucltransK(fEnGamma[59],fEbindeK,1.7e-3,0.);
           next1943=true;
        }
        else if (p<=67.61){
           nucltransK(fEnGamma[60],fEbindeK,1.8e-3,0.);
           next1975=true;
        }
        else if (p<=74.97){
           nucltransK(fEnGamma[61],fEbindeK,7.7e-3,0.);
           next1991=true;
        }
        else if (p<=75.95){
           nucltransK(fEnGamma[62],fEbindeK,2.5e-2,0.);
           next2212=true;
        }
        else if (p<=79.77){
           nucltransK(fEnGamma[63],fEbindeK,3.0e-2,0.);
           next2249=true;
        }
        else if (p<=99.02){
           nucltransK(fEnGamma[64],fEbindeK,3.4e-2,0.);
           next2262=true;
        }
        else{
           nucltransK(fEnGamma[65],fEbindeK,1.0e-1,0.);
           next2372=true;
        }
     }
     if (next2556){
        p=100*GetRandom();
        if (p<=6.82){
           nucltransK(fEnGamma[66],fEbindeK,2.6e-4,0.9e-4);
           next913=true;
        }
        else if (p<=15.08){
           nucltransK(fEnGamma[67],fEbindeK,7.0e-4,0.);
           next1560=true;
        }
        else if (p<=26.44){
           nucltransK(fEnGamma[68],fEbindeK,3.7e-3,0.);
           next1729=true;
        }
        else if (p<=63.01){
           nucltransK(fEnGamma[69],fEbindeK,4.2e-3,0.);
           next1777=true;
        }
        else if (p<=65.28){
           nucltransK(fEnGamma[70],fEbindeK,6.0e-3,0.);
           next1893=true;
        }
        else if (p<=75.61){
           nucltransK(fEnGamma[71],fEbindeK,8.2e-3,0.);
           next1975=true;
        }
        else if (p<=77.06){
           nucltransK(fEnGamma[72],fEbindeK,8.3e-3,0.);
           next1991=true;
        }
        else if (p<=91.94){
           nucltransK(fEnGamma[73],fEbindeK,3.2e-2,0.);
           next2212=true;
        }
        else if (p<=96.49){
           nucltransK(fEnGamma[74],fEbindeK,4.8e-2,0.);
           next2262=true;
        }
        else{
           nucltransK(fEnGamma[75],fEbindeK,1.6e-1,0.);
           next2372=true;
        }
     }
     if (next2516){
        p=100*GetRandom();
        if (p<=30.50){
           nucltransK(fEnGamma[76],fEbindeK,4.8e-3,0.);
           next1777=true;
        }
        else if (p<=72.00){
           nucltransK(fEnGamma[77],fEbindeK,4.9e-3,0.);
           next1797=true;
        }
        else if (p<=86.00){
           nucltransK(fEnGamma[78],fEbindeK,6.0e-3,0.);
           next1893=true;
        }
        else{
           nucltransK(fEnGamma[79],fEbindeK,1.1e-2,0.);
           next1991=true;
        }
     }
     if (next2500){
        p=100*GetRandom();
        if (p<=62.53){
           nucltransK(fEnGamma[80],fEbindeK,7.0e-4,1.1e-4);
           next913=true;
        }
        else if (p<=67.31){
           nucltransK(fEnGamma[81],fEbindeK,1.1e-3,0.);
           next1704=true;
        }
        else if (p<=72.09){
           nucltransK(fEnGamma[82],fEbindeK,1.1e-3,0.);
           next1707=true;
        }
        else if (p<=84.25){
           nucltransK(fEnGamma[83],fEbindeK,5.0e-3,0.);
           next1777=true;
        }
        else if (p<=91.63){
           nucltransK(fEnGamma[84],fEbindeK,2.0e-3,0.);
           next1893=true;
        }
        else{
           nucltransK(fEnGamma[85],fEbindeK,3.0e-3,0.);
           next2005=true;
        }
     }
     if (next2445){
        p=100*GetRandom();
        if (p<=75.00){
           nucltransK(fEnGamma[86],fEbindeK,9.5e-4,0.);
           next1560=true;
        }
        else{
           nucltransK(fEnGamma[87],fEbindeK,7.0e-3,0.);
           next1817=true;
        }
     }
     if (next2372){
        p=100*GetRandom();
        if (p<= 2.55){
           nucltransK(fEnGamma[88],fEbindeK,2.5e-4,0.5e-4);
           next913=true;
        }
        else if (p<= 4.20){
           nucltransK(fEnGamma[89],fEbindeK,2.5e-4,0.5e-4);
           next915=true;
        }
        else if (p<= 5.85){
           nucltransK(fEnGamma[90],fEbindeK,1.3e-2,0.);
           next1647=true;
        }
        else if (p<=19.06){
           nucltransK(fEnGamma[91],fEbindeK,5.0e-3,0.);
           next1729=true;
        }
        else if (p<=37.37){
           nucltransK(fEnGamma[92],fEbindeK,9.0e-3,0.);
           next1797=true;
        }
        else if (p<=39.02){
           nucltransK(fEnGamma[93],fEbindeK,7.0e-3,0.);
           next1817=true;
        }
        else if (p<=53.13){
           nucltransK(fEnGamma[94],fEbindeK,3.5e-3,0.);
           next1893=true;
        }
        else if (p<=86.29){
           nucltransK(fEnGamma[95],fEbindeK,4.5e-3,0.);
           next1943=true;
        }
        else if (p<=97.10){
           nucltransK(fEnGamma[96],fEbindeK,5.2e-3,0.);
           next1975=true;
        }
        else if (p<=98.75){
           nucltransK(fEnGamma[97],fEbindeK,9.0e-3,0.);
           next2049=true;
        }
        else{
           nucltransK(fEnGamma[98],fEbindeK,1.6e-1,0.);
           next2262=true;
        }
     }
     if (next2262){
        p=100*GetRandom();
        if (p<=29.62){
           nucltransK(fEnGamma[99],fEbindeK,5.0e-4,0.3e-4);
           next913=true;
        }
        else if (p<=47.12){
           nucltransK(fEnGamma[100],fEbindeK,1.1e-2,0.);
           next1729=true;
        }
        else if (p<=52.69){
           nucltransK(fEnGamma[101],fEbindeK,3.5e-3,0.);
           next1797=true;
        }
        else if (p<=93.44){
           nucltransK(fEnGamma[102],fEbindeK,1.6e-2,0.);
           next1817=true;
        }
        else if (p<=95.63){
           nucltransK(fEnGamma[103],fEbindeK,6.0e-3,0.);
           next1893=true;
        }
        else{
           nucltransK(fEnGamma[104],fEbindeK,4.0e-2,0.);
           next1943=true;
        }
     }
     if (next2249){
        p=100*GetRandom();
        if (p<=34.30){
           nucltransK(fEnGamma[105],fEbindeK,1.3e-2,0.);
           next1777=true;
        }
        else if (p<=54.96){
           nucltransK(fEnGamma[106],fEbindeK,7.0e-3,0.);
           next1886=true;
        }
        else if (p<=81.82){
           nucltransK(fEnGamma[107],fEbindeK,7.0e-3,0.);
           next1893=true;
        }
        else{
           nucltransK(fEnGamma[108],fEbindeK,6.5e-2,0.);
           next1991=true;
        }
     }
     if (next2212){
        p=100*GetRandom();
        if (p<=7.42){
           nucltransK(fEnGamma[109],fEbindeK,4.0e-4,0.3e-4);
           next913=true;
        }
        else if (p<=60.70){
           nucltransK(fEnGamma[110],fEbindeK,1.7e-2,0.);
           next1777=true;
        }
        else if (p<=89.52){
           nucltransK(fEnGamma[111],fEbindeK,1.6e-2,0.);
           next1799=true;
        }
        else{
           nucltransK(fEnGamma[112],fEbindeK,1.0e-1,0.);
           next1991=true;
        }
     }
     if (next2049){
        p=100*GetRandom();
        if (p<=50.00){
           nucltransK(fEnGamma[113],fEbindeK,1.5e-4,2.2e-4);
           return;
        }
        else if (p<=61.48){
           nucltransK(fEnGamma[114],fEbindeK,1.7e-3,0.3e-4);
           next913=true;
        }
        else if (p<=75.00){
           nucltransK(fEnGamma[115],fEbindeK,1.7e-3,0.3e-4);
           next915=true;
        }
        else if (p<=90.98){
           nucltransK(fEnGamma[116],fEbindeK,3.5e-3,0.);
           next1307=true;
        }
        else{
           nucltransK(fEnGamma[117],fEbindeK,3.0e-2,0.);
           next1704=true;
        }
     }
     if (next2005){
        p=100*GetRandom();
        if (p<=72.63){
           nucltransK(fEnGamma[118],fEbindeK,8.0e-4,2.3e-4);
           return;
        }
        else if (p<=72.86){
           nucltransK(fEnGamma[119],fEbindeK,4.5e-4,0.7e-4);
           next312=true;
        }
        else if (p<=75.21){
           nucltransK(fEnGamma[120],fEbindeK,2.0e-3,0.);
           next915=true;
        }
        else if (p<=95.30){
           nucltransK(fEnGamma[121],fEbindeK,5.5e-3,0.);
           next1307=true;
        }
        else if (p<=97.65){
           nucltransK(fEnGamma[122],fEbindeK,5.0e-1,0.);
           next1886=true;
        }
        else{
           nucltransK(fEnGamma[123],fEbindeK,5.5e-1,0.);
           next1893=true;
        }
     }
     if (next1991){
        p=100*GetRandom();
        if (p<= 1.96){
           nucltransK(fEnGamma[124],fEbindeK,6.0e-4,0.);
           next913=true;
        }
        else if (p<=91.68){
           nucltransK(fEnGamma[125],fEbindeK,6.5e-2,0.);
           next1729=true;
        }
        else if (p<=98.47){
           nucltransK(fEnGamma[126],fEbindeK,3.5e-2,0.);
           next1797=true;
        }
        else{
           nucltransK(fEnGamma[127],fEbindeK,2.2e-1,0.);
           next1893=true;
        }
     }
     if (next1975){
        p=100*GetRandom();
        if (p<= 1.13){
           nucltransK(fEnGamma[128],fEbindeK,4.0e-4,1.9e-4);
           return;
        }
        else if (p<=49.41){
           nucltransK(fEnGamma[129],fEbindeK,2.0e-3,0.1e-4);
           next913=true;
        }
        else if (p<=51.15){
           nucltransK(fEnGamma[130],fEbindeK,2.0e-3,0.1e-4);
           next915=true;
        }
        else if (p<=59.29){
           nucltransK(fEnGamma[131],fEbindeK,8.0e-3,0.);
           next1455=true;
        }
        else if (p<=62.49){
           nucltransK(fEnGamma[132],fEbindeK,1.5e-2,0.);
           next1516=true;
        }
        else if (p<=67.43){
           nucltransK(fEnGamma[133],fEbindeK,3.2e-2,0.);
           next1777=true;
        }
        else if (p<=73.83){
           nucltransK(fEnGamma[134],fEbindeK,1.9e-1,0.);
           next1797=true;
        }
        else{
           nucltransK(fEnGamma[135],fEbindeK,1.8,0.);
           next1893=true;
        }
     }
     if (next1943){
        p=100*GetRandom();
        if (p<=35.36){
           nucltransK(fEnGamma[136],fEbindeK,2.1e-3,0.);
           next913=true;
        }
        else if (p<=97.97){
           nucltransK(fEnGamma[137],fEbindeK,2.6e-2,0.);
           next1729=true;
        }
        else{
           nucltransK(fEnGamma[138],fEbindeK,1.4e+1,0.);
           next1893=true;
        }
     }
     if (next1893){
        p=100*GetRandom();
        if (p<=2.13){
           nucltransK(fEnGamma[139],fEbindeK,4.0e-4,1.4e-4);
           return;
        }
        else if (p<=23.29){
           nucltransK(fEnGamma[140],fEbindeK,2.5e-3,0.);
           next913=true;
        }
        else if (p<=92.90){
           nucltransK(fEnGamma[141],fEbindeK,2.5e-3,0.);
           next915=true;
        }
        else if (p<=96.02){
           nucltransK(fEnGamma[142],fEbindeK,2.5e-2,0.);
           next1516=true;
        }
        else{
           nucltransK(fEnGamma[143],fEbindeK,1.5e-1,0.);
           next1777=true;
        }
     }
     if (next1886){
        p=100*GetRandom();
        if (p<=18.33){
           nucltransK(fEnGamma[144],fEbindeK,2.0e-4,1.2e-4);
           return;
        }
        else if (p<=28.52){
           nucltransK(fEnGamma[145],fEbindeK,2.5e-3,0.);
           next913=true;
        }
        else if (p<=34.63){
           nucltransK(fEnGamma[146],fEbindeK,2.5e-3,0.);
           next915=true;
        }
        else if (p<=36.67){
           nucltransK(fEnGamma[147],fEbindeK,2.6e-2,0.);
           next1516=true;
        }
        else if (p<=40.74){
           nucltransK(fEnGamma[148],fEbindeK,2.0e-1,0.);
           next1707=true;
        }
        else{
           nucltransK(fEnGamma[149],fEbindeK,1.4,0.);
           next1797=true;
        }
     }
     if (next1817){
        p=100*GetRandom();
        if (p<=52.17){
           nucltransK(fEnGamma[150],fEbindeK,3.2e+1,0.);
           next1777=true;
        }
        else{
           nucltransK(fEnGamma[151],0.005,2.0e+1,0.);
           next1799=true;
        }
     }
     if (next1799){
        nucltransK(fEnGamma[152],fEbindeK,2.5e-1,0.);
        next1634=true;
     }
     if (next1797){
        p=100*GetRandom();
        if (p<=3.06){
           nucltransK(fEnGamma[153],fEbindeK,4.5e-4,0.9e-4);
           return;
        }
        else if (p<=19.90){
           nucltransK(fEnGamma[154],fEbindeK,3.0e-3,0.);
           next913=true;
        }
        else if (p<=57.49){
           nucltransK(fEnGamma[155],fEbindeK,3.0e-3,0.);
           next915=true;
        }
        else if (p<=65.99){
           nucltransK(fEnGamma[156],fEbindeK,3.0e-2,0.);
           next1455=true;
        }
        else if (p<=67.86){
           nucltransK(fEnGamma[157],fEbindeK,5.5e-2,0.);
        }
        else if (p<=75.34){
           nucltransK(fEnGamma[158],fEbindeK,3.3e-1,0.);
           next1647=true;
        }
        else{
           nucltransK(fEnGamma[159],0.005,2.6,0.);
           next1777=true;
        }
     }
     if (next1777){
        p=100*GetRandom();
        if (p<=98.61){
           nucltransK(fEnGamma[160],fEbindeK,2.5e-3,0.);
           next913=true;
        }
        else{
           nucltransK(fEnGamma[161],fEbindeK,1.7e+1,0.);
           next1729=true;
        }
     }
     if (next1729){
        fThlev=170.e-9;
        p=100*GetRandom();
        if (p<=37.93){
           nucltransK(fEnGamma[162],fEbindeK,4.7e-2,0.);
           next1560=true;
        }
        else{
           nucltransK(fEnGamma[163],fEbindeK,2.1,0.);
           next1634=true;
        }
     }
     if (next1707){
        p=100*GetRandom();
        if (p<=90.91){
           nucltransK(fEnGamma[164],fEbindeK,4.0e-3,0.);
           next913=true;
        }
        else{
           nucltransK(fEnGamma[165],fEbindeK,3.0e-3,0.);
           next915=true;
        }
     }
     if (next1704){
        p=100*GetRandom();
        if (p<=52.17){
           nucltransK(fEnGamma[166],fEbindeK,6.0e-4,1.4e-4);
           return;
        }
        else if (p<=60.14){
           nucltransK(fEnGamma[167],fEbindeK,8.5e-4,0.2e-4);
           next312=true;
        }
        else if (p<=68.11){
           nucltransK(fEnGamma[168],fEbindeK,4.0e-3,0.);
           next913=true;
        }
        else{
           nucltransK(fEnGamma[169],fEbindeK,4.0e-3,0.);
           next915=true;
        }
     }
     if (next1647){
        p=100*GetRandom();
        if (p<=9.88){
           nucltransK(fEnGamma[170],fEbindeK,5.0e-4,0.6e-4);
           return;
        }
        else if (p<=72.34){
           nucltransK(fEnGamma[171],fEbindeK,4.6e-3,0.);
           next913=true;
        }
        else if (p<=93.86){
           nucltransK(fEnGamma[172],fEbindeK,4.6e-3,0.);
           next915=true;
        }
        else{
           nucltransK(fEnGamma[173],fEbindeK,2.9,0.);
           next1560=true;
        }
     }
     if (next1634){
        fThlev=9.;
        nucltransK(fEnGamma[174],fEbindeK,2.4e+1,0.);
        next1560=true;
     }
     if (next1560){
        fThlev=0.;
        nucltransK(fEnGamma[175],fEbindeK,4.6e-3,0.);
        next913=true;
     }
     if (next1516){
        fThlev=0.;
        p=100*GetRandom();
        if (p<=78.54){
           nucltransK(fEnGamma[176],fEbindeK,8.0e-4,0.9e-4);
           return;
        }
        else if (p<=92.15){
           nucltransK(fEnGamma[177],fEbindeK,1.1e-3,0.1e-4);
           next312=true;
        }
        else{
           nucltransK(fEnGamma[178],fEbindeK,8.0e-3,0.);
           next915=true;
        }
     }
     if (next1455){
        fThlev=0.;
        p=100*GetRandom();
        if (p<=30.90){
           nucltransK(fEnGamma[179],fEbindeK,1.0e-3,0.9e-4);
           return;
        }
        else if (p<=87.98){
           nucltransK(fEnGamma[180],fEbindeK,1.8e-3,0.3e-4);
           next312=true;
        }
        else{
           nucltransK(fEnGamma[181],fEbindeK,1.0e-2,0.);
           next915=true;
        }
     }
     if (next1307){
        fThlev=0.;
        p=100*GetRandom();
        if (p<=39.00){
           nucltransK(fEnGamma[182],fEbindeK,1.2e-3,0.5e-4);
           return;
        }
        else if (p<=89.00){
           nucltransK(fEnGamma[183],fEbindeK,2.5e-3,0.);
           next312=true;
        }
        else{
           nucltransK(fEnGamma[184],fEbindeK,2.1e-2,0.);
           next915=true;
        }
     }
     if (next915){
        fThlev=0.;
        p=100*GetRandom();
        if (p<=99.84){
           nucltransK(fEnGamma[185],fEbindeK,2.7e-3,0.);
           return;
        }
        else{
           nucltransK(fEnGamma[186],fEbindeK,5.0e-3,0.);
           next312=true;
        }
     }
     if (next913){
        fThlev=0.;
        nucltransK(fEnGamma[187],fEbindeK,2.1e-3,0.);
        return;
     }
     if (next312){
        fThlev=0.;
        nucltransK(fEnGamma[188],fEbindeK,4.0e-2,0.);
        return;
     }
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Te134()
{
  // Yu.V.Sergeenkov,Nucl.Data Sheets 71(1994)557.
  // VIT, 7.10.2002.
  bool next923=false, next847=false, next645=false, next210=false;
  bool next181=false, next79=false, next44=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<= 3.33){
        nucltransK(fEnGamma[0],fEbindeK,1.5e-3,0.);
        next79=true;
     }
     else if (p<=14.45){
        nucltransK(fEnGamma[1],fEbindeK,2.5e-3,0.);
        next181=true;
     }
     else if (p<=17.78){
        nucltransK(fEnGamma[2],fEbindeK,3.0e-3,0.);
        next210=true;
     }
     else if (p<=91.15){
        nucltransK(fEnGamma[3],fEbindeK,1.5e-2,0.);
        next645=true;
     }
     else if (p<=94.70){
        nucltransK(fEnGamma[4],fEbindeK,6.0e-2,0.);
        next847=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,1.8e-1,0.);
        next923=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     next923=true;
  }
  else{
     beta(fEndPoint[2],0.,0.);
     next847=true;
  }

  if (next923){
     p=100*GetRandom();
     if (p<= 2.71){
        nucltransK(fEnGamma[6],fEbindeK,2.5e-3,0.);
        next79=true;
     }
     else if (p<=37.31){
        nucltransK(fEnGamma[7],fEbindeK,4.5e-3,0.);
        next181=true;
     }
     else if (p<=47.94){
        nucltransK(fEnGamma[8],fEbindeK,5.0e-3,0.);
        next210=true;
     }
     else if (p<=98.37){
        nucltransK(fEnGamma[9],fEbindeK,4.9e-2,0.);
        next645=true;
     }
     else{
        nucltransK(fEnGamma[10],fEbindeK,1.61,0.);
        next847=true;
     }
  }
  if (next847){
     p=100*GetRandom();
     if (p<=69.71){
        nucltransK(fEnGamma[11],fEbindeK,3.3e-3,0.);
        next79=true;
     }
     else if (p<=72.49){
        nucltransK(fEnGamma[12],fEbindeK,6.0e-3,0.);
        next181=true;
     }
     else if (p<=76.45){
        nucltransK(fEnGamma[13],fEbindeK,7.0e-3,0.);
        next210=true;
     }
     else{
        nucltransK(fEnGamma[14],fEbindeK,1.3e-1,0.);
        next645=true;
     }
  }
  if (next645){
     p=100*GetRandom();
     if (p<= 2.03){
        nucltransK(fEnGamma[15],fEbindeK,5.0e-3,0.);
        return;
     }
     else if (p<=45.01){
        nucltransK(fEnGamma[16],fEbindeK,9.0e-3,0.);
        next79=true;
     }
     else if (p<=55.87){
        nucltransK(fEnGamma[17],fEbindeK,1.5e-2,0.);
        next181=true;
     }
     else{
        nucltransK(fEnGamma[18],fEbindeK,1.4e-2,0.);
        next210=true;
     }
  }
  if (next210){
     fThlev=0.15e-9;
     p=100*GetRandom();
     if (p<=98.94){
        nucltransK(fEnGamma[19],fEbindeK,1.1e-1,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[20],fEbindeK,5.2e-1,0.);
        next79=true;
     }
  }
  if (next181){
     fThlev=0.1e-9;
     p=100*GetRandom();
     if (p<=95.83){
        nucltransK(fEnGamma[21],fEbindeK,1.8e-1,0.);
        return;
     }
     else if (p<=96.45){
        nucltransK(fEnGamma[22],fEbindeK,5.8e-1,0.);
        next44=true;
     }
     else{
        nucltransK(fEnGamma[23],fEbindeK,1.2,0.);
        next79=true;
     }
  }
  if (next79){
     fThlev=1.62e-9;
     nucltransK(fEnGamma[24],fEbindeK,1.50,0.);
     return;
  }
  if (next44){
     fThlev=0.;
     nucltransK(fEnGamma[25],fEbindeK,7.97,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Th234()
{
  // Scheme of Th234 decay to Pa234m (not Pa234!) in accordance with NDS
  // 108(2007)681 and ENSDF database at NNDC site on 8.08.2007.
  // VIT, 8.08.2007.
  bool next167=false, next103=false, next74=false;
  float pbeta, p;

  pbeta=100.*GetRandom();
  if (pbeta<= fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100*GetRandom();
     if (p<=17.0){
        nucltransK(fEnGamma[0],fEbindeK,2.3e-1,0.);
        next74=true;
     }
     else if (p<=21.7){
        nucltransK(fEnGamma[1],fEbindeK,2.0e-1,0.);
        next103=true;
     }
     else{
        nucltransK(fEnGamma[2],0.005,2.4e2,0.);
        next167=true;
     }
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     nucltransK(fEnGamma[3],fEbindeK,3.8e0,0.);
     next74=true;
  }
  else if (pbeta<= fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     next167=true;
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     p=100*GetRandom();
     if (p<=97.0){
        nucltransK(fEnGamma[4],fEbindeK,5.3e0,0.);
        next74=true;
     }
     else{
        nucltransK(fEnGamma[5],fEbindeK,2.5e1,0.);
        next103=true;
     }
  }
  else{
     beta(fEndPoint[4],0.,0.);
     next74=true;
  }

  if (next167){
     fThlev=0.55e-9;
     p=100*GetRandom();
     if (p<=32.1){
        nucltransK(fEnGamma[6],fEbindeK,1.5e-1,0.);
        next74=true;
     }
     else{
        nucltransK(fEnGamma[7],fEbindeK,4.1e-1,0.);
        next103=true;
     }

  }
  if (next103){
     fThlev=0.;
     nucltransK(fEnGamma[8],fEbindeK,4.4e3,0.);
     next74=true;
  }
  if (next74){
     // below is creation of Pa234m with T1/2=1.159 m which mainly
     // beta- decays to U234 (IT to Pa234 is only 0.16%);
     // decay of Pa234m should be generated independently
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Tl208()
{
  //"Table of Isotopes", 7th ed., 1978.
  // VIT, 27.07.1992, 22.10.1995.
  // VIT, 11.05.2005, updated to NDS 47(1986)797
  //     (last NDS issue for A=208).
  // VIT, 4.02.2009, updated to NDS 108(2007)1583;
  //     also were added: LM conversion electrons;
  //     more complex emission of X rays emitted in K-conversion.
  bool next3708=false, next3475=false, next3198=false, next2615=false;
  float pbeta, p;

  pbeta=100.*GetRandom(); // 82.5% beta decay to 133I
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     nucltransKLM_Pb(fEnGamma[0],fEbindeK,7.75e-3,fEbindeL,1.27e-3,fEbindeM,0.41e-3,2.3e-5);
     next3198=true;
  }
  else if (pbeta<= fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     nucltransKLM_Pb(fEnGamma[1],fEbindeK,9.49e-3,fEbindeL,1.56e-3,fEbindeM,0.47e-3,4.9e-6);
     next3198=true;
  }
  else if (pbeta<= fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     p=100*GetRandom();
     if (p<= 4.55){
        nucltransKLM_Pb(fEnGamma[2],fEbindeK,3.56e-3,fEbindeL,0.58e-3,fEbindeM,0.17e-3,2.6e-4);
        next2615=true;
     }
     else if (p<=29.55){
        nucltransKLM_Pb(fEnGamma[3],fEbindeK,9.99e-3,fEbindeL,1.64e-3,fEbindeM,0.51e-3,2.6e-6);
        next3198=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[4],fEbindeK,20.13e-3,fEbindeL,3.33e-3,fEbindeM,1.02e-3,0.);
        next3475=true;
     }
  }
  else if (pbeta<= fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     nucltransKLM_Pb(fEnGamma[5],fEbindeK,1.69e-3,fEbindeL,0.26e-3,fEbindeM,0.08e-3,2.1e-6);
     next3198=true;
  }
  else if (pbeta<= fProbBeta[4]){
     beta(fEndPoint[4],0.,0.);
     p=100*GetRandom();
     if (p<=87.23){
        nucltransKLM_Pb(fEnGamma[6],fEbindeK,2.43e-2,fEbindeL,0.40e-2,fEbindeM,0.12e-2,0.);
        next3475=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[7],fEbindeK,5.78e-2,fEbindeL,0.97e-2,fEbindeM,0.29e-2,0.);
        next3708=true;
     }
  }
  else if (pbeta<= fProbBeta[5]){
     beta(fEndPoint[5],0.,0.);
     nucltransKLM_Pb(fEnGamma[8],fEbindeK,4.11e-3,fEbindeL,0.67e-3,fEbindeM,0.20e-3,0.19e-3);
     next2615=true;
  }
  else if (pbeta<= fProbBeta[6]){
     beta(fEndPoint[6],0.,0.);
     p=100*GetRandom();
     if (p<=90.31){
        nucltransKLM_Pb(fEnGamma[9],fEbindeK,1.53e-2,fEbindeL,0.25e-2,fEbindeM,0.08e-2,0.);
        next3198=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[10],fEbindeK,3.60e-2,fEbindeL,0.60e-2,fEbindeM,0.18e-2,0.);
        next3475=true;
     }
  }
  else if (pbeta<= fProbBeta[7]){
     beta(fEndPoint[7],0.,0.);
     p=100*GetRandom();
     if (p<=96.15){
        nucltransKLM_Pb(fEnGamma[11],fEbindeK,1.77e-2,fEbindeL,0.29e-2,fEbindeM,0.10e-2,0.);
        next3198=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[12],fEbindeK,4.45e-2,fEbindeL,0.75e-2,fEbindeM,0.22e-2,0.);
        next3475=true;
     }
  }
  else if (pbeta<=fProbBeta[8]){
     beta(fEndPoint[8],0.,0.);
     nucltransKLM_Pb(fEnGamma[13],fEbindeK,6.43e-3,fEbindeL,1.05e-3,fEbindeM,0.32e-3,0.05e-3);
     next2615=true;
  }
  else if (pbeta<= fProbBeta[9]){
     beta(fEndPoint[9],0.,0.);
     p=100*GetRandom();
     if (p<=51.25){
        nucltransKLM_Pb(fEnGamma[14],fEbindeK,2.93e-2,fEbindeL,0.49e-2,fEbindeM,0.14e-2,0.);
        next3198=true;
     }
     else if (p<=64.82){
        nucltransKLM_Pb(fEnGamma[15],fEbindeK,9.54e-2,fEbindeL,1.61e-2,fEbindeM,0.49e-2,0.);
        next3475=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[16],fEbindeK,51.60e-2,fEbindeL,8.83e-2,fEbindeM,2.57e-2,0.);
        next3708=true;
     }
  }
  else if (pbeta<= fProbBeta[10]){
     beta(fEndPoint[10],0.,0.);
     nucltransKLM_Pb(fEnGamma[17],fEbindeK,3.08e-2,fEbindeL,0.51e-2,fEbindeM,0.16e-2,0.);
     next3198=true;
  }
  else if (pbeta<= fProbBeta[11]){
     beta(fEndPoint[11],0.,0.);
     p=100*GetRandom();
     if (p<=39.49){
        nucltransKLM_Pb(fEnGamma[18],fEbindeK,3.20e-2,fEbindeL,0.54e-2,fEbindeM,0.16e-2,0.);
        next3198=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[19],fEbindeK,9.22e-1,fEbindeL,1.59e-1,fEbindeM,0.45e-1,0.);
        next3708=true;
     }
  }
  else if (pbeta<=fProbBeta[12]){
     beta(fEndPoint[12],0.,0.);
     next3708=true;
  }
  else if (pbeta<=fProbBeta[13]){
     beta(fEndPoint[13],0.,0.);
     next3475=true;
  }
  else{
     beta(fEndPoint[14],0.,0.);
     next3198=true;
  }

  if (next3708){
     p=100*GetRandom();
     if (p<=1.66){
        nucltransKLM_Pb(fEnGamma[20],fEbindeK,4.49e-3,fEbindeL,0.84e-3,fEbindeM,0.27e-3,0.);
        next2615=true;
     }
     else if (p<=97.95){
        nucltransKLM_Pb(fEnGamma[21],fEbindeK,8.42e-2,fEbindeL,1.42e-2,fEbindeM,0.43e-2,0.);
        next3198=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[22],fEbindeK,5.47e-1,fEbindeL,1.16e-1,fEbindeM,0.37e-1,0.);
        next3475=true;
     }
  }
  if (next3475){
     fThlev=4.e-12;
     p=100*GetRandom();
     if (p<=55.95){
        nucltransKLM_Pb(fEnGamma[23],fEbindeK,2.17e-2,fEbindeL,0.36e-2,fEbindeM,0.11e-2,0.);
        next2615=true;
     }
     else{
        nucltransKLM_Pb(fEnGamma[24],fEbindeK,4.36e-1,fEbindeL,0.75e-1,fEbindeM,0.22e-1,0.);
        next3198=true;
     }
  }
  if (next3198){
     fThlev=294.e-12;
     nucltransKLM_Pb(fEnGamma[25],fEbindeK,1.51e-2,fEbindeL,0.41e-2,fEbindeM,0.13e-2,0.);
     next2615=true;
  }
  if (next2615){
     fThlev=16.7e-12;
     nucltransKLM_Pb(fEnGamma[26],fEbindeK,1.71e-3,fEbindeL,0.29e-3,fEbindeM,0.10e-3,0.37e-3);
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Xe133()
{
  // "Table of Isotopes", 8th ed., 1996.
  // VIT, 18.08.1997. Updated 5.12.2003 in accordance with NDS 75(1995)491.
  bool next161=false, next81=false;
  float pbeta, p;

  pbeta=100.*GetRandom(); // 82.5% beta decay to 133I
  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     fThlev=21.e-12;
     p=100.*GetRandom();
     if (p<=32.313){
        nucltransK(fEnGamma[0],fEbindeK,2.0e-2,0.);
        return;
     }
     else if (p<=98.259){
        nucltransK(fEnGamma[1],fEbindeK,4.4e-2,0.);
        next81=true;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,9.8e-2,0.);
        next161=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     next161=true;
  }
  else{
     beta(fEndPoint[2],0.,0.);
     next81=true;
  }

  if (next161){
     fThlev=172.e-12;
     p=100.*GetRandom();
     if (p<=10.287){
        nucltransK(fEnGamma[3],fEbindeK,3.0e-1,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[4],fEbindeK,1.8e+0,0.);
        next81=true;
     }
  }
  if (next81){
     fThlev=6.28e-9;
     nucltransK(fEnGamma[5],fEbindeK,1.7e+0,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Xe135()
{
  // Yu.V.Sergeenkov et al.,Nucl.Data Sheets 84 (1998) 115.
  // VIT, 9.10.2002.
  bool next608=false, next408=false, next250=false;
  float pbeta, p;
  pbeta=100.*GetRandom();

  if (pbeta<=fProbBeta[0]){
     beta(fEndPoint[0],0.,0.);
     p=100.*GetRandom();
     if (p<= 3.34){
        nucltransK(fEnGamma[0],fEbindeK,2.0e-3,0.);
        return;
     }
     else if (p<=60.40){
        nucltransK(fEnGamma[1],fEbindeK,3.8e-3,0.);
        next250=true;
     }
     else if (p<=97.07){
        nucltransK(fEnGamma[2],fEbindeK,6.5e-3,0.);
        next408=true;
     }
     else{
        nucltransK(fEnGamma[3],fEbindeK,1.5e-2,0.);
        next608=true;
     }
  }
  else if (pbeta<=fProbBeta[1]){
     beta(fEndPoint[1],0.,0.);
     p=100.*GetRandom();
     if (p<=73.53){
        nucltransK(fEnGamma[4],fEbindeK,5.0e-3,0.);
        next250=true;
     }
     else if (p<=79.95){
        nucltransK(fEnGamma[5],fEbindeK,9.0e-3,0.);
        next408=true;
     }
     else{
        nucltransK(fEnGamma[6],fEbindeK,2.5e-2,0.);
        next608=true;
     }
  }
  else if (pbeta<=fProbBeta[2]){
     beta(fEndPoint[2],0.,0.);
     next608=true;
  }
  else if (pbeta<=fProbBeta[3]){
     beta(fEndPoint[3],0.,0.);
     next408=true;
  }
  else{
     beta(fEndPoint[4],0.,0.);
     next250=true;
  }

  if (next608){
     p=100.*GetRandom();
     if (p<= 92.42){
        nucltransK(fEnGamma[7],fEbindeK,7.5e-3,0.);
        return;
     }
     else if (p<= 99.62){
        nucltransK(fEnGamma[8],fEbindeK,2.7e-2,0.);
        next250=true;
     }
     else{
        nucltransK(fEnGamma[9],fEbindeK,1.4e-1,0.);
        next408=true;
     }
  }
  if (next408){
     p=100.*GetRandom();
     if (p<=55.33){
        nucltransK(fEnGamma[10],fEbindeK,2.0e-2,0.);
        return;
     }
     else{
        nucltransK(fEnGamma[11],fEbindeK,2.5e-1,0.);
        next250=true;
     }
  }
  if (next250){
     fThlev=0.28e-9;
     nucltransK(fEnGamma[12],fEbindeK,7.6e-2,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Y88()
{
  // "Table of Isotopes", 7th ed., 1978.
  // Accuracy in description of: decay branches       - 0.001%,
  //                           : deexcitation process - 0.001%.
  //  VIT, 20.07.1993, 22.10.1995.
  //  VIT, 12.11.2006 (update to NDS 105(2005)419 and change
  //  of beta+ spectrum shape)
  bool next2734=false, next1836=false;
  float pdecay, p;

  pdecay=100.*GetRandom();
  if (pdecay<=fProbDecay[0]){
     particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
     fThlev=0.14e-9;
     nucltransK(fEnGamma[0],fEbindeK,8.5e-4,0.);
     next2734=true;
  }
  else if (pdecay<=fProbDecay[1]){
     particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
     fThlev=0.13e-12;
     p=100.*GetRandom();
     if (p<=25.){
        nucltransK(fEnGamma[1],fEbindeK,6.0e-5,8.7e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[2],fEbindeK,2.6e-4,4.8e-5);
        next1836=true;
     }
  }
  else if (pdecay<=fProbDecay[2]){
     particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
     next2734=true;
  }
  else if (pdecay<=fProbDecay[3]){
     particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0.,0.);
     next1836=true;
  }
  else{  //b+ to Sr88
     beta2f(fEndPoint[0],0.,0.,1,fShCorrFactor[0],fShCorrFactor[1],fShCorrFactor[2],fShCorrFactor[3]);
     next1836=true;
  }

  if (next2734){
     fThlev=0.78e-12;
     p=100.*GetRandom();
     if (p<=0.75){
        nucltransK(fEnGamma[3],fEbindeK,1.2e-4,3.3e-4);
        return;
     }
     else{
        nucltransK(fEnGamma[4],fEbindeK,3.1e-4,0.);
        next1836=true;
     }
  }
  if (next1836){
     fThlev=0.148e-12;
     nucltransK(fEnGamma[5],fEbindeK,1.4e-4,2.3e-4);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Zn65()
{
  // NDS 69(1993)209 and NNDC online corrections on 28.03.2007).
  float pec, p;

  pec=100.*GetRandom();
  if (pec<=fProbEC[0]){
     pair(0.329); // beta+ decay to g.s. of 65-Cu
     return;
  }
  else{  // X ray after EC to 65-Cu
     particle(1,fEbindeK,fEbindeK,0.,pi,0.,twopi,0,0);
     if (pec<=fProbEC[1]){
        fThlev=0.285e-12;
        p=100.*GetRandom();
        if (p<=99.994){
           nucltransK(fEnGamma[0],fEbindeK,1.9e-4,1.0e-6);
           return;
        }
        else{
           nucltransK(fEnGamma[1],fEbindeK,6.8e-3,0.);
           fThlev=99.e-15;
           nucltransK(fEnGamma[2],fEbindeK,3.8e-4,0.);
           return;
        }
     }
     else return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void Decay0::Nb96()
{
  // NDS 68(1993)165 and ENSDF at NNDC site on 13.11.2007).
  // VIT, 7.05.1998; update 13.11.2007.
  fThnuc=8.406e4;
  float tcnuc=0.;
  fTdnuc=tcnuc-fThnuc/log(2.)*log(GetRandom());
  fTclev=0.;
  fThlev=0.;
  fZdtr=42;
  bool next2755=false, next2441=false, next2439=false, next2219=false;
  bool next1978=false, next1870=false, next1628=false, next1626=false;
  bool next1498=false, next778=false;
  float pbeta, p;
  pbeta=100.*GetRandom();
  if (pbeta<=0.024){
     beta(0.212,0.,0.);
     nucltransK(1.347,0.020,4.3e-4,0.3e-4);
     next1628=true;
  }
  else if (pbeta<=0.524){
     beta(0.311,0.,0.);
     p=100.*GetRandom();
     if (p<=93.26){
        nucltransK(0.435,0.020,5.4e-3,0.);
        next2441=true;
     }
     else{
        nucltransK(0.120,0.020,1.5e-1,0.);
        next2755=true;
     }
  }
  else if (pbeta<=1.014){
     beta(0.432,0.,0.);
     next2755=true;
  }
  else if (pbeta<=3.314){
     beta(0.746,0.,0.);
     next2441=true;
  }
  else{
     beta(0.748,0.,0.);
     next2439=true;
  }

  if (next2755){
     p=100.*GetRandom();
     if (p<=75.63){
        nucltransK(1.127,0.020,5.2e-4,0.1e-5);
        next1628=true;
     }
     else if (p<=86.64){
        nucltransK(0.316,0.020,1.2e-2,0.);
        next2439=true;
     }
     else{
        nucltransKLM(0.314,0.020,1.1e-2,0.003,1.3e-3,0.001,4.2e-4,0.);
        next2441=true;
     }
  }
  if (next2441){
     nucltransK(0.813,0.020,1.3e-3,0.);
     next1628=true;
  }
  if (next2439){
     p=100.*GetRandom();
     if (p<=11.20){
        nucltransKLM(0.811,0.020,1.1e-3,0.003,1.3e-4,0.001,2.2e-5,0.);
        next1628=true;
     }
     else if (p<=69.87){
        nucltransKLM(0.569,0.020,2.6e-3,0.003,2.8e-4,0.001,5.8e-5,0.);
        next1870=true;
     }
     else if (p<=96.89){
        nucltransKLM(0.460,0.020,5.3e-3,0.003,6.4e-4,0.001,1.3e-4,0.);
        next1978=true;
     }
     else{
        nucltransKLM(0.219,0.020,3.2e-2,0.003,4.0e-3,0.001,1.4e-3,0.);
        next2219=true;
     }
  }
  if (next2219){
     p=100.*GetRandom();
     if (p<=11.19){
        nucltransK(1.441,0.020,4.3e-4,0.5e-4);
        next778=true;
     }
     else if (p<=36.94){
        nucltransKLM(0.722,0.020,1.5e-3,0.003,1.7e-4,0.001,3.6e-5,0.);
        next1498=true;
     }
     else if (p<=44.75){
        nucltransKLM(0.593,0.020,2.3e-3,0.003,2.6e-4,0.001,5.2e-5,0.);
        next1626=true;
     }
     else if (p<=68.44){
        nucltransKLM(0.591,0.020,2.4e-3,0.003,2.8e-4,0.001,4.9e-5,0.);
        next1628=true;
     }
     else if (p<=80.66){
        nucltransKLM(0.350,0.020,1.0e-2,0.003,1.2e-3,0.001,5.0e-4,0.);
        next1870=true;
     }
     else{
        nucltransKLM(0.241,0.020,2.1e-2,0.003,2.4e-3,0.001,4.9e-4,0.);
        next1978=true;
     }
  }
  if (next1978){
     p=100.*GetRandom();
     if (p<=71.90){
        nucltransK(1.200,0.020,4.6e-4,7.7e-6);
        next778=true;
     }
     else if (p<=92.93){
        nucltransK(0.481,0.020,4.5e-3,0.);
        next1498=true;
     }
     else if (p<=95.95){
        nucltransKLM(0.353,0.020,1.0e-2,0.003,1.2e-3,0.001,7.0e-4,0.);
        next1626=true;
     }
     else if (p<=99.81){
        nucltransKLM(0.350,0.020,1.0e-2,0.003,1.2e-3,0.001,5.0e-4,0.);
        next1628=true;
     }
     else{
        nucltransKLM(0.109,0.020,1.7e-1,0.003,2.0e-2,0.001,4.2e-3,0.);
        next1870=true;
     }
  }
  if (next1870){
     fThlev=6.4e-12;
     p=100.*GetRandom();
     if (p<=88.61){
        nucltransK(1.091,0.020,5.6e-4,0.);
        next778=true;
     }
     else if (p<=93.45){
        nucltransKLM(0.372,0.020,1.0e-2,0.003,1.3e-3,0.001,4.3e-4,0.);
        next1498=true;
     }
     else{
        nucltransKLM(0.241,0.020,2.1e-2,0.003,2.4e-3,0.001,7.8e-4,0.);
        next1628=true;
     }
  }
  if (next1628){
     fThlev=1.2e-12;
     nucltransKLM(0.850,0.020,1.0e-3,0.003,1.1e-4,0.001,2.3e-5,0.);
     next778=true;
  }
  if (next1626){
     p=100.*GetRandom();
     if (p<=11.72){
        nucltransK(1.626,0.020,2.8e-4,1.3e-4);
        return;
     }
     else if (p<=98.63){
        nucltransKLM(0.848,0.020,1.0e-3,0.003,1.1e-4,0.001,2.3e-5,0.);
        next778=true;
     }
     else{
        nucltransKLM(0.128,0.020,1.1e-1,0.003,1.3e-2,0.001,2.7e-3,0.);
        next1498=true;
     }
  }
  if (next1498){
     fThlev=0.78e-12;
     p=100.*GetRandom();
     if (p<=32.43){
        nucltransK(1.498,0.020,3.3e-4,0.8e-4);
        return;
     }
     else{
        nucltransKLM(0.720,0.020,1.5e-3,0.003,1.7e-4,0.001,3.5e-5,0.);
        next778=true;
     }
  }
  if (next778){
     fThlev=3.67e-12;
     nucltransKLM(0.778,0.020,1.2e-3,0.003,1.4e-3,0.001,2.9e-5,0.);
     return;
  }
}
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
/// End of functions
///^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

///************************************************/
/// Additional functions
///************************************************/
void Decay0::particle(int np,float E1,float E2,float teta1,float teta2,float phi1,float phi2,float tclev,float thlev)
{
  // Generation of isotropic emission of particle in the range of  energies and angles.
  // E1,E2       - range of kinetic energy of particle (MeV);
  // teta1,teta2 - range of teta angle (radians);
  // phi1,phi2   - range of phi  angle (radians);
  fTclev=tclev;
  fThlev=thlev;

  fNbPart=fNbPart+1;
  fPparent.push_back(fCurParentIdx);
  if (fNbPart>100) {
    warn << "Decay0::particle :WARNING: in event with more than 100 particles: " << fNbPart << newline;
  }

  if (np<1 || np>50 || (np>32 && np<45)) {
    warn << "Decay0::particle :WARNING: unknown particle number: " << np << newline;
  }

  fNpGeant[fNbPart]=np;
  float pmass=GetMass(np);
  float phi=phi1+(phi2-phi1)*GetRandom();

  float ctet1=1.;
  float ctet2=-1.;
  double p;
  float E, ctet;
  if(teta1!=0.) ctet1=cos(teta1);
  if(teta2!=pi) ctet2=cos(teta2);

  ctet=ctet1+(ctet2-ctet1)*GetRandom();
  float stet=sqrt(1.-ctet*ctet);
  E=E1;
  if(E1!=E2) E=E1+(E2-E1)*GetRandom();

  p=sqrt(E*(E+2.*pmass));
  fPmoment[0][fNbPart]=p*stet*cos(phi);
  fPmoment[1][fNbPart]=p*stet*sin(phi);
  fPmoment[2][fNbPart]=p*ctet;
  fTdlev=fTclev;

  if(fThlev>0) fTdlev=fTclev-fThlev/log(2.)*log(GetRandom());

  fPtime[fNbPart]=fTdlev;
  return;
}
///************************************************/
void Decay0::PbAtShell(int KLMenergy)
{
  // Approximation of the deexcitation process in atomic shell
  // of Pb after creation of electron vacation in K, L or M shell.
  // It is supposed electron binding energy on
  // Pb K-shell = 88 keV, on L-shell = 15 keV,
  // on M-shell = 3 keV.
  // The following values of KLMenergy are allowed:
  // 88 (hole in K-shell), 15 (in L-shell) and 3 (in M-shell).
  // VIT, 7.07.1995, 22.10.1995.

  float p;
  int Lhole=0, Mhole=0;
  bool gonext=false;
  if (KLMenergy==88){
    p=100*GetRandom();
    if (p<=22.){
      particle(1,0.085,0.085,0.,pi,0.,twopi,0.,0.);
      Mhole=Mhole+1;
      for (int i=1; i<=Mhole;i++){
        particle(1,0.003,0.003,0.,pi,0.,twopi,0.,0.);
      }
      return;
    }
    else{
      p=100*GetRandom();
      if (p<=96.) particle(1,0.073,0.073,0.,pi,0.,twopi,0.,0.);
      else{
        particle(3,0.058,0.058,0.,pi,0.,twopi,0.,0.);
        Lhole=Lhole+1;
      }
    }
    Lhole=Lhole+1;
    gonext=true;
  }
  if (KLMenergy==15){
    Lhole=1;
    gonext=true;
  }
  if (gonext){
    for (int i=1; i<=Lhole;i++){
      p=100*GetRandom();
      if (p<=40.) particle(1,0.012,0.012,0.,pi,0.,twopi,0.,0.);
      else{
        particle(3,0.009,0.009,0.,pi,0.,twopi,0.,0.);
        Mhole=Mhole+1;
      }
      Mhole=Mhole+1;
    }
    gonext=true;
  }
  if (KLMenergy==3){
    Mhole=1;
    gonext=true;
  }
  if (gonext){
    for (int i=1; i<=Mhole;i++){
      particle(1,0.003,0.003,0.,pi,0.,twopi,0.,0.);
    }
    return;
  }
  return;
}
///************************************************/
float Decay0::fe1_mod()
{
  //probability distribution for energy of first e-/e+ for
  // fModebb=1,2,3,7,10 in the doublebeta model

  float fe1mod=0.;
  if(fE1>fE0) return fe1mod;

  fE2=fE0-fE1;

  float p1=sqrt(fE1*(fE1+2.*GetMass(3)));
  float p2=sqrt(fE2*(fE2+2.*GetMass(3)));

  if (fModebb==1)
     fe1mod=(fE1+GetMass(3))*p1*fermi(fZdbb,fE1)*(fE2+GetMass(3))*p2*fermi(fZdbb,fE2);

  else if (fModebb==2)
     fe1mod=(fE1+GetMass(3))*p1*fermi(fZdbb,fE1)*(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)
            * (fE0-2.*fE1)*(fE0-2.*fE1);

  else if (fModebb==3)
     fe1mod=p1*fermi(fZdbb,fE1)*p2*fermi(fZdbb,fE2)*(2.*p1*p1*p2*p2
            +9.*((fE1+GetMass(3))*(fE2+GetMass(3))+GetMass(3)*GetMass(3))*(p1*p1+p2*p2));

  else if (fModebb==7)
     fe1mod=p1*fermi(fZdbb,fE1)*p2*fermi(fZdbb,fE2)*
            ((fE1+GetMass(3))*(fE2+GetMass(3))+GetMass(3)*GetMass(3))*(p1*p1+p2*p2);

  else if (fModebb==10)
     fe1mod=(fE1+GetMass(3))*p1*fermi(fZdbb,fE1)*pow(fE0-fE1,5);

  return fe1mod;
}
///************************************************/
float Decay0::fe2_mod()
{
  //probability distribution for energy of second e-
  //for fModebb=4,5,6,8,13,14,15,16

  float fe2mod=0.;
  if(fE2>fE0-fE1) return fe2mod;

  float p2=sqrt(fE2*(fE2+2.*GetMass(3)));
  if (fModebb==4)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*pow(fE0-fE1-fE2,5);
  else if (fModebb==5)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*(fE0-fE1-fE2);
  else if (fModebb==6)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*pow(fE0-fE1-fE2,3);
  else if (fModebb==8)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*pow(fE0-fE1-fE2,7)*pow(fE1-fE2,2);
  else if(fModebb==13)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*pow(fE0-fE1-fE2,7);
  else if(fModebb==14)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*pow(fE0-fE1-fE2,2);
  else if(fModebb==15)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*pow(fE0-fE1-fE2,5)*
            (9*pow(fE0-fE1-fE2,2)+21*pow(fE2-fE1,2));
  else if(fModebb==16)
     fe2mod=(fE2+GetMass(3))*p2*fermi(fZdbb,fE2)*pow(fE0-fE1-fE2,5)*pow(fE2-fE1,2);

  return fe2mod;
}
///************************************************/
float Decay0::fermi(const float &Z,const float &E)
{
  //Function fermi calculates the traditional function of Fermi
  //in theory of beta decay to take into account the Coulomb correction
  //to the shape of electron/positron energy spectrum.
  // Z - atomic number of daughter nuclei (>0 for e-, <0 for e+);
  // E - kinetic energy of particle (MeV; E>50 eV).
  // fermif - value of correction factor (without normalization -
  //          constant factors are removed - for MC simulation).
  float dE,alfaz, w,y, g, fermif;
  double p;
  complex<double> carg;

  dE=E;
  if (E<50e-06) dE=50e-06;

  alfaz=Z/137.036;
  w = dE/0.511+1.;
  p = sqrt(w*w-1.);
  y = alfaz*w/p;
  g = sqrt(1.-alfaz*alfaz);

  carg=complex<double>(g,y);
  fermif=pow(p,(2.*g-2.))*exp(pi*y+2.*log(abs(cgamma(carg))));

  return fermif;
}
///************************************************/
void Decay0::beta(float Qbeta, float tcnuc,float thnuc)
{
  // Calculation of the angles and energy of beta particles emitted
  // in the beta decay of nucleus. The decay is considered as allowed.
  // Only Coulomb correction to the shape of energy spectrum is taken into consideration.

  fQbeta=Qbeta;
  fThnuc=thnuc;

  TF1 fbeta ("funbeta",this,&Decay0::funbeta,0,Qbeta,3);
  float em=0.,fm=0., E=0.,fe=0., f=0.;
  int np;

  tgold(50.e-6,Qbeta,fbeta,0.001*Qbeta,2,em,fm);

  do{
    E=50.e-6+(Qbeta-50.e-6)*GetRandom();
    fe=fbeta.Eval(E);
    f=fm*GetRandom();
  }while(f>fe);

  if (fZdtr>=0.) np=3;
  if (fZdtr<0.)  np=2;

  particle(np,E,E,0.,pi,0.,twopi,tcnuc,fThnuc);
  return;
}
///************************************************/
void Decay0::beta1f(float Qbeta,float tcnuc,float thnuc,float c1,float c2,float c3,float c4)
{
  // Calculation of the angles and energy of beta particles emitted
  // in beta decay of nucleus. The decay is considered as forbidden;
  // correction factor to the allowed spectrum shape has a form
  // typical for empirical corrections:
  // cf(e)=(1+c1/w+c2*w+c3*w**2+c4*w**3), w=e/GetMass(3)+1.
  // VIT, 30.07.1992; 15.10.1995; 31.03.2006

  fQbeta=Qbeta;
  fThnuc=thnuc;
  fC1=c1;
  fC2=c2;
  fC3=c3;
  fC4=c4;
  TF1 fbeta1f ("funbeta1f",this,&Decay0::funbeta1f,0,fQbeta,3);
  float em=0.,fm=0., E=0.,fe=0., f=0.;
  int np=0;

  tgold(50.e-6,fQbeta,fbeta1f,0.001*fQbeta,2,em,fm);

  do{
    E=50.e-6+(fQbeta-50.e-6)*GetRandom();
    fe=fbeta1f.Eval(E);
    f=fm*GetRandom();
  }while(f>fe);

  if (fZdtr>=0.) np=3;
  if (fZdtr<0.)  np=2;

  particle(np,E,E,0.,pi,0.,twopi,tcnuc,fThnuc);
  return;
}
///************************************************/
void Decay0::beta1fu(float Qbeta,float tcnuc,float thnuc,float c1,float
    c2,float c3,float c4)
{
  // Calculation of the angles and energy of betas emitted in the decay
  // of nucleus. The decay is considered as 1st-forbidden unique.
  // Its shape is a product of theoretical spectrum shape for allowed decay
  // and the two correction factors:
  // 1. theoretical of BJ'1969     cf1(e)=pnu**2+lamda2*pel**2,
  // where lambda2 is the Coulomb function calculated in BJ'1969,
  // and pel and pnu are impulses of electron and neutrino:
  // pel=sqrt(w**2-1), pnu=(Qbeta-e)/GetMass(3) , w=e/GetMass(3)+1;
  // 2. empirical correction   cf2(e)=(1+c1/w+c2*w+c3*w**2+c4*w**3).
  // Values of the "lambda2" Coulomb function for some Zdtr values from:
  // H.Behrens, J.Janecke, "Numerical tables for beta-decay and electron
  // capture", Berlin, Springer-Verlag, 1969.
  // Values are calculated as product between unscreened lambda2
  // (Table II of BJ'1969) and screened corrections (Table III),
  // and are given for "standard" values of momentum
  // (0.1-50.0 in units of m_e*c).
  // (Log values of these momenta are in array plog69.)

  int i;
  for (i=0 ;i<48;i++){
    fSl[i]=0;
  }
  fQbeta=Qbeta;
  fThnuc=thnuc;
  fC1=c1;
  fC2=c2;
  fC3=c3;
  fC4=c4;
  if (fZdtr==19){ // 39Ar, fQbeta=0.565; 42Ar, Qbeta=0.600
     double Sl[]={2.0929, 1.2337, 1.0747, 1.0234, 0.99977, 0.98728, 0.98024, 0.97624, 0.97445, 0.97377, 0.97406, 0.97549, 0.9757, 0.9754, 0.9754, 0.9756, 0.9760};
     fSlSize=17;
     for (i=0;i<fSlSize;i++){
            fSl[i]=Sl[i];
     }
  }
  else if (fZdtr==20){ // 42K, fQbeta=3.525;
    double Sl[48]={2.2248, 1.2634, 1.0851, 1.0275, 1.0008, 0.98693, 0.97884, 0.97426, 0.97213, 0.97128, 0.97138, 0.97276, 0.9731, 0.9728, 0.9728, 0.9731, 0.9735, 0.9740, 0.9745, 0.9750, 0.9756, 0.9762, 0.9768, 0.9774, 0.9780, 0.9794, 0.9808, 0.9821, 0.9834, 0.9846, 0.9859, 0.9870, 0.9882, 0.9903, 0.9924};
    fSlSize=35;
    for (i=0;i<fSlSize;i++){
      fSl[i]=Sl[i];
    }
  }
  else if (fZdtr==39){ // 90Sr, fQbeta=0.546;
    double Sl[48]={5.6836, 2.0435, 1.3704, 1.1386, 1.0327, 0.97761, 0.94571, 0.92621, 0.91383, 0.90577, 0.89708, 0.89379, 0.89354, 0.89479, 0.89695, 0.89953, 0.90229};
    fSlSize=17;
    for (i=0;i<fSlSize;i++){
      fSl[i]=Sl[i];
    }
  }
  else if (fZdtr==40){ //90Y, fQbeta=2.228;
    double Sl[]={5.8992, 2.0922, 1.3883, 1.1454, 1.0345, 0.97692, 0.94344, 0.92294, 0.90998, 0.90153, 0.89243, 0.88892, 0.88848, 0.88970, 0.89186, 0.89454, 0.89739, 0.90037, 0.90330, 0.90631, 0.90931, 0.91223, 0.91507, 0.9174, 0.9195, 0.9246, 0.9295, 0.9343, 0.9388, 0.9432};
    fSlSize=30;
    for (i=0;i<fSlSize;i++){
      fSl[i]=Sl[i];
    }
  }
  else if (fZdtr==56){ // 137-Cs, Qbeta=0.514 , to level 0.662
    double Sl[]={9.3262, 2.8592, 1.6650, 1.2481, 1.0580, 0.95794, 0.89948, 0.86350, 0.84043, 0.82535, 0.80875, 0.80209, 0.80046, 0.80152, 0.80409, 0.80752, 0.81167};
    fSlSize=17;
    for (i=0;i<fSlSize;i++){
      fSl[i]=Sl[i];
    }
  }

  TF1 fbeta1fu ("funbeta1fu",this,&Decay0::funbeta1fu,50.e-6,fQbeta,3);
  float em=0.,fm=0., E,fe=0.;
  float f;
  int np;

  tgold(50.e-6,fQbeta,fbeta1fu,0.001*fQbeta,2,em,fm);
  do{
    E=50.e-6+(fQbeta-50.e-6)*GetRandom();
    fe=fbeta1fu.Eval(E);
    f=fm*GetRandom();
  }while(f>fe);
  if(fZdtr>=0.) np=3;
  if(fZdtr<0.)  np=2;
  particle(np,E,E,0.,pi,0.,twopi,tcnuc,fThnuc);
  return;
}
///************************************************/
void Decay0::beta2f(float Qbeta,float tcnuc,float thnuc,int kf,float c1,float c2,float c3,float c4)
{
  // Calculation of the angles and energy of beta particles emitted
  // in beta decay of nucleus. The decay is considered as forbidden;
  // correction factor to the allowed spectrum shape has one of a form,
  // typical for unique k-forbidden spectra:
  // k=1: cf(e)=pel**2+c1*       pnu**2,
  // k=2: cf(e)=pel**4+c1*pel**2*pnu**2+c2*       pnu**4,
  // k=3: cf(e)=pel**6+c1*pel**4*pnu**2+c2*pel**2*pnu**4+c3*       pnu**6,
  // k=4: cf(e)=pel**8+c1*pel**6*pnu**2+c2*pel**4*pnu**4+c3*pel**2*pnu**6+c4*pnu**8,
  // where pel and pnu are impulses of electron and neutrino:
  // pel=sqrt(w**2-1), pnu=(Qbeta-e)/GetMass(3) , w=e/GetMass(3)+1.
  // VIT, 30.07.1992; 15.10.1995; 31.03.2006.

  fQbeta=Qbeta;
  fThnuc=thnuc;
  fKf=kf;
  fC1=c1;
  fC2=c2;
  fC3=c3;
  fC4=c4;
  TF1 fbeta2f ("funbeta2f",this,&Decay0::funbeta2f,0,fQbeta,3);
  float em=0.,fm=0., E=0.,fe=0., f=0.;
  int np=0;
  tgold(50.e-6,fQbeta,fbeta2f,0.001*fQbeta,2,em,fm);
  do{
     E=50.e-6+(fQbeta-50.e-6)*GetRandom();
     fe=fbeta2f.Eval(E);
     f=fm*GetRandom();
  }while(f>fe);
  if (fZdtr>=0.) np=3;
  if (fZdtr<0.)  np=2;
  particle(np,E,E,0.,pi,0.,twopi,tcnuc,fThnuc);
  return;
}
///************************************************/
void Decay0::nucltransK(float Egamma,float Ebinde,float conve,float convp)
{
  // nucltransK chooses one of the three concurrent processes by which
  // the transition from one nuclear state to another is occurred:
  // gamma-ray emission, internal conversion and internal pair creation.
  // Conversion electrons are emitted only with one fixed energy
  // (usually with Egamma-E(K)_binding_energy).
  // Ebinde - binding energy of electron (MeV);
  // conve  - internal electron conversion coefficient [=Nelectron/Ngamma];
  // convp  - pair conversion coefficient [=Npair/Ngamma];
  float p=(1.+conve+convp)*GetRandom();

  if (p<=1) {
    particle(1,Egamma,Egamma,0,pi,0,twopi,fTclev,fThlev);//gamma
  }
  else if (p<=1+conve){
    particle(3,Egamma-Ebinde,Egamma-Ebinde,0.,pi,0.,twopi,fTclev,fThlev);//electron
    particle(1,Ebinde,Ebinde,0.,pi,0.,twopi,0.,0.);//gamma
  }
  else pair(Egamma-2.*GetMass(3));//e+e- pair
  return;
}
///************************************************/
void Decay0::nucltransKL(float Egamma,float EbindeK,float conveK,float EbindeL,float conveL,float convp)
{
  // ucltransKL chooses one of the three concurrent processes by which
  // the transition from one nuclear state to another is occurred:
  // gamma-ray emission, internal conversion and internal pair creation.
  // Conversion electrons are emitted with two fixed energies:
  // (Egamma-E(K)_binding_energy and Egamma-E(L)_binding_energy).
  // VIT, 5.07.1995.
  float p=(1.+conveK+conveL+convp)*GetRandom();

  if (p<=1.) particle(1,Egamma,Egamma,0.,pi,0.,twopi,fTclev,fThlev);//gamma
  else if (p<=1.+conveK){
     particle(3,Egamma-EbindeK,Egamma-EbindeK,0.,pi,0.,twopi,fTclev,fThlev);//electron
     particle(1,EbindeK,EbindeK,0.,pi,0.,twopi,0,0);//gamma
  }
  else if (p<=1.+conveK+conveL){
     particle(3,Egamma-EbindeL,Egamma-EbindeL,0.,pi,0.,twopi,fTclev,fThlev);//electron
     particle(1,EbindeL,EbindeL,0.,pi,0.,twopi,0,0);//gamma
  }
  else pair(Egamma-2.*GetMass(3));
  return;
}
///************************************************/
void Decay0::nucltransKLM(float Egamma,float EbindeK,float conveK,float EbindeL,float conveL,float EbindeM,float conveM, float convp)
{
  // nucltransKLM chooses one of the three concurrent processes by which
  // the transition from one nuclear state to another is occurred:
  // gamma-ray emission, internal conversion and internal pair creation.
  // Conversion electrons are emitted with three fixed energies:
  // Egamma-E(K)_binding_energy, fEgamma-E(L)_binding_energy and
  // Egamma-E(M)_binding_energy.
  // EbindeK - binding energy of electron (MeV) on K-shell;
  // conveK  - internal conversion coeff [=Nelectron/Ngamma] from K-shell;
  // EbindeL - binding energy of electron (MeV) on L-shell;
  // conveL  - internal conversion coeff [=Nelectron/Ngamma] from L-shell;
  // EbindeM - binding energy of electron (MeV) on M-shell;
  // conveM  - internal conversion coeff [=Nelectron/Ngamma] from M-shell;
  // convp   - pair conversion coefficient [=Npair/Ngamma];

  float p=(1.+conveK+conveL+conveM+convp)*GetRandom();
  if (p<=1.) particle(1,Egamma,Egamma,0.,pi,0.,twopi,fTclev,fThlev);//gamma
  else if (p<=1.+conveK){
     particle(3,Egamma-EbindeK,Egamma-EbindeK,0.,pi,0.,twopi,fTclev,fThlev);//electron
     particle(1,EbindeK,EbindeK,0.,pi,0.,twopi,0,0);//gamma
  }
  else if (p<=1.+conveK+conveL){
     particle(3,Egamma-EbindeL,Egamma-EbindeL,0.,pi,0.,twopi,fTclev,fThlev);//electron
     particle(1,EbindeL,EbindeL,0.,pi,0.,twopi,0,0);//gamma
  }
  else if (p<=1.+conveK+conveL+conveM){
     particle(3,Egamma-EbindeM,Egamma-EbindeM,0.,pi,0.,twopi,fTclev,fThlev);//electron
     particle(1,EbindeM,EbindeM,0.,pi,0.,twopi,0,0);//gamma
  }
  else pair(Egamma-2.*GetMass(3));

  return;
}
///************************************************/
void Decay0::nucltransKLM_Pb(float Egamma,float EbindeK,float conveK,float EbindeL,float conveL,float EbindeM,float conveM, float convp)
{
  // The same as nucltransKLM but two X rays are emitted after K conversion
  // in deexcitation of 208-Pb in decay 208Tl->208Pb.
  // VIT, 4.02.2009.
  float p, p1;
  p=(1.+conveK+conveL+conveM+convp)*GetRandom();
  if (p<=1.)
     particle(1,Egamma,Egamma,0.,pi,0.,twopi,fTclev,fThlev);//gamma
  else if (p<=1.+conveK){
     particle(3,Egamma-EbindeK,Egamma-EbindeK,0.,pi,0.,twopi,fTclev,fThlev);//electron
     p1=100.*GetRandom();
     if (p1<=73.9){
        particle(1,0.074,0.074,0.,pi,0.,twopi,0,0);//gamma
        particle(1,0.014,0.014,0.,pi,0.,twopi,0,0);//gamma
     }
     else{
        particle(1,0.085,0.085,0.,pi,0.,twopi,0,0);//gamma
        particle(1,0.003,0.003,0.,pi,0.,twopi,0,0);//gamma
        // in 4.8% few low energy particles are emitted; they are neglected
     }
  }
  else if (p<=1.+conveK+conveL){
     particle(3,Egamma-EbindeL,Egamma-EbindeL,0.,pi,0.,twopi,fTclev,fThlev);//electron
     particle(1,EbindeL,EbindeL,0.,pi,0.,twopi,0,0);//gamma
  }
  else if (p<=1.+conveK+conveL+conveM){
     particle(3,Egamma-EbindeM,Egamma-EbindeM,0.,pi,0.,twopi,fTclev,fThlev);//electron
     particle(1,EbindeM,EbindeM,0.,pi,0.,twopi,0,0);//gamma
  }
  else pair(Egamma-2.*GetMass(3));

  return;
}
///************************************************/
void Decay0::pair(float Epair)
{
  // Generation of e+e- pair in zero-approximation for INTERNAL pair creation:
  //  1) energy of e+ is equal to the energy of e-;
  //  2) e+ and e- are emitted in the same direction.

  float phi=twopi*GetRandom();
  float ctet=-1.+2.*GetRandom();
  float teta=acos(ctet);
  float E=0.5*Epair;
  particle(2,E,E,teta,teta,phi,phi,fTclev,fThlev);
  particle(3,E,E,teta,teta,phi,phi,0.,0.);
  return;
}
///************************************************/
void Decay0::tgold(float a,float b,TF1 &fb,float eps,int minmax,float&xextr,float &fextr)
{
  // tgold determines maximum or minimum of the function f(x) in
  // the interval [a,b] by the gold section method.
  float qc=0.61803395;
  float xR=a+(b-a)*qc;
  float xL=b-(b-a)*qc;
  fb.SetParameters(fQbeta,GetMass(3),fZdtr);
  float yR=fb.Eval(xR);
  float yL=fb.Eval(xL);

  do{
    if (minmax==1){
      if (yR<yL){
        a=xL;
        xL=xR;
        yL=yR;
        xR=a+(b-a)*qc;
        yR=fb.Eval(xR);
      }
    }
    else{
      if (yR>yL){
        a=xL;
        xL=xR;
        yL=yR;
        xR=a+(b-a)*qc;
        yR=fb.Eval(xR);
      }
    }
    b=xR;
    xR=xL;
    yR=yL;
    xL=b-(b-a)*qc;
    yL=fb.Eval(xL);
  }while(b-a>eps);

  xextr=0.5*(a+b);
  fextr=fb.Eval(xextr);
}
///************************************************/
complex<double> Decay0::cgamma(complex<double> z)
{
  complex<double> g,z0,z1;
  double x0=0.,q1=0.,q2=0.,x=0.,y=0.,th=0.,th1=0.,th2=0.;
  double g0=0.,gr=0.,gi=0.,gr1=0.,gi1=0.;
  double na=0.,t=0.,x1=0.,sr=0.,si=0.;
  int j=0,k=0;

  static double a[] = { 8.333333333333333e-02, -2.777777777777778e-03,
                        7.936507936507937e-04, -5.952380952380952e-04,
                        8.417508417508418e-04, -1.917526917526918e-03,
                        6.410256410256410e-03, -2.955065359477124e-02,
                        1.796443723688307e-01, -1.39243221690590};
    x = real(z);
    y = imag(z);
    if (x > 171) return complex<double>(1e308,0);
    if ((y == 0.0) && (x == (int)x) && (x <= 0.0))  return complex<double>(1e308,0);
    else if (x < 0.0) {
      x1 = x;
      x = -x;
      y = -y;
    }
    x0 = x;
    if (x <= 7.0) {
      na = (int)(7.0-x);
      x0 = x+na;
    }
    q1 = sqrt(x0*x0+y*y);
    th = atan(y/x0);
    gr = (x0-0.5)*log(q1)-th*y-x0+0.5*log(twopi);
    gi = th*(x0-0.5)+y*log(q1)-y;
    for (k=0;k<10;k++){
      t = pow(q1,-1.0-2.0*k);
      gr += (a[k]*t*cos((2.0*k+1.0)*th));
      gi -= (a[k]*t*sin((2.0*k+1.0)*th));
    }
    if (x <= 7.0) {
      gr1 = 0.0;
      gi1 = 0.0;
      for (j=0;j<na;j++) {
        gr1 += (0.5*log((x+j)*(x+j)+y*y));
        gi1 += atan(y/(x+j));
      }
      gr -= gr1;
      gi -= gi1;
    }
    if (x1 < 0.0) {
      q1 = sqrt(x*x+y*y);
      th1 = atan(y/x);
      sr = -sin(pi*x)*cosh(pi*y);
      si = -cos(pi*x)*sinh(pi*y);
      q2 = sqrt(sr*sr+si*si);
      th2 = atan(si/sr);
      if (sr < 0.0) th2 +=pi;
      gr = log(pi/(q1*q2))-gr;
      gi = -th1-th2-gi;
    }
    g0 = exp(gr);
    gr = g0*cos(gi);
    gi = g0*sin(gi);
    g = complex<double>(gr,gi);
    return g;
}
///************************************************/
float Decay0::divdif (double xtab[50], double xval)
{
  //Function Interpolation translated from cernlib function E105
  int i=0,j=0;
  double difval=0.;
  int N=fSlSize;
  if(N<2 ) return difval;
  int arr[]={2,10,fSlSize-1};
  int M=TMath::MinElement(3,arr);
  int mplus=M+1;
  int ix=0, mid;
  int iy=N;
  // Tabular interpolation using symmetrically placed argument points.
  if (xtab[0]>xtab[N-1]){
     do{
        mid=(ix+iy)/2;
        if (xval<=xtab[mid]) ix=mid;
        else iy=mid;
     }while ((iy-ix)>1);
  }
  else{
     do{
       mid=(ix+iy)/2;
       if (xval>=xtab[mid]) ix=mid;
       else iy=mid;
     }while((iy-ix)>1);
  }
  //Copy reordered interpolation points into T[],D[], setting "extra" to true
  //if M+2 points to be used.
  int npts=M+2-M%2;
  int ip=0;
  int l=0, isub;
  double T[50],D[50];
  for (int ii=0;ii<N;ii++) {T[ii]=0; D[ii]=0;}
  bool dothat=false;
  do {
     if (dothat){
        l=-l;
        if(l>=0) l=l+1;
     }
     isub=ix+l;
     if (0<=isub && isub<=N){
        T[ip]=xtab[isub];
        D[ip]=fSl[isub];
        ip++;
     }
     else{
        npts=mplus;
     }
     dothat=true;
  }while (ip<npts);
  bool extra=(npts!=mplus);
  bool next=false;
  //Replace D by the leading diagonal of a divided difference table,
  //supplemented by an extra line if extra=true
  for (int l=1;l<=M;l++){
      if (extra){
         isub=mplus-l;
         D[M+1]=(D[M+1]-D[M-1])/(T[M+1]-T[isub-1]);
         next=true;
      }
      if (!extra||next){
         i=mplus-1;
         for (int j=l;j<=M;j++){
             isub=i-l;
             D[i]=(D[i]-D[i-1])/(T[i]-T[isub]);
             i--;
         }
      }
  }
  // Evaluate the Newton interpolation formula at xval,
  // averaging two values of last difference if extra=true
  double sum=D[mplus-1];
  if(extra) sum=0.5*(sum+D[M+1]);

  j=M-1;
  for (int l=1;l<=M;l++){
      sum=D[j]+(xval-T[j])*sum;
      j--;
  }
  difval=sum;
  return difval;
}
///************************************************/
float Decay0::GetRandom()
{
  return G4UniformRand();
}
///************************************************/
G4double Decay0::GetMass(int gnp)
{
 int pdg=0;
 if      (gnp==1)  pdg=22;         // gamma
 else if (gnp==2)  pdg=-11;        // positron
 else if (gnp==3)  pdg=11;         // electron
 else if (gnp==47) pdg=1000020040; // alpha
 fPartDef=G4ParticleTable::GetParticleTable()->FindParticle(pdg);
 double mass=fPartDef->GetPDGMass();
 return mass;
}

} // namespace RAT

#pragma GCC diagnostic pop

