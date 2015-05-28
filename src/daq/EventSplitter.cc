#include <vector>
#include <RAT/EventSplitter.hh>
#include <RAT/DB.hh>
#include <G4ThreeVector.hh>
#include <RAT/DetectorConstruction.hh>

using namespace std;

namespace RAT {

EventSplitter::EventSplitter() : Processor("eventsplitterdaq") {
  //DBLinkPtr ldaq = DB::Get()->GetLink("DAQ");
  //fSPECharge = ldaq->GetDArray("SPE_charge"); // convert pC to gain-normalized units
  fEventCounter = 0;
}

Processor::Result EventSplitter::DSEvent(DS::Root *ds) {
  // This simple simulation assumes only tubes hit by a photon register
  // a hit, and that every MC event corresponds to one triggered event
  // The time of the PMT hit is that of the first photon.

  DS::MC *mc = ds->GetMC();
  if(ds->ExistEV()) {  // there is already a EV branch present 
    ds->PruneEV();     // remove it, otherwise we'll have multiple detector events
                       // in this physics event
                       // we really should warn the user what is taking place		    
  }
  DS::EV *ev = ds->AddNewEV();
  
  ev->SetID(fEventCounter);
  fEventCounter++;

  double totalQ = 0.0;
  double calibQ = 0.0;
  for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
      DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
      int pmtID = mcpmt->GetID();

      if (mcpmt->GetMCPhotonCount() > 0) {
        // Need at least one photon to trigger
        DS::PMT* pmt = ev->AddNewPMT();
        pmt->SetID(pmtID);

        // Create one sample, hit time is determined by first hit,
        // "infinite" charge integration time
        // WARNING: gets multiphoton effect right, but not walk correction
        // Write directly to calibrated waveform branch

        double time = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
          //need to understand this, not realistic
        double charge = 0;

        for (int i=0; i < mcpmt->GetMCPhotonCount(); i++)  {
          std::printf("%3d %3d %4d %10.2f %10.2f %5.2f\n",fEventCounter,\
                      i,pmtID,\
                      time,\
                      mcpmt->GetMCPhoton(i)->GetHitTime(),\
                      time - mcpmt->GetMCPhoton(i)->GetHitTime());
            
          if (time > mcpmt->GetMCPhoton(i)->GetHitTime())
            time = mcpmt->GetMCPhoton(i)->GetHitTime();//end if
          charge += mcpmt->GetMCPhoton(i)->GetCharge();
        }
        // I do not understand the above condition time > GetHitTime M.B.
        // Might cause issue with later pulses like michel electron or
        // neutron captures

        //pmt->SetCalibratedCharge(charge);
        totalQ += charge;

        //charge *= fSPECharge[pmtID] * 1e12; /* convert to pC */
        pmt->SetTime(time);
        pmt->SetCharge(charge);
        calibQ += charge;
    }
  }

  ev->SetTotalCharge(totalQ);
  //ev->SetCalibQ(calibQ);
  
  return Processor::OK;
}

} // namespace RAT

