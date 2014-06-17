#ifndef __RAT_ObjInt__
#define __RAT_ObjInt__

#include <TObject.h>

namespace RAT {
  
  // TObject wrapper for an integer so you can write it to a file
  class ObjInt : public TObject {
  public:
    ObjInt() : TObject() { fVal = 0; };
    ObjInt(int val) : TObject() { fVal = val; };
    ObjInt(const ObjInt &other) : TObject() { fVal = other.fVal; };
    ObjInt& operator=(const ObjInt &rhs) { fVal = rhs.fVal; return *this; };

    int GetInt() const { return fVal; };
    void SetInt(int val) { fVal = val; };
    
    ObjInt& operator=(const int &val) { fVal = val; return *this; };
    operator int() const { return fVal; };

    ClassDef(ObjInt, 1);
    
  protected:
    int fVal;
  };

} // namespace RAT

#endif
