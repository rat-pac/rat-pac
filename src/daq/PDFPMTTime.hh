///////////////////////////////////////////////////////////////////////////////
/// \class RAT::PDFPMTCharge
///
/// \brief  Simulates PMT time spread and cable delay from DB defined PDFs.
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Automatically chooses the right table for a given PMT model, or 
///          uses default table if no model is given.
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_PDFPMTTime__
#define __RAT_PDFPMTTime__

#include <RAT/DB.hh>
#include <RAT/PMTTime.hh> 

namespace RAT {

class PDFPMTTime : public PMTTime {
public:
  PDFPMTTime(std::string pmt_model = "");
  virtual ~PDFPMTTime();

  /** Returns front end time for hit time. */
  virtual double PickTime(double time) const;

protected:
  std::vector<double> fTime,fTimeProb,fTimeProbCumu;
  double fCableDelay;
};

} // namespace RAT

#endif

