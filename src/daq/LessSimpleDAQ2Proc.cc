//NOTE: Unlike LessSimpleDAQ, here in LessSimpleDaq2 ...
//   pmt->GetTime() will give the front end time w.r.t the global time (t = 0 is when the event is first generated)
//   ev->GetDeltaT() will give the time BETWEEN the subevents. For example,
//      for the first subevent, the DeltaT is the dt between t = 0 (event is generated) and cluster time of the first subevent
//      for the second subevent, the DeltaT is the dt between the cluster time of the first subevent and the cluster time of the second subevent
//      for the third subevent, the DeltaT is the dt between the cluster time of the second subevent and the cluster time of the third subevent
//      and so on...
//  the cluster time is taken as the earliest time among the PMTs

#include <vector>
#include <RAT/LessSimpleDAQ2Proc.hh>
#include <RAT/DB.hh>
#include <G4ThreeVector.hh>
#include <RAT/DetectorConstruction.hh>
#include <iostream>
using namespace std;

namespace RAT {
    
    //always intialize the event count to 0 for every triggered event
    LessSimpleDAQ2Proc::LessSimpleDAQ2Proc() : Processor("lesssimpledaq2") { fEventCounter = 0; }
    
    Processor::Result LessSimpleDAQ2Proc::DSEvent(DS::Root *ds) {
        
        //remove any existing branch to avoid having multiple detectors in this event
        DS::MC *mc = ds->GetMC();  if(  ds->ExistEV()  ) {  ds->PruneEV();  }
        
        //declare several vectors
        vector <int>    iArray , iArraySort;  //index
        vector <double> tArray , tArraySort;  //front end time
        vector <double> qArray , qArraySort;  //charge
        vector <double> idArray, idArraySort; //PMT ID
        vector <double> phArray, phArraySort; //photon count ...we are not using this...
        vector <double> htArray, htArraySort; //hit time     ...we are not using this either...
        
        //get the information for 1 full event and put them in vectors
        for ( int imcpmt = 0; imcpmt < mc->GetMCPMTCount() ; imcpmt++ ) {
            
            DS::MCPMT *mcpmt = mc->GetMCPMT(imcpmt);
            
            if ( mcpmt->GetMCPhotonCount() > 0 ) {
                
                for ( int i = 0 ; i < mcpmt->GetMCPhotonCount() ; i++ )  {
                    
                    tArray.push_back(mcpmt->GetMCPhoton(i)->GetFrontEndTime());
                    tArraySort.push_back(mcpmt->GetMCPhoton(i)->GetFrontEndTime());
                    qArray.push_back(mcpmt->GetMCPhoton(i)->GetCharge());
                    idArray.push_back(mcpmt->GetID());
                    iArray.push_back(i);
                    phArray.push_back(mcpmt->GetMCPhotonCount());
                    htArray.push_back(mcpmt->GetMCPhoton(i)->GetHitTime());
                }
            }
        }
        
        //then sort these events based on the front end time
        sort(  tArraySort.begin() , tArraySort.end()  );
        
        //create iArraySort to keep track of the indexes
        for( unsigned long q = 0 ; q < tArraySort.size() ; q++ ){
            for( unsigned long qq = 0 ; qq < tArray.size() ; qq++ ){
                if(tArraySort[q] == tArray[qq]){  iArraySort.push_back(  iArray[qq]  );  }
            }
        }
        
        //sort the rest of the vectors
        for( unsigned long m = 0 ; m < tArraySort.size() ; m++ ){
            qArraySort.push_back   (   qArray [  iArraySort[m]  ]   );
            idArraySort.push_back  (   idArray[  iArraySort[m]  ]   );
            phArraySort.push_back  (   phArray[  iArraySort[m]  ]   );
            htArraySort.push_back  (   htArray[  iArraySort[m]  ]   );
        }
        
        //if there is at least 1 event ...
        if(  tArraySort.size()  >  0  ){
            
            //group these events and get bunch of subevents. the cluster time will be the earliest time among the PMTs.
            double                  timeWindow  = 400.0;
            int                     numSub      = 0;
            double                  timeNow     = tArraySort[0];
            vector <int>            subIndex;
            vector <double>         clusterTime; clusterTime.push_back(  timeNow  );
            vector <unsigned long>  startIndex ; startIndex.push_back (  0        );
            
            for( unsigned long a = 0 ; a<tArraySort.size() ; a++ ){
                if( tArraySort[a] - timeNow   <  timeWindow ){
                    subIndex.push_back   (  numSub   );
                }
                else{
                    timeNow = tArraySort[a];
                    numSub++;
                    subIndex.push_back   (  numSub   );
                    clusterTime.push_back(  timeNow  );
                    startIndex.push_back (  a        );
                }
            }
            
            //ok, we have the number of subevents now (numSub+1). Let's set things up!
            double qSum = 0.0;
            for( int b = 0; b < numSub+1 ; b++ ){
                
                DS::EV *ev = ds->AddNewEV();   DS::PMT* pmt;  fEventCounter+=1;
                
                if( b > 0 ){  ev->SetDeltaT (  clusterTime[b] - clusterTime[b-1]  );  }
                else       {  ev->SetDeltaT (  clusterTime[0]                     );  }
                ev->SetCalibratedTriggerTime(  clusterTime[b]                     );
                ev->SetID                   (  subIndex[b]                        );
                
                qSum   = 0.0;
                
                for( unsigned long bb = startIndex[b] ; bb < tArraySort.size() ; bb++ ){
                    if  ( subIndex[bb] == b   ){
                        
                        pmt = ev->AddNewPMT();
                        pmt->SetID    (  idArraySort[bb]   );
                        pmt->SetTime  (  tArraySort [bb]   );
                        pmt->SetCharge(  qArraySort [bb]   );
                        
                        qSum += qArraySort[bb];
                    }
                    else{
                        break;
                    }
                }
                ev->SetTotalCharge(   qSum   );
            }
        }
        return Processor::OK;
    }
}
