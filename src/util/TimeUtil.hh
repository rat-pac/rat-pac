#ifndef __RAT_TimeUtil__
#define __RAT_TimeUtil__

#include <TTimeStamp.h>

namespace RAT {
  // I am going to type the wrong number of zeros if I don't do this
  // and I want an integer, not a double
  const long TIME_UTIL_BILLION = 1000000000L;
  
  // Return a new TTimeStamp offset by nanoseconds.  nanoseconds can be negative
  inline TTimeStamp AddNanoseconds(const TTimeStamp &a, const long nanoseconds) {
    time_t new_sec = a.GetSec();
    long new_nsec = a.GetNanoSec() + nanoseconds;
    // Normalize
    if (new_nsec < 0) {
      new_sec -= 1 + (-new_nsec / TIME_UTIL_BILLION);
      new_nsec = TIME_UTIL_BILLION - (-new_nsec % TIME_UTIL_BILLION);
    } else if (new_nsec >= TIME_UTIL_BILLION) {
      new_sec += new_nsec / TIME_UTIL_BILLION;
      new_nsec = new_nsec % TIME_UTIL_BILLION;
    }
    return TTimeStamp(new_sec, new_nsec);
  }
  
  // Get time difference (a - b) in nanoseconds.  Note that maximum deltaT is +/-2^63 = 292 years
  inline long TimeDifference(const TTimeStamp &a, const TTimeStamp &b) {
    return (a.GetSec() - b.GetSec()) * TIME_UTIL_BILLION + (a.GetNanoSec() - b.GetNanoSec());
  }
}

#endif
