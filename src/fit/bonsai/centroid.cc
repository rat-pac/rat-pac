#include <math.h>
#include "BONSAI/centroid.h"

 
// **********************************************
// align unitary matrix with a given vector
// **********************************************
short int axes::align(double *nor)
{
  double    sprod1,sprod2,sprod3;
  short int maxprod;

  nor[3]=*nor*(*nor)+nor[1]*nor[1]+nor[2]*nor[2];
  if (nor[3]<=0) return(-1);
  nor[3]=sqrt(nor[3]);
  sprod1=1/nor[3];
  *nor*=sprod1;
  nor[1]*=sprod1;
  nor[2]*=sprod1;
  sprod1=fabs(*nor*(*vectors)+nor[1]*vectors[3]+nor[2]*vectors[6]);
  sprod2=fabs(*nor*vectors[1]+nor[1]*vectors[4]+nor[2]*vectors[7]);
  sprod3=fabs(*nor*vectors[2]+nor[1]*vectors[5]+nor[2]*vectors[8]);
  if (sprod1>sprod2)
    if (sprod1>sprod3) maxprod=0; else maxprod=2;
  else
    if (sprod2>sprod3) maxprod=1; else maxprod=2;
  if (maxprod!=2)
    {
      sprod1=vectors[2];
      sprod2=vectors[5];
      sprod3=vectors[8];
    }
  vectors[2]=*nor; vectors[5]=nor[1]; vectors[8]=nor[2];
  if (maxprod!=2)
    {
      vectors[maxprod]=sprod1;
      vectors[maxprod+3]=sprod2;
      vectors[maxprod+6]=sprod3;
    }
  sprod1=fabs(*nor*(*vectors)+nor[1]*vectors[3]+nor[2]*vectors[6]);
  sprod2=fabs(*nor*vectors[1]+nor[1]*vectors[4]+nor[2]*vectors[7]);
  if (sprod1<sprod2)
    {
      vectors[1]=vectors[5]*vectors[6]-vectors[8]*vectors[3];
      vectors[4]=vectors[8]*vectors[0]-vectors[2]*vectors[6];
      vectors[7]=vectors[2]*vectors[3]-vectors[5]*vectors[0];
      sprod1=vectors[1]*vectors[1]+vectors[4]*vectors[4]+vectors[7]*vectors[7];
      sprod1=1/sqrt(sprod1);
      vectors[1]*=sprod1;
      vectors[4]*=sprod1;
      vectors[7]*=sprod1;

      vectors[0]=vectors[4]*vectors[8]-vectors[7]*vectors[5];
      vectors[3]=vectors[7]*vectors[2]-vectors[1]*vectors[8];
      vectors[6]=vectors[1]*vectors[5]-vectors[4]*vectors[2];
    }
  else
    {
      vectors[0]=vectors[4]*vectors[8]-vectors[7]*vectors[5];
      vectors[3]=vectors[7]*vectors[2]-vectors[1]*vectors[8];
      vectors[6]=vectors[1]*vectors[5]-vectors[4]*vectors[2];
      sprod1=vectors[0]*vectors[0]+vectors[3]*vectors[3]+vectors[6]*vectors[6];
      sprod1=1/sqrt(sprod1);
      vectors[0]*=sprod1;
      vectors[3]*=sprod1;
      vectors[6]*=sprod1;

      vectors[1]=vectors[5]*vectors[6]-vectors[8]*vectors[3];
      vectors[4]=vectors[8]*vectors[0]-vectors[2]*vectors[6];
      vectors[7]=vectors[2]*vectors[3]-vectors[5]*vectors[0];
    }
  return(maxprod);
}

/* add in quadrature */
inline double centroid::pythag(double a,double b)
{
  double ma=fabs(a),mb=fabs(b),rat;

  if (ma>mb)
    {
      rat=mb/ma;
      return(ma*sqrt(1+rat*rat));
    }
  if (mb==0) return(0);
  rat=ma/mb;
  return(mb*sqrt(1+rat*rat));
}

/* test zero offdiagonal element */
inline int centroid::iszero(int sta)
{
  double add=fabs(matrix[sta+1])+fabs(matrix[sta]);
  return(add+fabs(matrix[sta+3])==add);
}

/* first a plane rotation, then a Givens rotation */
int centroid::planegivens(axes &ax,double shift)
{
  double si,co,sub,siodiag,coodiag,denom;
  int    ii;

  shift+=matrix[2];
  si=co=1;
  sub=0;
  for(ii=1; ii>=0; ii--)
    {
      siodiag=si*matrix[ii+3];
      coodiag=co*matrix[ii+3];
      denom=pythag(siodiag,shift);
      matrix[ii+4]=denom;
      if (denom==0)
	{
	  matrix[ii+1]-=sub;
	  matrix[5]=0;
	  return(-1);
	}
      si=siodiag/denom; /* do rotation */
      co=shift/denom;
      shift=matrix[ii+1]-sub;
      denom=(matrix[ii]-shift)*si+2*co*coodiag;
      sub=si*denom;
      matrix[ii+1]=shift+sub;
      shift=co*denom-coodiag;
      ax.rotate(ii,si,co);
    }
  matrix[0]-=sub;
  matrix[3]=shift;
  matrix[5]=0;
  return(0);
}

/* tridiagonalize matrix with a Householder transformation */
void centroid::tridiag(axes &ax)
{
  double n=pythag(matrix[3],matrix[5]);
  if (n==0) return;
  double u1=matrix[3]-n,u2=matrix[5],u=u1*u1+u2*u2;
  if (u==0) return;

  double h=2/u;
  double p1=h*(matrix[1]*u1+matrix[4]*u2);
  double p2=h*(matrix[4]*u1+matrix[2]*u2);

  ax.set(0,1,0,        0);
  ax.set(1,0,1-u1*u1*h, -u1*u2*h);
  ax.set(2,0, -u1*u2*h,1-u2*u2*h);

  h*=0.5*(u1*p1+u2*p2);
  p1-=h*u1;
  p2-=h*u2;

  matrix[1]-=2*p1*u1;  matrix[4]-=p1*u2+p2*u1;  matrix[5]=0;  
                       matrix[2]-=2*p2*u2;
                                                matrix[3]=n;
}

// **********************************************
// generate the center of gravity of an array of
// points (matrix[0] to matrix[2]) as well as the
// variation matrix (matrix[3] to matrix[8])
// **********************************************
centroid::centroid(int n,float *points,float *weight)
{
  int    i;
  double x,y,z,w,ws;
 
  matrix=centr+3;
  for(i=0; i<9; i++) centr[i]=0;
  ws=0;
  if (n<3) return;
  for(i=0; i<n; i++)
    {
      x=*points++; y=*points++; z=*points++; w=*weight++;
      if (w>0)
	{
	  *centr+=x*w;
	  centr[1]+=y*w;
	  centr[2]+=z*w;
	  centr[3]+=x*x*w;
	  centr[4]+=y*y*w;
	  centr[5]+=z*z*w;
	  centr[6]+=x*y*w;
	  centr[7]+=y*z*w;
	  centr[8]+=x*z*w;
	  ws+=w;
	}
    }
  if (ws==0) return;
  for(i=0; i<9; i++) centr[i]/=ws;
  for(i=0; i<3; i++) matrix[i]-=centr[i]*centr[i];
  matrix[3]-=*centr*centr[1];
  matrix[4]-=centr[1]*centr[2];
  matrix[5]-=*centr*centr[2];
}

centroid::centroid(hitsel *h)
{
  int    i;

  matrix=centr+3;
  for(i=0; i<9; i++) centr[i]=0;
  if (h->nselected()<3) return;
  for(i=0; i<h->nselected(); i++)
    h->addhitloc(centr,h->sel(i),1);
  for(i=0; i<9; i++) centr[i]/=h->nselected();
  for(i=0; i<3; i++) matrix[i]-=centr[i]*centr[i];
  matrix[3]-=*centr*centr[1];
  matrix[4]-=centr[1]*centr[2];
  matrix[5]-=*centr*centr[2];
}

// -------------------------------------------------------------------
// find eigenvalues and vectors of a 3x3 matrix
// -------------------------------------------------------------------
void centroid::eigen(axes &ax)
{
  double rat,root,add,si,co;
  int   sta,iter;

  tridiag(ax); //tridiagonalize
  for(sta=0; sta<2; sta++)
    for(iter=0; iter<20; iter++)
      {
	if (iszero(sta)) break;
	rat=(matrix[sta+1]-matrix[sta])/(2*matrix[sta+3]);
	root=pythag(1,rat);
	if (rat*root<0) root=-root;
	root+=rat;
	add=matrix[sta+3]/root;
	/* in case a simple plane rotation is needed */
	if ((sta==1) || iszero(sta+1))
	  {
	    co=root/pythag(1,root);
	    si=co/root;
	    matrix[sta]-=add;
	    matrix[sta+1]+=add;
	    matrix[sta+3]=0;
	    ax.rotate(sta,si,co);
	    return;
	  }
	/* otherwise do a QL algorithm with implicit shift */
	planegivens(ax,add-matrix[sta]);
      }
  return;
}
