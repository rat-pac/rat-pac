#ifndef SEARCHGRID
#define SEARCHGRID

// *************************************************************
// * manage a set of 3D points to use as initial search grid   *
// * for a vertex fitter                                       *
// *************************************************************
class searchgrid
{
  short int        npoint,mpoint;  // number and maximum number of points
  short int        *mult;          // number of points that were averaged
  short int        nsparse;        // number average point sets;
  short int        *set_starts;    // begin index of a point set
  double           *points;        // point array
  double           rmax,rmax2,zmax;// maximum allowed radius radius^2 and |z|

  // find distance^2 between two points p1 and p2
  inline double    dis2(double &dx,double &dy,double &dz,int p1,int p2);
  // find the first point in interval (p2min,p2max) to which
  // point p1 is closer than sqrt(d2max)
  inline int       close_to(double &dx,double &dy,double &dz,
			    int p1,int p2min,int p2max,double d2max);
  // tests, if a point is inside the allowed fitting volume
  inline short int fit_volume(double px,double py,double pz);

 protected:
  // create empty grid from packed structure
         searchgrid(float * geom);
  // add a point, if inside the allowed fitting volume
  inline void      add_point(double px,double py,double pz);
  // add points out of a packed structure
         short int add_point(void * buffer);
  // increase the size of the point array
  inline void      expand_size(short int addsize);

 public:
  // create empty grid
  inline searchgrid(double r,double z,double dwall);
  // destroy grid
  inline ~searchgrid(void);
  // close current set of points and start a new set
  inline void close(void);
  // average together points that are closer to each other than dmin
         void sparsify(float d2min);
  // return number of grid point sets
  inline int  nset(void);
  // return size of a set of grid points
  inline int  size(int set);
  // copy a set of grid points to an array p of dimension dim with an
  // offset offx (x-coordinate), offy (y-coordinate) and offz (z-coordinate)
  inline void copy_points(int set,float *p,int dim,int offx,int offy,int offz);
  // pack a set of grid points into a buffer, if there's enough space
  void packset(void *buffer,short int max_size,short int set);
};

#include "searchgrid.inline"
#endif
