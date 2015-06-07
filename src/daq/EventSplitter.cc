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
        }
        
        double      totalQ    = 0.0;
//        double      calibQ    = 0.0;
        double      time, charge;
        int         subevent  = 1;
        double      timeDiff;
        bool        subPMTEventFound   = 0.0;
        int         countPMTinSubEvents = 0;
        
        Double_t chargeTmp[200];
        Double_t timeTmp[200];
        
        //Reset the container of the PMT information
        matr.erase (matr.begin() ,matr.end());
        recordSubTime.erase (recordSubTime.begin() ,recordSubTime.end());
        
        //Fill the containers of the PMT information
        for (int imcpmt=0; imcpmt < mc->GetMCPMTCount(); imcpmt++) {
            DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
            int pmtID = mcpmt->GetID();
            
            if (mcpmt->GetMCPhotonCount() > 0) {
                time                = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                charge              = 0;
                subevent            = 0;
                timeTmp[0]   = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                chargeTmp[0] = mcpmt->GetMCPhoton(0)->GetCharge();
                
                //If PMT only has one hit:
                if(mcpmt->GetMCPhotonCount()==1){
                    timeTmp[0]   = mcpmt->GetMCPhoton(0)->GetFrontEndTime();
                    chargeTmp[0] = mcpmt->GetMCPhoton(0)->GetCharge();
                    
                    Vec.erase  (Vec.begin() ,Vec.end());
                    Vec.push_back(timeTmp[subevent]);
                    Vec.push_back(chargeTmp[subevent]);
                    Vec.push_back(pmtID);
                    matr.push_back(Vec);
                    
                    chargeTmp[0] = timeTmp[0] = 0;
                }
                
                if(mcpmt->GetMCPhotonCount()>1){
                    for (int i=0; i < mcpmt->GetMCPhotonCount(); i++)  {
                        
                        subPMTEventFound = 0;
                        
                        for (int k = 0; k<subevent; k++) {
                            
                            timeDiff  = timeTmp[k] - mcpmt->GetMCPhoton(i)->GetFrontEndTime();
                            
                            if (timeDiff > 0  &&  abs(timeDiff) < collectionWindow){
                                
                                timeTmp[k]   = mcpmt->GetMCPhoton(i)->GetFrontEndTime();//end if
                                chargeTmp[k] += mcpmt->GetMCPhoton(i)->GetCharge();
                                subPMTEventFound = 1;
                                
                            }else if(timeDiff < 0 && abs(timeDiff) < collectionWindow){
                        
                                timeTmp[k]   = timeTmp[k];
                                chargeTmp[k] += mcpmt->GetMCPhoton(i)->GetCharge();
                                subPMTEventFound = 1;
                            }/*else {
                                std::printf("There is something wrong: %d %f %f %f %f.\n",\
                                            pmtID,timeTmp[k],timeDiff,abs(timeDiff),chargeTmp[k]);
                            }*/
                        }
                        if(subPMTEventFound == 0){
                            subevent++;
                            timeTmp[subevent]   = mcpmt->GetMCPhoton(i)->GetFrontEndTime();//end if
                            chargeTmp[subevent] += mcpmt->GetMCPhoton(i)->GetCharge();
                        }
                    }
                    for (int k = 0; k<=subevent; k++) {
                        
                        if (timeTmp[k]==0) {
                            std::printf("There is something wrong in the eventSplitter code.\n");
                        }
                        
                        Vec.erase  (Vec.begin() ,Vec.end());
                        Vec.push_back(timeTmp[k]);
                        Vec.push_back(chargeTmp[k]);
                        Vec.push_back(pmtID);
                        
                        matr.push_back(Vec);
                        
                        chargeTmp[k] = timeTmp[k] = 0.0;
                    }
                }
            }
        }
        //Sort the PMT information by time, from earliest to latest:
        std::sort(matr.begin(),matr.end());
        
        Double_t t_i = matr[0][0] ; // Set the first PMT hit of the event
        Double_t t_f = matr[matr.size()-1][0] + 1; //Add a second so to make sure there is
        
        subevent = 1;
        countPMTinSubEvents = 0;

        //Evaluate the number of subevents
        if ((t_f-t_i) < (collectionWindow)){
            
//            std::cout << "\nOnly one subevent " << t_i << " " << t_f << std::endl;
            subevent = 1;
            
            Vec.erase  (Vec.begin() ,Vec.end());
            Vec.push_back(subevent);
            Vec.push_back(t_i);
            Vec.push_back(matr.size());
            
            if(matr.size()>=nhitThresh){
                recordSubTime.push_back(Vec);
            }
            
        }else{
//            std::cout << "\nPotentially more than one subevent " << t_i << " " << t_f << std::endl;
            
            for ( std::vector<std::vector<double> >::size_type i = 0; i < matr.size(); i++ )
            {
                //                std::printf("%f %d %d\n",matr[i][0] -t_i, subevent,countPMTinSubEvents);
                if ( matr[i][0] < t_i +int(collectionWindow)){
                    countPMTinSubEvents++;
                }else{
                    // std::printf("Number of PMT in subevent %d has %d PMT at time %5.3f\n",subevent,countPMTinSubEvents,t_i);
                    Vec.erase  (Vec.begin() ,Vec.end());
                    Vec.push_back(subevent);
                    Vec.push_back(t_i);
                    Vec.push_back(countPMTinSubEvents);
                    
                    if(countPMTinSubEvents>=nhitThresh){
                        recordSubTime.push_back(Vec);
                    }
                    countPMTinSubEvents = 1;
                    subevent++;
                    t_i =matr[i][0];
                    
                }
                
            }
            //std::printf("Number of PMT in subevent %d has %d PMT at time %5.3f\n",subevent,countPMTinSubEvents,t_i);
            
            Vec.erase  (Vec.begin() ,Vec.end());
            Vec.push_back(subevent);
            Vec.push_back(t_i);
            Vec.push_back(countPMTinSubEvents);
            
            recordSubTime.push_back(Vec);
            countPMTinSubEvents = 0;
            
        }
        
        // Loop through found sub events and assign PMTs and evaluate charge.
        for ( std::vector<std::vector<double> >::size_type i = 0; i < recordSubTime.size(); i++ ){
            
            DS::EV *ev = ds->AddNewEV();
            
            ev->SetTotalSubEV(recordSubTime.size());
            ev->SetSubEV(int(i));
            ev->SetID(fEventCounter);
            ev->SetSubTriggerTime(recordSubTime[i][1]);
            
            fEventCounter++;
            totalQ = 0.0;
            
            for ( std::vector<std::vector<double> >::size_type j = 0; j < matr.size(); j++ ){
                
                if (matr[j][0] >= recordSubTime[i][1] && matr[j][0] < recordSubTime[i][1] + collectionWindow ) {
                // Prints out the PMTs and
                //    std::printf("%f %f %d %d %f %f\n",matr[j][0],recordSubTime[i][1],i,j,matr[j][1],matr[j][2]);
                    DS::PMT* pmt = ev->AddNewPMT();
                    pmt->SetID(int(matr[j][2]));
                    pmt->SetTime(matr[j][0]-recordSubTime[i][1]+offsetToWindow);
                    pmt->SetCharge(matr[j][1]);
                    totalQ+=matr[j][1];
                }
            }
            ev->SetTotalCharge(totalQ);
        }
        
        return Processor::OK;
    }// end of Processor::Result EventSplitter::DSEvent
    
    
    void EventSplitter::SetI(std::string param, int value)
    {
        if (param == "offsetToTimeWindowNanoSec"){
            offsetToWindow = double(value);
            std::printf("Offset to time window: %4.1f ns\n",offsetToWindow);
        }else if (param == "nhitThreshold"){
            nhitThresh = value;
            std::printf("Nhit Threshold: %9.0f hit\n",nhitThresh);
        }else if (param == "collectionTimeWindowNanoSec"){
            collectionWindow = value;
            std::printf("Collection window: %7.1f ns\n",collectionWindow);
        }// End of EventSplitter::SetI
    }
} // namespace RAT

