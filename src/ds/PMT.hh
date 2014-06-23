/**
 * @class PMT
 * Data Structure: PMT in triggered event
 * 
 * This represents a PMT in a detector event.
 */

#ifndef __RAT_DS_PMT__
#define __RAT_DS_PMT__

#include <Rtypes.h>

namespace RAT {
  namespace DS {

class PMT : public TObject {
public:
  PMT() : TObject() {}
  virtual ~PMT() {}

  /** ID number of PMT */
  virtual void SetID(Int_t _id) { this->id = _id; }
  virtual Int_t GetID() { return id; }

  /** Total charge in waveform (pC) */
  virtual void SetCharge(Float_t _charge) { this->charge = _charge; }
  virtual Float_t GetCharge() { return charge; }

  /** Hit time in ns */
  virtual void SetTime(Float_t _time) { this->time = _time; }
  virtual Float_t GetTime() { return time; }

 ClassDef(PMT, 1);

protected:
  Int_t id;
  Float_t charge;
  Float_t time;
};

  } // namespace DS
} // namespace RAT

#endif

