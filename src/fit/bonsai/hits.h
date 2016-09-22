#ifndef HITS
#define HITS
#include <stdio.h>
#include <math.h>
//#define NS_TO_CM   21.58333 /* speed of light (group speed) */
//#define NS_TO_CM    21.98
#define NS_TO_CM      21.8

#define CM_TO_NS   (1./NS_TO_CM)
#define CM_TO_NS2 1./(NS_TO_CM*NS_TO_CM)
#include "pmt_geometry.h"

typedef struct
{
  float hits[20000][3];  /*RECALL:C array's 0-n, f77 1-n  */
  int   it_index;
} comtype2;

// **********************************************
// Stores hit times, cable numbers and positions
// calculates time-of-flights and distances
// **********************************************

class hits
{
  int       nchargeset,*nhit;       //# of charge sets, # of hits for each set
  short int *places;                // array position of nth hit
  int   *cables;                    // cable #'s, times, charges, and
  float *times,*charges,*positions; // locations for each hit
  float *avcharges;                 // average charge for each charge set

  inline double absrat(double el);
  inline double findmin(double *mat,int *rowind,int *colind,int pass);
  inline void force_zero(double *mat,int *rowind,int *colind,int pass,int ind);
  int  inv35(double *mat,int *rowind,int *colind);
  void qsort(short int *list,short int n,short int *first);
  inline void create_arrays(int n_raw,int n_hit);

 protected:
  inline short int position(int i);
  inline int hits_in_set(int set,int i);
  inline int invalid_hit(int i);
  inline void swap(int i,int j);
  inline float tdiff(int i,int j);
  inline float dist2(int i,int j);

 public:
  hits(int ns,float *set,float *pmt_loc,int *bad_ch,
       int n_raw,int *cable,float *tim_raw,float *chg_raw);
  hits(int ns,float *set,pmt_geometry *geom,int nh,int *cab,float *t,float *q);
  hits(int ns,float *set,pmt_geometry *geom,comtype2 *itevent);
  ~hits(void);

  inline int nset();
  inline int first_hit(int set);
  inline int beyond_last_hit(int set);
  inline int nhits(int set);
  inline float charge(int set);
  inline int sumhits(int set);
  inline int hitcable(int hit);
  inline float x(int i);
  inline float y(int i);
  inline float z(int i);
  inline float hittime(int hit);
  inline float hitcharge(int i);
  inline void printhit(int hit);
  inline int ntot(void);
  inline void frontof(float *pos,int i,float top);
  inline float tof(float *pos,float *dir,int hit);
  inline float tof(float *pos,int hit);
  inline float gaussweight(float t0,float tmi,float tpl,int hit);
  inline void addhitloc(double *loc,int hit,double weight);
  inline void print_cables(int n);
  inline void qsort(short int *list,short int n);
  inline void qsort(short int *list);
  float time_av(float tmin,float tmax);
  int vertex4(int *hits,double *vert);
  int vertex4(int *fourcombo,int *hitlist,int listsize,
	      int &hbegin,int &hend,float tsig,
	      float cyl_radius,float cyl_height,float dwallmin,
	      double *vert,float *gdn);
};

#include "hits.inline"
#endif
