#ifndef TIMEFIT
#define TIMEFIT
#include "hitsel.h"
#include "centroid.h"
// **********************************************
// fits the vertex time, assuming a given vertex
// calculates the likelihood for the timing dist.
// **********************************************

class timefit
{
  short    int bgfit;
  unsigned int nlike,*offset,*nneg,*lmax;
  unsigned int *loglike,*time_histo;
  float        *qminmax,*pdf,*pdf_sum,*pdf_max;
  int          nbin,s0,sx4,sx2,det;
  float        *tof,*dir,*like,*sig;

         void  delete_dist(void);
         void  create_dist(void);
         void  integrate_dist(void);
         void  load_dist(void);
         void  maketof(float *vertex);
         void  makedirtof(float *vertex);
  inline float histo_t0(void); 
  inline int   addloglik(int sub,int ndist);
  inline float addloglik(float &tpeak,float &dt,int &max);
         float fastaddloglik(float &tpeak,float &dt,int &max);
	 float addloglik(float &t0,float &dt,int &max,float tmi,float tpl);
  inline float maxloglike(int max);
         float peakfit(int max,float &tpeak,float &dt);
  inline void  tofminmax(float &tofmin,float &tofmax);
  inline void  sigrange(float tofmin,float tofmax,float *norm_fact,float *time_range);
         float makelike(float t0);
	 float makesiglike(float t0,float *bg);
  inline float makelike(int set,float &bg,float range);
  inline void  fitdir(float *vertex,float &dt,
		      centroid *&centr,float &mucang);
  inline void  corfunc(int *&bin,float *&cor,double *direct,double cang);
  inline void  lcor(float &alpha,float &t0,float &tout,int *bin,float *cor);
  inline void  maketof(float alpha,float t0,float tout,float *origtof,int *bin,float *cor);
  inline float addloglik(float *vertex,double *direct,double cang,float ll0);

 protected:
  hitsel       *event_hits;

 public:
  timefit(void);
  timefit(hitsel *hs);
  ~timefit();

  inline float fittime(short int fast,float *vertex,float &dt0);
  void  fittime(int ndt,float *dts,float &t0,float &dt);
  float fittime(short int fast,float *vertex,float *direct,float &dt);
  float fittime(float *vertex,float *direct,float &dt,float tmi,float tpl);
  float fittime_lcor(short int fast,float *vertex,float *direct,float &dt);
  float chargetest(float *vertex,float *direct,
		  float &qin,float &qoutplus,float *qoutminus);
  float maxconecharge(float *vert,float *direct);
  float fittime(float *vertex,float &dt0,float &signal);
  float goodness(float &ll,float *vertex,float *gdnset);
  int   nwind(float *vertex,float tmin,float tmax);
  int   nwind(float *vertex,float tmin,float tmax,float *ttof,int *cab);
  int   nwind(float *vertex,float tmin,float tmax,
	      float *ttof,float *d,int *cab);
  float tgood(float *vertex,float bgrate,float &guncor);
  float shapetest(void);
  inline int sets(void);
  inline int gethits(short int fast,float *vertex,
		     int *cabs,float *tofs,float *dirs,float *weight);
  inline void getdist(int set,float t0,int nbin,
		      float tmin,float tmax,float *cont);
  inline float *chargebins(void);
};
#include "timefit.inline"
#endif
