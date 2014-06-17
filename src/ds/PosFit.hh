/**
 * @class DS::PosFit
 * Data Structure: Position fit base class
 *
 * A reconstructed position for an event.  All position fitters should use
 * a subclass of PosFit to record position information.  A combined
 * position and energy fitter should subclass both PosFit and EFit.
 */

#ifndef __RAT_DS_PosFit__
#define __RAT_DS_PosFit__

#include <map>
#include <string>
#include <TObject.h>
#include <TVector3.h>

namespace RAT {
  namespace DS {

class PosFit {
public:
  PosFit(const char *name) : posname(name), pos(-99999, -99999, -99999) {}
  virtual ~PosFit() {}

  /** Name of fitter processor used to compute this position */
  virtual const std::string &GetPosFitName() const { return posname; }
  virtual void SetPosFitName(const std::string &_name) { posname = _name; }

  /** Position of event (mm). */
  virtual const TVector3 &GetPosition() const { return pos; }
  virtual void SetPosition(const TVector3 &_pos) { pos = _pos; }
   
  ClassDef(PosFit, 1)

protected:
  std::string posname;
  TVector3 pos;
};

  } // namespace DS
} // namespace RAT

#endif

