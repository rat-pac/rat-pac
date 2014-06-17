#ifndef __RAT_DecayChain__
#define __RAT_DecayChain__

// 04-Jan-2006 WGS: Drag this code into the 21st century: use CLHEP,
// use strings, use STL.

#include <RAT/BetaFunction.hh>

#include <CLHEP/Vector/LorentzVector.h>

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>

namespace RAT {

  static const double N_A = 6.022e+23;

  class DecayChain 
  {
  public:

    // Structure to hold (particle ID, four-vector):
    typedef struct {
      int ID;
      CLHEP::HepLorentzVector vector;
    } ParticleInfo_t;

    DecayChain();
    DecayChain(const std::string Name);
    ~DecayChain();
  
    void Reset();
    void Show();
  
    void AddElement(const std::string Name, int iLocation = 1, int iDecay = DecayBeta, double tau = 1.,double wt = 1.);
    void RemoveElement(int iBranch);
  
    void SetElementName(int iBranch, const std::string Name);
    void SetElementNumber(int iBranch);
    void SetElementBranch(int iBranch, int iLocation);
    void SetElementWt(int iBranch, double wt = 1.);
    void SetElementDecay(int iBranch, int iDecay);
    void SetLifetime(int iBranch, double tau);
  
    bool ReadInputFile(const std::string dName);

    const std::string GetElementName(int iBranch);
    int GetElementBranch(int iBranch);
    int GetElementDecay(int iBranch);
    double GetElementWt(int iBranch);
    double GetLifetime(int iBranch);
  
    void GenerateDecayElement(int iBranch);
    void GenerateDecayElement(const std::string iElement);
  
    void GenerateFullChain(const std::string iElement);
    void GenerateFullChain(int iBranch = 0);
  
    void GenerateDecayChain(const std::string iNameStart);
    void GenerateDecayChain(int iStart = 0);
  
    const std::string GetParentName(int iEvent);
    const std::string GetDaughterName(int iEvent);
    int GetEventID(int iEvent);
    double GetEventEnergy(int iEvent);
    double GetEventTime(int iEvent);
  
    bool CheckParentMatch(int iEvent);

    void SetDecaySequence();
    void SetElementProb(int iBranch, double prob);
    void SetEventTime(int iEvent, double time);
    double GetElementProb(int iBranch);
  
    int  CheckChainLevel(int iElement);
    bool CheckInChain(int iElement, int jElement);
  
    void SetParticleInfo(int iTag, int jTag, int kTag);

    ParticleInfo_t AddDaughterInfo(int iTag);
    ParticleInfo_t GetParticleInfo(int iEvent);

    double GetRandomNumber(double rmin = 0., double rmax = 1.);
  
    inline void SetVerbose(bool iSet=false) {isVerbose = iSet;};
    inline void SetFileName(const std::string Name) { inputFileName = Name; }
    inline void SetChainName(const std::string Name) { ChainName = Name; }
    inline void SetEquilibrium(bool iEq = true) {isEquilibrium = iEq;};
    inline void SelectParent(const std::string iElement = "ALL") { oneParent = iElement; }
    inline void SetNGenerated(int nP) {NumberOfParticles = nP;};
    inline void IncludeDaughter(bool iSelect = true){CreateDaughter = iSelect;};

    inline bool GetEquilibrium() {return isEquilibrium;};

    inline int GetNElements() {return NumberOfElements;}
    inline int GetElementNumber() {return ElementNumber;};
    inline int GetNGenerated() {return NumberOfParticles;};
  
    inline const std::string GetFileName() {return inputFileName;};
    inline const std::string GetChainName() {return ChainName;};

  private:
    inline void SetParentName(int iParent, const std::string name) { ParentName[iParent] = name; }
    inline void SetDaughterName(int iDaughter, const std::string name) { DaughterName[iDaughter] = name; }

  private:
  
    bool isVerbose;
    bool isEquilibrium;
    bool isChainElement;
  
    std::string ChainName;
  
    int NumberOfElements;
    int ElementNumber;
    std::map< size_t, std::string > ElementName;
    std::map< size_t, int > ElementBranch;
    std::map< size_t, int > ElementDecay;
    std::map< size_t, double > ElementWeight;
    std::map< size_t, double > ElementLife;
    std::map< size_t, double > ElementProb;
    double tstart;
  
    std::map< size_t, BetaFunction* > Element;
  
    bool CreateDaughter;

    std::string inputFileName;
    std::string oneParent;
  
    int NumberOfParticles;
    std::map< size_t, std::string > ParentName;
    std::map< size_t, std::string > DaughterName;
    std::map< size_t, int > ParticleID;
    std::map< size_t, double > ParticleEnergy;
    std::map< size_t, double > ParticleTime;
  
    std::map< size_t, ParticleInfo_t > theParticleInfo;

  };

} // end namespace RAT

#endif
