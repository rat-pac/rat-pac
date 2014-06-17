#ifndef __RAT_GaussianRatioPDF__
#define __RAT_GaussianRatioPDF__

#include <TMath.h>
#include <cmath>

namespace RAT{

  /* The gaussian ratio PDF from equation 1 in
     http://www.jstor.org/stable/2334671.
     The parameters are ordered as: 
          -normalization 
          -mean prompt number of PE
          -width of the prompt distribution 
          -mean total number of PE, 
          -width of the distribution of total PE
          -correlation parameter.
  */

  Double_t GaussianRatioPDF(Double_t* x, Double_t* par);

}

#endif
