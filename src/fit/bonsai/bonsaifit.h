#ifndef BONSAIFIT
#define BONSAIFIT
#include "fitquality.h"
#include "searchgrid.h"

//#define DEBUG
//#define DEBUG_TWO

// *************************************************************
// * maximize the vertex quality function  provided by qual    *
// *************************************************************
class bonsaifit
{
  fitquality   *qual;           // quality criterion to be maximized
  float        rmax,rmax2,zmax; // maximum allowed x^2+y^2, |z|
  short int    nbranch,current; // number of branches and current br. 
  short int    best,worst;      // best and worst fit branch
  float        *branches;       // positions, radii, qualities,...
  float        rad;             // active radius
  float        *vertex,*q;      // active search positons, qualities

  // find deviation to fiducial volume edge, adjust search rad
  inline void      fiducial_dev(float &inter_x,float &inter_y,
				float &inter_z,float &dev);
  // set position out of vertex array and set end radius
  inline void      end_search(float *point,int indx,int indy,int indz,
			      float rmax);
  // find minimum and maximum quality
  inline void      find_extreme(void);
  // de-activate all branches with quality<minqual
  inline void      ream(float delta_qual);
  // if the range of qualities is above reamthr, ream off the
  // upper reamfrac portion of the qualities
  inline void      ream(float reamthr,float reamfrac);
  // combine searches that are close to each other
  inline void      sphere(void);

public:
  // set number of branches to zero
  inline bonsaifit(fitquality *qu);
  // destroy branches
  inline ~bonsaifit(void);
  // search for maximum from start with radius rmax
         void      search(float *start,float rmax);
  // calculate the qualities for the initial points; set search radii
  inline void search(float radius,searchgrid *grid,int set);
  // if the quality range between best and worst fit>reamthr, keep
  // active only reamfrac of the branches; then find the best for each
  // each active branch with cut-off radius rmax
  inline void search(float reamthr,float reamfrac,float rmax);
  // if the quality range between best and worst fit>reamthr, keep
  // active only reamfrac of the branches; then average close points
  // together and find the best fit for each remaining branch (after
  // increasing search radii<rmin) with cut-off radius rmax
  inline void search(float radius,float rmax,float *point);
  // calculate the quality for one initial point, set search
  // radius to radius, optimize until search radius<rmax
  inline void search(float reamthr,float reamfrac,
		     float rmin,float rmax);
  // print out all currently active branches
  inline void print_branch_list(void);
  // find best fit at the end of the search
  inline void search(float reamthr,float reamfrac);
  inline float xfit(void);     // return best x position
  inline float yfit(void);     // return best y position
  inline float zfit(void);     // return best z position
  inline float maxq(void);     // return quality of best fit position
  inline float fitrad(void);   // return radius of best fit position
  inline void  fitresult(void);// return best fit result
};

#include "bonsaifit.inline"
#endif
