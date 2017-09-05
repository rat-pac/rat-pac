// VertexGen_Decay0.cc
// See notes in Decay0.hh, COPYING.decay0
#include <RAT/VertexGen_Decay0.hh>
#include <RAT/Log.hh>
#include <RAT/PrimaryVertexInformation.hh>
#include <G4NistManager.hh>
#include <G4IonTable.hh>
#include <G4Ions.hh>

#include <Randomize.hh>
#include <G4Event.hh>
#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4ThreeVector.hh>
#include <G4UnitsTable.hh>

#include <sstream>

using std::ostringstream;
using CLHEP::MeV;
using CLHEP::s;
using CLHEP::ns;

#define G4std  std
namespace RAT {

VertexGen_Decay0::VertexGen_Decay0(const char *arg_dbname)
  : GLG4VertexGen(arg_dbname), fHasTimeCutoff(false), fHasAlphaCut(false),fDecay0(0)
{
  fLoE=-1;
  fHiE=-1;
  fLdecay=0;
}
///-------------------------------------------------------------------------
VertexGen_Decay0::~VertexGen_Decay0()
{
}
///-------------------------------------------------------------------------
void VertexGen_Decay0::GeneratePrimaryVertex(G4Event *event, G4ThreeVector &dx,
      G4double dt)
{
  G4int      nParticles  = 0;
  G4String   particleName;
  G4double   px, py, pz, time;
  G4int      partCode;
  if (fType=="2beta"){  //to simulate double beta
     fDecay0->GenBBDeex();
  }
  else if(fType=="backg"){ // to simulate the background events
    fDecay0->GenEvent();
  }

  // Get the parent information - could be two if it is a chain
  G4ParticleTable* theParticleTable = G4ParticleTable::GetParticleTable();
  G4IonTable* theIonTable = G4IonTable::GetIonTable();

  G4int A1, A2, Z1, Z2;
  this->GetParentAZ(A1,Z1,A2,Z2);
#ifdef RATDEBUG
  info << "VertexGen_Decay0::GeneratePrimaryVertex : Received parents [Z,A] :: 1=[" << Z1 << ","
      << A1 << "] :: 2=[" << Z2 << "," << A2 << "]" << endl;
#endif
  G4ParticleDefinition* particleDef1 = theIonTable->GetIon(Z1, A1, 0);
  // Add the parent isotope information (stationary, no excitation)
  G4PrimaryParticle *parent1 = new G4PrimaryParticle(particleDef1,0,0,0);
  G4PrimaryParticle *parent2 = NULL;
  if(A2>0){
    G4ParticleDefinition* particleDef2 = theIonTable->GetIon(Z2, A2, 0);
    // Add the parent isotope information (stationary, no excitation)
    parent2 = new G4PrimaryParticle(particleDef2,0,0,0);
  }

  // We need this to look up the particles
  nParticles=fDecay0->GetNbPart();

#ifdef RATDEBUG
  info << "VertexGen_Decay0::GeneratePrimaryVertex : Got " << nParticles << " particles." << endl;
#endif
  for (G4int j=0;j<nParticles;j++){
      partCode=fDecay0->GetNpGeant(j+1);
      particleName = fCodeToName(partCode);
      px=fDecay0->GetPmoment(0,j+1);
      py=fDecay0->GetPmoment(1,j+1);
      pz=fDecay0->GetPmoment(2,j+1);
      time=fDecay0->GetPtime(j+1);

      // Create primary particle
      G4PrimaryParticle* particle = new G4PrimaryParticle(theParticleTable->FindParticle(particleName),
                        px*MeV,py*MeV,pz*MeV);
      // Don't set relative time of particle as each particle has its own vertex and time is set there (don't do it twice!)
      particle->SetProperTime(0);

      // Generate the vertex with the creation time and position
      // of this particular particle
      G4PrimaryVertex* vertex = new G4PrimaryVertex (dx,time*s+dt);

      // Add particle to vertex
      vertex->SetPrimary(particle);

      // Set the parent for this vertex
      PrimaryVertexInformation *vertinfo = new PrimaryVertexInformation();
      // N. Barros -- The logic to assign the parent was flawed. If the first parent
      //              produces a delayed primary, this primary was going to be
      //              assigned to the second parent. It is better to modify the underlying
      //              Decay0 class to contain a reference to which parent each primary belongs
      //              I'm leaving within the ifdef a sample of the code that was used before, for comparison
#ifdef RATDEBUG
      int val = 0;
      if(time>0 && A2>0)
        val = 1;
      else
        val = 0;
      info << "VertexGen_Decay0::GeneratePrimaryVertex : Particle " << j << " has parent "
          << fDecay0->GetParentIdx(j) << " (old eval :" << val << ")" << endl;
#endif
      if (fDecay0->GetParentIdx(j) == 1) {
        vertinfo->AddNewParentParticle(parent2);
      } else {
        vertinfo->AddNewParentParticle(parent1);
      }
      vertex->SetUserInformation(vertinfo);


      // Add vertex to event for each particle
      event->AddPrimaryVertex(vertex);
  }
}
///-------------------------------------------------------------------------
G4String VertexGen_Decay0::fCodeToName(G4int code)
{
  if (code == 1){
     return "gamma";
  }
  else if (code == 2){
     return "e+";
  }
  else if (code == 3){
     return "e-";
  }
  else if (code == 47){
     return "alpha";
  }
  else{
      std::ostringstream msg;
      msg << "VertexGen_Decay0: Unknown particle code "  <<  code;
     Log::Die(msg.str());
     return " ";
  }
}
///-------------------------------------------------------------------------
void VertexGen_Decay0::SetState(G4String newValues)
{
  // State setting is the input file to be used
  if (newValues.length() == 0) {
     // print help and current state
     info << "Format of argument to VertexGen_Decay0::SetState: double beta\n";
     info << " /generator/vtx/set 2beta [ISOTOPE] [LEVEL] [MODE] [LoELim] [HiELim]\n"
          << " ISOTOPE:\n";
     info << "   Ca48   Ni58   Zn64   Zn70   Ge76   Se74   Se82   Sr84\n"
          << "   Zr94   Zr96   Mo92  Mo100   Ru96  Ru104 \n"
          << "  Cd106  Cd108  Cd114  Cd116  Sn112  Sn122  Sn124\n"
          << "  Te120  Te128  Te130  Xe136  Ce136  Ce138  Ce142\n"
          << "  Nd148  Nd150   W180   W186  Bi214  Pb214  Po218  Rn222\n***\n";
     info << " Energy LEVEL 0,1,2...see: Decay0.ratdb\n"<<"***\n";
     info << " bb-decy MODE:\n"
          << "      1. 0nubb(mn) 0+ -> 0+     {2n}\n"
          << "      2. 0nubb(rc) 0+ -> 0+     {2n}\n"
          << "      3. 0nubb(rc) 0+ -> 0+, 2+ {N*}\n"
          << "      4. 2nubb     0+ -> 0+     {2n}\n"
          << "      5. 0nubbM1   0+ -> 0+     {2n}\n"
          << "      6. 0nubbM2   0+ -> 0+     (2n}\n"
          << "      7. 0nubbM3   0+ -> 0+     {2n}\n"
          << "      8. 0nubbM7   0+ -> 0+     {2n}\n"
          << "      9. 0nubb(rc) 0+ -> 2+     {2n}\n"
          << "     10. 2nubb     0+ -> 2+     {2n}, {N*}\n"
          << "     11. 0nuKb+    0+ -> 0+, 2+\n"
          << "     12. 2nuKb+    0+ -> 0+, 2+\n"
          << "     13. 0nu2K     0+ -> 0+, 2+\n"
          << "     14. 2nu2K     0+ -> 0+, 2+\n"
          << "     15. 2nubb     0+ -> 0+ with bosonic neutrinos\n"
          << "     16. 2nubb     0+ -> 2+ with bosonic neutrinos\n"
          << "\n   5-8: Majoron(s) with spectral index SI:\n"
          << "     SI=1 - old M of Gelmini-Roncadelli\n"
          << "     SI=2 - bulk M of Mohapatra\n"
          << "     SI=3 - double M, vector M, charged M\n"
          << "     SI=7\n";

     info << "Format of argument to VertexGen_Decay0::SetState: background and sources study:\n"
          << " /generator/vtx/set backg [ISOTOPE] \n"
          << " ISOTOPE:\n"<<"***\n";
     info << "Ac228        Ar39   Ar42        As79  Bi207  Bi208  Bi210 \n"
          << "Bi212     BiPo212  Bi214     BiPo214    C14   Ca48  Cd113 \n"
          << " Co60       Cs136  Cs137       Eu147  Eu152  Eu154  Gd146 \n"
          << "Hf182        I126   I133        I134   I135    K40    K42 \n"
          << " Kr81        Kr85   Mn54        Na22    P32 Pa234m  Pb210 \n"
          << "Pb211       Pb212  Pb214       Po212  Po214  Ra228   Rb87 \n"
          << "Rh106       Sb125  Sb126       Sb133   Sr90  Ta182  Te133 \n"
          << "Te133m      Te134  Th234       Tl207  Tl208 Xe129m Xe131m \n"
          << "Xe133       Xe135    Y88         Y90   Zn65   Zr96 \n"
          <<"***\n";
     Log::Die("VertexGen_Decay0: Set the arguments");
     return;
  }
  std::istringstream is(newValues.c_str());
  // argument for double beta "2beta" or  for background&sources: "backg"
  G4String type;
  is >> type;
  // this should not be stored here, but inside the iterations, since the type time_cut should not be
  // propagated
  if (type=="2beta"){
     fType=type;
     // argument : [ISOTOPE]
     G4String isotope;
     is >> isotope;
     if (is.fail() || isotope.length()==0){
        Log::Die("VertexGen_Decay0: Incorrect vertex setting " + newValues);
     }
     fIsotopeRawIn = isotope;
     // Strip the isotope name
     StripIsotopeSuffix();

     //seems that doesn't work so the next try-catch should make the job
     try { fLdecay = DB::Get()->GetLink("Decay0", fIsotope);}
     catch (DBNotFoundError &e) {Log::Die("VertexGen_Decay0: can't find isotope " + fIsotope);}

     #ifdef DEBUG
      info << "VertexGen_Decay0: isotope: " << fIsotope << "\n";
     #endif
     // argument : [LEVEL]
     int level;
     is >> level;
     std::vector<int> vlevel;

     try {vlevel=fLdecay->GetIArray("Level");}
     catch (DBNotFoundError &e) {Log::Die("VertexGen_Decay0: can't find isotope " + fIsotope);}

     if (level < (int)vlevel.size()){
        #ifdef DEBUG
         info<< "VertexGen_Decay0: level: "<<level<<"\n";
        #endif
        fLevel= level;
     }
     else{
        Log::Die(dformat("VertexGen_Decay0: for isotope %s the levels are from 0 to %d inclusive \n",isotope.data(),vlevel.size()-1));
     }
     // argument : [MODE]
     int mode;
     is >> mode;
     if (mode>=0 && mode<=16){
        fMode = mode;
        #ifdef DEBUG
         info<< "VertexGen_Decay0: bb-decy mode: "<<mode<<"\n";
        #endif
     }
     else{
        warn << " bb-decy mode:\n"
             << "      1. 0nubb(mn) 0+ -> 0+     {2n}\n"
             << "      2. 0nubb(rc) 0+ -> 0+     {2n}\n"
             << "      3. 0nubb(rc) 0+ -> 0+, 2+ {N*}\n"
             << "      4. 2nubb     0+ -> 0+     {2n}\n"
             << "      5. 0nubbM1   0+ -> 0+     {2n}\n"
             << "      6. 0nubbM2   0+ -> 0+     (2n}\n"
             << "      7. 0nubbM3   0+ -> 0+     {2n}\n"
             << "      8. 0nubbM7   0+ -> 0+     {2n}\n"
             << "      9. 0nubb(rc) 0+ -> 2+     {2n}\n"
             << "     10. 2nubb     0+ -> 2+     {2n}, {N*}\n"
             << "     11. 0nuKb+    0+ -> 0+, 2+\n"
             << "     12. 2nuKb+    0+ -> 0+, 2+\n"
             << "     13. 0nu2K     0+ -> 0+, 2+\n"
             << "     14. 2nu2K     0+ -> 0+, 2+\n"
             << "     15. 2nubb     0+ -> 0+ with bosonic neutrinos\n"
             << "     16. 2nubb     0+ -> 2+ with bosonic neutrinos\n"
             << "\n   5-8: Majoron(s) with spectral index SI:\n"
             << "     SI=1 - old M of Gelmini-Roncadelli\n"
             << "     SI=2 - bulk M of Mohapatra\n"
             << "     SI=3 - double M, vector M, charged M\n"
             << "     SI=7\n";
        Log::Die("VertexGen_Decay0: choose a mode from 1 to 16");
     }
     // arguments : [LoELim] [HiELim]
     float loE=-1, hiE=-1;
     is >> loE>> hiE;
     if (is.fail()) {
        fLoE = 0;
        fHiE = 4.3;
     }
     if ((mode>= 4 && mode<= 8) || mode== 10 || mode==12) {
        if (loE>hiE && hiE!=-1 ){
           Log::Die(dformat("VertexGen_Decay0: wrong limits: from %lg  to %lg",loE,hiE));
        }
        fLoE = loE;
        fHiE = hiE;
        if (fHiE==-1 && fLoE==-1){
           fLoE = 0;
           fHiE = 4.3;
        }
        if (fHiE==-1){
           fLoE = 0;
           fHiE = loE;
        }
        #ifdef DEBUG
         info<< "VertexGen_Decay0: energy range: "<<fLoE<<", "<<fHiE<<"\n";
        #endif
     }
     else{
        info<< "limiting the energy possible only for modes: \n4,5,6,7,8,10 and 12 for others is ignored"<<"\n";
     }
     if ( fDecay0 == 0){
       fDecay0 = new Decay0(fIsotope, fLevel, fMode, fLoE, fHiE);
       fDecay0->SetAlphaCut(fHasAlphaCut);
       fDecay0->SetTimeCutoff(fHasTimeCutoff);
       fDecay0->GenBBTest();
     }
  }
  else if (type=="backg"){
    fType = type;
    // argument : [ISOTOPE]
    G4String isotope; vector<double> tim;
    is >> isotope;
    if (is.fail() || isotope.length()==0){
      Log::Die("VertexGen_Decay0: Incorrect vertex setting " + newValues);
    }
#ifdef RATDEBUG
    info << "VertexGen_Decay0::SetState : Parsing isotope [" << isotope << "]" << endl;
#endif
    fIsotopeRawIn = isotope;
    // Strip the isotope name
    StripIsotopeSuffix();

     //seems that doesn't work
    try { fLdecay = DB::Get()->GetLink("BackgDecay0", fIsotope); }
    catch (DBNotFoundError &e) {Log::Die("VertexGen_Decay0: can't find isotope " + fIsotope);}
    #ifdef DEBUG
     info << "VertexGen_Decay0: isotope: " << fIsotope << "\n";
    #endif

    if ( fDecay0 == 0){
       fDecay0 = new Decay0(fIsotope);
       // Pass any other flags that were parsed with the isotope
       fDecay0->SetAlphaCut(fHasAlphaCut);
       fDecay0->SetTimeCutoff(fHasTimeCutoff);
       fDecay0->GenBackgTest();
    }
  }
  else if (type == "time_cut") {
    double tcut;
    string units;
    double unit_value = 1.0;
    is >> tcut;
    if (is.fail()){
      ostringstream msg;
      msg << "VertexGen_Decay0: Failed to set the time window cut. Unknown state input [" << newValues << "]";
      Log::Die(msg.str());
    }

    is >> units;
    if (is.fail() || units.size() == 0) {
      // No units were specified. Assume ns
      warn << "VertexGen_Decay0: No time cut units were specified. Assuming ns." << endl;
    } else {
      unit_value = G4UnitDefinition::GetValueOf(units);
    }
    // This converts the unit to G4 internal unit
    tcut *= unit_value;

    if (fDecay0 == 0) {
      ostringstream msg;
      msg << "VertexGen_Decay0: Attempting to set a time cut without a valid Decay0 generator.";
      Log::Die(msg.str());
    } else if (!fDecay0->GetTimeCutoff()) {
      warn << "VertexGen_Decay0: WARNING: Setting a coincidence decay time cutoff time, without having passed the \"-timecut\" suffix with the isotope name." << endl;
    }
    info << "VertexGen_Decay0: Setting the coincidence cutoff time window in Decay0 to "
               << " (" << tcut/ns
               << " ns )."  << newline;

    fDecay0->SetCutoffWindow(tcut);
  }
  else{
    Log::Die("VertexGen_Decay0: Incorrect vertex setting: define \n'   2beta' for double beta  or \n   'backg' for background study ");
  }
}
///-------------------------------------------------------------------------
G4String VertexGen_Decay0::GetState()
{
  // State setting is the input file to be used
  G4std::ostringstream os;
  os << fIsotope << " " <<fLevel<<" "<< fMode << G4std::ends;
  G4String rv(os.str());
  return rv;
}
///-------------------------------------------------------------------------
void VertexGen_Decay0::GetParentAZ(G4int &A1, G4int &Z1, G4int &A2, G4int &Z2)
{
#ifdef RATDEBUG
  debug << "VertexGen_Decay0::GetParentAZ parsing " << fIsotope << newline;
#endif
  A1 = 0; Z1 = 0; A2 = 0; Z2 = 0;
  G4String Element;
  G4String value;
  G4NistManager* man = G4NistManager::Instance();
  const size_t size = fIsotope.size();
  size_t a = 0;
  size_t b = size;
  size_t c = size;
  // want to split fIsotope:
  // eg: BiPo214 -> Bi_a_214_b_Po_c_214,
  // K40 -> K_a_40, b=c=0
  // Xe131m -> Xe_a_131_b_m c=0
  for(size_t i=0;i<size; ++i){
    if(a==0){
      if(isdigit(fIsotope[i])){
        a = i;
      }
    }else if(b==size){
      if(!isdigit(fIsotope[i])){
        b = i;
      }
    }else if(c==size){
      if(isdigit(fIsotope[i])){
        c = i;
      }
    }
  }
  Element = fIsotope.substr(0,a);
  // Add a protection for 1 special case: BiPo

  value = fIsotope.substr(a,b-a);
  A1 = atoi(value.c_str());
  if (Element == "BiPo") {
    Z1 = man->GetZ("Bi");
    Z2 = man->GetZ("Po");
    A2 = A1;
  } else {
    Z1 = man->GetZ(Element);
  }

  if(c<size){
    Element = fIsotope.substr(b,c-b);
    Z2 = man->GetZ(Element);
    value = fIsotope.substr(c,size);
    A2 = atoi(value.c_str());
  }
#ifdef RATDEBUG
  info << "VertexGen_Decay0::GetParentAZ : " << fIsotope << " ! " << A1 << " " << Z1 << " " << A2 << " " << Z2 << newline;
#endif
}

///************************************************/

void VertexGen_Decay0::StripIsotopeSuffix() {

  vector<string> suffixes;
  suffixes.push_back("-pure");
  suffixes.push_back("-timecut");

  fIsotope = fIsotopeRawIn;
  size_t pos = 0;

  if ((pos = fIsotope.find(suffixes.at(0))) != string::npos) {
    debug << "VertexGen_Decay0::StripIsotopeSuffix : Found \"" << suffixes.at(0) << "\" suffix. Flagging it to eliminate initial alpha." << endl;
    fHasAlphaCut = true;
    fIsotope = fIsotope.erase(pos,suffixes.at(0).length());
  }

  if ((pos = fIsotope.find(suffixes.at(1))) != string::npos) {
    debug << "VertexGen_Decay0::StripIsotopeSuffix : Found \"" << suffixes.at(1) << "\" suffix. Flagging it to enable coincidence time cutoff." << endl;
    fHasTimeCutoff = true;
    fIsotope = fIsotope.erase(pos,suffixes.at(1).length());
  }

#ifdef RATDEBUG
  debug << "VertexGen_Decay0::StripIsotopeSuffix : Final isotope name : " << fIsotope << endl;
#endif
}

} // namespace RAT
