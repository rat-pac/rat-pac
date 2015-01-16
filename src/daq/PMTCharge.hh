///////////////////////////////////////////////////////////////////////////////
/// \class RAT::PMTCharge
///
/// \brief  Pure virtual class for simulating PMT charge collection.
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Wraps various statistical methods of determining the charge 
///          collected by a PMT due to a single photoelectron. New methods
///          should extend this class, however there is not yet a system
///          in place for selecting a new method (see Gsim).
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_PMTCharge__
#define __RAT_PMTCharge__

namespace RAT {

class PMTCharge {
public:
  PMTCharge() { }
  virtual ~PMTCharge() { }
  /** Returns charge for one photoelectron. */
  virtual double PickCharge() const = 0;

  /** Value of charge PDF at charge q (not normalized) */
  virtual double PDF(double q) const = 0;

};

} // namespace RAT

#endif

