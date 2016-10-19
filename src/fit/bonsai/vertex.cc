// Standard Parameter
#define DISLIMIT 1250.      // Mr. Clean cut on distance
#define TLIMIT     35.      // Mr. Clean cut on time
#define TWIN      220.      // maximum possible timing difference
                            // between direct light hits
#define TCLOSE      3.      // timing difference to pass unchecked
#define TRES        3.      // timing resolution
#define CANG0       0.7     // cos of the Cherenkov angle
#define PLUSDANG    0.05    // positive error
#define MINUSDANG   0.05    // negative error

#include <stdio.h>
#include "BONSAI/vertex.h"

// **********************************************
// creates array
// **********************************************
vertex::vertex(void):
  timefit()
{
  vert=new float[NVERTFLOAT];
  maxvert=NVERTFLOAT;
  nvert=0;
  bestfit=worstfit=NULL;
  set_cylinder(0,0,0);
  set_hitsel(DISLIMIT,TLIMIT,TWIN,TCLOSE,TRES);
  set_cang(CANG0,PLUSDANG,MINUSDANG);
}

// **********************************************
// creates array and fills hits
// **********************************************
vertex::vertex(int n_raw,int *cable,int *bad_ch,float *pmt_loc,
			 float *tim_raw,float *chg_raw): timefit()
{
  vert=new float[NVERTFLOAT];
  maxvert=NVERTFLOAT;
  set_cylinder(0,0,0);
  set_hitsel(DISLIMIT,TLIMIT,TWIN,TCLOSE,TRES);
  set_cang(CANG0,PLUSDANG,MINUSDANG);
  loadhits(n_raw,cable,bad_ch,pmt_loc,tim_raw,chg_raw);
}

// **********************************************
// destroys array
// **********************************************
vertex::~vertex(void)
{
  if (vert!=NULL) delete(vert);
  nvert=maxvert=0;
}

// **********************************************
// free and recreate hitsel object with new times
// **********************************************
void vertex::loadhits(int n_raw,int *cable,int *bad_ch,
	              float *pmt_loc,float *tim_raw,float *chg_raw)
{
  if (event_hits!=NULL) delete(event_hits);
  event_hits=new hitsel(dislimit,timlimit,twin,tres,tclose,
                  sets(),chargebins(),pmt_loc,
		  bad_ch,n_raw,cable,tim_raw,chg_raw);
  nvert=0;
}


// **********************************************
// creates new vertices around an existing one
// **********************************************
int vertex::surround(int index,int *&newi)
{
  float *new_vert,x,y,z,newr;
  int   i,n,cind;
  axes  ax(vert[index+STORETHETA],
	   vert[index+STOREPHI],
	   vert[index+STOREALPHA]);

  if (nvert+NVERTFLOAT*plat.ndir()>=maxvert)
    {
      int add=maxvert/(500*NVERTFLOAT);

      if (add<1) add=500; else add*=500;
      addsize(add);
    }
  newi=new int[plat.ndir()];
  new_vert=new float[3*plat.ndir()];
  *new_vert=vert[index+STOREX];
  new_vert[1]=vert[index+STOREY];
  new_vert[2]=vert[index+STOREZ];
  plat.surround(new_vert,ax,vert[index+STORERAD]);
  vert[index+STORERAD]*=0.36;
  newr=2*vert[index+STORERAD];
  for(n=i=0; i<plat.ndir(); i++)
    {
      x=new_vert[3*i];
      y=new_vert[3*i+1];
      z=new_vert[3*i+2];
      if (inside_cylinder(x,y,z))
	{
	  cind=closest_dist2(new_vert+3*i,0.01*newr*newr);
	  if (cind>=0)
	    {
	      if (vert[cind+STORERAD]<newr)
		vert[cind+STORERAD]=newr;
	      continue;
	    }
	  set(x,y,z,newr);
	  vert[nvert]=test(1);
	  if (vert[nvert]>*bestfit) bestfit=vert+nvert;
	  if (vert[nvert]<*worstfit) worstfit=vert+nvert;
	  if (!toobad(nvert))
	    newi[n++]=nvert;
	  nvert+=NVERTFLOAT;
	}
    }
  delete new_vert;
  return(n);
}

float vertex::bestwall(float dwall,float rmin,float *wallv)
{
  int   best,i;
  float dx,dy,dz,d2,r2c;

  r2c=rpmt-dwall;
  r2c*=r2c;
  rmin*=rmin;
  for(best=-1,i=0; i<nvert; i+=NVERTFLOAT)
    {
      dx=vert[i+STOREX];
      dy=vert[i+STOREY];
      dz=vert[i+STOREZ];
      if ((dx*dx+dy*dy>r2c) || (fabs(dz)>zpmt-dwall))
        {
          dx-=bestfit[STOREX]; if (fabs(dx)<1e-5) dx=0;
          dy-=bestfit[STOREY]; if (fabs(dy)<1e-5) dy=0;
          dz-=bestfit[STOREZ]; if (fabs(dz)<1e-5) dz=0;
          d2=dx*dx+dy*dy+dz*dz;
          if (d2>rmin)
	    if ((best==-1) || (vert[i]>vert[best]))
	      best=i;
	}
    }
  if (best==-1) return(-1e10);
  *wallv=vert[best+STOREX];
  wallv[1]=vert[best+STOREY];
  wallv[2]=vert[best+STOREZ];
  wallv[3]=vert[best+STORET];
  return(vert[best]-*bestfit);
}


// **********************************************
// checks, if two vertices are too close together
// (for testing to be sensible)
// **********************************************
inline int vertex::closest_dist2(float *v,float lim)
{
  int   i;
  float d2,dx,dy,dz;

  dx=*v-vert[STOREX];   if (fabs(dx)<1e-5) dx=0;
  dy=v[1]-vert[STOREY]; if (fabs(dy)<1e-5) dy=0;
  dz=v[2]-vert[STOREZ]; if (fabs(dz)<1e-5) dz=0;
  d2=dx*dx+dy*dy+dz*dz;
  if (d2<lim) return(0);
  for(i=NVERTFLOAT+STOREX; i<nvert; i+=NVERTFLOAT)
    {
      dx=*v-vert[i];     if (fabs(dx)<1e-5) dx=0;
      dy=v[1]-vert[i+1]; if (fabs(dy)<1e-5) dy=0;
      dz=v[2]-vert[i+2]; if (fabs(dz)<1e-5) dz=0;
      if (dx*dx+dy*dy+dz*dz<d2)
	d2=dx*dx+dy*dy+dz*dz;
      if (d2<lim) return(i-1);
    }
  return(-1);
} 

// delete all vertices except the worst fit
void vertex::purge(void)
{
  int i;
  if (nvert<NVERTFLOAT) return;
  bestfit=worstfit=vert;
  nvert=0;
  return;
  if (nvert<=NVERTFLOAT) return;
  if (bestfit==worstfit) return;
  for(i=0; i<NVERTFLOAT; i++)
    vert[i]=worstfit[i];
  bestfit=worstfit=vert;
  nvert=NVERTFLOAT;
}
