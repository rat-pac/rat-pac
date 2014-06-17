void make_plots(TFile *event_file, TTree *T, TFile *out_file)
{
  TH1F *hStepSize = new TH1F("hStepSize", "Distance between volume boundaries for isotropic particle tracks", 1000, 0, 3500);
  hStepSize->SetYTitle("Events per bin");
  hStepSize->SetXTitle("Distance between volume boundaries (mm)");
  T->Draw("mc.track.step.length>>hStepSize","","goff");
  hStepSize->SetMinimum(0);
  hStepSize->Write();
}
