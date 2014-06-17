#ifndef __RAT_Trajectory__
#define __RAT_Trajectory__

#include <G4Trajectory.hh>
#include <RAT/DS/MCTrack.hh>
#include <string>

namespace RAT {


class Trajectory : public G4Trajectory 
{ 
public: 
  Trajectory(); 
  Trajectory(const G4Track* aTrack); 
  virtual ~Trajectory();

  inline void* operator new(size_t);
  inline void  operator delete(void*);

  
  virtual void AppendStep(const G4Step* aStep); 
  virtual void MergeTrajectory(G4VTrajectory* secondTrajectory); 

  virtual void FillStep(const G4StepPoint *point, const G4Step *step, 
                        DS::MCTrackStep *ratStep, double stepLength);
  DS::MCTrack* GetTrack() { return ratTrack; };
  
  static void SetDoAppendMuonStepSpecial(const bool& _doAppend)
  { fgDoAppendMuonStepSpecial=_doAppend; };
  static bool GetDoAppendMuonStepSpecial()
  { return fgDoAppendMuonStepSpecial; };

protected:
  std::string creatorProcessName;
  DS::MCTrack *ratTrack;
  static bool fgDoAppendMuonStepSpecial;

}; 

// GEANT4 uses a custom allocator on subclass, so we need to override it here.

#if defined G4TRACKING_ALLOC_EXPORT
  extern G4DLLEXPORT G4Allocator<Trajectory> aTrajectoryAllocator;
#else
  extern G4DLLIMPORT G4Allocator<Trajectory> aTrajectoryAllocator;
#endif


inline void* Trajectory::operator new(size_t)
{
  void* aTrajectory;
  aTrajectory = (void*)aTrajectoryAllocator.MallocSingle();
  return aTrajectory;
}

inline void Trajectory::operator delete(void* aTrajectory)
{
  aTrajectoryAllocator.FreeSingle((Trajectory*)aTrajectory);
}


} // namespace RAT

#endif 
