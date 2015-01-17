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
#include <algorithm>

namespace RAT {
  namespace DS {

class PMTInfo : public TObject {
public:
  PMTInfo() : TObject() {}
  virtual ~PMTInfo() {}

  virtual void AddPMT(const TVector3& _pos,
                      const TVector3& _dir,
                      const int _type,
                      const int _pmtid,
                      const std::string _model) {
    pos.push_back(_pos);
    dir.push_back(_dir);
    type.push_back(_type);
    pmtid.push_back(_pmtid);
    std::vector<std::string>::iterator which = std::find(models.begin(),models.end(),_model);
    if (which != models.end()) {
        modeltype.push_back(which-models.begin());
    } else {
        models.push_back(_model);
        modeltype.push_back(models.size()-1);
    }
  }
  
  virtual void AddPMT(const TVector3& _pos,
                      const TVector3& _dir,
                      const int _type) {
    AddPMT(_pos,_dir,_type,pos.size(),"");                   
  }

  virtual Int_t GetPMTCount() const { return pos.size(); }

  virtual TVector3 GetPosition(int index) const { return pos.at(index); }
  virtual void SetPosition(int index, const TVector3& _pos) { pos.at(index) = _pos; }

  virtual TVector3 GetDirection(int index) const { return dir.at(index); }
  virtual void SetDirection(int index, const TVector3& _dir) { dir.at(index) = _dir; }

  virtual int GetType(int index) const { return type.at(index); }
  virtual void SetType(int index, int _type) { type.at(index) = _type; }
  
  virtual int GetPMTID(int index) const { return pmtid.at(index); }
  virtual void SetPMTID(int index, int _pmtid) { pmtid.at(index) = _pmtid; }
  
  virtual int GetModel(int index) const { return modeltype.at(index); }
  virtual int SetModel(int index, std::string _model) {
    std::vector<std::string>::iterator which = std::find(models.begin(),models.end(),_model);
    int _modeltype;
    if (which != models.end()) {
        _modeltype = which - models.begin();
    } else {
        models.push_back(_model);
        _modeltype = models.size()-1;
    }
    modeltype.at(index) = _modeltype;
    return _modeltype;
  }
  virtual std::string GetModelName(int _modeltype) const { return models.at(_modeltype); }
  virtual int GetModelCount() const { return models.size(); }
  
  virtual int GetIndex(int _pmtid) { 
    std::vector<int>::iterator index = std::find(pmtid.begin(),pmtid.end(),_pmtid);
    if (index != pmtid.end()) {
      return *index;
    } else {
      return -1;
    }
  }

  ClassDef(PMTInfo, 1)

protected:
  std::vector<TVector3> pos;
  std::vector<TVector3> dir;
  std::vector<int> type;
  std::vector<int> pmtid;
  std::vector<int> modeltype;
  std::vector<std::string> models;
};

  } // namespace DS
} // namespace RAT

#endif

