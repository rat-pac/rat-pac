#ifndef __RAT_HashFunc__
#define __RAT_HashFunc__

#include <string>

namespace RAT {


// String hashing algorithm taken from Python 2.4.1, stringobject.c
// Why doesn't STL include this stuff?!
struct pyhash
{
  unsigned operator()(const std::string& s) const {
    register int len;
    register int full_len;
    register unsigned char *p;
    register long x;
    
    full_len = len = s.size();
    p = (unsigned char *) s.c_str();
    x = *p << 7;
    while (--len >= 0)
      x = (1000003*x) ^ *p++;
    x ^= full_len;
    return x;
  }
};


} // namespace RAT

#endif
