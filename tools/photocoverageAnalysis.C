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
#include <TClass.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TVector3.h>
//#include <libRATEvent.h>

void photocoverageAnalysis(const char *file, float nhit_min =3) {
    Double_t reconstructedRadius = 0.0;
    
    static Int_t nbins 	= 500;
    Double_t xbins[500]; //Needs to be the same number as above
    
    Double_t xmin 	= 1e-2;
    Double_t xmax 	= 20;
    Double_t logxmin = log10(xmin);
    Double_t logxmax = log10(xmax);
    Double_t binwidth= (logxmax-logxmin)/double(nbins);
    xbins[0] = xmin;
    for(Int_t i=0; i<nbins;i++){
        xbins[i] = xmin + pow(10,logxmin+i*binwidth);
    }
    
    static Int_t nbinsT 	= 500;
    Double_t xbinsT[500]; //Needs to be the same number as above
    Double_t xminT 	= 1e-9;
    Double_t xmaxT 	= 200;
    logxmin = log10(xminT);
    logxmax = log10(xmaxT);
    binwidth= (logxmax-logxmin)/double(nbinsT);
    xbinsT[0] = xminT;
    for(Int_t i=0; i<nbinsT;i++){
        xbinsT[i] = xminT + pow(10,logxmin+i*binwidth);
    }
    
    // Define the incoming out outgoing Trees
    TFile *f = new TFile(file);
    TTree *tree = (TTree*) f->Get("T");
    
    TFile *f_out = new TFile(Form("ntuple_%s",f->GetName()),"Recreate");
    
    RAT::DS::Root *rds = new RAT::DS::Root();
    tree->SetBranchAddress("ds", &rds);
    
    int nEvents = tree->GetEntries();
    
    //Define all the analysis parameters
    Double_t totPE = 0.0,totNHIT = 0,totMom,goodness,dirGoodness,qTmp,timeTmp,timeTmp1,oldX,oldY,oldZ;
    Double_t totQB = 0.0, q2 = 0.0, pmtCount = 0.0,reconstructedRadiusFC,reconstructedRadiusFP,reconstructedRadiusFB, reconstructedRadiusFPMinusFB;
    Int_t ibd=0,es=0,cc=0,icc=0,nc=0,old_singal;
    
    Double_t cosTheta,cosThetaSN,cosThetaSNIBD, local_time,local_time_tmp,delta_time,mc_nu_energy,mc_energy;
    
    int subEvNumber=0;
    Int_t subevents = 0,cnt,cntLoop;
    Int_t single[40],_single,prev_single;
    Double_t timeDiff[40];

    TVector3 mcmomv_nu, mcmomv_particle;
    TVector3 posTruth,posReco,dirTruth,dirNu,dirReco,dirIBD,pos1,pos2;
    
    Int_t totSingles, totMultiples;
    Double_t inner_dist,inner_time;;
    Int_t SV,old_FV,FV,GSV,IV,EV,OV,FV_t,GSV_t,IV_t,EV_t,OV_t;
    Double_t ES_ratio_s, IBD_ratio_s,CC_ratio_s,ICC_ratio_s,NC_ratio_s;
    Double_t ES_prod_ratio, IBD_prod_ratio,CC_prod_ratio,ICC_prod_ratio,NC_prod_ratio;
    
    TTree *data = new TTree("data","supernova events");
    data->Branch("pe",&totPE,"pe/D");
    data->Branch("r_bonsai_true",&reconstructedRadiusFB,"r_bonsai_true/D");
    data->Branch("cosTheta",&cosTheta,"cosTheta/D");
    data->Branch("cosThetaSN",&cosThetaSN,"cosThetaSN/D");
    data->Branch("cosThetaSNIBD",&cosThetaSNIBD,"cosThetaSNIBD/D");
    data->Branch("local_time_ns",&local_time,"local_time_ns/D");
    data->Branch("delta_time_ns",&delta_time,"delta_time_s/D");
    data->Branch("sub_ev",&subEvNumber,"sub_ev/I");
    data->Branch("sub_ev_cnt",&cnt,"sub_ev_cnt/I");
    data->Branch("single",&_single,"single/I");
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
    data->Branch("FV",&FV,"FV/I");
    data->Branch("GSV",&GSV,"GSV/I");
    data->Branch("EV",&EV,"EV/I");
    data->Branch("OV",&OV,"OV/I");
    data->Branch("IV",&IV,"IV/I");
    data->Branch("FV_t",&FV_t,"FV_t/I");
    data->Branch("GSV_t",&GSV_t,"GSV_t/I");
    data->Branch("EV_t",&EV_t,"EV_t/I");
    data->Branch("OV_t",&OV_t,"OV_t/I");
    data->Branch("IV_t",&IV_t,"IV_t/I");
    data->Branch("inner_dist",&inner_dist,"inner_dist/D");
    data->Branch("inner_time",&inner_time,"inner_time/D");
    data->Branch("SV",&SV,"SV/I");
    data->Branch("old_FV",&old_FV,"old_FV/I");

    TTree *summary = new TTree("summary","Summary");
    summary->Branch("evts",&totEvents,"evts/I" );
    summary->Branch("evts_singles",&totSingles,"evts_singles/I" );
    summary->Branch("evts_multiples",&totMultiples,"evts_multiples/I");
    
    TH1D *hPos0FB = new TH1D("hPos0FB","primary event",1000,0.01,10);
    hPos0FB->SetLineColor(4);
    hPos0FB->SetXTitle("r_{bonsai}(m)");
    hPos0FB->SetYTitle("Counts");
    TH1D *hPos1FB = new TH1D("hPos1FB","secondary event",1000,0.01,10);
    hPos1FB->SetLineColor(kRed+3);
    
    TH1D *hPos0FBLOG = new TH1D("hPos0FBLOG","primary event",nbins-1,xbins);
    hPos0FBLOG->SetLineColor(4);
    hPos0FBLOG->SetXTitle(" 3d vertex r_{bonsai} [m]");
    hPos0FBLOG->SetYTitle("Counts");
    TH1D *hPos1FBLOG = new TH1D("hPos1FBLOG","secondary event",nbins-1,xbins);
    hPos1FBLOG->SetLineColor(kRed+3);
    
    TH1D *hTime0FBLOG = new TH1D("hTime0FBLOG","primary event",nbinsT-1,xbinsT);
    hTime0FBLOG->SetLineColor(4);
    hTime0FBLOG->SetXTitle("#Delta t (s)");
    hTime0FBLOG->SetYTitle("Counts");
    TH1D *hTime1FBLOG = new TH1D("hTime1FBLOG","secondary event",nbinsT-1,xbinsT);
    hTime1FBLOG->SetLineColor(kRed+3);
    hTime1FBLOG->SetXTitle("#Delta t_{inner-event} (s)");
    hTime1FBLOG->SetYTitle("Counts");

    
    TH2D *hPosTimeFBLOG = new TH2D("hPosTimeFBLOG","primary event",nbins-1,xbins,nbinsT-1,xbinsT);
    hPosTimeFBLOG->SetLineColor(4);
    hPosTimeFBLOG->SetXTitle(" 3d vertex r_{bonsai} [m]");
    hPosTimeFBLOG->SetYTitle("Counts");
    
    
    
    
    TH1D *hPhotoelectron0 = new TH1D("hPhotoelectron0","primary event",600,0,600);
    hPhotoelectron0->SetLineColor(4);
    hPhotoelectron0->SetXTitle("photoelectrons");
    hPhotoelectron0->SetYTitle("Counts");
    TH1D *hPhotoelectron1 = new TH1D("hPhotoelectron1","secondary event",600,0,600);
    hPhotoelectron1->SetLineColor(kRed+3);
    
    TH1D *hCos0FB = new TH1D("hCos0FB","primary event",1000,-1.05,1.05);
    hCos0FB->SetLineColor(4);
    hCos0FB->SetXTitle("cos #theta_{reconstruction}");
    hCos0FB->SetYTitle("Counts");
    TH1D *hCos1FB = new TH1D("hCos1FB","secondary event",1000,-1.05,1.05);
    hCos1FB->SetLineColor(kRed+3);
    
    TH1D *hCos0FB_SN = new TH1D("hCos0FB_SN","primary event",1000,-1.05,1.05);
    hCos0FB_SN->SetLineColor(4);
    hCos0FB_SN->SetXTitle("cos #theta_{SN}");
    hCos0FB_SN->SetYTitle("Counts");
    TH1D *hCos1FB_SN = new TH1D("hCos1FB_SN","secondary event",1000,-1.05,1.05);
    hCos1FB_SN->SetLineColor(kRed+3);
    
    TH1D *hNuE = new TH1D("hNuE","kinetic energy",1000,0,1000);
    hNuE->SetLineColor(4);
    hNuE->SetXTitle("kinetic energy (MeV)");
    hNuE->SetYTitle("Counts");
    
    TH1D *hNuP = new TH1D("hNuP","particle energy (e^{#pm} or #gamma)",1000,0,100);
    hNuP->SetLineColor(4);
    hNuP->SetXTitle("Cherenkov-inducing particle Energy (MeV)");
    hNuP->SetYTitle("Counts");
    
    TH1D *hMult = new TH1D("hMult","detector event multiplicity",20,-0.5,19.5);
    hMult->SetLineColor(4);
    hMult->SetXTitle("detector event multiplicity");
    hMult->SetYTitle("Counts");
    
    for (int i = 0; i < nEvents; i++) {
        
        tree->GetEntry(i);
        RAT::DS::MC *mc             = rds->GetMC();
        Int_t particleCountMC       = mc->GetMCParticleCount();
        for (int mcP =0; mcP < particleCountMC; mcP++) {
            RAT::DS::MCParticle *prim = mc->GetMCParticle(mcP);
        }
        
        //Get the direction of the neutrino. Saved as last particle
        RAT::DS::MCParticle *prim   = mc->GetMCParticle(particleCountMC-1);
        mcmomv_nu                   = prim->GetMomentum();
        dirNu                       = prim->GetMomentum();
        mc_nu_energy                = prim->ke;
        hNuE->Fill(mc_nu_energy);
        
        Double_t r_t = sqrt(pow(prim->GetPosition().X(),2)+ pow(prim->GetPosition().Y(),2))/1000.;
        Double_t z_t = sqrt(pow(prim->GetPosition().Z(),2))/1000.;
        
        FV_t = GSV_t = IV_t = EV_t = OV_t = 0;
        if (r_t < 5.4 && z_t < 5.4) {
            FV_t =1;
            IV_t =1;
            EV_t =1;
        }else if(!(r_t < 5.4 && z_t < 5.4) && (r_t < 6.4 && z_t < 6.4)){
            GSV_t =1;
            IV_t  =1;
            EV_t =1;
        }else if(!(r_t < 6.4 && z_t < 6.4) && (r_t < 8.0 && z_t < 8.0)){
            OV_t =1;
            EV_t =1;
        }else{
            EV_t = 0;
        }
        
        
        
        //Find out how many subevents:
        subevents                   = rds->GetEVCount();
        timeTmp1                    = cnt = 0;
        prev_single                 = 2;
        
        for (int ii=0; ii<40;ii++) {
            timeDiff[ii]            = 0.0;
            single[ii]              = 0;
        }
        for (int k = 0; k<subevents; k++) {
            RAT::DS::EV *ev         = rds->GetEV(k);
            qTmp = ev->Nhits();
            if (qTmp>nhit_min) {
                cnt                  +=1;
                timeTmp = ev->GetDeltaT(); // Badly name variable by my part.
                if (cnt ==1) {
                    timeTmp1        = timeTmp;
                }else if(cnt>1){
                    timeDiff[cnt-2] = timeTmp-timeTmp1;
                    timeTmp1        = timeTmp;
                }
            }
        }
        if (cnt ==1) {
            single[0]               = 1;
        }else if(cnt>1){
            for(int kk=1;kk<cnt;kk++){
                if (timeDiff[kk-1]<100000.) {
                    single[kk-1]    = 0;// Also tag previous event as single
                    single[kk]      = 0;
                }else{
                    single[kk]      = 1;
                }
            }
        }

        
        cntLoop = 0;
        local_time_tmp = 0;
        hMult->Fill(cnt);

        old_FV = FV = GSV = IV = EV = OV = 0;

        for (int k = 0; k<subevents; k++) {
            
            RAT::DS::EV *ev = rds->GetEV(k);
            
            totPE = totQB = q2      = 0.0;
            
            pmtCount = float(ev->GetPMTCount());
            
            local_time_tmp          = local_time;
            local_time              = ev->GetDeltaT();
            if(k==0){
                delta_time              = local_time;
                old_FV = 0;
            }else{
                delta_time              = local_time -local_time_tmp;
                old_FV = FV;
            }
            RAT::DS::BonsaiFit *pb  = ev->GetBonsaiFit();
            TVector3 pFitFB         = pb->GetPosition();
            goodness                = pb->GetGoodness();
            dirGoodness             = pb->GetDirGoodness();
            
            posReco                 =  pb->GetPosition();
            reconstructedRadiusFB   = sqrt(pow(pFitFB.X()-prim->GetPosition().X(),2)+ pow(pFitFB.Y()-prim->GetPosition().Y(),2)+ pow(pFitFB.Z()-prim->GetPosition().Z(),2))/1000.;
            
            
            Double_t r = sqrt(pow(pFitFB.X(),2)+ pow(pFitFB.Y(),2))/1000.;
            Double_t z = sqrt(pow(pFitFB.Z(),2))/1000.;
            
            FV = GSV = IV = EV = OV = 0;
            if (r < 5.4 && z < 5.4) {
                FV                  = 1;
                IV                  = 1;
                EV                  = 1;
            }else if(!(r < 5.4 && z < 5.4) && (r < 6.4 && z < 6.4)){
                GSV                 = 1;
                IV                  = 1;
                EV                  = 1;
            }else if(!(r < 6.4 && z < 6.4) && (r < 8.0 && z < 8.0)){
                OV                  = 1;
                EV                  = 1;
            }else{
                EV                  = 0;
            }

            
            for (int j = 0; j<pmtCount;j++) {
                RAT::DS::PMT *pmt   = ev->GetPMT(j);
                totPE               +=pmt->GetCharge();
            }
            
            cosTheta                =  (pb->GetDirection()* mcmomv_particle)/mcmomv_particle.Mag();
            cosThetaSN              =  (pb->GetDirection()* mcmomv_nu      )/mcmomv_nu.Mag();
            
            dirReco = pb->GetDirection();
            
            if (totNHIT>nhit_min) {
                subEvNumber = cntLoop+1;
                _single = single[cntLoop];
                if (cntLoop>0) {
                    prev_single = single[cntLoop-1];
                }
                if (subEvNumber == cnt){
                    totEvents+=1;
                }
                if (_single==1) {
                    totSingles+=1;
                }else{
                    totMultiples+=1;
                }
                if(_single ==0  && prev_single ==0){
                    dirIBD = TVector3(pFitFB.X()-oldX,pFitFB.Y()-oldY,pFitFB.Z()-oldZ);
                    cosThetaSNIBD =    (dirIBD* mcmomv_nu      )/mcmomv_nu.Mag()/dirIBD.Mag();
                }else{
                    cosThetaSNIBD = -2;
                }

                inner_dist = sqrt(pow(pFitFB.X()-oldX,2)+pow(pFitFB.Y()-oldY,2)+pow(pFitFB.Z()-oldZ,2))/1000.;
                inner_time = delta_time/1e9;
                
                oldX = pFitFB.X();
                oldY = pFitFB.Y();
                oldZ = pFitFB.Z();
                
                data->Fill();
                
                if(k ==0 && totNHIT>nhit_min){
                    hPhotoelectron0->Fill(totPE);
                    hPos0FB->Fill(reconstructedRadiusFB);
                    hPos0FBLOG->Fill(reconstructedRadiusFB);
                    hTime0FBLOG->Fill(local_time/1e9);
                    hCos0FB->Fill(cosTheta);
                    hCos0FB_SN->Fill(cosThetaSN);
                    
                    hPosTimeFBLOG->Fill(dirIBD.Mag(),delta_time/1e9);
                }
                if(k > 0 &&  totNHIT>nhit_min){
                    hPhotoelectron1->Fill(totPE);
                    hPos1FB->Fill(reconstructedRadiusFB);
                    hPos1FBLOG->Fill(reconstructedRadiusFB);
                    hTime1FBLOG->Fill(delta_time/1e9);
                    hTime0FBLOG->Fill(local_time/1e9);//Also fill the time with respect to start
                    hCos1FB->Fill(cosTheta);
                    hCos1FB_SN->Fill(cosThetaSN);
                    
                    hPosTimeFBLOG->Fill(dirIBD.Mag(),delta_time/1e9);

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
        int nTracks = mc->GetMCTrackCount();
    }
    
    //Display the results
    TCanvas *c1 = new TCanvas("c1","Detector Reconstruction Information",1200,800);
    c1->Divide(2,2);
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
    
    TCanvas *c2 = new TCanvas("c2","MC truth",1200,800);
    c2->Divide(2,2);
    c2->cd(1);
    hNuE->Draw();
    gPad->SetGrid();
    gPad->BuildLegend();
    c2->cd(2);
    hMult->Draw();
    gPad->SetLogy();
    gPad->SetGrid();
    gPad->BuildLegend();
    c2->cd(3);
    hTime0FBLOG->Draw();
    gPad->SetGrid();
    gPad->SetLogx();
    gPad->BuildLegend();
    c2->cd(4);
    hTime1FBLOG->Draw();
    gPad->SetGrid();
    gPad->SetLogx();
    gPad->BuildLegend();
    c2->Update();
    
    //Write the data and analysis summary to file
    data->Write();
    summary->Fill();
    summary->Write();

    hPos0FB->Write();
    hPos1FB->Write();
    hPos0FBLOG->Write();
    hPos1FBLOG->Write();
    hTime0FBLOG->Write();
    hPosTimeFBLOG->Write();
    hPhotoelectron0->Write();
    hPhotoelectron1->Write();
    hCos0FB->Write();
    hCos1FB->Write();
    hCos0FB_SN->Write();
    hCos1FB_SN->Write();
    hNuE->Write();
    hNuP->Write();
    hMult->Write();
    
    c1->Write();
    c2->Write();
    
    c1->Close();
    c2->Close();
    
    rds->Delete();
    
    f_out->Close();
    
    f->Close();

    
    
}
