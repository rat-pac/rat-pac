import ROOT
import numpy as np

//TF1 *landauDistr = new TF1("landauDistr","TMath::Landau(x,[0],[1])",0,200.);

_file0 = ROOT.TFile("watchman_20.root");


EMIN = 0.5;
EMAX = 5.5;
EDELTA = 0.25;
NBINS  =600.;

MAXCNT = (EMAX-EMIN)/EDELTA/2.;

c1 = ROOT.TCanvas("c1","c1",1618,1000);
c1.Divide(4,3);
c1.cd(1);
x = {}
y = {}

ent = 0.
cnt = 0
for(Double_t E = EMIN;E < EMAX;E+=2*EDELTA){
    cnt+=1
    c1.cd(cnt);
    ent = T.Draw(Form("sqrt((ds.ev.bonsaifits.GetPosition().x()-ds.mc.particle.GetPosition().x())**2+(ds.ev.bonsaifits.GetPosition().y()-ds.mc.particle.GetPosition().y())**2+(ds.ev.bonsaifits.GetPosition().z()-ds.mc.particle.GetPosition().z())**2)/10.>>h%d(400,0,400)",cnt),
                  Form("ds.ev.bonsaifits.IDHit>7 && ds.mc.particle.GetKE() > (%f) && ds.mc.particle.GetKE() < (%f)",E-EDELTA,E+EDELTA))
    
    if ent !=0. :
        h = T.GetHistogram()
        h.Fit("landau","LMRQ","",0,400.)
        res = h.GetFunction("landau")
        res.Draw("same")
        tmp = 0.0
        for(double i = 0.01;i<1000.0;i++){
            tmp = res->Integral(0,i ) / res->Integral(0,1000.);
            if (tmp>0.683){
                printf("E,sig = %3.1f %f      [%f/%d]\n",E,i,res->GetChisquare(),res->GetNDF());
                x.push_back(E);
                y.push_back(i);
                break;
            }
        }
    else:
        printf("E,sig = %3.1f no_data\n",E);
        cnt-=1;

    }
}
TCanvas *c2 = new TCanvas("c2","c2",1618,1000);
c2->Divide(1,1);
c2->cd(1);
TGraph *g = new TGraph(cnt,&x[0],&y[0]);
g->Draw("AL");
c1->Update();
printf("%f %d\n", MAXCNT,cnt);
}
