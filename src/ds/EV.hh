/**
 * @class DS::EV
 * Data Structure: Triggered event
 *
 * @author Stan Seibert <sseibert@hep.upenn.edu>
 *
 * This class represents a detected event as defined by the trigger
 * simulation.
 */

#ifndef __RAT_DS_EV__
#define __RAT_DS_EV__

// Clang 5.0 headers confuse rootcint...
#ifdef __MAKECINT__
#define __signed signed
#endif

#include <TObject.h>
#include <TTimeStamp.h>
#include <RAT/DS/PMT.hh>
#include <RAT/DS/Centroid.hh>
#include <RAT/DS/PathFit.hh>
#include <vector>

namespace RAT {
  namespace DS {

class EV : public TObject {
public:
  EV() : TObject() {}
  virtual ~EV() {}

  /** Event number. */
  virtual Int_t GetID() const { return id; }
  virtual void SetID(Int_t _id) { id = _id; }
    
  /** Sub Event information. Including trigger time of sub event with respect 
   to event start.*/
  virtual Int_t GetSubEV() const { return subevent; }
  virtual void SetSubEV(Int_t _subevent) { subevent = _subevent; }
  virtual Int_t GetTotalSubEV() const { return nSubevents; }
  virtual void SetTotalSubEV(Int_t _nSubevents) { nSubevents = _nSubevents; }
  Float_t GetSubTriggerTime() const { return triggerTime; }
  void SetSubTriggerTime(Float_t _triggerTime) { triggerTime = _triggerTime; }

    
  /** Date/time of event trigger (UTC)*/
  virtual TTimeStamp GetUTC() const { return utc; }
  virtual void SetUTC(const TTimeStamp& _utc) { utc = _utc; }

  /** List of pmts with at least one charge sample in this event. */
  virtual PMT* GetPMT(Int_t i) { return &pmt[i]; }
  virtual Int_t GetPMTCount() const { return pmt.size(); }
  virtual PMT* AddNewPMT() {
    pmt.resize(pmt.size() + 1);
    return &pmt.back();
  }
  virtual void PrunePMT() { pmt.resize(0); }
  
  /** Number of PMTs which were hit at least once. (Convenience method) */
  virtual Int_t Nhits() const { return GetPMTCount(); }

  /** Time since last trigger in ns. */
  Float_t GetDeltaT() const { return deltat; }
  void SetDeltaT(Float_t _deltat) { deltat = _deltat; }

  /** Total charge in all PMT waveforms (pC). */
  Float_t GetTotalCharge() const { return qTotal; }
  void SetTotalCharge(Float_t _qTotal) { qTotal = _qTotal; }

    
  /** Centroid position fitter. */
  virtual Centroid* GetCentroid() {
    if (centroid.empty()) {
      centroid.resize(1);
    }
    return &centroid.back();
  }
  virtual bool ExistCentroid() const { return !centroid.empty(); }
  virtual void PruneCentroid() { centroid.resize(0); }
  
  /** PathFit position, direction, time fitter. */
  virtual PathFit* GetPathFit() {
    if (pathfit.empty()) {
      pathfit.resize(1);
    }
    return &pathfit.back();
  }
  virtual bool ExistPathFit() const { return !pathfit.empty(); }
  virtual void PrunePathFit() { pathfit.resize(0); }

  ClassDef(EV, 1)

protected:
  Int_t id;
  Float_t qTotal;
  Float_t calibratedTriggerTime;
  Float_t triggerTime;  //Trigger time of subevent. PMT times of subevents are
                        //subtracted by this value
  Float_t deltat;
  Int_t nSubevents; //Total number of subevent in event.
  Int_t subevent;   //subevent index. Allows to select PMT in a specific range
  TTimeStamp utc;
  std::vector<PMT> pmt;
  std::vector<Centroid> centroid;
  std::vector<PathFit> pathfit;
};

  } // namespace DS
} // namespace RAT

#endif

