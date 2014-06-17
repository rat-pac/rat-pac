#ifndef __RAT_ObjDbl__
#define __RAT_ObjDbl__

#include <TObject.h>

namespace RAT {
  
  // TObject wrapper for a double so you can write it to a file
  class ObjDbl : public TObject {
  public:
    ObjDbl() : TObject() { fVal = 0; };
    ObjDbl(double val) : TObject() { fVal = val; };
    ObjDbl(const ObjDbl &other) : TObject() { fVal = other.fVal; };
    ObjDbl& operator=(const ObjDbl &rhs) { fVal = rhs.fVal; return *this; };

    double GetDbl() const { return fVal; };
    void SetDbl(double val) { fVal = val; };
    
    ObjDbl& operator=(const double &val) { fVal = val; return *this; };
    operator double() const { return fVal; };

    ClassDef(ObjDbl, 1);
    
  protected:
    double fVal;
  };

} // namespace RAT

#endif
