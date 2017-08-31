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
#include <vector>
#include <TH1D>


void timeResiduals(const char *file) {

    TH1D *timeRes = new TH1D("timeRes","time residuals",5000,-20,430);
    timeRes->SetXTitle("time [ns]");

    RAT::DS::Root *rds = new RAT::DS::Root();

    TFile *f = new TFile(file);
    TTree *tree = (TTree*) f->Get("T");
    tree->SetBranchAddress("ds", &rds);

    Double_t v_x,v_y,v_z,p_x,p_y,p_z,v_t,p_t,t_t;
    // v_ : vertex_
    // p_ : pmt_
    // t_ : trigger
    // x,y,z,t : 3d position,time

    TTree *posTree = (TTree*) f->Get("runT");
    //Giving up doing it properly and cheating
    runT->Draw("pmtinfo.pos.X():pmtinfo.pos.Y():pmtinfo.pos.Z():pmtinfo.type","Entry$>-1","goff");

    Double_t *x = runT->GetV1();
    Double_t *y = runT->GetV2();
    Double_t *z = runT->GetV3();
    Double_t *typ = runT->GetV4();

    for (int i = 0; i < T->GetEntries(); i++) {

        tree->GetEntry(i);
        RAT::DS::MC *mc = rds->GetMC();
        RAT::DS::MCParticle *prim = mc->GetMCParticle(0); // Find position of vertext

        v_x    = prim->GetPosition().X();
        v_y    = prim->GetPosition().Y();
        v_z    = prim->GetPosition().Z();

        //Find out how many subevents:
        Int_t subevents = rds->GetEVCount();
        for (int k = 0; k<subevents; k++) {

            RAT::DS::EV *ev = rds->GetEV(k);

            for (int j = 0; j<ev->GetPMTCount();j++) {
                RAT::DS::PMT *pmt = ev->GetPMT(j);
                t_t = ev->GetCalibratedTriggerTime();

                p_x = x[pmt->GetID()];
                p_y = y[pmt->GetID()];
                p_z = z[pmt->GetID()];

                v_t = sqrt(pow(p_x-v_x,2)+pow(p_y-v_y,2)+pow(p_z-v_z,2)) / (0.2998* 1000. * 0.7519); //c (m/ns * mm/m *index refraction)
                p_t = pmt->GetTime() + t_t;
//                printf("%f %f\n",v_t,p_t);
                if(typ[pmt->GetID()]==1){ // Only ID PMTs
                    timeRes->Fill(p_t-v_t);
                }
            }


        }

    }
    timeRes->SaveAs("timeRes.C");
    timeRes->Delete();
}
