#include <math.h>
#include <RAT/FitCentroidProc.hh>

namespace RAT {

FitCentroidProc::FitCentroidProc() : Processor("fitcentroid") { 
  fPower = 2.0; 
  fRescale = 1.0;
}

void FitCentroidProc::SetD(std::string param, double value) {
  if (param == "power") {
    fPower = value;
  }
  else if (param == "rescale") {
    fRescale = value;
  }
  else {
    throw ParamUnknown(param);
  }
}


Processor::Result FitCentroidProc::Event(DS::Root* /*ds*/, DS::EV* ev) {
  double totalQ = 0;
  TVector3 centroid(0.0, 0.0, 0.0);

  for (int i=0; i < ev->GetPMTCount(); i++) {
    DS::PMT *pmt = ev->GetPMT(i);
    
    double Qpow = 0.0;
    Qpow = pow(pmt->GetCharge(), fPower);
    totalQ += Qpow;
    TVector3 pmtpos(0.0, 0.0, 0.0);
    // FIXME
    //fDetector->GetPMTPosition(pmt->GetID(), pmtpos);
    //if(fRescale != 1.0)
    //  pmtpos.SetMag(pmtpos.Mag() * fRescale);
    //centroid += Qpow * pmtpos;
  }

  centroid *= 1.0 / totalQ;
  DS::Centroid *fit = ev->GetCentroid();
  fit->SetPosition(centroid);
  fit->SetPower(fPower);

  return Processor::OK;
}

} // namespace RAT

