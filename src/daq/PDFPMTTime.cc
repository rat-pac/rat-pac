#include <cmath>
#include <algorithm>
#include <vector>
#include <Randomize.hh>
#include <RAT/PDFPMTTime.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>

using namespace std;

namespace RAT {

PDFPMTTime::PDFPMTTime(string pmt_model) {
    DBLinkPtr model = DB::Get()->GetLink("PMTTRANSIT",pmt_model);
    
    fTime = model->GetDArray("time");
    fTimeProb = model->GetDArray("time_prob");
    fCableDelay = model->GetD("cable_delay");
    
    info << "Setting up PDF PMTTime model for ";
    if (pmt_model == "") { 
        info << "DEFAULT" << endl;
    } else {
        info << pmt_model << endl;
    }
    
    if (fTime.size() != fTimeProb.size()) 
        Log::Die("PDFPMTTime: time and probability arrays of different length");
    if (fTime.size() < 2) 
        Log::Die("PDFPMTTime: cannot define a PDF with fewer than 2 points");
        
    double integral = 0.0;
    fTimeProbCumu = vector<double>(fTime.size());
    fTimeProbCumu[0] = 0.0; 
    for (size_t i = 0; i < fTime.size()-1; i++) {
        integral += (fTime[i+1]-fTime[i])*(fTimeProb[i]+fTimeProb[i+1])/2.0; //trapazoid integration
        fTimeProbCumu[i+1] = integral;
    }
    for (size_t i = 0; i < fTime.size(); i++) {
        fTimeProb[i] /= integral;
        fTimeProbCumu[i] /= integral;
    }
    
}

PDFPMTTime::~PDFPMTTime() {}

double PDFPMTTime::PickTime(double time) const {
    double rval = G4UniformRand();
    for (size_t i = 1; i < fTime.size(); i++) {
        if (rval <= fTimeProbCumu[i]) {
            return time + fCableDelay + (rval - fTimeProbCumu[i-1])*(fTime[i]-fTime[i-1])/(fTimeProbCumu[i]-fTimeProbCumu[i-1]) + fTime[i-1]; //linear interpolation
        }
    }
    info << "PDFPMTTime::PickTime: impossible condition encountered - returning highest defined time" << endl;
    return time + fCableDelay + fTime[fTime.size()-1];
}
  
} // namespace RAT

