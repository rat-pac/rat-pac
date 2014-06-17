/**
 * @class DS::Calib
 * Data Structure: Calibration source
 *
 * Information about calibration source active during this event.  If
 * no source was active, then @p id will be set to -1.
 *
 * The precise meaning of these attributes depends on the source
 * type.  Currently known sources are listed below.
 *
 * @b LED
 *  - @p name : "LED"
 *  - @p id : Number of LED in array, indexed from 0.
 *  - @p mode : Wavelength of source in nanometers
 *  - @p Int_tensity : Number of photons emitted
 */

#ifndef __RAT_DS_Calib__
#define __RAT_DS_Calib__

#include <TObject.h>
#include <TVector3.h>
#include <string>
#include <TTimeStamp.h>

namespace RAT {
  namespace DS {

class Calib : public TObject {
public:
  Calib() : TObject(), id(-1), name("") {}
  virtual ~Calib() {}

  /** Short name of calibration source. */
  virtual const std::string &GetSourceName() const { return name; }
  virtual void SetSourceName(const std::string &_name) { name = _name; }

  /** ID number of source, -1 if no source active during this event */
  virtual Int_t GetID() const { return id; }
  virtual void SetID(Int_t _id) { id = _id; }
  
  /** Source mode (meaning depends on type of source) */
  virtual Int_t GetMode() const { return mode; }
  virtual void SetMode(Int_t _mode) { mode = _mode; }
  
  /** Source intensity (meaning depends on type of source) */
  virtual Float_t GetIntensity() const { return intensity; }
  virtual void SetIntensity(Float_t _intensity) { intensity = _intensity; }
  
  /** Absolute time of source activation */
  virtual TTimeStamp GetUTC() const { return utc; }
  virtual void SetUTC(const TTimeStamp& _utc) { utc = _utc; }
  
  /** Location (mm) of source when activated. */
  virtual const TVector3& GetPosition() const { return pos; }
  virtual void SetPosition(const TVector3& _pos) { pos = _pos; }

  ClassDef(Calib, 1)

protected:
  Int_t id;  
  Int_t mode;       
  Float_t intensity;
  std::string name;
  TTimeStamp utc;
  TVector3 pos;
};

  } // namespace DS
} // namespace RAT

#endif

