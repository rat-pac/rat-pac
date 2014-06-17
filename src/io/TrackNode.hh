#ifndef __RAT_TrackNode__
#define __RAT_TrackNode__

#include <RAT/DS/MCTrackStep.hh>


namespace RAT {


class TrackNode : public DS::MCTrackStep {
public:
  TrackNode() { trackStart = true; prev = next = 0; };
  virtual ~TrackNode() { 
    if (next) delete next;
    for (unsigned i=0; i < child.size(); i++)
      if (child[i]) delete child[i];
  };

  /** True if this is the first step in the track */
  virtual bool IsTrackStart() const { return trackStart; };
  virtual void SetTrackStart(bool state=true) { trackStart = state; };

  /** True if this is the last step in the track */
  virtual bool IsTrackEnd() const { return next == 0; };

  /** ID number for this track.  
   * 
   *  Guaranteed to be unique for all tracks within this event, numbered
   *  starting from 1.
   */
   virtual int GetTrackID() const { return trackID; };
   virtual void SetTrackID(int _trackID) { trackID = _trackID; };

  /** ID number for this step */
   virtual int GetStepID() const { return stepID; };
   virtual void SetStepID(int _stepID) { stepID = _stepID; };

  /** Particle type as defined in MCParticle::pdgcode. */
    virtual int GetPDGCode() const { return pdgcode; };
    virtual void SetPDGCode(int _pdgcode) { pdgcode = _pdgcode; };
    
  /** Name of particle.
   *
   *  Often this is more accurate than the pdgcode, which is used inconsistently. */
   virtual const std::string& GetParticleName() const { return particleName; };
   virtual void SetParticleName(const std::string &_particleName) {
     particleName = _particleName;
   }


  // Step information inherited from DS::MCTrackStep

  virtual TrackNode *GetPrev() const { return prev; };
  virtual void SetPrev(TrackNode *_prev) { prev = _prev; };

  virtual TrackNode *GetNext() const { return next; };
  virtual void SetNext(TrackNode *_next) { next = _next; };
  
  std::vector<TrackNode*> child; // Any additional tracks connected to this one
  // Not using TClonesArray because this object does not go into a TTree ever!

  // Util methods
  void AddNext(TrackNode *n) { n->prev = this;n->trackStart = false;next = n; };
  void AddChild(TrackNode *c) {c->prev = this;c->trackStart = true;child.push_back(c);};
  virtual TrackNode &operator= (const DS::MCTrackStep &rhs) {
    *dynamic_cast<DS::MCTrackStep *>(this) = rhs;
    return *this;
  };

  ClassDef(TrackNode,2);
protected:
  bool trackStart;
  int trackID;
  int stepID;
  int pdgcode;
  std::string particleName;
  
  TrackNode *prev;
  TrackNode *next; // Next node for this same particle
};


} // namespace RAT

#endif
