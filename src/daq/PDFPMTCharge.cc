#include <RAT/PDFPMTCharge.hh>

namespace RAT {

PDFPMTCharge::PDFPMTCharge(DBLinkPtr model) {

}

PDFPMTCharge::~PDFPMTCharge() {

}

/** Returns charge for one photoelectron. */
double PDFPMTCharge::PickCharge() const {
    return 0.0;
}

/** Value of charge PDF at charge q (not normalized) */
double PDFPMTCharge::PDF(double q) const {
    return 0.0;
}
  

} // namespace RAT

