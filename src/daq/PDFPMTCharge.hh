///////////////////////////////////////////////////////////////////////////////
/// \class RAT::PDFPMTCharge
///
/// \brief  Implementation of PMTCharge using database defined charge PDFs.
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Enabled by setting MC.pmt_charge_model to the index of a table 
///          named PMTCHARGE. This table contains the charge PDF (which will be 
///          integrated and normalized) with charge values in the field `charge`
///          and corresponding probabilities in the field `charge_prob`. A 
///          linear interpolation is done between discrete charge values, so the
///          `charge` vector must be ordered by ascending charge.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_PDFPMTCharge__
#define __RAT_PDFPMTCharge__

#include <RAT/PMTCharge.hh>
#include <RAT/DB.hh>
#include <vector>

namespace RAT {

class PDFPMTCharge : public PMTCharge {

public:
    PDFPMTCharge(std::string pmt_model = "");
    virtual ~PDFPMTCharge();

    /** Returns charge for one photoelectron. */
    virtual double PickCharge() const;

    /** Value of charge PDF at charge q (not normalized) */
    virtual double PDF(double q) const;

protected:
    std::vector<double> fCharge,fChargeProb,fChargeProbCumu;

};

} // namespace RAT

#endif

