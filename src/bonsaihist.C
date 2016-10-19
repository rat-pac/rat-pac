
// Extract the tracks from a file using root command prompts
// example of use. The command
// >  root bonsaihist.C\(\"/Users/marcbergevin/RAT_ROOT/output.root\"\)
// Will apply this routine of the output.root file
// M.B
#include <iostream>
#include <iomanip>
#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

void bonsaihist(const char *file,const char *out,double gmin,double agmin)
{
    TH1I *truem=new TH1I("truem","MC Momentum",400,0,20);
    TH1I *bx=new TH1I("bx","BONSAI x position",800,-8000,8000);
    TH1I *by=new TH1I("by","BONSAI y position",800,-8000,8000);
    TH1I *bz=new TH1I("bz","BONSAI z position",800,-8000,8000);
    TH1I *bt=new TH1I("bt","BONSAI time",800,-800,800);
    TH1I *bg=new TH1I("bg","BONSAI goodness",200,0,1);
    TH1I *bdg=new TH1I("bdg","BONSAI Direction goodness",200,0,1);
    TH1I *bn9=new TH1I("bn9","BONSAI N9",201,-0.5,200.5);
    TH1I *bdx=new TH1I("bdx","BONSAI x deviation",800,-8000,8000);
    TH1I *bdy=new TH1I("bdy","BONSAI y deviation",800,-8000,8000);
    TH1I *bdz=new TH1I("bdz","BONSAI z deviation",800,-8000,8000);
    TH1I *bdt=new TH1I("bdt","BONSAI time",800,-800,800);
    TH1I *bdd=new TH1I("bdd","BONSAI cos deviation angle",200,-1,1);
    
    TFile *f = new TFile(file);
    TTree *tree = (TTree*) f->Get("T");
    
    RAT::DS::Root *rds = new RAT::DS::Root();
    tree->SetBranchAddress("ds", &rds);
    
    
    int nEvents = tree->GetEntries();
    //TCanvas *c1 = new TCanvas("c1","c1",800,1200);
    //  c1->Divide(2,3);
    
    for (int i = 0; i < nEvents; i++)
    {
        tree->GetEntry(i);
        RAT::DS::MC *mc = rds->GetMC();
        RAT::DS::MCParticle *prim = mc->GetMCParticle(0);
        
        //Find out how many subevents:
        Int_t subevents = rds->GetEVCount();
        for (int k = 0; k<subevents; k++)
        {
            RAT::DS::EV *ev = rds->GetEV(k);
            totPE = totQB = q2 = 0.0;
            pmtCount = float(ev->GetPMTCount());
            
            RAT::DS::BonsaiFit *pb = ev->GetBonsaiFit();
            if (pb->GetGoodness()<gmin) continue;
            if (pb->GetDirGoodness()<agmin) continue;
            TVector3 bonsai_pos = pb->GetPosition();
            TVector3 deviation=bonsai_pos-prim->GetPosition();
            TVector3 mcmomv=prim->GetMomentum();
            double   mcmom=sqrt(mcmomv.X()*mcmomv.X()+
                                mcmomv.Y()*mcmomv.Y()+
                                mcmomv.Z()*mcmomv.Z());
            
            printf("#################### event %4d, sub %2 ##########################\n",i,k );
            truem->Fill(mcmom,1);
            bx->Fill(bonsai_pos.X(),1);
            by->Fill(bonsai_pos.Y(),1);
            bz->Fill(bonsai_pos.Z(),1);
            bt->Fill(pb->GetTime(),1);
            bg->Fill(pb->GetGoodness());
            bdg->Fill(pb->GetDirGoodness());
            bn9->Fill(pb->GetN9());
            bdx->Fill(deviation.X(),1);
            bdy->Fill(deviation.Y(),1);
            bdz->Fill(deviation.Z(),1);
            bdt->Fill(pb->GetTime(),1);
            bdd->Fill(pb->GetDirection()*mcmomv/mcmom);
            /*printf("mcdir=%lf %lf %lf: %lf %lf\n",
             mcmomv.X()/mcmom,mcmomv.Y()/mcmom,mcmomv.Z()/mcmom,mcmom,
             pb->GetDirection()*mcmomv/mcmom);*/
        }
    }
    TFile *o=new TFile(out,"RECREATE");
    o->cd();
    truem->Write();
    bx->Write();
    by->Write();
    bz->Write();
    bt->Write();
    bg->Write();
    bdg->Write();
    bn9->Write();
    bdx->Write();
    bdy->Write();
    bdz->Write();
    bdt->Write();
    bdd->Write();
}

