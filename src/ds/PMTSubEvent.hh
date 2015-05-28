/**
 * @class PMTSubEvent
 * Data Structure: PMTSubEvent in triggered event
 * 
 * This represents a PMTSubEvent in a detector event.
 */

#ifndef __RAT_DS_PMTSubEvent__
#define __RAT_DS_PMTSubEvent__

#include <Rtypes.h>

namespace RAT {
  namespace DS {

class PMTSubEvent : public TObject {
public:
  PMTSubEvent() : TObject() {}
  virtual ~PMTSubEvent() {}

  /** ID number of PMTSubEvent */
  virtual void SetID(Int_t _id) { this->id = _id; }
  virtual Int_t GetID() { return id; }

  /** Total charge in waveform (pC) */
  virtual void SetCharge(Float_t _charge) { this->charge = _charge; }
  virtual Float_t GetCharge() { return charge; }

  /** Hit time in ns */
  virtual void SetTime(Float_t _time) { this->time = _time; }
  virtual Float_t GetTime() { return time; }

 /** Hit time in ns */
 virtual void SetGlobalTime(Float_t _gTime) { this->time = _gTime; }
 virtual Float_t GetGlobalTime() { return gTime; }
 ClassDef(PMTSubEvent, 1);

protected:
  Int_t id;
  Float_t charge;
  Float_t time;
  Float_t gTime;

};

  } // namespace DS
} // namespace RAT

#endif

