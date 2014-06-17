#ifndef __RAT_TrackCursor__
#define __RAT_TrackCursor__

#include <RAT/TrackNode.hh>
#include <string>
#include <functional>

namespace RAT {

class TrackTest : public std::unary_function<TrackNode *, bool> {
public:
  virtual bool operator() (TrackNode *) = 0;
}; // Common predicates at the end


class TrackCursor {
public:
  TrackCursor(TrackNode *node, bool verbose=false) 
      { fVerbose = verbose; Go(node); };

  bool GetVerbose() { return fVerbose; };
  void SetVerbose(bool verbose=true) { fVerbose = verbose; };

  // Node peeking (does not change current position)
  TrackNode *TrackStart() const;
  TrackNode *Prev() const { return fCur->GetPrev(); };
  TrackNode *Here() const { return fCur; };
  TrackNode *Next() const { return fCur->GetNext(); };
  TrackNode *TrackEnd() const;

  int StepCount() const { return TrackEnd()->GetStepID() + 1; };
  TrackNode *Step(int i) const;
  int ChildCount() const { return fCur->child.size(); };
  TrackNode *Child(int i=0) const;
  int TrackChildCount() const;
  TrackNode *TrackChild(int i) const;
  TrackNode *Parent() const;

  // Information
  double TrackLength() const;
  bool IsTrackStart() const { return fCur->IsTrackStart(); };
  bool IsTrackEnd() const { return fCur->IsTrackEnd(); };

  // Navigation (returns node at new location as well as updating current pos)
  void Go(TrackNode *node);
  TrackNode *GoTrackStart();
  TrackNode *GoPrev();
  TrackNode *GoNext();
  TrackNode *GoTrackEnd();

  TrackNode *GoStep(int i);
  TrackNode *GoChild(int i=0);
  TrackNode *GoTrackChild(int i); 
  TrackNode *GoParent(); // Go to the step in the parent particle where
                            // this track was made

  // Output
  void Print() const;
  void PrintTrack() const;

  static std::string PrintTrackIDs(TrackNode *node);
  static std::string Print(TrackNode *node);
  static std::string PrintTrack(TrackNode *node);

  // Generic Search
  TrackNode *FindNextTrack();
  TrackNode *FindNextTrackNoDescend();

  TrackNode *FindNextTrack(TrackTest *predicate);


  // Specialized Search
  TrackNode *FindNextParticle(const std::string &particleName);
  //TrackNode *FindNextProcess(std::string process);

protected:
  TrackNode *fCur;
  bool fVerbose;
};


// Useful predicates

class TrackTest_Particle : public TrackTest {
  std::string fParticleName;
public:
  TrackTest_Particle(const std::string &particleName) : fParticleName(particleName) { };
  virtual bool operator() (TrackNode *c) { return fParticleName == c->GetParticleName(); };
};

// class TrackTest_HasProcess : public TrackTest {
//   std::string fProcess;
// public:
//   TrackTest_Particle(std::string process) : fProcess(process) { };
//   virtual bool operator() (TrackNode *c) { 
//     int totalSteps;
//   };
// };

} // namespace RAT

#endif

