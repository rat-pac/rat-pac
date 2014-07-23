#include <cmath>
#include <string>
#include <TVector3.h>
#include <RAT/DS/Centroid.hh>
#include <RAT/DS/EV.hh>
#include <RAT/DS/PMT.hh>
#include <RAT/DS/Root.hh>
#include <RAT/DS/Run.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/Processor.hh>
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


Processor::Result FitCentroidProc::Event(DS::Root* ds, DS::EV* ev) {
  double totalQ = 0;
  TVector3 centroid(0.0, 0.0, 0.0);

  for (int i=0; i < ev->GetPMTCount(); i++) {
    DS::PMT *pmt = ev->GetPMT(i);
    
    double Qpow = 0.0;
    Qpow = pow(pmt->GetCharge(), fPower);
    totalQ += Qpow;

    DS::Run* run = DS::RunStore::Get()->GetRun(ds);
    DS::PMTInfo* pmtinfo = run->GetPMTInfo();
    TVector3 pmtpos = pmtinfo->GetPosition(pmt->GetID());

    if (fRescale != 1.0) {
      pmtpos.SetMag(pmtpos.Mag() * fRescale);
    }

    centroid += Qpow * pmtpos;
  }

  centroid *= 1.0 / totalQ;
  DS::Centroid* fit = ev->GetCentroid();
  fit->SetPosition(centroid);
  fit->SetPower(fPower);

  return Processor::OK;
}

} // namespace RAT

