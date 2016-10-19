#include <stdio.h>
#include "BONSAI/vertexfit.h"

vertexfit *vf;

void bsloadinit(float res)
{
  vf=new vertexfit(res);
}

void bsinit(int np,float *ps,float res)
{
  vf=new vertexfit(np,ps,res);
}

void bsexit(void)
{
  delete(vf);
}

void bsloadhits(int nhit,int *cab,int *badch,float *t,float *q)
{
  vf->loadhits(nhit,cab,badch,t,q);
}

float bstimedirfit(short int fast,float *vertex,float *direct,float *dt)
{
  if (fast>1)
    return(vf->fittime_lcor(fast-2,vertex,direct,*dt));
  else
    return(vf->fittime(fast,vertex,direct,*dt));
}

void bssetcang(float cang0,float pang,float mang)
{
  vf->set_cang(cang0,pang,mang);
}

void bssetfactor(float d,float t)
{
  vf->set_depth(d);
  vf->set_time(t);
}

float bsvertexfit(float *vertex)
{
  float gdn[vf->sets()],ll;

  vf->search();
  vf->bestvertex(vertex);
  return(vf->goodness(ll,vertex,gdn));
}
