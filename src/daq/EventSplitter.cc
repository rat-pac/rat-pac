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
        
        double totalQ    = 0.0;
        double calibQ    = 0.0;
        double time, charge;
        int    subevent  = 0;
        double timeDiff;
        bool subEventFound   = 0.0;
        
        Double_t chargeTmp[200];
        Double_t timeTmp[200];
//        Double_t timeMin[200];
//        Double_t timeMax[200];
        

        //        Double_t pidTmp[200];
        
        
        //Reset the container of the PMT information
        timeVec.erase   (timeVec.begin()  ,timeVec.end());
        chargeVec.erase (chargeVec.begin(),chargeVec.end());
        pmtIDVec.erase  (pmtIDVec.begin() ,pmtIDVec.end());
        
        //Fill the containers of the PMT information
        for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
            DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
            int pmtID = mcpmt->GetID();
            if (mcpmt->GetMCPhotonCount() > 0) {
                time                = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                charge              = 0;
                subevent            = 0;
                timeTmp[subevent]   = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                chargeTmp[subevent] = mcpmt->GetMCPhoton(0)->GetCharge();

                
                if(mcpmt->GetMCPhotonCount()==1){
                    time                = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                    charge              = mcpmt->GetMCPhoton(0)->GetCharge();
                    timeTmp[subevent]   = time;
                    chargeTmp[subevent] = charge;
                    
                    chargeVec.push_back(chargeTmp[subevent]);
                    timeVec.push_back(timeTmp[subevent]);
                    pmtIDVec.push_back(pmtID);
                    chargeTmp[subevent] = timeTmp[subevent] = 0;
                    
                }
                
                if(mcpmt->GetMCPhotonCount()>1){
                    for (int i=0; i < mcpmt->GetMCPhotonCount(); i++)  {
                        
                        subEventFound = 0;
                        for (int k = 0; k<=subevent; k++) {
                            timeDiff  = timeTmp[k] - mcpmt->GetMCPhoton(i)->GetFrontEndTime();
                            if (    timeDiff > 0  &&  abs(timeDiff) < collectionWindow){
                                timeTmp[k]   = mcpmt->GetMCPhoton(i)->GetFrontEndTime();//end if
                                chargeTmp[k] += mcpmt->GetMCPhoton(i)->GetCharge();
//                                if (timeTmp[k]<timeMin[k]) {
//                                    timeMin[k] = timeTmp[k];
//                                }
//                                if (timeTmp[k]>timeMax[k]) {
//                                    timeMax[k] = timeTmp[k];
//                                }
//                                std::printf("(1) a PMT hit %d %f!\n",pmtID,timeTmp[subevent]);
                                subEventFound = 1;
                                
                            }else if(timeDiff < 0 && abs(timeDiff) < collectionWindow){
                                timeTmp[k]   = timeTmp[k];//Wrong here
                                chargeTmp[k] += mcpmt->GetMCPhoton(i)->GetCharge();
//                                std::printf("(2) a PMT hit %d %f %f!\n",pmtID,timeTmp[subevent],mcpmt->GetMCPhoton(i)->GetFrontEndTime());

                                subEventFound = 1;
                            }
                        }
                        if(subEventFound == 0){
                            subevent++;
                            timeTmp[subevent]   = mcpmt->GetMCPhoton(i)->GetFrontEndTime();//end if
                            chargeTmp[subevent] += mcpmt->GetMCPhoton(i)->GetCharge();
//                            std::printf("(3) a PMT hit %d %f %f!\n",pmtID,timeTmp[subevent],timeTmp[subevent-1]);
                        
                        }
                    }
                    for (int k = 0; k<=subevent; k++) {
                        
                        if (timeTmp[k]==0) {
                            std::printf("What what what?????\n");
                        }
                        chargeVec.push_back(chargeTmp[k]);
                        timeVec.push_back(timeTmp[k]);
                        pmtIDVec.push_back(pmtID);
                        chargeTmp[k] = timeTmp[k] = 0;
                    }
                    
                }
            }
        }
        
        
        //
        //                chargeVec.push_back(charge);
        //                timeVec.push_back(time);
        //                pmtIDVec.push_back(pmtID);
        if(timeVec.size()>nhitThresh){
            std::printf("My events contains %lu distinct PMT hits\n",timeVec.size());
        }
        std::cout << "myvector contains:";
        for (std::vector<double>::iterator it=timeVec.begin(); it!=timeVec.end(); ++it)
            std::cout << ' ' << *it;
        std::cout << '\n';
        
        DS::EV *ev = ds->AddNewEV();
        ev->SetID(fEventCounter);
        fEventCounter++;
        
        //Dangerous here, adding new event
        //ev = ds->AddNewEV();
        
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
} // namespace RAT

