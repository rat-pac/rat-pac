#include <vector>
#include <RAT/WatchmanDAQProc.hh>
#include <RAT/DB.hh>
#include <G4ThreeVector.hh>
#include <RAT/DetectorConstruction.hh>

using namespace std;

namespace RAT {

WatchmanDAQProc::WatchmanDAQProc() : Processor("watchmandaq") {
  fEventCounter = 0;
}

Processor::Result WatchmanDAQProc::DSEvent(DS::Root *ds) {
  // For now this is basically a clone of SimpleDAQ but with frontend time instead
  // of raw monte-carlo photon hit time.

  DS::MC *mc = ds->GetMC();
  if(ds->ExistEV()) {
    ds->PruneEV();		    
  }
  DS::EV *ev = ds->AddNewEV();
  
  ev->SetID(fEventCounter);
  fEventCounter++;

  double totalQ = 0.0;
  for (int imcpmt = 0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
      DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
      int pmtID = mcpmt->GetID();

      if (mcpmt->GetMCPhotonCount() > 0) {
        // Need at least one photon to trigger
        DS::PMT* pmt = ev->AddNewPMT();
        pmt->SetID(pmtID);

        // Create one sample, hit time is determined by first hit time with 
        // transit time spread (frontend time), "infinite" charge integration time
        double time = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
        double charge = 0;

        for (int i=0; i < mcpmt->GetMCPhotonCount(); i++)  {
          if (time > mcpmt->GetMCPhoton(i)->GetFrontEndTime())
            time = mcpmt->GetMCPhoton(i)->GetFrontEndTime();
          charge += mcpmt->GetMCPhoton(i)->GetCharge();
        }
        
        totalQ += charge;

        pmt->SetTime(time);
        pmt->SetCharge(charge);
    }
  }

  ev->SetTotalCharge(totalQ);
  
  return Processor::OK;
}

} // namespace RAT

