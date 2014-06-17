#ifndef __RAT_BetaFunction__
#define __RAT_BetaFunction__

// 04-Jan-2006 WGS: Drag this code into the 21st century: use CLHEP,
// use strings, use STL.


#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>

namespace RAT {

  enum EReactions {
    iBAD = -9999,
    NullParticle = -2, // for stable particles
    DecayBeta = +1,
    DecayEC = -1,
    DecayGamma = 0,
    DecayAlpha = +2
  };

  class BetaFunction {

    enum EBetaFunctionConsts { PartMax = 1000, BrMax = 1000 };

  public:

    BetaFunction();
    BetaFunction(const std::string Name);
    BetaFunction(const std::string Name, double Z, double A = 0., int reaction =
		 DecayBeta, double tau = 0.);
    ~BetaFunction();
  
    void Reset();
    void Show();

    void SetTarget(const std::string Name, double Z, double A = 0., int reaction =
		   DecayBeta, double tau = 0.);
    void SetBranches(double Branch, int Spin, double EndPoint);
    void RemoveBranch(int iBranch);
    void SetGammas(int iBranch, int pid, double energy);
    void SetGammas(double energy);
    void SetParticleID(int iBranch, int n, int pid);
    void SetTargetMass(double A);
    void SetNorm(int iBranch);
  
    int GetNParticles(int iBranch);
    int GetParticleID(int iBranch, int n);
    int GetSpin(int iBranch);

    double GetBranch(int iBranch);
    double GetEndPoint(int iBranch);
    double GetEnergy(int iBranch, int n);
    double GetValue(double energy, int iBranch);
  
    void   GenerateEvent();
    void   SetEventTime();

    int    GetEventID(int n);
    double GetEventEnergy(int n);
    double GetEventTotE();
  
    double GetRandomNumber(double rmin = 0., double rmax = 1.);

    bool ReadInputFile(const std::string dName);
    bool ReadInputFile(const std::string dName, int iType);
    bool ReadInputFile(const std::string dName, int iZ, int iA, int iType = DecayBeta);
  
    void ErrorLog(int iFlag = -1);
    
    inline void SetFileName(const std::string Name) { inputFileName = Name; }
    inline void SetVerbose(bool iSet = true) {isVerbose = iSet;}
    inline void SetProbabilityCulmulative(bool iSel = true) {isCulmulative = iSel;}

    inline void SetName(const std::string Name) { TargetName = Name; }
    inline void SetCharge(double Z) {TargetCharge = Z;}
    inline void SetDecayType(int iType) {TargetDecayType = iType;}
    inline void SetNeutrinoMass(double Mass) {MassNu = Mass;}
    inline void SetLifeTime(double Tau) {TargetDecayTime = Tau;}

    inline int GetDecayType() {return TargetDecayType;}
    inline int GetNBranches() {return NumberOfBranches;}
    inline int GetNGenerated() {return nGenerated;}

    inline const std::string GetName() {return TargetName;}
    inline const std::string GetFileName() {return inputFileName;}

    inline double GetLifeTime() {return TargetDecayTime;}
    inline double GetTargetMass() {return TargetMass;}
    inline double GetCharge() {return TargetCharge;}
    inline double GetNeutrinoMass() {return MassNu;}
    inline double GetEventTime() {return nTime;}
    // WGS: Initialize DecayNorm if it hasn't been set before.
    inline double GetNorm(int iBranch) 
    {
      std::map< size_t, double >::iterator i = DecayNorm.find(iBranch);
      if ( i == DecayNorm.end() ) 
	{
	  DecayNorm[iBranch] = 1.;
	  return 1.; // We could return DecayNorm[iBranch], but that takes longer.
	}
      // Otherwise return DecayNorm[iBranch]... which we already point
      // to with i.
      return (*i).second;
    }

  private:

    bool isVerbose;
    bool isCulmulative;
  
    std::string TargetName;
    std::string inputFileName;
  
    int NumberOfBranches;
    std::map< size_t, int > ParticleSpin;
    std::map< size_t, double > ParticleEndPoint;
    std::map< size_t, double > ParticleBranch;
  
    std::map< size_t, double > DecayNorm;
    std::map< size_t, int > NumberOfParticles;
    std::map< size_t, std::map< size_t, int > > ParticleID;
    std::map< size_t, std::map< size_t, double > > ParticleEnergy;
  
    double TargetMass;
    double TargetCharge;
    double MassNu;
    double TargetDecayTime;
    int TargetDecayType;
  
    int nGenerated;
    std::map< size_t, int > IDn;
    std::map< size_t, double > En;
    double nTime;
  
  };

} // namespace RAT

#endif // __RAT_BetaFunction__
