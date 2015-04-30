
#include <Randomize.hh>
#include <RAT/PDFPMTCharge.hh>
#include <RAT/Log.hh>

using namespace std;

namespace RAT {

PDFPMTCharge::PDFPMTCharge(string pmt_model) {
    DBLinkPtr model = DB::Get()->GetLink("PMTCHARGE",pmt_model);
    
    fCharge = model->GetDArray("charge");
    fChargeProb = model->GetDArray("charge_prob");
    
    info << "Setting up PDF PMTCharge model for ";
    if (pmt_model == "") { 
        info << "DEFAULT" << endl;
    } else {
        info << pmt_model << endl;
    }
    
    if (fCharge.size() != fChargeProb.size()) 
        Log::Die("PDFPMTCharge: charge and probability arrays of different length");
    if (fCharge.size() < 2) 
        Log::Die("PDFPMTCharge: cannot define a PDF with fewer than 2 points");
        
    double integral = 0.0;
    fChargeProbCumu = vector<double>(fCharge.size());
    fChargeProbCumu[0] = 0.0; 
    for (size_t i = 0; i < fCharge.size()-1; i++) {
        integral += (fCharge[i+1]-fCharge[i])*(fChargeProb[i]+fChargeProb[i+1])/2.0; //trapazoid integration
        fChargeProbCumu[i+1] = integral;
    }
    for (size_t i = 0; i < fCharge.size(); i++) {
        fChargeProb[i] /= integral;
        fChargeProbCumu[i] /= integral;
    }
}

PDFPMTCharge::~PDFPMTCharge() {

}

/** Returns charge for one photoelectron. */
double PDFPMTCharge::PickCharge() const {
    double rval = G4UniformRand();
    for (size_t i = 1; i < fCharge.size(); i++) {
        if (rval <= fChargeProbCumu[i]) {
            return (rval - fChargeProbCumu[i-1])*(fCharge[i]-fCharge[i-1])/(fChargeProbCumu[i]-fChargeProbCumu[i-1]) + fCharge[i-1]; //linear interpolation
        }
    }
    info << "PDFPMTCharge::PickCharge: impossible condition encountered - returning highest defined charge" << endl;
    return fCharge[fCharge.size()-1];
}

/** Value of charge PDF at charge q */
double PDFPMTCharge::PDF(double q) const {
    if (q < fCharge[0]) return 0.0; //below defined range
    for (size_t i = 1; i < fCharge.size(); i++) {
        if (q <= fCharge[i]) {
            return (q - fCharge[i-1])*(fChargeProb[i]-fChargeProb[i-1])/(fCharge[i]-fCharge[i-1]); //linear interpolation
        }
    }
    return 0.0; //above defined range
}
  

} // namespace RAT

