#include <vector>
#include <RAT/LessSimpleDAQProc.hh>
#include <RAT/DB.hh>
#include <G4ThreeVector.hh>
#include <RAT/DetectorConstruction.hh>

using namespace std;

namespace RAT {
    
    LessSimpleDAQProc::LessSimpleDAQProc() : Processor("lesssimpledaq") {
        //DBLinkPtr ldaq = DB::Get()->GetLink("DAQ");
        //fSPECharge = ldaq->GetDArray("SPE_charge"); // convert pC to gain-normalized units
        fEventCounter = 0;
    }
    
    Processor::Result LessSimpleDAQProc::DSEvent(DS::Root *ds) {
        // This simple simulation assumes only tubes hit by a photon register
        // a hit, and that every MC event corresponds to one triggered event
        // The time of the PMT hit is that of the first photon.
        
        DS::MC *mc = ds->GetMC();
        if(ds->ExistEV()) {  // there is already a EV branch present
            ds->PruneEV();     // remove it, otherwise we'll have multiple detector events
            // in this physics event ** we really should warn the user what is taking place
        }
        
        double totalQ = 0.0;
        double pmtQ = 0.0;
        double time,timeTmp;
        int nSubEvents = 0;
        double timeWindow  = 400., oldGroup;
        
        
        vector <double> timeAndChargeAndID;
        vector<vector <double> > pmtARRAY;
        //Place the time and charge of a PMT into an matrix
        for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
            DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
            if (mcpmt->GetMCPhotonCount() > 0) {
                for (int i=0; i < mcpmt->GetMCPhotonCount(); i++)  {
                    timeAndChargeAndID.push_back(mcpmt->GetMCPhoton(i)->GetHitTime());
                    timeAndChargeAndID.push_back(mcpmt->GetMCPhoton(i)->GetCharge());
                    timeAndChargeAndID.push_back(mcpmt->GetID());
                    timeAndChargeAndID.push_back(i);
                    timeAndChargeAndID.push_back(mcpmt->GetMCPhotonCount());
                    timeAndChargeAndID.push_back(mcpmt->GetMCPhoton(i)->GetHitTime());
                    pmtARRAY.push_back(timeAndChargeAndID);
                    timeAndChargeAndID.resize(0);
//                    pmtQ += mcpmt->GetMCPhoton(i)->GetCharge();
//                    totalQ += mcpmt->GetMCPhoton(i)->GetCharge();
                }
            }
        }
        vector <Double_t> clusterTime;
        clusterTime.push_back(1000000000000000000);
        
        for (unsigned long pmtIndex = 0; pmtIndex < pmtARRAY.size(); pmtIndex++) {
//            printf("%10d pmt %lu : (%4.3e %4.3e %4.0f %4.0f %4.3f %4.3f)\n",fEventCounter,pmtIndex,pmtARRAY[pmtIndex][0],pmtARRAY[pmtIndex][1],pmtARRAY[pmtIndex][2],pmtARRAY[pmtIndex][3],pmtARRAY[pmtIndex][4],pmtARRAY[pmtIndex][5]);
            
            time = pmtARRAY[pmtIndex][0];
            oldGroup = 0;
            timeTmp = 0 ;
            for (unsigned long jj = 0; jj< clusterTime.size();jj++){
                if(fabs(time-clusterTime[jj]) < timeWindow){
                    oldGroup+=1;
                }else{
                    timeTmp = time;
                }
            }
            if (oldGroup==0) {
                if(nSubEvents ==0){
                    clusterTime.pop_back();
                    clusterTime.push_back(timeTmp);
                }else{
                    clusterTime.push_back(timeTmp);
                }
                //                h2->Fill(timeTmp/1e3);
//                printf("%lu %f %f %d %lu %3.1f\n",pmtIndex,time, timeTmp,nSubEvents,clusterTime.size(),clusterTime[nSubEvents]);
                nSubEvents+=1;
            }
        }
        std::sort(clusterTime.begin(), clusterTime.end());
        
        timeTmp = 0.;
        int oldID = -1;
        for (int kk = 0; kk<nSubEvents; kk++) {
            DS::EV *ev = ds->AddNewEV();
            DS::PMT* pmt;
            ev->SetDeltaT(clusterTime[kk]);
            ev->SetID(kk);//fEventCounter
//            ev->SetUniqueID(fEventCounter);
            fEventCounter+=1;
            oldID = -1;
            totalQ = 0.0;
            pmtQ = 0.0;
            for (unsigned long pmtIndex = 0; pmtIndex < pmtARRAY.size(); pmtIndex++) {
                time = pmtARRAY[pmtIndex][0];
                if (fabs(time-clusterTime[kk]) <400) {
                    if (pmtARRAY[pmtIndex][2] != oldID){
                        timeTmp = time;
                        pmt = ev->AddNewPMT();
                        pmt->SetID(int(pmtARRAY[pmtIndex][2]));
                        pmtQ = 0.0;
                    }
                    
                    pmtQ += pmtARRAY[pmtIndex][1];
                    totalQ += pmtARRAY[pmtIndex][1];
                    //Set an offset of 200 ns for the PMT time and have a relative PMT time
                    pmt->SetTime(timeTmp-clusterTime[kk]+200);
                    pmt->SetCharge(pmtQ);
                    oldID = pmtARRAY[pmtIndex][2];

                }
            }
            ev->SetTotalCharge(totalQ);
        }
        
        
        
        //        for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
        //
        //            DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
        ////MB            int pmtID = mcpmt->GetID();
        //
        //            if (mcpmt->GetMCPhotonCount() > 0) {
        //                // Need at least one photon to trigger
        ////MB                DS::PMT* pmt = ev->AddNewPMT();
        ////MB                pmt->SetID(pmtID);
        //
        //                // Create one sample, hit time is determined by first hit,
        //                // "infinite" charge integration time
        //                // WARNING: gets multiphoton effect right, but not walk correction
        //                // Write directly to calibrated waveform branch
        //
        //                double time = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
        //                double charge = 0;
        //
        //                for (int i=0; i < mcpmt->GetMCPhotonCount(); i++)  {
        //                    if (time > mcpmt->GetMCPhoton(i)->GetHitTime())
        //                        time = mcpmt->GetMCPhoton(i)->GetHitTime();
        //                    charge += mcpmt->GetMCPhoton(i)->GetCharge();
        //                }
        //
        //                //pmt->SetCalibratedCharge(charge);
        //                totalQ += charge;
        //
        //                //charge *= fSPECharge[pmtID] * 1e12; /* convert to pC */
        ////MB                pmt->SetTime(time);
        ////MB                pmt->SetCharge(charge);
        //                pmtQ += charge;
        //            }
        //        }
//        DS::EV *ev = ds->AddNewEV();
//        ev->SetID(fEventCounter);
//        fEventCounter++;
//        ev->SetTotalCharge(totalQ);
        //ev->SetpmtQ(pmtQ);
        
        return Processor::OK;
    }
    
} // namespace RAT

