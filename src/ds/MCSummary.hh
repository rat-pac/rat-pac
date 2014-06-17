/**
 * @class DS::MCSummary
 * Data Structure: Summary information about this event
 *
 * @author Stan Seibert <sseibert@hep.upenn.edu>
 *
 * This class summarizes the physical interactions that occurred during
 * this event.
 */

#ifndef __RAT_DS_MCSummary__
#define __RAT_DS_MCSummary__

#include <TObject.h>
#include <TVector3.h>
#include <map>
#include <string>

namespace RAT {
  namespace DS {

typedef std::map<std::string, double> StringDoubleMap;

class MCSummary : public TObject {
public:
  MCSummary() : TObject() {}
  virtual ~MCSummary() {}

  /**
   * Centroid of energy loss.
   *
   * This is the average position of all steps in this event, weighted by
   * the energy lost in that step. Optical photons, and the rest mass of
   * particles when a track terminates are not included.
   */
  virtual const TVector3& GetEnergyCentroid() const { return energyCentroid; }
  virtual void SetEnergyCentroid(const TVector3& _energyCentroid) {
    energyCentroid = _energyCentroid;
  }

  /** RMS of energy loss. */
  virtual const TVector3& GetEnergyRMS() const { return energyRMS; }
  virtual void SetEnergyRMS(const TVector3& _energyRMS) {
    energyRMS = _energyRMS;
  }

  /** Convenience method to look up individual elements in CINT */
  virtual Float_t GetEnergyLossByVolume(const char *volume) {
    return energyLoss[volume];
  }
  virtual void SetEnergyLossByVolume(StringDoubleMap _energyLoss) {
    energyLoss = _energyLoss;
  }

  /** Total energy deposited in the scintillator in this event (MeV) */
  virtual Float_t GetTotalScintEdep() const { return totalScintEdep; };
  virtual void SetTotalScintEdep(Float_t _totalScintEdep) {
    totalScintEdep = _totalScintEdep;
  }

  /** Total energy deposited in scintillator with quenched energy
   *  removed.
   *
   * This represents the amount of "visible energy" that was converted
   * into optical photons.
   */
  virtual Float_t GetTotalScintEdepQuenched() const {
    return totalScintEdepQuenched;
  }
  virtual void SetTotalScintEdepQuenched(Float_t _totalScintEdepQuenched) { 
    totalScintEdepQuenched = _totalScintEdepQuenched; 
  }
  
  /** Centroid of the scintillation energy loss */
  virtual const TVector3 &GetTotalScintCentroid() const { 
    return totalScintCentroid; 
  }
  virtual void SetTotalScintCentroid(const TVector3 &_totalScintCentroid) {
    totalScintCentroid = _totalScintCentroid;
  }

  /** Initial time of the scintillation (ns) */
  virtual Float_t GetInitialScintTime() const { return initialScintTime; }
  virtual void SetInitialScintTime(Float_t _initialScintTime) { 
    initialScintTime = _initialScintTime; 
  }

  /** Number of photons produced by the scintillation process */
  virtual Int_t GetNumScintPhoton() const { return numScintPhoton; }
  virtual void SetNumScintPhoton(Int_t _numScintPhoton) {
    numScintPhoton = _numScintPhoton;
  }

  /** Number of photons produced by the re-emission process */
  virtual Int_t GetNumReemitPhoton() const { return numReemitPhoton; }
  virtual void SetNumReemitPhoton(Int_t _numReemitPhoton) {
    numReemitPhoton = _numReemitPhoton;
  }

  ClassDef(MCSummary, 1)
    
protected:
  Int_t numScintPhoton;
  Int_t numReemitPhoton;
  Float_t totalScintEdep;
  Float_t totalScintEdepQuenched;
  Float_t initialScintTime;
  TVector3 energyCentroid;
  TVector3 energyRMS;
  TVector3 opticalCentroid;
  TVector3 opticalRMS;
  TVector3 totalScintCentroid;
  StringDoubleMap energyLoss;
};

  } // namespace DS
} // namespace RAT

#endif

