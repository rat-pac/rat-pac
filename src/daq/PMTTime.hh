///////////////////////////////////////////////////////////////////////////////
/// \class RAT::PMTTime
///
/// \brief  Pure virtual class for simulating PMT time spread and cable delay.
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Wraps various methods for handling PMT time calculations.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_PMTTime__
#define __RAT_PMTTime__

#include <RAT/DB.hh>

namespace RAT {

class PMTTime {
public:
  PMTTime() { };
  virtual ~PMTTime() { };

  /** Returns front end time for hit time. */
  virtual double PickTime(double time) const = 0;

};

} // namespace RAT

#endif

