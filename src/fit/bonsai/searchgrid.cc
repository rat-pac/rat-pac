#include "RAT/BONSAI/searchgrid.h"
#define MAXSHORT 32767

namespace BONSAI {

// *************************************************************
// * create empty grid from packed structure                   *
// *************************************************************
searchgrid::searchgrid(float *geom)
{
  printf("other constructor\n");
  rmax=geom[0];
  rmax2=rmax*rmax;
  zmax=geom[1];
  npoint=mpoint=nsparse=0;
  printf("done\n");
}

// *************************************************************
// * find distance^2 between two points p1 and p2              *
// *************************************************************
inline double searchgrid::dis2(double &dx,double &dy,double &dz,int p1,int p2)
{
  dx=points[3*p1]-points[3*p2];     // difference in x coordinate
  dy=points[3*p1+1]-points[3*p2+1]; // difference in y coordinate
  dz=points[3*p1+2]-points[3*p2+2]; // difference in z coordinate

  return(dx*dx+dy*dy+dz*dz);
}

// *************************************************************
// * find the first point in interval (p2min,p2max) to which   *
// * point p1 is closer than sqrt(d2max)                       *
// *************************************************************
inline int searchgrid::close_to(double &dx,double &dy,double &dz,
				int p1,int p2min,int p2max,double d2max)
{
  // loop through point p2 from p2min to p2max
  for(; p2min<p2max; p2min++)
    // stop, if distance is closer than limit
    if (dis2(dx,dy,dz,p1,p2min)<d2max) return(p2min);
  return(-1);
}

//--------------------------------------------------------------
//protected

// *************************************************************
// add points out of a packed structure
// *************************************************************
short int searchgrid::add_point(void *buffer)
{
  double    rfac,zfac;
  short int *ibuffer=((short int *) buffer)+4,np;

  np=*ibuffer++;
  if (np<0) return(0);
  if (npoint+np>mpoint)
    expand_size(npoint+np-mpoint);
  rfac=rmax/MAXSHORT;
  zfac=zmax/MAXSHORT;
  for(np=0; npoint<mpoint; npoint++,np++)
    {
      points[3*npoint]=rfac*ibuffer[3*np];
      points[3*npoint+1]=rfac*ibuffer[3*np+1];
      points[3*npoint+2]=zfac*ibuffer[3*np+2];
      mult[npoint]=1;
    }
  return(np);
}

//--------------------------------------------------------------
//public

// *************************************************************
// * average together points that are closer to each other     *
// * than dmin                                                 *
// *************************************************************
void searchgrid::sparsify(float dmin)
{
  short int point,start,stop,sec;
  double    dx,dy,dz,multfac;

  // if there are no points, quit
  if (npoint==0) return;
  // if no averageing has been done before, reserve
  // space for the point array containing the average
  if (nsparse==0)
    {
      start=0;
      set_starts=new short int[1];
      stop=set_starts[0]=npoint;
      nsparse=1;
    }
  else
    { // otherwise, extend existing average point array
      short int *save_starts=set_starts;

      start=set_starts[nsparse-1];
      set_starts=new short int[nsparse+1];
      for(point=0; point<nsparse; point++)
	set_starts[point]=save_starts[point];
      delete save_starts;
      stop=set_starts[nsparse]=npoint;
      nsparse++;
    }
  if (stop==start) return; // nothing to average
  // add space for an additional stop-start points
  if (npoint>=mpoint) expand_size(stop-start);
  dmin*=dmin; // square minimum distance

  // copy first point
  points[3*npoint]=points[3*start];
  points[3*npoint+1]=points[3*start+1];
  points[3*npoint+2]=points[3*start+2];
  mult[npoint++]=1;
  // loop through all other points
  for(point=start+1; point<stop; point++)
    { // find another point close-by?
      sec=close_to(dx,dy,dz,point,stop,npoint,dmin);
      if (sec==-1)
	{ // no, then copy the pont
	  if (npoint>=mpoint) expand_size(stop-point);
	  points[3*npoint]=points[3*point];
	  points[3*npoint+1]=points[3*point+1];
	  points[3*npoint+2]=points[3*point+2];
	  mult[npoint++]=1;
	}
      else
	{ // yes, then add the point to the one found
	  mult[sec]+=mult[point];
	  multfac=double(mult[point])/double(mult[sec]);
	  points[3*sec]+=dx*multfac;
	  points[3*sec+1]+=dy*multfac;
	  points[3*sec+2]+=dz*multfac;
	}
    }
}

// *************************************************************
// * pack a set of grid points into a buffer, if size of buffer*
// * is large enough                                           *
// *************************************************************
void searchgrid::packset(void *buffer,short int max_size,short int set)
{
  short int *ibuffer=(short int *) buffer,np=size(set),start,i;
  float     *fbuffer=(float *) buffer,val;

  if (max_size<10) return;
  ibuffer[4]=-1;
  if (np<=0) return;
  if (10+6*np>max_size) return;
  fbuffer[0]=rmax;
  fbuffer[1]=zmax;
  ibuffer+=4;
  *ibuffer++=np;
  if (set==0) start=0;              // first set;
  else                              // last set
    if ((set<0) || (set==nsparse)) start=3*set_starts[nsparse-1];
    else start=3*set_starts[set-1]; // any other set
   for(i=0; i<np; i++)
    {
      val=MAXSHORT*points[start+3*i]/rmax;
      if (val<0) *ibuffer++=-((short int) (0.5-val));
	 else    *ibuffer++=((short int) (0.5+val));
      val=MAXSHORT*points[start+3*i+1]/rmax;
      if (val<0) *ibuffer++=-((short int) (0.5-val));
	 else    *ibuffer++=((short int) (0.5+val));
      val=MAXSHORT*points[start+3*i+2]/zmax;
      if (val<0) *ibuffer++=-((short int) (0.5-val));
	 else    *ibuffer++=((short int) (0.5+val));
    }
  return;
}

}
