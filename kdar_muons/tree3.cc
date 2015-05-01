#define tree_cxx
#include "tree3.h"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <string>

using namespace std;

void tree::Loop()
{

// TTree *LArTree;
// TChain ch("treeout");

// ch.Add("/net/nudsk0001/d00/scratch/spitzj/eventsout_nuwroxsec_numu_kpipe_3_25_2015.root");

// tree aStep(treeout);
// Long64_t nentries = ch.GetEntries();
// cout << "Number of Entries " << nentries << endl;
// double Eproton, Elepton, Ehadron, anglelep, Eall, Eleptrue, Elepkin, Elep_mev, Elepton_nosmear, Plep_mev;
// double MNminusV=939.56536-34.;
// double Mmuon=105.658;
// double bindingE=34.;
// double deltaM2=pow(939.56536,2)-pow(938.27203,2);

// for (int event = 0; event<nentries; event++) 
// {
// aStep.GetEntry(event);
// if(event%10000==0)
// cout<<((double)event/(double)nentries)*100.<<"%"<<endl;
// Elep_mev=aStep.post_t[0];
// Elepton=aStep.post_t[0]/1000.;
// Eleptrue=0.;
// Eproton=0.;
// Elepkin=0.;
// Eall=0.;

// //no NC events considered
// if(aStep.flag_cc==0)
// continue;

// //loop over particles that exit nucleus. be careful: the zeroth entry in the array always refers to the outgoing lepton
// for (int out = 0; out<aStep.post_; out++) //I confirmed this is correct! Should NOT be <= !
// {
// 	if(aStep.post_t[out]>0)
// 	{
// 		//only count protons for now
// 		if(aStep.post_pdg[out]==2212)
// 		{
// 		Eproton+=(aStep.post_t[out]/1000.)-.938272;
// 		Eall+=(aStep.post_t[out]/1000.)-.938272;
// 		}
// 	}
// }

// //the zeroth entry in the array always refers to the outgoing lepton
// Eall+=(aStep.post_t[0]/1000.);
// Eleptrue=Elepton-.1057;

// anglelep=acos(aStep.post_z[0]/sqrt(pow(aStep.post_x[0],2)+pow(aStep.post_y[0],2)+pow(aStep.post_z[0],2)));

// Elepton=(Elepton-.1057)+r->Gaus(0.,.1*(Elepton-.1057));

// Plep_mev=sqrt(pow(Elep_mev,2)-pow(105.658,2));

// //The "CCQE formula" energy reconstruction
// Elepkin=((2*Elep_mev*MNminusV)-pow(bindingE,2)+(2*939.56536*bindingE)-pow(Mmuon,2)-deltaM2)/(2*(MNminusV-Elep_mev+(Plep_mev*cos(anglelep))));
// Elepkin=Elepkin*.001;

// //the "in" refers to the neutrino or target nucleon
// //this if statement makes sure we are dealing with the monoenergetic neutrinos
// if(aStep.in_t[0]/1000.>.235&&aStep.in_t[0]/1000.<.237)
// {
// hist_all->Fill(Eall+0.016);//16 MeV proton separation energy for 12C);
// histleptrue->Fill(Eleptrue);

// 	if(Elepton>0.)
// 	{
// 	histlep->Fill(Elepton);
// 	histanglelep->Fill(anglelep,Eleptrue);
// 	histlepkin->Fill(Elepkin);
// 	}
	
// 	if(Eproton>0.)
// 	histprot->Fill(Eproton);
// }

// }//event loop

// new TCanvas;
// hist_all->SetXTitle("Energy (GeV)");
// hist_all->SetYTitle("Rate (arbitrary units)");
// hist_all->GetXaxis()->SetTitleOffset(.9);
// hist_all->GetYaxis()->SetTitleOffset(.5);
// hist_all->GetXaxis()->SetTitleSize(.05);
// hist_all->GetYaxis()->SetTitleSize(.05);
// hist_all->GetYaxis()->SetLabelOffset(999);
// hist_all->SetLineColor(1);
// hist_all->DrawNormalized();
// histlep->SetLineColor(2);
// histlep->SetLineStyle(2);
// histlep->DrawNormalized("SAME");
// histleptrue->SetLineColor(2);
// histleptrue->DrawNormalized("SAME");
// histlepkin->SetLineColor(4);
// histlepkin->DrawNormalized("SAME");

// TLine line1;
// line1.SetLineWidth(5);
// line1.SetLineStyle(2);
// line1.DrawLine(.2355,0,.2355,.096);

// TLegend *legend = new TLegend(0.13,0.65,0.3,0.88);
// legend->SetTextSize(0.04);
// legend->AddEntry(hist_all,"#bar{E}_{#nu} (w/ perfect reco, post-FSI)","l");
// legend->AddEntry(histlepkin,"#tilde{E}_{#nu} (w/ perfect muon-only reco)","l");
// legend->AddEntry(histleptrue,"T_{#mu} (true)","l");
// legend->AddEntry(histlep,"T_{#mu} (#frac{#DeltaT}{T}=10\%)","l");
// legend->SetFillColor(0);
// legend->SetBorderSize(0);
// legend->Draw();
// gPad->GetCanvas()->Print("muon.pdf");

// new TCanvas;
// histanglelep->GetXaxis()->SetTitleOffset(.9);
// histanglelep->GetYaxis()->SetTitleOffset(.9);
// histanglelep->GetXaxis()->SetTitleSize(.05);
// histanglelep->GetYaxis()->SetTitleSize(.05);
// histanglelep->SetYTitle("T_{#mu} (GeV)");
// histanglelep->SetXTitle("#theta_{#mu} (radians)");
// histanglelep->Draw("colz");
// gPad->GetCanvas()->Print("muon_kinematics.pdf");



}
