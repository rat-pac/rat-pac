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
    TH1D *hPos0C = new TH1D("hPos0C","centroid",1000,0.01,10);
    hPos0C->SetLineColor(4);
    TH1D *hPos1C = new TH1D("hPos1C","centroid",1000,0.01,10);
    
    TH1D *hPos0FP = new TH1D("hPos0FP","fit path",1000,0.01,10);
    hPos0FP->SetLineColor(kGreen+3);
    TH1D *hPos1FP = new TH1D("hPos1FP","fit path",1000,0.01,10);
    hPos1FP->SetLineColor(kGreen+3);
    
    TH1D *hPos0FB = new TH1D("hPos0FB","bonsai",1000,0.01,10);
    hPos0FB->SetLineColor(kRed+3);
    TH1D *hPos1FB = new TH1D("hPos1FB","bonsai",1000,0.01,10);
    hPos1FB->SetLineColor(kRed+3);
    
    
    TH2D *hPos0FPFB = new TH2D("hPos0FPFB","fitpath:bonsai",1000,0.01,10,1000,0.01,10);
    hPos0FPFB->SetMarkerColor(kRed+3);
    hPos0FPFB->SetMarkerStyle(3);
    
    hPos0FPFB->SetMarkerSize(1.5);
    
    TH2D *hPos1FPFB = new TH2D("hPos1FPFB","fitpath:bonsai",1000,0.01,10,1000,0.01,10);
    hPos1FPFB->SetMarkerColor(kRed+3);
    hPos1FPFB->SetMarkerStyle(3);
    hPos1FPFB->SetMarkerSize(1.5);
    
    hPos0FPFB->SetXTitle("r_{fitpath}(m)");
    hPos0FPFB->SetYTitle("r_{bonsai}(m)");
    hPos1FPFB->SetXTitle("r_{fitpath}(m)");
    hPos1FPFB->SetYTitle("r_{bonsai}(m)");
    
    TH1D *hPhotoelectron1 = new TH1D("hPhotoelectron1","hPhotoelectron1",200,0,200);
    hPhotoelectron1->SetLineColor(4);
    TH1D *hPhotoelectron2 = new TH1D("hPhotoelectron2","hPhotoelectron2",200,0,200);
    
    TH1D *hqBalance1 = new TH1D("hqBalance1","hqBalance1",100,0,1);
    hqBalance1->SetLineColor(4);
    TH1D *hqBalance2 = new TH1D("hqBalance2","hqBalance2",100,0,1);
    
    
    TFile *f = new TFile(file);
    TTree *tree = (TTree*) f->Get("T");
    
    TFile *f_out = new TFile(Form("ntuple_%s",f->GetName()),"Recreate");
    TNtuple* data = new TNtuple("data","Ntuple for Watchman Reconstruction Studies",
                                "pe:r_pathfit_true:r_bonsai_true:r_pathfit_bonsai:sub_ev:sub_ev_cnt");
    
    
    RAT::DS::Root *rds = new RAT::DS::Root();
    tree->SetBranchAddress("ds", &rds);
    
    
    int nEvents = tree->GetEntries();
    TCanvas *c1 = new TCanvas("c1","c1",800,1200);
    c1->Divide(2,3);
    Double_t totPE = 0.0;
    Double_t totQB = 0.0, q2 = 0.0, pmtCount = 0.0,reconstructedRadiusFC,reconstructedRadiusFP,reconstructedRadiusFB, reconstructedRadiusFPMinusFB;
    Double_t ibd=0.0,es=0.0,cc=0.0,icc=0.0,nc=0.0;
    for (int i = 0; i < nEvents; i++) {
        
//        printf("###################### event %4d ############################\n",i );
        tree->GetEntry(i);
        RAT::DS::MC *mc = rds->GetMC();
        Int_t particleCountMC = mc->GetMCParticleCount();
        for (int mcP =0; mcP < particleCountMC; mcP++) {
            RAT::DS::MCParticle *prim = mc->GetMCParticle(mcP);
//            printf("%4d momentum  : %8.3f %8.3f %8.3f\n", prim->GetPDGCode(), prim->GetMomentum().X(), prim->GetMomentum().Y(), prim->GetMomentum().Z());
            
        }
        if(particleCountMC ==2 && mc->GetMCParticle(0)->GetPDGCode()==11){
            printf("ES Interaction       ... ");
            es+=1;
        }
        else if(particleCountMC ==2 && mc->GetMCParticle(0)->GetPDGCode()==22){
            printf("NC Interaction       ... ");
            nc+=1;
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==-11 && mc->GetMCParticle(1)->GetPDGCode()==2112){
            printf("IBD Interaction      ... ");
            ibd+=1;
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==11 && mc->GetMCParticle(1)->GetPDGCode()==1000090160){
            printf("CC (16F) Interaction ... ");
            cc+=1;
        }
        else if(particleCountMC ==3 && mc->GetMCParticle(0)->GetPDGCode()==-11 && mc->GetMCParticle(1)->GetPDGCode()==1000070160){
            printf("ICC (16N) Interaction ... ");
            icc+=1;;
        }else{
            printf("What is this interaction (particles %d, %d ) ... ",particleCountMC, mc->GetMCParticle(1)->GetPDGCode());
        }
        
        
        RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
        
        //Find out how many subevents:
        Int_t subevents = rds->GetEVCount();
        printf("Found %d detector subevents.\n",subevents);
        for (int k = 0; k<subevents; k++) {
            
            RAT::DS::EV *ev = rds->GetEV(k);
            totPE = totQB = q2 = 0.0;
            pmtCount = float(ev->GetPMTCount());
            
            RAT::DS::Centroid *fc = ev->GetCentroid();
            TVector3 pFitFC = fc->GetPosition();
            reconstructedRadiusFC = sqrt(pow(pFitFC.X()-prim->GetPosition().X(),2)+ pow(pFitFC.Y()-prim->GetPosition().Y(),2)+ pow(pFitFC.Z()-prim->GetPosition().Z(),2))/1000.;
            
            RAT::DS::PathFit *pf = ev->GetPathFit();
            TVector3 pFitFP = pf->GetPosition();
            reconstructedRadiusFP = sqrt(pow(pFitFP.X()-prim->GetPosition().X(),2)+ pow(pFitFP.Y()-prim->GetPosition().Y(),2)+ pow(pFitFP.Z()-prim->GetPosition().Z(),2))/1000.;
            
            RAT::DS::BonsaiFit *pb = ev->GetBonsaiFit();
            TVector3 pFitFB = pb->GetPosition();
            reconstructedRadiusFB = sqrt(pow(pFitFB.X()-prim->GetPosition().X(),2)+ pow(pFitFB.Y()-prim->GetPosition().Y(),2)+ pow(pFitFB.Z()-prim->GetPosition().Z(),2))/1000.;
            
            
            reconstructedRadiusFPMinusFB =sqrt(pow(pFitFB.X()-pFitFP.X(),2)+ pow(pFitFB.Y()-pFitFP.X(),2)+ pow(pFitFB.Z()-pFitFP.X(),2))/1000.;
            for (int j = 0; j<ev->GetPMTCount();j++) {
                RAT::DS::PMT *pmt = ev->GetPMT(j);
                
//                printf("EV %2d PMT %2d: (pmtID, charge,time) (%3.0d, %8.2f, %8.2f)\n", k,j,pmt-
//                       >GetID(),pmt->GetCharge(),pmt->GetTime());
                totPE+=pmt->GetCharge()/1.6;
                q2+= (pmt->GetCharge())**2;
                
            }
            totQB = sqrt(q2/totPE**2-1./pmtCount);
            data->Fill(totPE,reconstructedRadiusFP,reconstructedRadiusFB,reconstructedRadiusFPMinusFB,Double_t(k),Double_t(subevents));
            if(k ==0 && totPE> 8){
                hPhotoelectron1->Fill(totPE);
                hqBalance1->Fill(totQB);
//                printf("%f %f %f %f\n ",reconstructedRadiusFC,reconstructedRadiusFP,reconstructedRadiusFB, reconstructedRadiusFPMinusFB);
                
                hPos0C->Fill(reconstructedRadiusFC);
                hPos0FP->Fill(reconstructedRadiusFP);
                hPos0FB->Fill(reconstructedRadiusFB);
                hPos0FPFB->Fill(reconstructedRadiusFP,reconstructedRadiusFB);
                
            }
            if(k > 0 &&  totPE> 12 ){
                hPhotoelectron2->Fill(totPE);
                hqBalance2->Fill(totQB);
                
                
                hPos1C->Fill(reconstructedRadiusFC);
                hPos1FP->Fill(reconstructedRadiusFP);
                hPos1FB->Fill(reconstructedRadiusFB);
                hPos1FPFB->Fill(reconstructedRadiusFP,reconstructedRadiusFB);
                
            }
            
        }
        
        if (i%100 == 0){
            c1->cd(1);
            hPhotoelectron1->Draw();
            hPhotoelectron2->Draw("same");
            c1->cd(2);
            hqBalance1->Draw();
            hqBalance2->Draw("same");
            c1->cd(3);
            //    hPos0C->Draw();
            hPos0FB->Draw();
            hPos0FP->Draw("same");
            
            c1->cd(4);
            //    hPos1C->Draw();
            hPos1FB->Draw();
            hPos1FP->Draw("same");
            c1->Update();
            
            c1->cd(5);
            //    hPos1C->Draw();
            hPos0FPFB->Draw();
            c1->cd(6);
            //    hPos1C->Draw();
            hPos1FPFB->Draw();
            
            c1->Update();
        }
        
//        printf("MC PE     : %8.0f\n", mc->GetNumPE());
//        printf("MC PMT Cnt: %8.0f\n", mc->GetMCPMTCount());
//        printf("MC Trk Cnt: %8.0f\n", mc->GetMCTrackCount());
//        printf("MCPart Cnt: %8.0f\n", mc->GetMCParticleCount());
//        printf("Kin E     : %8.2f\n", prim->GetKE());
        //        printf("position  : %8.2f %8.2f %8.2f\n", prim->GetPosition().X(), prim->GetPosition().Y(), prim->GetPosition().Z());
        //        printf("momentum  : %8.3f %8.3f %8.3f\n", prim->GetMomentum().X(), prim->GetMomentum().Y(), prim->GetMomentum().Z());
        //        printf("fit position  : %8.2f %8.2f %8.2f\n", pFit.X(), pFit.Y(), pFit.Z());
        //
        
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
    hPhotoelectron1->Draw();
    hPhotoelectron2->Draw("same");
    c1->cd(2);
    hqBalance1->Draw();
    hqBalance2->Draw("same");
    c1->cd(3);
    //    hPos0C->Draw();
    hPos0FP->Draw();
    hPos0FB->Draw("same");
    
    c1->cd(4);
    //    hPos1C->Draw();
    hPos1FP->Draw();
    hPos1FB->Draw("same");
    c1->Update();
    
    //    f_out->cd();
    data->Write();
    
    hPhotoelectron1->Write();
    hPhotoelectron2->Write();
    hqBalance1->Write();
    hqBalance2->Write();
    hPos0FP->Write();
    hPos0FB->Write();
    hPos1FP->Write();
    hPos1FB->Write();
    c1->Write();
    
    f_out->Close();
    
    Double_t tot = ibd+es+cc+icc+nc;
    
    printf("(ibd,es,cc,icc,nc): (%5.4f, %5.4f, %5.4f, %5.4f, %5.4f)  (tot:%d)\n",ibd/tot,es/tot,cc/tot,icc/tot,nc/tot,tot);
    
    
}