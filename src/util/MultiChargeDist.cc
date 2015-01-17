#include <RAT/MultiChargeDist.hh>
#include <RAT/MiniCleanPMTCharge.hh>
#include <RAT/PDFPMTCharge.hh>
#include <RAT/string_utilities.hpp>
#include <RAT/DB.hh>
#include <RAT/DBLink.hh>
#include <TMath.h>
#include <TH1D.h>
#include <cassert>
#include <RAT/Log.hh>

namespace RAT {

    static TGraph *HistToNormGraph(TH1 *hist)
    {
	const int nbins = hist->GetNbinsX();
	const double lowEdge = 0.0;
	assert(nbins > 2);
  
	int start_bin = hist->FindBin(lowEdge);
	if (hist->GetBinCenter(start_bin) < lowEdge)
	    start_bin++;
  
	std::vector<double> x(nbins+2-start_bin);
	std::vector<double> y(nbins+2-start_bin);
  
	// Extrapolate first point back to lower edge
        x[0] = lowEdge;
        y[0] = hist->GetBinContent(start_bin) - 
	  (hist->GetBinContent(start_bin+1) - hist->GetBinContent(start_bin)) 
	  * (hist->GetBinCenter(start_bin) - lowEdge)/hist->GetBinWidth(start_bin);
  
	double integral = 0.0;
	for (unsigned i=1; i < x.size(); i++) {
	    x[i] = hist->GetBinCenter(start_bin+i-1);
	    y[i] = hist->GetBinContent(start_bin+i-1);
	    integral += (y[i] + y[i-1])/2.0 * (x[i] - x[i-1]);
	}
  
	//warn << "Integral: " << integral << newline;
  
	// Normalize
	if (integral > 0.0) {
	    for (unsigned i=0; i < y.size(); i++) {
		y[i] /= integral;
	    }
	}
  
	TGraph *gr = new TGraph(x.size(), &x.front(), &y.front());
	gr->SetName( (std::string(hist->GetName()) + "_interp").c_str() );
	gr->GetHistogram(); // ensure thread-safety later
	return gr;
    }

// Does linear interpolation of TGraph to avoid TString multithreading
// problem with TGraph::Eval()
    static double interpolate(TGraph *gr, double x)
    {
	// Setup some fake local variables to emulate environment of TGraph::Eval()
	int fNpoints = gr->GetN();
	Double_t *fX = gr->GetX();
	Double_t *fY = gr->GetY();

	//linear interpolation
	//find point in graph immediatly below x
	//In case x is < fX[0] or > fX[fNpoints-1] return the extrapolated point
	Int_t low = TMath::BinarySearch(fNpoints,fX,x);
	Int_t up = low+1;
	if (low == fNpoints-1) {up=low; low = up-1;}
	if (low == -1) {low=0; up=1;}
	if (fX[low] == fX[up]) return fY[low];
	Double_t yn = x*(fY[low]-fY[up]) +fX[low]*fY[up] - fX[up]*fY[low];
	return yn/(fX[low]-fX[up]);
    }


    
    // Convolves by integrating over range of histScan
    static double convolution(double q, double stepSize, TGraph *f, TGraph *g)
    {
	double sum = 0.0;
	TAxis *f_axis = f->GetXaxis();
	TAxis *g_axis = g->GetXaxis();
	double min = fmin(f_axis->GetXmin(), g_axis->GetXmin());
	int steps = (int) ((q - min) / stepSize);
	if(steps <= 0) 
	  return 0.0;
	
	for (int istep=0; istep < steps; istep++) {
	    double x = min + istep * stepSize;
	    double f_val;
	    if (x < f_axis->GetXmin() || x > f_axis->GetXmax())
		f_val = 0.0;
	    else
	        f_val = interpolate(f, x);
	    
	    double g_val;
	    if ( (q-x) < g_axis->GetXmin() || (q-x) > g_axis->GetXmax())
		g_val = 0.0;
	    else
		g_val = interpolate(g, q-x);
	    
	    double term = f_val * g_val * stepSize;
	    if (term < 0.0) term = 0.0;
	    
	    sum += term;
	}

	return sum;
    }

  std::vector<TGraph*> MultiChargeDist(int maxPE, double qMaxStepSize,
				       int qBins, double meanSinglePE)
    {
    //FIXME: future people might appreciate being able to specify a pmt_model
    //to be used with PDFPMTCharge, but this was original behavior
    PMTCharge* pmtCharge = new RAT::MiniCleanPMTCharge();
    
	// Calculate the range of the 1 PE PDF.  The value of the PDF
	// at this value sets the boundaries for the PE > 1 PDFs.
	double maxSinglePE = 5 * meanSinglePE;
	double singlePErange = 1.0e9;
	while(singlePErange > maxSinglePE){
	  maxSinglePE *= 2;
	  double rms = 0.0;
	  int nsamples = 1000;
	  double stepsize = maxSinglePE / (meanSinglePE * nsamples);
	  for(int i=0; i<nsamples; i++)
	    rms += pow(pmtCharge->PDF(stepsize*(i+0.5)) - 1, 2);
	  rms = sqrt(rms / nsamples) * meanSinglePE;
	  singlePErange = meanSinglePE + rms * 5;
	}
	double minprob = pmtCharge->PDF(singlePErange / meanSinglePE);

	int nbins = qBins;
	double stepsize = singlePErange / nbins;
	if(stepsize > qMaxStepSize){
	  stepsize = qMaxStepSize;
	  nbins = (int) (singlePErange / stepsize);
	}

	std::vector<TH1D *> chargeDist(maxPE + 1);
	std::vector<TGraph *> chargeDistInterp(maxPE + 1);

	// 0 pe: Charge distribution of noise (not dark current)
	chargeDist[0] = new TH1D("pmt_q0", "", nbins, 0, singlePErange);
	chargeDist[0]->Sumw2();
	chargeDistInterp[0] = HistToNormGraph(chargeDist[0]);
	// No noise in simulation

	// 1 pe: Do separate to collect stats on distribution
	chargeDist[1] = new TH1D("pmt_q1", "", nbins, 0, singlePErange);
	chargeDist[1]->Sumw2();
	for (int ibin=1; ibin < chargeDist[1]->GetNbinsX(); ibin++) {
	    double q = chargeDist[1]->GetBinCenter(ibin);
	    chargeDist[1]->SetBinContent(ibin, pmtCharge->PDF(q/meanSinglePE));
	}
	chargeDist[1]->Scale(1.0/chargeDist[1]->Integral("width"));
	chargeDistInterp[1] = HistToNormGraph(chargeDist[1]);

	// 2 thru M pe
	double qmin = 0.0;
	double qmax = singlePErange;
	for (int numPE=2; numPE <= maxPE; numPE++) {
	  nbins = qBins;
	  
	  // Start where the last PDF did unless the value there is too large.
	  // Then move up until we reach the specified probability.
	  if(convolution(qmax + 2 * meanSinglePE, stepsize,
			 chargeDistInterp[1],
			 chargeDistInterp[numPE-1]) > minprob)
	    qmin = fmax(meanSinglePE*numPE - singlePErange*sqrt(numPE), qmin);
	  while(convolution(qmin, stepsize,
			    chargeDistInterp[1],
			    chargeDistInterp[numPE-1]) < minprob)
	    qmin += stepsize;

	  // End 2 times the mean further away from where the last PDF ended
	  // unless the value there is too large.  Then move back until
	  // we reach the specified probability.
	  if(convolution(qmax + 2 * meanSinglePE, stepsize,
			 chargeDistInterp[1],
			 chargeDistInterp[numPE-1]) < minprob)
	    qmax += 2 * meanSinglePE;
	  else
	    qmax = fmax(meanSinglePE*numPE + singlePErange*sqrt(numPE), qmax);
	  
	  while(convolution(qmax, stepsize,
			    chargeDistInterp[1],
			    chargeDistInterp[numPE-1]) < minprob)
	    qmax -= stepsize;

	  // Set the stepsize and number of bins using the new min and max
	  stepsize = (qmax - qmin) / nbins;
	  if(stepsize > qMaxStepSize){
	    stepsize = qMaxStepSize;
	    nbins = (int) ((qmax - qmin) / stepsize);
	  }

	  TH1D *h = new TH1D(("pmt_q"+::to_string(numPE)).c_str(), "",
			     nbins, qmin, qmax);
	  h->Sumw2();

	  // Speed things up by drawing a charge from the N-1 distribution
	  // and a charge from the single pe distribution
	  for (int ibin=1; ibin < h->GetNbinsX(); ibin++) {
	    double q = h->GetBinCenter(ibin);
	    double pdf_value = convolution(q, stepsize,
					   chargeDistInterp[1],
					   chargeDistInterp[numPE-1]);
	    // thread-safe, unlike SetBinContent(), but does not check bounds
	    h->SetBinContent(ibin, pdf_value);
	  }

	  h->Scale(1.0/h->Integral("width"));
	  chargeDist[numPE] = h;
	  chargeDistInterp[numPE] = HistToNormGraph(chargeDist[numPE]);
	}

	// Deallocate the TH1Ds and check the means
	int inorm = -1;
	for (unsigned i=0; i < chargeDist.size(); i++){
	  if(fabs(chargeDist[i]->GetMean() -
		  i*meanSinglePE) / (i*meanSinglePE) > 0.01 && inorm < 0)
	    inorm = i;
	  delete chargeDist[i];
	}
	if(inorm > 0)
	  warn << "WARNING--Charge PDFs " << inorm << " to " << maxPE << 
	    " are biased by more than 1%.  Increase number of bins" <<
	    " and/or decrease the maximum bin size to reduce the bias." << endl;
	
	return chargeDistInterp;
    }


} // namespace RAT
