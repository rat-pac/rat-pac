// Extract the tracks from a file using root command prompts
// example of use. The command
// >  root trackExtraction.C\(\"/Users/marcbergevin/RAT_ROOT/output.root\"\)
// Will apply this routine of the output.root file
// M.B
#include <iostream>
#include <iomanip>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

void supernovaAnalysis(const char *file) {
    Double_t reconstructedRadius = 0.0;
    
    TH1D *hPos0FB = new TH1D("hPos0FB","primary event",1000,0.01,10);
    hPos0FB->SetLineColor(4);
    TH1D *hPos1FB = new TH1D("hPos1FB","secondary event",1000,0.01,10);
    hPos1FB->SetLineColor(kRed+3);
    
    hPos0FB->SetXTitle("r_{bonsai}(m)");
    hPos0FB->SetYTitle("Counts");
    //
    
    static Int_t nbins 	= 100;
    Double_t xbins[100]; //Needs to be the same number as above
    
    Double_t xmin 	= 1e-2;
    Double_t xmax 	= 20;
    Double_t logxmin = log10(xmin);
    Double_t logxmax = log10(xmax);
    Double_t binwidth= (logxmax-logxmin)/double(nbins);
    xbins[0] = xmin;
    for(Int_t i=0; i<nbins;i++){
        xbins[i] = xmin + pow(10,logxmin+i*binwidth);
        //        printf("%6.3f\n",xbins[i]);
    }
    
    TH1D *hPos0FBLOG = new TH1D("hPos0FBLOG","primary event",nbins-1,xbins);
    hPos0FBLOG->SetLineColor(4);
    TH1D *hPos1FBLOG = new TH1D("hPos1FBLOG","secondary event",nbins-1,xbins);
    hPos1FBLOG->SetLineColor(kRed+3);
    
    hPos0FBLOG->SetXTitle(" 3d vertex r_{bonsai} [m]");
    hPos0FBLOG->SetYTitle("Counts");
    
    TH1D *hPhotoelectron0 = new TH1D("hPhotoelectron0","primary event",600,0,600);
    hPhotoelectron0->SetLineColor(4);
    TH1D *hPhotoelectron1 = new TH1D("hPhotoelectron1","secondary event",600,0,600);
    hPhotoelectron1->SetLineColor(kRed+3);
    hPhotoelectron0->SetXTitle("photoelectrons");
    hPhotoelectron0->SetYTitle("Counts");
    
    TH1D *hCos0FB = new TH1D("hCos0FB","primary event",1000,-1.05,1.05);
    hCos0FB->SetLineColor(4);
    TH1D *hCos1FB = new TH1D("hCos1FB","secondary event",1000,-1.05,1.05);
    hCos1FB->SetLineColor(kRed+3);
    hCos0FB->SetXTitle("cos #theta_{reconstruction}");
    hCos0FB->SetYTitle("Counts");
    
    TH1D *hCos0FB_SN = new TH1D("hCos0FB_SN","primary event",1000,-1.05,1.05);
    hCos0FB_SN->SetLineColor(4);
    TH1D *hCos1FB_SN = new TH1D("hCos1FB_SN","secondary event",1000,-1.05,1.05);
    hCos1FB_SN->SetLineColor(kRed+3);
    hCos0FB_SN->SetXTitle("cos #theta_{SN}");
    hCos0FB_SN->SetYTitle("Counts");
    
    TH1D *hNuE = new TH1D("hNuE","neutrino energy",1000,0,100);
    hNuE->SetLineColor(4);
    hNuE->SetXTitle("neutrino energy (MeV)");
    hNuE->SetYTitle("Counts");
    
    TH1D *hNuP = new TH1D("hNuP","particle energy (e^{#pm} or #gamma)",1000,0,100);
    hNuP->SetLineColor(4);
    hNuP->SetXTitle("Cherenkov-inducing particle Energy (MeV)");
    hNuP->SetYTitle("Counts");
    
    
    TFile *f = new TFile(file);
    TTree *tree = (TTree*) f->Get("T");
    
    TFile *f_out = new TFile(Form("ntuple_%s",f->GetName()),"Recreate");
    //    TNtuple* data = new TNtuple("data","Ntuple for Watchman Reconstruction Studies",
    //                                "pe:r_bonsai_true:cosTheta:cosThetaSN:local_time_ns:sub_ev:sub_ev_cnt:interaction");
    
    RAT::DS::Root *rds = new RAT::DS::Root();
    tree->SetBranchAddress("ds", &rds);
    
    int nEvents = tree->GetEntries();
    
    TCanvas *c1 = new TCanvas("c1","Detector Reconstruction Information",1200,800);
    c1->Divide(2,2);
    
    TCanvas *c2 = new TCanvas("c2","MC truth",1200,800);
    c2->Divide(2,2);
    
    
    Double_t totPE = 0.0,totMom,goodness,dirGoodness,qTmp,timeTmp,timeTmp1,oldX,oldY,oldZ;
    Double_t totQB = 0.0, q2 = 0.0, pmtCount = 0.0,reconstructedRadiusFC,reconstructedRadiusFP,reconstructedRadiusFB, reconstructedRadiusFPMinusFB;
    Int_t ibd=0,es=0,cc=0,icc=0,nc=0,old_singal;
    Double_t cosTheta,cosThetaSN,cosThetaSNIBD, local_time,mc_nu_energy,mc_energy;
    int subEvNumber=0;
    Int_t subevents = 0,cnt,cntLoop;
    Int_t single[40],_single,prev_single;
    Double_t timeDiff[40];
    
    Int_t interaction_type;
    TVector3 mcmomv_nu, mcmomv_particle;
    
    TTree *data = new TTree("data","supernova events");
    
    TVector3 posTruth,posReco,dirTruth,dirNu,dirReco,dirIBD,pos1,pos2;
    
    data->Branch("pe",&totPE,"pe/D");
    data->Branch("r_bonsai_true",&reconstructedRadiusFB,"r_bonsai_true/D");
    data->Branch("cosTheta",&cosTheta,"cosTheta/D");
    data->Branch("cosThetaSN",&cosThetaSN,"cosThetaSN/D");
    data->Branch("cosThetaSNIBD",&cosThetaSNIBD,"cosThetaSNIBD/D");

    
    data->Branch("local_time_ns",&local_time,"local_time_ns/D");
    data->Branch("sub_ev",&subEvNumber,"sub_ev/I");
    data->Branch("sub_ev_cnt",&cnt,"sub_ev_cnt/I");
    data->Branch("single",&_single,"single/I");
    data->Branch("interaction",&interaction_type,"interaction/I");
    data->Branch("mc_nu_energy",&mc_nu_energy,"mc_nu_energy/D");
    data->Branch("mc_energy",&mc_energy,"mc_energy/D");
    
    data->Branch("pos_goodness",&goodness,"pos_goodness/D");
    data->Branch("posReco","TVector3",&posReco,32000,0);
    data->Branch("posTruth","TVector3",&posTruth,32000,0);
    
    data->Branch("dir_goodness",&dirGoodness,"dir_goodness/D");
    data->Branch("dirReco","TVector3",&dirReco,32000,0);
    data->Branch("dirIBD","TVector3",&dirIBD,32000,0);
    data->Branch("dirTruth","TVector3",&dirTruth,32000,0);
    data->Branch("dirNu","TVector3",&dirNu,32000,0);
    
    Int_t ES_true, IBD_true,CC_true,ICC_true,NC_true;
    
    Int_t totSingles, totMultiples, ES_cnt, IBD_cnt,CC_cnt,ICC_cnt,NC_cnt;
    Double_t ES_ratio, IBD_ratio,CC_ratio,ICC_ratio,NC_ratio;
    TTree *summary = new TTree("summary","supernova summary");
    
    Int_t ES_cnt_s, IBD_cnt_s,CC_cnt_s,ICC_cnt_s,NC_cnt_s;
    Double_t ES_ratio_s, IBD_ratio_s,CC_ratio_s,ICC_ratio_s,NC_ratio_s;
    
    summary->Branch("evts_that_are_singles",&totSingles,"evts_that_are_singles/I" );
    summary->Branch("evts_that_are_multiples",&totMultiples,"evts_that_are_multiples/I");
    
    summary->Branch("single_evts_from_ES",&ES_cnt_s,"single_evts_from_ES/I");
    summary->Branch("single_evts_from_IBD",&IBD_cnt_s,"single_evts_from_IBD/I");
    summary->Branch("single_evts_from_CC",&CC_cnt_s,"single_evts_from_CC/I");
    summary->Branch("single_evts_from_ICC",&ICC_cnt_s,"single_evts_from_ICC/I");
    summary->Branch("single_evts_from_NC",&NC_cnt_s,"single_evts_from_NC/I");
    summary->Branch("single_evts_from_ratio",&ES_ratio_s,"single_evts_from_ES_ratio/D");
    summary->Branch("single_evts_from_ratio",&IBD_ratio_s,"single_evts_from_IBD_ratio/D");
    summary->Branch("single_evts_from_CC_ratio",&CC_ratio_s,"single_evts_from_CC_ratio/D");
    summary->Branch("single_evts_from_ICC_ratio",&ICC_ratio_s,"single_evts_from_ICC_ratio/D");
    summary->Branch("single_evts_from_NC_ratio",&NC_ratio_s,"single_evts_from_NC_ratio/D");
    
    summary->Branch("physevts_observed_ES",&ES_cnt,"physevts_observed_ES/I");
    summary->Branch("physevts_observed_IBD",&IBD_cnt,"physevts_observed_IBD/I");
    summary->Branch("physevts_observed_CC",&CC_cnt,"physevts_observed_CC/I");
    summary->Branch("physevts_observed_ICC",&ICC_cnt,"physevts_observed_ICC/I");
    summary->Branch("physevts_observed_NC",&NC_cnt,"physevts_observed_NC/I");
    summary->Branch("physevts_observed_ratio",&ES_ratio,"physevts_observed_ES_ratio/D");
    summary->Branch("physevts_observed_ratio",&IBD_ratio,"physevts_observed_IBD_ratio/D");
    summary->Branch("physevts_observed_CC_ratio",&CC_ratio,"physevts_observed_CC_ratio/D");
    summary->Branch("physevts_observed_ICC_ratio",&ICC_ratio,"physevts_observed_ICC_ratio/D");
    summary->Branch("physevts_observed_NC_ratio",&NC_ratio,"physevts_observed_NC_ratio/D");
    
    Double_t ES_prod_ratio, IBD_prod_ratio,CC_prod_ratio,ICC_prod_ratio,NC_prod_ratio;
    
    
    summary->Branch("phys_evt_ES_produced",&es ,"phys_evt_ES_produced/I");
    summary->Branch("phys_evt_IBD_produced",&ibd,"phys_evt_IBD_produced/I");
    summary->Branch("phys_evt_CC_produced",&cc,"phys_evt_CC_produced/I");
    summary->Branch("phys_evt_ICC_produced",&icc ,"phys_evt_ICC_produced/I");
    summary->Branch("phys_evt_NC_produced",&nc,"phys_evt_NC_produced/I");
    summary->Branch("phys_evt_ES_prod_ratio",&ES_prod_ratio,"phys_evt_ES_prod_ratio/D");
    summary->Branch("phys_evt_IBD_prod_ratio",&IBD_prod_ratio,"phys_evt_IBD_prod_ratio/D");
    summary->Branch("phys_evt_CC_prod_ratio",&CC_prod_ratio,"phys_evt_CC_prod_ratio/D");
    summary->Branch("phys_evt_ICC_prod_ratio",&ICC_prod_ratio,"phys_evt_ICC_prod_ratio/D");
    summary->Branch("phys_evt_NC_prod_ratio",&NC_prod_ratio,"phys_evt_NC_prod_ratio/D");
    
    for (int i = 0; i < nEvents; i++) {
        
        //        //printf("###################### event %4d ############################\n",i );
        tree->GetEntry(i);
        RAT::DS::MC *mc = rds->GetMC();
        Int_t particleCountMC = mc->GetMCParticleCount();
        for (int mcP =0; mcP < particleCountMC; mcP++) {
            RAT::DS::MCParticle *prim = mc->GetMCParticle(mcP);
            //            //printf("%4d momentum  : %8.3f %8.3f %8.3f\n", prim->GetPDGCode(), prim->GetMomentum().X(), prim->GetMomentum().Y(), prim->GetMomentum().Z());
            
        }
        //Get the direction of the neutrino. Saved as last particle
        RAT::DS::MCParticle *prim = mc->GetMCParticle(particleCountMC-1);
        mcmomv_nu=prim->GetMomentum();
        dirNu =  prim->GetMomentum();
        mc_nu_energy = prim->ke;
        hNuE->Fill(mc_nu_energy);
        
        interaction_type = 0.0;
        ES_true = IBD_true = CC_true = ICC_true = NC_true = 0;
        
        
         if(particleCountMC ==2 && mc->GetMCParticle(0)->GetPDGCode()==-11 && mc->GetMCParticle(1)->GetPDGCode()==2112){
            //            //printf("IBD Interaction      ... ");
            ibd+=1;
            IBD_true =1;
            interaction_type = 2;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            mc_energy = prim->ke;
            
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
            
            
        }
        
        
        else if(particleCountMC ==2 && mc->GetMCParticle(0)->GetPDGCode()==11){
            //            //printf("ES Interaction       ... ");
            es+=1;
            ES_true =1;
            interaction_type = 1;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            mc_energy = prim->ke;
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==-11 && mc->GetMCParticle(1)->GetPDGCode()==2112){
            //            //printf("IBD Interaction      ... ");
            ibd+=1;
            IBD_true =1;
            interaction_type = 2;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            mc_energy = prim->ke;

            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
            
            
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==11 && mc->GetMCParticle(1)->GetPDGCode()==1000090160){
            //            //printf("CC (16F) Interaction ... ");
            cc+=1;
            CC_true = 1;
            interaction_type = 3;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            mc_energy = prim->ke;

            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
            
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==-11 && mc->GetMCParticle(1)->GetPDGCode()==1000070160){
            //            //printf("ICC (16N) Interaction ... ");
            icc+=1;
            ICC_true =1;
            
            interaction_type = 4;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            mc_energy = prim->ke;
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
            
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==2112){
            //printf("NC Interaction       ... 5\n");
            nc+=1;
            NC_true = 1;
            interaction_type = 5;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(1);
            mc_energy = prim->ke;

            
            hNuP->Fill(0.0);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==2212){
            //printf("NC Interaction       ... 7\n");
            nc+=1;
            NC_true = 1;
            interaction_type = 7;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(1);
            mc_energy = prim->ke;

            hNuP->Fill(0.0);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
        }
        else if(particleCountMC ==4 && mc->GetMCParticle(0)->GetPDGCode()==2112){
            //printf("NC Interaction       ... 6\n");
            nc+=1;
            NC_true = 1;
            interaction_type = 6;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(2);
            mc_energy = prim->ke;

            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
        }
        else if(particleCountMC ==4 && mc->GetMCParticle(0)->GetPDGCode()==2212){
            //printf("NC Interaction       ... 8\n");
            nc+=1;
            NC_true = 1;
            interaction_type = 8;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(2);
            mc_energy = prim->ke;

            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
            totMom = sqrt(pow(prim->GetMomentum().X(),2) +pow(prim->GetMomentum().Y(),2) + pow(prim->GetMomentum().Z(),2));
            dirTruth =  TVector3(prim->GetMomentum().X()/totMom,prim->GetMomentum().Y()/totMom,prim->GetMomentum().Z()/totMom);
            posTruth = prim->GetPosition();
            
        }
        else{
            //printf("What is this interaction -> particles %d:(%d, %d, %d) ... \n",particleCountMC, mc->GetMCParticle(0)->GetPDGCode(),mc->GetMCParticle(1)->GetPDGCode(),mc->GetMCParticle(2)->GetPDGCode());
        }
        
//        RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
        
        //Find out how many subevents:
        subevents = rds->GetEVCount();
        timeTmp1  = cnt = 0;
        prev_single =2;
        for (int ii=0; ii<40;ii++) {
            timeDiff[ii] = 0.0;
            single[ii]=0;
        }
        for (int k = 0; k<subevents; k++) {
            RAT::DS::EV *ev = rds->GetEV(k);
            qTmp = ev->GetTotalCharge();
            if (qTmp>12.) {
                cnt+=1;
                timeTmp = ev->GetDeltaT(); // Badly name variable by my part.
                
                if (cnt ==1) {
                    timeTmp1 = timeTmp;
                }else if(cnt>1){
                    timeDiff[cnt-2] = timeTmp-timeTmp1;
                    timeTmp1 = timeTmp;
                }
            }
        }
        if (cnt ==1) {
            single[0]=1;
        }else if(cnt>1){
            for(int kk=1;kk<cnt;kk++){
                //                printf("ev,Cnt,time : %d, %d, %f\n",kk,cnt,timeDiff[kk-1]);
                if (timeDiff[kk-1]<100000.) {
                    //                    printf("Found a double\n");
                    single[kk-1] = 0;// Also tag previous event as single
                    single[kk] = 0;
                }else{
                    //                    printf("Next event is a single\n");
                    single[kk] = 1;
                }
            }
        }
        //        //printf("Found %d detector subevents.\n",subevents);
        cntLoop = 0;
        for (int k = 0; k<subevents; k++) {
            
            RAT::DS::EV *ev = rds->GetEV(k);
            totPE = totQB = q2 = 0.0;
            pmtCount = float(ev->GetPMTCount());
            
            local_time = ev->GetDeltaT();
            
            RAT::DS::BonsaiFit *pb = ev->GetBonsaiFit();
            TVector3 pFitFB = pb->GetPosition();
            goodness = pb->GetGoodness();
            dirGoodness = pb->GetDirGoodness();
            
            posReco =  pb->GetPosition();
            reconstructedRadiusFB = sqrt(pow(pFitFB.X()-prim->GetPosition().X(),2)+ pow(pFitFB.Y()-prim->GetPosition().Y(),2)+ pow(pFitFB.Z()-prim->GetPosition().Z(),2))/1000.;
            
            for (int j = 0; j<pmtCount;j++) {
                RAT::DS::PMT *pmt = ev->GetPMT(j);
                totPE+=pmt->GetCharge();
            }
            cosTheta   =    (pb->GetDirection()* mcmomv_particle)/mcmomv_particle.Mag();
            cosThetaSN =    (pb->GetDirection()* mcmomv_nu      )/mcmomv_nu.Mag();
            
            dirReco = pb->GetDirection();
            if (totPE>12) {
                
                subEvNumber = cntLoop+1;
                _single = single[cntLoop];
                if (cntLoop>0) {
                    prev_single = single[cntLoop-1];
                }
                
                if (subEvNumber == cnt){
                    ES_cnt  += ES_true;
                    IBD_cnt += IBD_true;
                    CC_cnt  += CC_true;
                    ICC_cnt += ICC_true;
                    NC_cnt  += NC_true;
                }
                if (_single==1) {
                    totSingles+=1;
                    ES_cnt_s  += ES_true;
                    IBD_cnt_s += IBD_true;
                    CC_cnt_s  += CC_true;
                    ICC_cnt_s += ICC_true;
                    NC_cnt_s  += NC_true;
                }else{
                    totMultiples+=1;
                }
                if(_single ==0  && prev_single ==0){
                    dirIBD = TVector3(pFitFB.X()-oldX,pFitFB.Y()-oldY,pFitFB.Z()-oldZ);
                    cosThetaSNIBD =    (dirIBD* mcmomv_nu      )/mcmomv_nu.Mag()/dirIBD.Mag();
                }else{
                    cosThetaSNIBD = -2;
                }
                oldX = pFitFB.X();
                oldY = pFitFB.Y();
                oldZ = pFitFB.Z();
                
                data->Fill();
                
                if(k ==0 && totPE> 12){
                    hPhotoelectron0->Fill(totPE);
                    hPos0FB->Fill(reconstructedRadiusFB);
                    hPos0FBLOG->Fill(reconstructedRadiusFB);
                    hCos0FB->Fill(cosTheta);
                    hCos0FB_SN->Fill(cosThetaSN);
                }
                if(k > 0 &&  totPE> 12 ){
                    hPhotoelectron1->Fill(totPE);
                    hPos1FB->Fill(reconstructedRadiusFB);
                    hPos1FBLOG->Fill(reconstructedRadiusFB);
                    hCos1FB->Fill(cosTheta);
                    hCos1FB_SN->Fill(cosThetaSN);
                }
                
                
                cntLoop+=1;
            }
            
        }
        if(cnt==0){
            subEvNumber = 0;
            totPE = -10.0;
            cosTheta = cosThetaSN = cosThetaSNIBD = local_time = -2.0;
            _single =-1;
            reconstructedRadiusFB = -10.0;
            
            goodness = dirGoodness = -2;
            posReco = TVector3(-10.0,-10.0,-10.0);
            dirReco =  TVector3(-10.0,-10.0,-10.0);
            data->Fill();
            
        }
        if (i%500 == 0){
            c1->cd(1);
            hPhotoelectron0->Draw();
            hPhotoelectron1->Draw("same");
            gPad->SetGrid();
            gPad->BuildLegend();
            
            c1->cd(2);
            
            hPos0FBLOG->Draw();
            hPos1FBLOG->Draw("same");
            gPad->SetGrid();
            gPad->SetLogx();
            gPad->BuildLegend();
            
            c1->cd(3);
            hCos0FB->Draw();
            hCos1FB->Draw("same");
            gPad->SetGrid();
            gPad->BuildLegend();
            
            c1->cd(4);
            hCos0FB_SN->Draw();
            hCos1FB_SN->Draw("same");
            gPad->BuildLegend();
            gPad->SetGrid();
            
            c1->Update();
            
            c2->cd(1);
            hNuE->Draw();
            
            c2->cd(2);
            hNuP->Draw();
            
            c2->Update();
        }
        
        int nTracks = mc->GetMCTrackCount();
        
        //        Particle *trackmap = new Particle[nTracks+1];
        for (int j = 0; j < nTracks; j++) {
            RAT::DS::MCTrack *track = mc->GetMCTrack(j);
            int tid = track->GetID();
            int pid = track->GetParentID();
            
            RAT::DS::MCTrackStep *first = track->GetMCTrackStep(0);
            RAT::DS::MCTrackStep *last = track->GetLastMCTrackStep();
            
            if((track->GetParticleName() != "opticalphoton" ) ){
                if(first->ke>1.0 || track->pdgcode>22){//!((last->GetProcess()=="eIoni")||(last->GetProcess()=="hIoni"))){//
                    //printf("%7d  %7d %7d %7d %10d %8.3f %10.3f\n",i,j,pid,tid,track->pdgcode, first->ke,first->globalTime);//
                }
            }// if((track->GetParticleName() != "opticalphoton" ) ){
            int nSteps = track->GetMCTrackStepCount();
            TVector3 *steps = new TVector3[nSteps];
            for (int k = 0; k < nSteps; k++) {
                steps[k] = track->GetMCTrackStep(k)->GetEndpoint();
            }//for (int k = 0; k < nSteps; k++)
            
        }//for (int j = 0; j < nTracks; j++) {
    }
    c1->cd(1);
    hPhotoelectron0->Draw();
    hPhotoelectron1->Draw("same");
    gPad->SetGrid();
    gPad->BuildLegend();
    
    c1->cd(2);
    hPos0FBLOG->Draw();
    hPos1FBLOG->Draw("same");
    gPad->SetGrid();
    gPad->SetLogx();
    gPad->BuildLegend();
    
    c1->cd(3);
    hCos0FB->Draw();
    hCos1FB->Draw("same");
    gPad->SetGrid();
    gPad->BuildLegend();
    
    c1->cd(4);
    hCos0FB_SN->Draw();
    hCos1FB_SN->Draw("same");
    gPad->BuildLegend();
    gPad->SetGrid();
    
    c1->Update();
    
    c2->cd(1);
    hNuE->Draw();
    c2->cd(2);
    hNuP->Draw();
    
    
    c2->Update();
    
    
    //    f_out->cd();
    //    data->Write();
    data->Write();
    Double_t TOT_cnt =  Double_t(ES_cnt  + IBD_cnt + CC_cnt  + ICC_cnt + NC_cnt);
    
    ES_ratio  = ES_cnt/TOT_cnt;
    IBD_ratio = IBD_cnt/TOT_cnt;
    CC_ratio  = CC_cnt/TOT_cnt;
    ICC_ratio = ICC_cnt/TOT_cnt;
    NC_ratio  = NC_cnt/TOT_cnt;
    
    Double_t TOT_cnt_s =  Double_t(ES_cnt_s  + IBD_cnt_s + CC_cnt_s  + ICC_cnt_s + NC_cnt_s);
    
    ES_ratio_s  = ES_cnt_s/TOT_cnt_s;
    IBD_ratio_s = IBD_cnt_s/TOT_cnt_s;
    CC_ratio_s  = CC_cnt_s/TOT_cnt_s;
    ICC_ratio_s = ICC_cnt_s/TOT_cnt_s;
    NC_ratio_s  = NC_cnt_s/TOT_cnt_s;
    
    
    Double_t tot = Double_t(ibd+es+cc+icc+nc);
    
    IBD_prod_ratio = ibd/tot;
    ES_prod_ratio = es/tot;
    CC_prod_ratio = cc/tot;
    ICC_prod_ratio = icc/tot;
    NC_prod_ratio = nc/tot;
    
    
    summary->Fill();
    summary->Write();
    
    hPhotoelectron0->Write();
    hPhotoelectron1->Write();
    hPos0FB->Write();
    hPos1FB->Write();
    hNuE->Write();
    c1->Write();
    c2->Write();
    
    f_out->Close();
    
    
    printf("(ibd,es,cc,icc,nc): (%5.4f, %5.4f, %5.4f, %5.4f, %5.4f)  (tot:%d)\n",ibd/tot,es/tot,cc/tot,icc/tot,nc/tot,tot);
    
    
}
