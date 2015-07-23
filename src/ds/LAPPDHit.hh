/**
 * @class LAPPDHit
 * Data Structure: Photoelectron generated at LAPPD.
 *
 * This represents a photoelectron hit in a LAPPD as read by
 * the front-end electronics
 *
 */

#ifndef __RAT_DS_LAPPDHit__
#define __RAT_DS_LAPPDHit__

#include <TObject.h>
#include <TVector3.h>

namespace RAT {
  namespace DS {

class LAPPDHit : public TObject {
public:
  LAPPDHit() : TObject() {}
  virtual ~LAPPDHit() {}

  /** Time of photon hit at photocathode relative to event start time (ns). */
  virtual Float_t GetTime() const { return time; }
  virtual void SetTime(Float_t _time) { time = _time; }

  /** Location of photon hit in local LAPPD coordinates (mm). */
  virtual TVector3 GetPosition() const { return pos; }
  virtual void SetPosition(const TVector3 &_pos) { pos = _pos; }

  /** Charge created by photon in photoelectron (pe) units. */
  virtual Float_t GetCharge() const { return charge; }
  virtual void SetCharge(Float_t _charge) { charge = _charge; }

  ClassDef(LAPPDHit, 1) 

protected:
  Float_t time;
  Float_t charge;
  TVector3 pos;

};

  } // namespace DS
} // namespace RAT

#endif

