{
  gROOT->SetStyle("Plain");
  gStyle->SetCanvasBorderMode(0);
  gStyle->SetPadBorderMode(0);
  gStyle->SetPadColor(0);
  gStyle->SetCanvasColor(0);
  gStyle->SetLegendFillColor(0);
  gStyle->SetLegendBorderSize(1);
  gStyle->SetLegendFont(132);
  gStyle->SetLabelFont(132);
  gStyle->SetTitleFont(132);

  gSystem->Load("libRATEvent");
  gSystem->AddIncludePath(" -I$RATROOT/include");
}

