/**
 * \class RAT::Decay0
 * \brief The Decay0 Generator for Initial Kinematics in Alpha, Beta and
 *        Double Beta Decays
 *
 * \author V. Tretyak and the SNO+ Collaboration
 *
 * History:
 *
 *   The Decay0 generator was originally written by V. Tretyak in FORTRAN;
 *   see Reference [1]. This code was ported to C++ by members of the SNO+
 *   Collaboration [2] for internal use. This C++ implementation has been made
 *   available in RAT-PAC by agreement of the SNO+ Collaboration.
 *
 *   See the COPYING and COPYING.decay0 files, which should be included with
 *   this source code distribution, for license details.
 *
 *   [1] O.A.Ponkratenko, V.I.Tretyak, Yu.G.Zdesenko, Phys. At. Nucl. 63
 *   (2000) 1282, (nucl-ex/0104018).
 *   [2] http://snoplus.phy.queensu.ca/
 *
 * Units:
 *
 *   Energy    MeV
 *   Momentum  MeV/c
 *   Time      seconds
 *
 * List of the isotopes included in the generator:
 *
 *   Ac228   Ar39   Ar42   As79  Be11   Bi207  Bi208  Bi210  Bi212  Bi214
 *     C10    C11    C14   Ca48  Cd113   Co60  Cs136  Cs137  Eu147  Eu152
 *   Eu154  Gd146    He6  Hf182   I126   I133   I134   I135    K40    K42
 *    Kr81   Kr85   Mn54   Na22    P32 Pa234m  Pb210  Pb211  Pb212  Pb214
 *   Ra228   Rb87  Rh106  Sb125  Sb126  Sb133   Sr90  Ta182  Te133 Te133m
 *   Te134  Th234  Tl207  Tl208 Xe129m Xe131m  Xe133  Xe135    Y88    Y90
 *    Zn65   Zr96
 *
 * Usage:
 *
 *   In the macro, define:
 *
 *       /generator/add combo decay0:[POSITION]
 *
 *   where POSITION is a generator generator (point, fill, ...).
 *
 * Examples:
 *
 *   To simulate the double-beta decay events:
 *
 *     /generator/vtx/set 2beta [ISOTOPE] [LEVEL] [MODE] [LoELim] [HiELim]
 *
 *   To simulate background events:
 *
 *     /generator/vtx/set backg [ISOTOPE]
 *
 * Revision history:
 *   2017/08/29 - A. Mastbaum: Import from SNO+ RAT
*/

#ifndef __RAT_Decay0__
#define __RAT_Decay0__

#include "Randomize.hh"
#include <iostream>
#include <complex>
#include "RAT/DB.hh"
#include "TF1.h"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"

/***************
 GEANT number for particle identification:
 1 - gamma   2 - positron   3 - electron   47 - alpha
**************/

//using namespace std;
using std::string;
using std::vector;
using std::complex;

namespace RAT {

class Decay0  {
  public:
    Decay0();
    Decay0(const std::string isotope,const int level, const int mode, const float lE, const float hE);
    Decay0(const std::string isotope);
    ~Decay0();

    void GenBBTest();
    void GenBackgTest();
    void GenBBDeex();
    void GenEvent();
    float GetRandom();
    G4ParticleDefinition *fPartDef;
    double GetMass(int pdg); // get mass of particle from GEANT

    ///************************************************/
    /// sampling the energies and angles of electrons in various
    /// modes of doublebeta decay without Primakoff-Rosen approximation.
    void bb();
    /// Generation of isotropic emission of particle in the range of
    /// energies and angles
    void particle(int np, float E1, float E2, float teta1, float teta2,
                  float phi1, float phi2, float tclev, float thlev);
    ///Generation of e+e- pair in zero-approximation to real subroutine for
    /// internal pair creation: 1) energy of e+ is equal to the energy of e-;
    /// 2) e+ and e- are emitted in the same direction
    void pair(float Epair);
    /// probability distribution for energy of e-/e+ in doublebeta decay
    float fe1_mod();
    float fe2_mod();
    /// Function to calculate the traditional function of Fermi in
    /// theory of beta decay to take into account the Coulomb correction
    float fermi(const float &Z,const float &E);
    /// simulation of the angles and energy of beta particles
    /// emitted in beta decay of nucleus.
    void beta(float Qbeta,float tcnuc,float thnuc);
    void beta1f(float Qbeta,float tcnuc,float thnuc,float c1,float c2,float c3,float c4);
    void beta1fu(float Qbeta,float tcnuc, float thnuc,float c1,float c2,float c3,float c4);
    void beta2f(float Qbeta,float tcnuc,float thnuc,int kf, float c1,float c2,float c3,float c4);
    /// determines maximum or minimum of the function f(x) in the interval [a,b]
    /// by the gold section method.
    void tgold(float a,float b,TF1 &f,float eps, int nmin,float&xextt,float &fextr);
    ///************************************************/

    ///-----------------------------------------------
    /// functions describe the de-excitation process after 2b-decay
    /// to ground and excited 0+ and 2+ levels
    /// conditions still hard-coded - work in progress
    void Ti48low();          void Fe58low();
    void Se76low();          void Ge74low();
    void Kr82low();          void Mo96low();
    void Zr92low();         void Ru100low();
    void Pd106low();        void Sn116low();
    void Cd112low();        void Te124low();
    void Xe130low();        void Ba136low();
    void Sm148low();        void Sm150low();

    ///functions for decays (alpha/beta)
    void As79();    void At214();    void Ac228();
    void Bi207();   void Bi210();    void Bi212();
    void Bi214();    void Co60();    void Cs136();
    void Eu147();   void Eu152();    void Eu154();
    void Gd146();    void I126();     void I133();
    void I134();     void I135();      void K40();
    void K42();    void Pa234m();    void Pb211();
    void Pb212();   void Pb214();    void Po214();
    void Rn218();   void Ra222();    void Ra228();
    void Rh106();   void Sb125();    void Sb126();
    void Sb133();    void Sc48();    void Ta182();
    void Te133();  void Te133m();    void Te134();
    void Th234();   void Tl208();    void Xe133();
    void Xe135();     void Y88();     void Zn65();
    void Nb96();    void Be11();

    /// describes in some approximation the deexcitation process in atomic
    /// shell of Pb after creation of electron vacation in K, L or M shell.
    void PbAtShell(int KLMenergy);
    ///-----------------------------------------------

    //************************************************/
    // chooses one of the three concurrent processes by which the transition
    // from one nuclear state to another occurs:
    // gamma-ray emission, internal conversion and internal pair creation.
    // Conversion electrons are emitted only with one fixed energy
    // (usually with Egamma-E(K)_binding_energy)
    void nucltransK(float Egamma,float Ebinde,float conve,float convp);
    void nucltransKL(float Egamma,float EbindeK,float conveK,float EbindeL,float conveL,float convp);
    void nucltransKLM(float Egamma,float EbindeK,float conveK,float EbindeL,float conveL,float EbindeM,float conveM, float convp);
    void nucltransKLM_Pb(float Egamma,float EbindeK,float conveK,float EbindeL,float conveL,float EbindeM,float conveM, float convp);
    ///************************************************/
    Double_t funbeta(Double_t *x, Double_t *par);
    Double_t funbeta1fu(Double_t *x, Double_t *par);
    Double_t funbeta1f(Double_t *x, Double_t *par);
    Double_t funbeta2f(Double_t *x, Double_t *par);
    //************************************************/
    //cernlib functions:
    complex<double> cgamma(complex<double> z);
    float divdif (double xtab[50], double xval);
    ///************************************************/

    inline int    GetNbPart() {return fNbPart;};
    inline double GetPmoment(int i, int j){return fPmoment[i][j]; };
    inline double GetPtime(int i) {return fPtime[i]; };
    inline int    GetNpGeant (int i) {return fNpGeant[i];};

    void SetCutoffWindow(double time) { fCutoffWindow = time;};
    double GetCutoffWindow() {return fCutoffWindow;};

    void SetTimeCutoff(bool status) {fHasTimeCutoff = status;}
    bool GetTimeCutoff( ) {return fHasTimeCutoff;};

    void SetAlphaCut(bool status) { fHasAlphaCut = status;}
    bool GetAlphaCut() {return fHasAlphaCut;}

    inline unsigned int GetParentIdx(unsigned int i) {return fPparent.at(i);}

private:
    double fCutoffWindow; //Time window to restrict coincidence backgrounds
    bool fHasTimeCutoff;  // Flag to indicate if there is a time cutoff for coincidences (-timecut)
    bool fHasAlphaCut;    // Flag to indicate if the first alpha should be cut out (-pure)
    string fIsotope;  // parent isotope
    int fLevel;       // daughter energy level
    int fMode;        // decay mode
    int fModebb;      // decay mode number inside code  (fMode!=fModebb for few modes)
    int fZdbb;        // atomic number of daughter nucleus (Z>0 for b-b- and Z<0 for b+b+ and eb+ processes);
    int fZdtr;        //  atomic number of daughter nucleus (Zdtr>0 for e- and  Zdtr<0 for e+ particles);
    float fEbindeK;   // binding energy of electron on K-shell (MeV)
    float fEbindeL;   // binding energy of electron on L-shell (MeV)
    float fEbindeM;   // binding energy of electron on M-shell (MeV)
    float fEbindeK2;  // binding energy of electron on K-shell (MeV)  (second decay)
    float fEbindeL2;  // binding energy of electron on L-shell (MeV)  (second decay)
    float fEbindeM2;  // binding energy of electron on M-shell (MeV)  (second decay)
    float fLoE, fHiE; // limit for spectrum
    float fTdlev;     //time of decay of level (sec);
    float fTdnuc;     //time of decay of nucleus (sec);
    float fTclev;     //time of creation of level from which particle will be emitted (sec);
    float fThlev;     //level halflife (sec).
    float fThnuc;     //nucleus halflife (sec);
    float fEgamma;    //gamma-ray energy (MeV) [=difference in state energies];
    float fTevst;     //time of event's start (sec);
    float fQbb;       //energy release in double beta process: difference between masses of parent and daughter atoms (MeV);
    float fQbeta;     //beta energy endpoint (MeV; Qbeta>50 eV);
    float fEK;        //binding energy of electron on K shell of parent atom (MeV)
    int fStartbb;     //must be 0 for first call of bb function for a given mode

    float  fEbb1, fEbb2;//left and right energy limits for sum of energies of emitted e-/e+;
                        //other events will be thrown away

    /// members used to collect information from data base file
    DBLinkPtr fLdecay;
    vector<int>    fLevelE;
    vector<int>    fTrans;
    vector<int>    fDaughterZ;
    vector<double> fHLifeTime;
    vector<double>  fProbDecay;
    vector<double>  fProbBeta;
    vector<double>  fEndPoint;
    vector<double>  fProbAlpha;
    vector<double>  fEnAlpha;
    vector<double>  fProbEC;
    vector<double>  fEnGamma;
    vector<double>  fShCorrFactor;

    double fSpthe1[4300],fSpthe2[4300], fSpmax, fFe2m;
    float fE0;     // possible amount of energy released
    float fE1, fE2;// energy of first and second beta
   public:
    double fSl[48];
    int fSlSize;
    float fC1,fC2,fC3,fC4;   // shape correction factors
    float fKf;               // degree of forbiddeness for unique spectra
    int fNbPart;             // current number of last particle in event;
    double fPmoment[3][100]; // x,y,z components of particle momentum (MeV);
    double fPtime[100];      // time shift from previous particle time
    int fNpGeant[100];       // GEANT number for particle identification see above (line 23)

    unsigned int fCurParentIdx; // Index of the current parent for each particle
    vector<unsigned int> fPparent;       // Index of the parent for each particle.

/************************************************/
double operator() (double *x, double *par)
{
  // to use with GaussLegendreIntegrator
  // function implementation using class data members
  double fe1mod=0.;
  double xx ;//e2

  xx = x[0];//e2
  //   double yy = x[1];//fE1
  par[0]=fE1;
  par[1]=fE0;
  par[2]=GetMass(3);
  par[3]=fZdbb;

  if (fModebb==4 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
             pow(par[1]-par[0]-xx,5);

  if (fModebb==5 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
            (par[1]-par[0]-xx);

  if (fModebb==6 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
             pow(par[1]-par[0]-xx,3);

  if (fModebb==8 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
             pow(par[1]-par[0]-xx,7)* pow(par[0]-xx,2);

  if (fModebb==13 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
             pow(par[1]-par[0]-xx,7);
  if (fModebb==14 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
             pow(par[1]-par[0]-xx,2);

  if (fModebb==15 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
             pow(par[1]-par[0]-xx,5)*(9*pow(par[1]-par[0]-xx,2)+21*pow(xx-par[0],2));

  if (fModebb==16 && xx<(par[1]-par[0]))
     fe1mod=(par[0]+par[2])*sqrt(par[0]*(par[0]+2.*par[2]))*fermi(par[3],par[0])*
            (xx+par[2])*sqrt(xx*(xx+2.*par[2]))*fermi(par[3],xx)*
             pow(par[1]-par[0]-xx,5)*pow(xx-par[0],2);

  return fe1mod;
}
/************************************************/
};
} // namespace RAT

#endif
