void make_plots(TFile *event_file, TTree *T, TFile *out_file)
{
  TH1F *acr_attn_100 = new TH1F("acr_attn_100", "Photon track length (100 nm)", 50, 0, 50);
  acr_attn_100->SetXTitle("Track length (mm)");
  acr_attn_100->SetYTitle("Count");
  T->Draw("mc.track.GetLastMCTrackStep()->length>>acr_attn_100","TMath::Abs(1.23997279736421566e-03/(mc.track.GetLastMCTrackStep()->ke)-100)<10","goff");
  //acr_attn_100->Fit("expo");
  //acr_attn_100->Draw("goff");
  acr_attn_100->Write();

  TH1F *acr_attn_200 = new TH1F("acr_attn_200", "Photon track length (200 nm)", 50, 0, 50);
  acr_attn_200->SetXTitle("Track length (mm)");
  acr_attn_200->SetYTitle("Count");
  T->Draw("mc.track.GetLastMCTrackStep()->length>>acr_attn_200","TMath::Abs(1.23997279736421566e-03/(mc.track.GetLastMCTrackStep()->ke)-200)<10","goff");
  //acr_attn_200->Fit("expo");
  //acr_attn_200->Draw("goff");
  acr_attn_200->Write();

  TH1F *acr_attn_300 = new TH1F("acr_attn_300", "Photon track length (300 nm)", 20, 0, 2500);
  acr_attn_300->SetXTitle("Track length (mm)");
  acr_attn_300->SetYTitle("Count");
  T->Draw("mc.track.GetLastMCTrackStep()->length>>acr_attn_300","TMath::Abs(1.23997279736421566e-03/(mc.track.GetLastMCTrackStep()->ke)-300)<10","goff");
  acr_attn_300->Fit("expo");
  acr_attn_300->Draw("goff");
  acr_attn_300->Write();

  TH1F *acr_attn_400 = new TH1F("acr_attn_400", "Photon track length (400 nm)", 20, 0, 8000);
  acr_attn_400->SetXTitle("Track length (mm)");
  acr_attn_400->SetYTitle("Count");
  T->Draw("mc.track.GetLastMCTrackStep()->length>>acr_attn_400","TMath::Abs(1.23997279736421566e-03/(mc.track.GetLastMCTrackStep()->ke)-400)<10","goff");
  acr_attn_400->Fit("expo");
  acr_attn_400->Draw("goff");
  acr_attn_400->Write();

  TH1F *acr_attn_500 = new TH1F("acr_attn_500", "Photon track length (500 nm)", 20, 0, 10000);
  acr_attn_500->SetXTitle("Track length (mm)");
  acr_attn_500->SetYTitle("Count");
  T->Draw("mc.track.GetLastMCTrackStep()->length>>acr_attn_500","TMath::Abs(1.23997279736421566e-03/(mc.track.GetLastMCTrackStep()->ke)-500)<10","goff");
  acr_attn_500->Fit("expo");
  acr_attn_500->Draw("goff");
  acr_attn_500->Write();

}
