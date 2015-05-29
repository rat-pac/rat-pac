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
        double time, charge;
        
        //Get min and max time in the event
        timeVec.erase   (timeVec.begin()  ,timeVec.end());
        chargeVec.erase (chargeVec.begin(),chargeVec.end());
        pmtIDVec.erase  (pmtIDVec.begin() ,pmtIDVec.end());

        for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
            DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
            int pmtID = mcpmt->GetID();
            
            if (mcpmt->GetMCPhotonCount() > 0) {
                time = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                charge = 0;
                
                if(mcpmt->GetMCPhotonCount()==1){
                    time   = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                    charge = mcpmt->GetMCPhoton(0)->GetCharge();
                }
                if(mcpmt->GetMCPhotonCount()>1){
                    for (int i=1; i < mcpmt->GetMCPhotonCount(); i++)  {
                        if (time > mcpmt->GetMCPhoton(i)->GetFrontEndTime())
                            time = mcpmt->GetMCPhoton(i)->GetFrontEndTime();//end if
                        charge += mcpmt->GetMCPhoton(i)->GetCharge();
                    }
                }
                
                chargeVec.push_back(charge);
                timeVec.push_back(time);
                pmtIDVec.push_back(pmtID);
                
            }
        }
       // double maxTime = max_element(timeVec.begin(),timeVec.end());
        
        std::printf("My events contains %lu distinct PMT hits\n",timeVec.size());
        
        
        //Dangerous here, adding new event
        ev = ds->AddNewEV();
        
        ev->SetID(fEventCounter);
        fEventCounter++;
        
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
                
                time = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                //need to understand this, not realistic
                charge = 0;
                
                if(mcpmt->GetMCPhotonCount()==1){
                    time   = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                    charge = mcpmt->GetMCPhoton(0)->GetCharge();
                    //          std::printf("%3d %3d %4d %10.2f %10.2f %5.2f\n",fEventCounter,\
                    //                        0,pmtID,\
                    //                        time,\
                    //                        mcpmt->GetMCPhoton(0)->GetHitTime(),\
                    //                        time - mcpmt->GetMCPhoton(0)->GetHitTime());
                }
                if(mcpmt->GetMCPhotonCount()>1){
                    for (int i=1; i < mcpmt->GetMCPhotonCount(); i++)  {
                        if (time > mcpmt->GetMCPhoton(i)->GetFrontEndTime())
                            time = mcpmt->GetMCPhoton(i)->GetFrontEndTime();//end if
                        charge += mcpmt->GetMCPhoton(i)->GetCharge();
                    }
                }
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
    
    
    void EventSplitter::SetI(std::string param, int value)
    {
        if (param == "clockSpeedMHz") {
            clockSpeed= double(value);
            std::printf("Clock speed: %13.1f MHz\n",clockSpeed);
        }else if (param == "slidingTimeWindowNanoSec"){
            slidingWindow = double(value);
            std::printf("Sliding time window: %4.1f ns\n",slidingWindow);
        }else if (param == "nhitThreshold"){
            nhitThresh = value;
            std::printf("Nhit Threshold: %9.0f hit\n",nhitThresh);
        }else if (param == "collectionTimeWindowNanoSec"){
            collectionWindow = value;
            std::printf("Collection window: %7.1f ns\n",collectionWindow);
        }
        
        
    }
//    
//    template <class ForwardIterator>
//    ForwardIterator max_element ( ForwardIterator first, ForwardIterator last )
//    {
//        if (first==last) return last;
//        ForwardIterator largest = first;
//        
//        while (++first!=last)
//            if (*largest<*first)    // or: if (comp(*largest,*first)) for version (2)
//                largest=first;
//        return largest;
//    }
//    
} // namespace RAT

