#ifndef __RAT_TrackInfo__
#define __RAT_TrackInfo__

#include <G4VUserTrackInformation.hh>
#include <G4Allocator.hh>
#include <string>
#include <map>
#include <RAT/CentroidCalculator.hh>

namespace RAT {


class TrackInfo : public G4VUserTrackInformation
{ 
public: 
  TrackInfo() {};
  virtual ~TrackInfo() {};

  inline void* operator new(size_t);
  inline void  operator delete(void*);


  void SetCreatorProcess(std::string &creatorProcess)
    { fCreatorProcess = creatorProcess; };
  void SetCreatorProcess(const char *creatorProcess)
    { fCreatorProcess = creatorProcess; };
  std::string GetCreatorProcess() const { return fCreatorProcess; };

  // Ok, I'm tired of getter/setter C++ bondage crap.  Just expose the
  // interface already.

  /** Centroid of steps, weighted by energy loss. */
  CentroidCalculator energyCentroid;
  /** Centroid of optical photon creation vertices. */
  CentroidCalculator opticalCentroid;

  /** Energy lost by this track, indexed by volume name */
  std::map<std::string, double> energyLoss;
  
  /** Step in the parent track at which this track was created */
  void SetCreatorStep(int _CreatorStep)
  { CreatorStep=_CreatorStep; };
  int GetCreatorStep() const { return CreatorStep; };

  virtual void Print() const { };
  
protected:
  std::string fCreatorProcess;
  int CreatorStep;
}; 

// GEANT4 uses a custom allocator on subclass, so we need to override it here.
extern G4Allocator<TrackInfo> aTrackInfoAllocator;


inline void* TrackInfo::operator new(size_t)
{
  void* aTrackInfo;
  aTrackInfo = (void*)aTrackInfoAllocator.MallocSingle();
  return aTrackInfo;
}

inline void TrackInfo::operator delete(void* aTrackInfo)
{
  aTrackInfoAllocator.FreeSingle((TrackInfo*)aTrackInfo);
}


} // namespace RAT

#endif 
