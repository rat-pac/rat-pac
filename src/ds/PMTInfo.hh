/**
 * @class DS::PMTInfo
 * Data Structure: PMT properties
 *
 * Information about PMTs, including positions, rotations, and types
 */

#ifndef __RAT_DS_PMTInfo__
#define __RAT_DS_PMTInfo__

#include <TObject.h>
#include <TVector3.h>

namespace RAT {
  namespace DS {

class PMTInfo : public TObject {
public:
  PMTInfo() : TObject() {}
  virtual ~PMTInfo() {}

  virtual void AddPMT(const TVector3& _pos,
                      const TVector3& _dir,
                      const int _type) {
    pos.push_back(_pos);
    dir.push_back(_dir);
    type.push_back(_type);
  }

  virtual Int_t GetPMTCount() const { return pos.size(); }

  virtual TVector3 GetPosition(int i) const { return pos.at(i); }
  virtual void SetPosition(int i, const TVector3& _pos) { pos.at(i) = _pos; }

  virtual TVector3 GetDirection(int i) const { return dir.at(i); }
  virtual void SetDirection(int i, const TVector3& _dir) { dir.at(i) = _dir; }

  virtual int GetType(int i) const { return type.at(i); }
  virtual void SetType(int i, int _type) { type.at(i) = _type; }

  ClassDef(PMTInfo, 1)

protected:
  std::vector<TVector3> pos;
  std::vector<TVector3> dir;
  std::vector<int> type;
};

  } // namespace DS
} // namespace RAT

#endif

