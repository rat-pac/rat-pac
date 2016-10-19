#ifndef LIKELIHOOD
#define LIKELIHOOD
#include "timefit.h"
#include "fit_param.h"
#include "fitquality.h"
#include "bonsaifit.h"
#include "searchgrid.h"
#include "plato.h"

// *************************************************************
// * Defines BONSAI likelihood and maximization                *
// *************************************************************
class likelihood: public fit_param,public fitquality, public timefit
{
  float        cang0,plusdang,minusdang,dev;   // direction fit constraint
  float        verfit[4],dirfit[5],like,like0; // vertex, direction, likelihoods
  float        dt;                             // timefit uncertainty
  short int    nlike;                          // number of fit vertices
  dodecahedron dod;                            // defines how surrounding points
  axes         orientation;                    // are calculated

 public:
  inline likelihood(float r,float z);            // construct timefit and likelihood
  inline void set_hits(hitsel *h);               // define hits to fit
         virtual float    quality(float *vertex);// calculate likelihood
  // surround a point with test vertices
  inline virtual void check_around(float *vertex,float *result,
				   float radius,float *q,int &max);
  inline virtual char ncheck(void);              // return # of surrounding test vertices
  // interpolate best fit position
  inline virtual void interpolate(float *vertex,
				  float radius,float *q,float *inter);
  inline virtual int nresult(void);              // return size of result array
  inline virtual void get_result(float *r);      // get result array
  inline virtual void set_result(float *r);      // set result array
  // maximizing procedure
  inline void      maximize(bonsaifit *fit,searchgrid *grid);
  inline void      maximize(bonsaifit *fit,float *point);
  inline float     get_zero(void);               // get t0
  inline float     residual(short int hit);      // get time residual
  inline float     get_ll(void);                 // get likelihood
  inline float     get_ll0(void);                // get likelihood w/o angle constraint
  inline void      get_dir(float *dir);          // get direction fit
  inline int       nfit(void);                   // return # of test vertices
};

#include "likelihood.inline"
#endif
