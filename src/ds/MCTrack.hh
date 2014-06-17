/**
 * @class DS::MCTrack
 * Data Structure: Particle track in detector
 *
 * @author Stan Seibert <sseibert@hep.upenn.edu>
 *
 * This class represents the trajectory of a particle as it moves
 * through the detector.
 */

#ifndef __RAT_DS_MCTrack__
#define __RAT_DS_MCTrack__

#include <TObject.h>
#include <RAT/ListHelp.hh>
#include <RAT/DS/MCTrackStep.hh>

namespace RAT {
  namespace DS {

class MCTrack : public TObject {
public:
  MCTrack() : TObject() {}
  virtual ~MCTrack() {}

  /**
   * ID number for this track.  
   * 
   * Guaranteed to be unique for all tracks within this event, numbered
   * starting from 1.
   */
   virtual Int_t GetID() const { return id; }
   virtual void SetID(Int_t _id) { id = _id; }

  /**
   * Track ID number of parent.
   *
   * Tracks for the initial particles in the event have no parents, and are given
   * the parent ID of 0.
   */
   virtual Int_t GetParentID() const { return parentID; }
   virtual void SetParentID(Int_t _parentID) { parentID = _parentID; }

  /** Particle type as defined in MCParticle::pdgcode. */
  virtual Int_t GetPDGCode() const { return pdgcode; }
  virtual void SetPDGCode(Int_t _pdgcode) { pdgcode = _pdgcode; }
   
  /**
   * Name of particle.
   *
   * Often more accurate than the pdgcode, which is used inconsistently.
   */
  virtual const std::string& GetParticleName() const { return particleName; }
  virtual void SetParticleName(const std::string& _particleName) {
    particleName = _particleName;
  }

  /**
   * List of segments in this track.
   *
   * The first step in this list specifies the origin of the track,
   * and has zero length
   */
  virtual MCTrackStep* GetMCTrackStep(Int_t i) { return &step[i]; }
  virtual Int_t GetMCTrackStepCount() const { return step.size(); }
  virtual MCTrackStep* AddNewMCTrackStep() {
    step.resize(step.size() + 1);
    return &step.back();
  }
  virtual void PruneMCTrackStep() { step.resize(0); }
  virtual MCTrackStep* GetLastMCTrackStep() {
    if (step.size() == 0) {
      return NULL;
    }
    return &step.back();
  }

  /**
   * Eliminate all but first and last step of track.
   *
   * Used to condense tracks with too many steps for output.
   */
  virtual void PruneIntermediateMCTrackSteps() {
    if (step.size() > 2) {
      step[1] = step.back();
      step.resize(2);
    }
  }

  /** Total distance traveled along the track (mm). */
  virtual Float_t GetLength() const { return length; }
  virtual void SetLength(Float_t _length) { length = _length; }
  
  ClassDef(MCTrack, 1)

protected:
  Int_t id;
  Int_t parentID;
  Int_t pdgcode;
  Float_t length;
  std::string particleName;
  std::vector<MCTrackStep> step;
};

  } // namespace DS
} // namespace RAT

#endif

