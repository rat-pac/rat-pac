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

