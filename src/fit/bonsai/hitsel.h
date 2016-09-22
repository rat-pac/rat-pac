#ifndef HITSEL
#define HITSEL
#include "hits.h"

// **********************************************
// does  hit selection, when needed
// **********************************************
class hitsel: public hits
{
  short int *selected;
  int       nsel;

  short int make_causal_table(short int *&related,short int *&relations,
			      float twin,float resolution,float tcoincidence);
  short int reduce(int size,short int *source,
		            short int *destination,
		            short int *occurence);
  void select(float dlim,float tlim,
	      float twin,float resolution,float tcoincidence);

 public:
  inline hitsel(float dlim,float tlim,
		float twin,float resolution,float tcoincidence,
		int ns,float *set,float *pmt_loc,int *bad_ch,
		int n_raw,int *cable,float *tim_raw,float *chg_raw);
  inline hitsel(float dlim,float tlim,
		float twin,float resolution,float tcoincidence,
		int ns,float *set,pmt_geometry *geom,
		int n_raw,int *cable,float *tim_raw,float *chg_raw);
  inline hitsel(float dlim,float tlim,
		float twin,float resolution,float tcoincidence,
		int ns,float *set,pmt_geometry *geom,comtype2 *itevent);
  inline hitsel(int ns,float *set,pmt_geometry *geom,comtype2 *itevent);
  ~hitsel(void);
  inline int nselected(void);
  inline short int sel(int i);
  inline float tpeak(void);
  inline int clean(int i,int j,float d2lim,float tlim);
  inline int causal(int i,int j,float twin,float resolution,float tcoincidence);
  inline int vertex4(int *hits,double *vert);
  inline void eigen(double *ctrd,float *axes);
  void pmtcentroid(int n,float *ctrd,float *axes,float rmax,float zmax);

  short int mrclean(float dlim,float tlim);
  short int clus_sel(float dlim,float tlim,
		     float twin,float resolution,float tcoincidence);
  inline void qsort(void);
  void printset(int nset,short int *set);
 };
#include "hitsel.inline"
#endif
