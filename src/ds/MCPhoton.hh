/**
 * @class DS::MCPhoton
 * Data Structure: Photoelectron generated at PMT.
 *
 * @author Stan Seibert <sseibert@hep.upenn.edu>
 *
 * This class represents a single photoelectron generated at the
 * photocathode of the PMT.  The time jitter and delay in transit to the
 * anode are not included here, but the distribution of charge is,
 * which is slightly incongruous.
 *
 * Note that we require that the photon generates a photoelectron.
 * Absorbed photons are not included here.
 */

#ifndef __RAT_DS_MCPhoton__
#define __RAT_DS_MCPhoton__

#include <TObject.h>
#include <TVector3.h>

namespace RAT {
  namespace DS {

class MCPhoton : public TObject {
public:
  MCPhoton() : TObject(), isDarkHit(false) {}
  virtual ~MCPhoton() {}

  /** Time of photon hit at photocathode relative to event start time (ns). */
  virtual Float_t GetHitTime() const { return hitTime; }
  virtual void SetHitTime(Float_t _hitTime) { hitTime = _hitTime; }

  /** Time of pulse arriving at front-end electronics. */
  virtual Float_t GetFrontEndTime() const { return frontEndTime; }
  virtual void SetFrontEndTime(Float_t _frontEndTime) {
    frontEndTime = _frontEndTime;
  }
  
  /** Location of photon hit in local PMT coordinates (mm). */
  virtual TVector3 GetPosition() const { return pos; }
  virtual void SetPosition(const TVector3 &_pos) { pos = _pos; }

  /** Wavelength of photon (mm). */
  virtual Float_t GetLambda() const { return lambda; }
  virtual void SetLambda(Float_t _lambda) { lambda = _lambda; }

  /** Momentum of photon (MeV/c). */
  virtual TVector3 GetMomentum() const { return mom; }
  virtual void SetMomentum(const TVector3 &_mom) { mom = _mom; }

  /** Polarization vector. */
  virtual TVector3 GetPolarization() const { return pol; }
  virtual void SetPolarization(const TVector3 &_pol) { pol = _pol; }

  /** Charge created by photon in photoelectron (pe) units.
   *
   *  One pe is defined to be the peak of the single photoelectron
   *  charge distribution for this PMT.
   */
  virtual Float_t GetCharge() const { return charge; }
  virtual void SetCharge(Float_t _charge) { charge = _charge; }

  /** what processs created this photon? **/
  virtual void SetOriginFlag( int _origin ) { origin = _origin; };
  virtual Int_t GetOriginFlag() const { return origin; };

  /** Is this photoelectron due to a dark hit? */
  virtual void SetDarkHit(Bool_t _isDarkHit){ isDarkHit = _isDarkHit;}
  virtual Bool_t IsDarkHit() const { return isDarkHit; }

  /** Track ID of photon which generated this photoelectron */
  virtual void SetTrackID(Int_t _trackID){ trackID = _trackID;}
  virtual Int_t GetTrackID() const { return trackID; }

  ClassDef(MCPhoton, 1) 

protected:
  Float_t hitTime;
  Float_t frontEndTime;
  Float_t lambda;
  TVector3 pos;
  TVector3 mom;
  TVector3 pol;

  Float_t charge;
  Int_t origin;
  Bool_t isDarkHit;
  Int_t trackID;
};

  } // namespace DS
} // namespace RAT

#endif

