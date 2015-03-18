#include <stdio.h>
#include "RAT/BONSAI/tree.h"

namespace BONSAI {

// **********************************************
// new bonsai is empty
// **********************************************
tree::tree(void): pot()
{
  set_depth(0.03);
  set_time(0.0015);
  root=NULL;
}

// **********************************************
// load hits and do search
// **********************************************
tree::tree(int n_raw,int *cable,int *bad_ch,float *pmt_loc,
	   float *tim_raw,float *chg_raw):
  pot(n_raw,cable,bad_ch,pmt_loc,tim_raw,chg_raw)
{
  set_depth(0.03);
  set_time(0.0015);
  search();
}

// **********************************************
// kill bonsai
// **********************************************
tree::~tree(void)
{
  if (root!=NULL) delete(root);
}

// **********************************************
// do tree search with bonsai, tracing a range
// of branches
// **********************************************
void tree::search()
{
  int   i,active=0;

  pot::search();
  root=new bonsai(bestid(),this);
  timefactor=32;
  root->sprout();
  for(i=0; (i<15) && ((bestradius()>1) || (timefactor>0.01)); i++)
    {
      if (timefactor>1e-20)
        timefactor*=exp(-depthfactor*depth()-tfactor*ntest());
      if (active*tfactor<5) root->sprout();
      active=root->prune();
    }
  return;
}

// **********************************************
// do tree search with bonsai, tracing a range
// of branches from a specified starting vertex
// **********************************************
void tree::search(float *vert,float rad,int maxiter)
{
  int   i,active=0;

  purge();
  if (root!=NULL) delete(root);
  addvertex(vert,rad);
  root=new bonsai(bestid(),this);
  timefactor=10;
  root->sprout();
  for(i=0; (i<maxiter) && ((bestradius()>1) || (timefactor>0.01)); i++)
    {
      timefactor*=exp(-depthfactor*depth()-tfactor*ntest());
      if (active*tfactor<5) root->sprout();
      active=root->prune();
    }
  return;
}

}
