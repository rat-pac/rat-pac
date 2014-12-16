#ifndef __RAT_PDFPMTCharge__
#define __RAT_PDFPMTCharge__

#include <RAT/PMTCharge.hh>
#include <RAT/DB.hh>

namespace RAT {

class PDFPMTCharge : public PMTCharge {
public:
  PDFPMTCharge(DBLinkPtr model);
  virtual ~PDFPMTCharge();
  
  /** Returns charge for one photoelectron. */
  virtual double PickCharge() const;

  /** Value of charge PDF at charge q (not normalized) */
  virtual double PDF(double q) const;

};

} // namespace RAT

#endif

