#ifndef GOODNESS
#define GOODNESS
#include "RAT/BONSAI/fit_param.h"
#include "RAT/BONSAI/fitquality.h"
#include "RAT/BONSAI/hitsel.h"
#include "RAT/BONSAI/bonsaifit.h"
#include "RAT/BONSAI/searchgrid.h"
#include "RAT/BONSAI/plato.h"
#include "RAT/BONSAI/likelihood.h"

namespace BONSAI {

// *************************************************************
// * Defines clusfit goodness function and maximization        *
// *************************************************************
class goodness: public fit_param,public fitquality, public hitsel
{
  float        cosc0,plusdang,minusdang,dirweight; // direction constraints
  float        *ttof,*tau,*hitdir;                 // arrays for time residuals and directions
  float        t0,cosc,theta,phi,gdns,gdn0;        // fit results
  float        twin,tcent,tnorm;                   // time window, ave. time, time normal.
  short int    *lists,*buffer,*sort_list,nbranch;  // for sorting
  short int    ngdn;                               // number of fit vertices
  dodecahedron dod;                                // defines how surrounding points
  axes         orientation;                        // are calculated

  inline void      create_lists(int n); // create hitsel sorting lists and buffer
         void      qsort(short int *list,short int n); // recursive QuickSort program
         void      bubble(void);        // Bubble sort program

 public:
  // construct hit, hitsel and goodness objects from zbs commons
  inline goodness(float r,float z,
		  int ns,float *set,float *pmt_loc,int *bad_ch,
		  int n_raw,int *cable,float *tim_raw,float *chg_raw);
  // construct hit, hitsel and goodness objects with time/charge list
  inline goodness(int ns,float *set,pmt_geometry *geom,
		  int n_raw,int *cable,float *tim_raw,float *chg_raw);
  // construct hit, hitsel and goodness objects with IT structure
  inline goodness(int ns,float *set,pmt_geometry *geom,comtype2 *itevent);
  // destroy goodness object
  inline ~goodness(void);
  // calculate goodness
         virtual float quality(float *vertex);
  // surround a point with test vertices
  inline virtual void check_around(float *vertex,float *result,
				   float radius,float *q,int &max);
  // return number of surrounding test vertices
  inline virtual char ncheck(void);
  // interpolate best fit position
  inline virtual void interpolate(float *vertex,
				  float radius,float *q,float *inter);
  inline virtual int nresult(void);          // return size of result array
  inline virtual void get_result(float *r);  // get result array
  inline virtual void set_result(float *r);  // set result array
  inline virtual void set_branch(short int point);  // switch memory to search branch
  // maximizing procedure
  inline void      maximize (bonsaifit *fit,searchgrid *grid);
  inline void      maximize (bonsaifit *cfit,bonsaifit *bfit,
			     likelihood *bslike,searchgrid *grid);
  inline float     get_zero(void);          // get t0
  inline float     get_gdn(void);           // get goodness
  inline float     get_gdn0(void);          // get goodness
  inline float     residual(short int hit); // get time residual
  inline float     get_theta(void);         // get zenith angle
  inline float     get_phi(void);           // get azimuth angle
  inline float     get_cos_theta(void);     // get opening angle
  inline int       nfit(void);              // get # of test vertices
};

// *************************************************************
// * create hitsel object and time-of-flight array             *
// *************************************************************
inline goodness::goodness(float r,float z,
		          int ns,float *set,float *pmt_loc,int *bad_ch,
		          int n_raw,int *cable,float *tim_raw,float *chg_raw):
fitquality(r,z,dwallfit()),
hitsel(2*pair_distance_limit_fraction()*(r+z),
       2*CM_TO_NS*pair_time_limit_fraction()*(r+z),
       2*CM_TO_NS*sqrt(r*r+z*z),tresolution(),tcoincidence(),
       ns,set,pmt_loc,bad_ch,n_raw,cable,tim_raw,chg_raw)
{
  cosc0=plusdang=minusdang=FIT_PARAM_NONE;
  cosc=theta=phi=0;
  ngdn=0;
  nbranch=0;
  if (nselected()>0)
    {
      ttof=new float[5*nselected()];
      tau=ttof+nselected();
      hitdir=ttof+2*nselected();
      buffer=new short int[2*nselected()];
      lists=buffer+nselected();
      nbranch=1;
      lists[0]=-1;
    }
}

// *************************************************************
// * create hitsel object and time-of-flight array             *
// *************************************************************
inline goodness::goodness(int ns,float *set,pmt_geometry *geom,
		          int n_raw,int *cable,float *tim_raw,float *chg_raw):
fitquality(geom->cylinder_radius(),geom->cylinder_height(),dwallfit()),
hitsel(pair_distance_limit_fraction()*geom->cylinder_circonference(),
       CM_TO_NS*pair_time_limit_fraction()*geom->cylinder_circonference(),
       CM_TO_NS*geom->cylinder_diagonal(),tresolution(),tcoincidence(),
       ns,set,geom,n_raw,cable,tim_raw,chg_raw)
{
  cosc0=plusdang=minusdang=FIT_PARAM_NONE;
  cosc=theta=phi=0;
  ngdn=0;
  nbranch=0;
  if (nselected()>0)
    {
      ttof=new float[5*nselected()];
      tau=ttof+nselected();
      hitdir=ttof+2*nselected();
      buffer=new short int[2*nselected()];
      lists=buffer+nselected();
      nbranch=1;
      lists[0]=-1;
    }
}

// *************************************************************
// * create hitsel object and time-of-flight array             *
// *************************************************************
inline goodness::goodness(int ns,float *set,pmt_geometry *geom,comtype2 *itevent):
fitquality(geom->cylinder_radius(),geom->cylinder_height(),dwallfit()),
hitsel(pair_distance_limit_fraction()*geom->cylinder_circonference(),
       CM_TO_NS*pair_time_limit_fraction()*geom->cylinder_circonference(),
       CM_TO_NS*geom->cylinder_diagonal(),tresolution(),tcoincidence(),
       ns,set,geom,itevent)
{
  cosc0=plusdang=minusdang=FIT_PARAM_NONE;
  cosc=theta=phi=0;
  ngdn=0;
  nbranch=0;
  if (nselected()>0)
    {
      ttof=new float[5*nselected()];
      tau=ttof+nselected();
      hitdir=ttof+2*nselected();
      buffer=new short int[2*nselected()];
      lists=buffer+nselected();
      nbranch=1;
      lists[0]=-1;
    }
}

// *************************************************************
// * destroy time-of-flight array and sorting lists and buffer *
// *************************************************************
inline goodness::~goodness(void)
{
  if (nselected()>0)
    {
      delete(ttof);
      delete(buffer);
    }
}

// *************************************************************
// * calculate dodecahedran of search radius and all thirteen  *
// * qualitites                                                *
// *************************************************************
inline void goodness::check_around(float *vertex,float *result,
		                   float radius,float *q,int &max)
{
  int i;

  orientation.set(result[2],result[3],0);
  dod.surround(vertex+3,orientation,radius);
  for(max=i=0; i<13; i++)
    {
      if ((q[i]=quality(vertex+3*i))>q[max]+1e-7)
	{
	  max=i;
	  *result=t0;
	  result[1]=gdn0;
	  result[2]=theta;
	  result[3]=phi;
	  result[4]=cosc;
	}
#ifdef DEBUG_TWO
      printf("%2d(%2d) (%8.2f,%8.2f,%8.2f,%6.1f): %8.6f (%6.1f,%6.1f) %4.1f\n",
	     i,max,vertex[3*i],vertex[3*i+1],vertex[3*i+2],t0,q[i],
	     theta*57.29578,phi*57.29578,acos(cosc)*57.29578);
#endif
    }
}

// *************************************************************
// * return number of checked points for each iteration        *
// *************************************************************
inline char goodness::ncheck(void)
{
  return(13);
}

// *************************************************************
// * calculate quality at interpolated point                   *
// *************************************************************
inline void goodness::interpolate(float *vertex,
                                  float radius,float *q,float *inter)
{
  dod.interpolate(q,inter);
  vertex[39]=vertex[0]+radius*orientation.getx(inter);
  vertex[40]=vertex[1]+radius*orientation.gety(inter);
  vertex[41]=vertex[2]+radius*orientation.getz(inter);
  q[13]=quality(vertex+39);
#ifdef DEBUG_TWO
  printf("interpolate to (%8.2f,%8.2f,%8.2f): %8.6f\n",
         vertex[39],vertex[40],vertex[41],q[13]);
#endif
}

// *************************************************************
// * create hitsel sorting lists and buffer                    *
// *************************************************************
inline void goodness::create_lists(int n)
{
  nbranch=n;
  delete(buffer);
  buffer=new short int[(nbranch+1)*nselected()];
  lists=buffer+nselected();
  for (n=0; n<nbranch; n++)
    lists[n*nselected()]=-1;
}

// *************************************************************
// * maximize goodness criterion using bonsaifit object        *
// *************************************************************
inline void goodness::maximize(bonsaifit *fit,searchgrid *grid)
{
  short int n;

  if (npass()<=0) return;
  if (grid->nset()<1) return;
  twin=grid_time_window();
  cosc0=clusfit_grid_cos_theta();
  plusdang=clusfit_grid_plus_deviation();
  minusdang=clusfit_grid_minus_deviation();
  dirweight=clusfit_grid_direction_weight();
#ifdef DEBUG
  printf("Initial search: time window=%8.2fns cos_theta=%6.2f",twin,cosc0);
  if (plusdang>0) printf("+%6.2f",sqrt(0.5/plusdang)); else printf("+none");
  if (minusdang>0) printf("-%6.2f\n",sqrt(0.5/minusdang)); else printf("\n");
#endif
  if (grid->nset()<2)
    {
      if (grid->size(0)+grid->size(1)<1) return;
      create_lists(grid->size(0)+grid->size(1));
      fit->search(minimum_radius(0),grid,1);
    }
  else
    {
      if(bongrid()<clusgrid())
        {
          if (grid->size(0)+grid->size(4)<1) return;
          create_lists(grid->size(0)+grid->size(4));
          fit->search(minimum_radius(0),grid,4);
        }
      else
        {
          if (grid->size(0)+grid->size(3)<1) return;
          create_lists(grid->size(0)+grid->size(3));
          fit->search(minimum_radius(0),grid,3);
        }
    }
  twin=time_window(0);
  cosc0=clusfit_cos_theta(0);
  plusdang=clusfit_plus_deviation(0);
  minusdang=clusfit_minus_deviation(0);
  dirweight=clusfit_direction_weight(0);

  if (walldist(fit->xfit(),fit->yfit(),fit->zfit())<clus_dwall(0))
    {
      fit->search(last_min_gdn_difference(),last_gdn_fraction());
#ifdef DEBUG
      printf(" 1 dwall=%8.2f<%8.2f: Stop fit\n",
             walldist(fit->xfit(),fit->yfit(),fit->zfit()),clus_dwall(0));
      fit->print_branch_list();
#endif
      return;
    }
#ifdef DEBUG
  printf(" 1. search: time window=%8.2fns cos_theta=%6.2f",twin,cosc0);
  if (plusdang>0) printf("+%6.2f",sqrt(0.5/plusdang)); else printf("+none");
  if (minusdang>0) printf("-%6.2f\n",sqrt(0.5/minusdang)); else printf("\n");
#endif
  fit->search(min_gdn_difference(0),gdn_fraction(0),stop_radius(0));
  for(n=1; n<npass(); n++)
    {
      if (walldist(fit->xfit(),fit->yfit(),fit->zfit())<clus_dwall(n))
        {
#ifdef DEBUG
          printf("%2d dwall=%8.2f<%8.2f: Stop fit\n",n+1,
                 walldist(fit->xfit(),fit->yfit(),fit->zfit()),clus_dwall(n));
#endif
	  break;
        }
      twin=time_window(n);
      cosc0=clusfit_cos_theta(n);
      plusdang=clusfit_plus_deviation(n);
      minusdang=clusfit_minus_deviation(n);
      dirweight=clusfit_direction_weight(n);
#ifdef DEBUG
      printf("%2d. search: time window=%8.2fns cos_theta=%6.2f",n+1,twin,cosc0);
      if (plusdang>0) printf("+%6.2f",sqrt(0.5/plusdang)); else printf("+none");
      if (minusdang>0) printf("-%6.2f\n",sqrt(0.5/minusdang)); else printf("\n");
#endif
      fit->search(min_gdn_difference(n),gdn_fraction(n),
	          minimum_radius(n),stop_radius(n));
    }
  fit->search(last_min_gdn_difference(),last_gdn_fraction());
#ifdef DEBUG
  fit->print_branch_list();
#endif
}

// *************************************************************
// * maximize goodness criterion using bonsaifit object and    *
// * BONSAI likelihood                                         *
// *************************************************************
inline void goodness::maximize(bonsaifit *cfit,bonsaifit *bfit,
                               likelihood *bslike,searchgrid *grid)
{
  short int n;

  if (npass()<=0) return;
  if (grid->nset()<1) return;
  twin=grid_time_window();
  cosc0=clusfit_grid_cos_theta();
  plusdang=clusfit_grid_plus_deviation();
  minusdang=clusfit_grid_minus_deviation();
  dirweight=clusfit_grid_direction_weight();
#ifdef DEBUG
  printf("Initial search: time window=%8.2fns cos_theta=%6.2f",twin,cosc0);
  if (plusdang>0) printf("+%6.2f",sqrt(0.5/plusdang)); else printf("+none");
  if (minusdang>0) printf("-%6.2f\n",sqrt(0.5/minusdang)); else printf("\n");
#endif
  if (grid->nset()<2)
    {
      if (grid->size(0)+grid->size(1)<1) return;
      create_lists(grid->size(0)+grid->size(1));
      cfit->search(minimum_radius(0),grid,1);
    }
  else
    {
      if(bongrid()<clusgrid())
        {
          if (grid->size(0)+grid->size(4)<1) return;
          create_lists(grid->size(0)+grid->size(4));
          cfit->search(minimum_radius(0),grid,4);
        }
      else
        {
          if (grid->size(0)+grid->size(3)<1) return;
          create_lists(grid->size(0)+grid->size(3));
          cfit->search(minimum_radius(0),grid,3);
        }
    }
  twin=time_window(0);
  cosc0=clusfit_cos_theta(0);
  plusdang=clusfit_plus_deviation(0);
  minusdang=clusfit_minus_deviation(0);
  dirweight=clusfit_direction_weight(0);

  if (walldist(cfit->xfit(),cfit->yfit(),cfit->zfit())<clus_dwall(0))
    {
      cfit->search(last_min_gdn_difference(),last_gdn_fraction());
#ifdef DEBUG
      printf(" 1 dwall=%8.2f<%8.2f: Stop fit\n",
             walldist(cfit->xfit(),cfit->yfit(),cfit->zfit()),clus_dwall(0));
      cfit->print_branch_list();
#endif
      return;
    }
#ifdef DEBUG
  printf(" 1. search: time window=%8.2fns cos_theta=%6.2f",twin,cosc0);
  if (plusdang>0) printf("+%6.2f",sqrt(0.5/plusdang)); else printf("+none");
  if (minusdang>0) printf("-%6.2f\n",sqrt(0.5/minusdang)); else printf("\n");
#endif
  cfit->search(min_gdn_difference(0),gdn_fraction(0),stop_radius(0));
  for(n=1; n<npass()-1; n++)
    {
      if (walldist(cfit->xfit(),cfit->yfit(),cfit->zfit())<clus_dwall(n))
        {
#ifdef DEBUG
          printf("%2d dwall=%8.2f<%8.2f: Stop fit\n",n+1,
                 walldist(cfit->xfit(),cfit->yfit(),cfit->zfit()),clus_dwall(n));
#endif
	  break;
        }
      twin=time_window(n);
      cosc0=clusfit_cos_theta(n);
      plusdang=clusfit_plus_deviation(n);
      minusdang=clusfit_minus_deviation(n);
      dirweight=clusfit_direction_weight(n);
#ifdef DEBUG
      printf("%2d. search: time window=%8.2fns cos_theta=%6.2f",n+1,twin,cosc0);
      if (plusdang>0) printf("+%6.2f",sqrt(0.5/plusdang)); else printf("+none");
      if (minusdang>0) printf("-%6.2f\n",sqrt(0.5/minusdang)); else printf("\n");
#endif
      cfit->search(min_gdn_difference(n),gdn_fraction(n),
	          minimum_radius(n),stop_radius(n));
    }
  cfit->search(last_min_gdn_difference(),last_gdn_fraction());
#ifdef DEBUG
  cfit->print_branch_list();
#endif
  if (walldist(cfit->xfit(),cfit->yfit(),cfit->zfit())<clus_dwall(n))
    {
#ifdef DEBUG
      printf("%2d dwall=%8.2f<%8.2f: Stop fit\n",n+1,
             walldist(cfit->xfit(),cfit->yfit(),cfit->zfit()),clus_dwall(n));
#endif
    }
  else
    {
      float clfit[3];

      clfit[0]=cfit->xfit();
      clfit[1]=cfit->yfit();
      clfit[2]=cfit->zfit();
      bslike->maximize(bfit,clfit);
    }
}

// *************************************************************
// * get last fitted light emission time                       *
// *************************************************************
inline float goodness::get_zero(void)
{
  return(t0);
}

// *************************************************************
// * get last fitted goodness                                  *
// *************************************************************
inline float goodness::get_gdn(void)
{
  return(gdns);
}

// *************************************************************
// * get last fitted goodness                                  *
// *************************************************************
inline float goodness::get_gdn0(void)
{
  return(gdn0);
}

// *************************************************************
// * get time residual hit assuming latest light emission time *
// *************************************************************
inline float goodness::residual(short int hit)
{
  return(ttof[hit]-t0);
}

// *************************************************************
// * get number of times, goodn was called                     *
// *************************************************************
inline int goodness::nfit(void)
{
  return(ngdn);
}

// *************************************************************
// * return size of output memory segment                      *
// *************************************************************
inline int goodness::nresult(void)
{
  return(5);
}

// *************************************************************
// * copy emission time to memory segment r                    *
// *************************************************************
inline void goodness::get_result(float *r)
{
  *r=t0;
  r[1]=gdn0;
  r[2]=theta;
  r[3]=phi;
  r[4]=cosc;
  return;
}

// *************************************************************
// * copy emission time from memory segment r                  *
// *************************************************************
inline void goodness::set_result(float *r)
{
  t0=*r;
  gdn0=r[1];
  theta=r[2];
  phi=r[3];
  cosc=r[4];
  return;
}

// *************************************************************
// * get zenith angle                                          *
// *************************************************************
inline float goodness::get_theta(void)
{
  return(theta);
}

// *************************************************************
// * get azimuthal angle                                       *
// *************************************************************
inline float goodness::get_phi(void)
{
  return(phi);
}

// *************************************************************
// * get opening angle                                         *
// *************************************************************
inline float goodness::get_cos_theta(void)
{
  return(cosc);
}

// *************************************************************
// * set sortlist to desired point                             *
// *************************************************************
inline void goodness::set_branch(short int point)
{
  sort_list=lists+nselected()*point;
  return;
}

}
#endif
