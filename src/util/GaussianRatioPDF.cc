#include <RAT/GaussianRatioPDF.hh>

namespace RAT{

  Double_t GaussianRatioPDF(Double_t* x, Double_t* par){
    double a = pow(x[0] / par[2], 2);
    a -= 2 * par[5] * x[0] / (par[2] * par[4]);
    a += 1 / pow(par[4], 2);
    a = sqrt(a);

    double b = par[1] * x[0] / pow(par[2], 2);
    b -= par[5] * (par[1] + par[3] * x[0]) / (par[2] * par[4]);
    b += par[3] / pow(par[4], 2);

    double c = pow(par[1] / par[2], 2);
    c -= 2 * par[5] * par[1] * par[3] / (par[2] * par[4]);
    c += pow(par[3] / par[4], 2);

    double d = pow(b, 2) - c * pow(a, 2);
    d /= 2 * (1 - pow(par[5], 2)) * pow(a, 2);
    d = TMath::Exp(d);

    /* Stray factors of 2 and some varying signs relative to the 
       reference to convert root's erfc to the function used in 
       the reference.
    */
    double arg = b / (sqrt(2 * (1-pow(par[5], 2))) * a);
    double term1 = 0.5 * (-TMath::Erfc(arg) + TMath::Erfc(-arg));
    term1 *= b * d / (sqrt(2 * TMath::Pi()) * a);
    double term2 = TMath::Exp(-c / (2 * (1 - pow(par[5], 2))));
    term2 *= sqrt(1 - pow(par[5], 2)) / TMath::Pi();

    double val = term1 + term2;
    val /= pow(a, 2) * par[2] * par[4];
    return par[0] * val;
  }

}
