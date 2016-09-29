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
    TNtuple* data = new TNtuple("data","Ntuple for Watchman Reconstruction Studies",
                                "pe:r_bonsai_true:cosTheta:cosThetaSN:local_time_ns:sub_ev:sub_ev_cnt:interaction");
    
    
    RAT::DS::Root *rds = new RAT::DS::Root();
    tree->SetBranchAddress("ds", &rds);
    
    int nEvents = tree->GetEntries();
    
    TCanvas *c1 = new TCanvas("c1","Detector Reconstruction Information",1200,800);
    c1->Divide(2,2);
    
    TCanvas *c2 = new TCanvas("c2","MC truth",1200,800);
    c2->Divide(2,2);
    
    

    
    Double_t totPE = 0.0;
    Double_t totQB = 0.0, q2 = 0.0, pmtCount = 0.0,reconstructedRadiusFC,reconstructedRadiusFP,reconstructedRadiusFB, reconstructedRadiusFPMinusFB;
    Double_t ibd=0.0,es=0.0,cc=0.0,icc=0.0,nc=0.0,cosTheta,cosThetaSN,local_time;

    Int_t interaction_type;
    TVector3 mcmomv_nu, mcmomv_particle;

    for (int i = 0; i < nEvents; i++) {
        
//        printf("###################### event %4d ############################\n",i );
        tree->GetEntry(i);
        RAT::DS::MC *mc = rds->GetMC();
        Int_t particleCountMC = mc->GetMCParticleCount();
        for (int mcP =0; mcP < particleCountMC; mcP++) {
            RAT::DS::MCParticle *prim = mc->GetMCParticle(mcP);
//            printf("%4d momentum  : %8.3f %8.3f %8.3f\n", prim->GetPDGCode(), prim->GetMomentum().X(), prim->GetMomentum().Y(), prim->GetMomentum().Z());
            
        }
        //Get the direction of the neutrino. Saved as last particle
        RAT::DS::MCParticle *prim = mc->GetMCParticle(particleCountMC-1);
        mcmomv_nu=prim->GetMomentum();
        hNuE->Fill(prim->ke);
        
        interaction_type = 0.0;
        
        if(particleCountMC ==2 && mc->GetMCParticle(0)->GetPDGCode()==11){
//            printf("ES Interaction       ... ");
            es+=1;
            interaction_type = 1;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==-11 && mc->GetMCParticle(1)->GetPDGCode()==2112){
//            printf("IBD Interaction      ... ");
            ibd+=1;
            interaction_type = 2;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();

        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==11 && mc->GetMCParticle(1)->GetPDGCode()==1000090160){
//            printf("CC (16F) Interaction ... ");
            cc+=1;
            interaction_type = 3;
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();

        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==-11 && mc->GetMCParticle(1)->GetPDGCode()==1000070160){
//            printf("ICC (16N) Interaction ... ");
            icc+=1;;
            interaction_type = 4;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();

        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==2112){
            printf("NC Interaction       ... 5\n");
            nc+=1;
            interaction_type = 5;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            hNuP->Fill(0.0);
            mcmomv_particle = prim->GetMomentum();
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==2212){
            printf("NC Interaction       ... 7\n");
            nc+=1;
            interaction_type = 7;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
            hNuP->Fill(0.0);
            mcmomv_particle = prim->GetMomentum();
        }
        else if(particleCountMC ==4 && mc->GetMCParticle(0)->GetPDGCode()==2112){
            printf("NC Interaction       ... 6\n");
            nc+=1;
            interaction_type = 6;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(2);
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
        
        }
        else if(particleCountMC ==4 && mc->GetMCParticle(0)->GetPDGCode()==2212){
            printf("NC Interaction       ... 8\n");
            nc+=1;
            interaction_type = 8;
            RAT::DS::MCParticle *prim = mc->GetMCParticle(2);
            hNuP->Fill(prim->ke);
            mcmomv_particle = prim->GetMomentum();
        }
        else{
            printf("What is this interaction -> particles %d:(%d, %d, %d) ... \n",particleCountMC, mc->GetMCParticle(0)->GetPDGCode(),mc->GetMCParticle(1)->GetPDGCode(),mc->GetMCParticle(2)->GetPDGCode());
        }
        
        RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
        
        //Find out how many subevents:
        Int_t subevents = rds->GetEVCount();
//        printf("Found %d detector subevents.\n",subevents);
        for (int k = 0; k<subevents; k++) {
            
            RAT::DS::EV *ev = rds->GetEV(k);
            totPE = totQB = q2 = 0.0;
            pmtCount = float(ev->GetPMTCount());
            
            local_time = ev->GetDeltaT();
            
            RAT::DS::BonsaiFit *pb = ev->GetBonsaiFit();
            TVector3 pFitFB = pb->GetPosition();
            reconstructedRadiusFB = sqrt(pow(pFitFB.X()-prim->GetPosition().X(),2)+ pow(pFitFB.Y()-prim->GetPosition().Y(),2)+ pow(pFitFB.Z()-prim->GetPosition().Z(),2))/1000.;
            
            for (int j = 0; j<pmtCount;j++) {
                RAT::DS::PMT *pmt = ev->GetPMT(j);
                totPE+=pmt->GetCharge()/1.6;
            }
            cosTheta   =    (pb->GetDirection()* mcmomv_particle)/mcmomv_particle.Mag();
            cosThetaSN =    (pb->GetDirection()* mcmomv_nu      )/mcmomv_nu.Mag();

            data->Fill(totPE,reconstructedRadiusFB,cosTheta,cosThetaSN,local_time,Double_t(k)+1,Double_t(subevents),interaction_type);
            if(k ==0 && totPE> 8){
                hPhotoelectron0->Fill(totPE);
                hPos0FB->Fill(reconstructedRadiusFB);
                hCos0FB->Fill(cosTheta);
                hCos0FB_SN->Fill(cosThetaSN);
            }
            if(k > 0 &&  totPE> 12 ){
                hPhotoelectron1->Fill(totPE);
                hPos1FB->Fill(reconstructedRadiusFB);
                hCos1FB->Fill(cosTheta);
                hCos1FB_SN->Fill(cosThetaSN);
            }
            
        }
        
        if (i%100 == 0){
            c1->cd(1);
            hPhotoelectron0->Draw();
            hPhotoelectron1->Draw("same");
            gPad->SetGrid();
            gPad->BuildLegend();
            
            c1->cd(2);
            hPos0FB->Draw();
            hPos1FB->Draw("same");
            gPad->SetGrid();
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
                    printf("%7d  %7d %7d %7d %10d %8.3f %10.3f\n",i,j,pid,tid,track->pdgcode, first->ke,first->globalTime);//
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
    hPos0FB->Draw();
    hPos1FB->Draw("same");
    gPad->SetGrid();
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
    data->Write();
    
    hPhotoelectron0->Write();
    hPhotoelectron1->Write();
    hPos0FB->Write();
    hPos1FB->Write();
    hNuE->Write();
    c1->Write();
    c2->Write();
    
    f_out->Close();
    
    Double_t tot = ibd+es+cc+icc+nc;
    
    printf("(ibd,es,cc,icc,nc): (%5.4f, %5.4f, %5.4f, %5.4f, %5.4f)  (tot:%d)\n",ibd/tot,es/tot,cc/tot,icc/tot,nc/tot,tot);
    
    
}