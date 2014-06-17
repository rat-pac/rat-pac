#ifndef __RAT_RadicalInverse__
#define __RAT_RadicalInverse__

namespace RAT {

inline double RadicalInverse(int n, int base)
{
  double val = 0;
  double invBase = 1.0/base, invBi = invBase;
  
  while (n > 0) {
    int d_i = (n % base);
    val += d_i * invBi;
    n /= base;
    invBi *= invBase;
  }
  
  return val;
}


} // namespace RAT

#endif
