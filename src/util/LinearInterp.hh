#ifndef __RAT_LinearInterp__
#define __RAT_LinearInterp__

#include <vector>

namespace RAT {


template <class NumType>
class LinearInterp {
public:
  LinearInterp() { npoints = 0; };
  LinearInterp(int _npoints, const NumType _x[], const NumType _y[]) {
    Set(_npoints, _x, _y);
  };
  LinearInterp(const std::vector<NumType> &_x, const std::vector<NumType> &_y) {
    Set(_x, _y);
  };
  
  void Set(int _npoints, const NumType _x[], const NumType _y[]);
  void Set(const std::vector<NumType> &_x, const std::vector<NumType> &_y) {
    npoints = _x.size(); x = _x; y = _y;
  };

  NumType operator() (const NumType xeval) const;

  NumType Min() { return x.front(); };
  NumType Max() { return x.back(); };
  int Points() { return npoints; };

  class RangeError {
  public:
    RangeError(const NumType _xlow, const NumType _xhigh, const NumType _xeval) 
      : xlow(_xlow), xhigh(_xhigh), xeval(_xeval)
      { /* no-op */ };
    bool operator== (const RangeError &other) const {
      return xlow == other.xlow && xhigh == other.xhigh
	&& xeval == other.xeval;
    };
    NumType xlow, xhigh, xeval;
  };

protected:
  int LowerElement(const NumType xeval) const;

  int npoints;
  std::vector<NumType> x, y;
};

#include "LinearInterp.icc"

} // namespace RAT

#endif
