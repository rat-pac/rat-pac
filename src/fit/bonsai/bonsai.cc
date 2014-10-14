#include <stdio.h>
#include "bonsai.h"
#include "vertex.h"

inline void bonsai::makebranch(int index)
{
  point_index=index;
  nbranch=0;
  branches=NULL;
}

inline bonsai::bonsai(int index)
{
  makebranch(index);
  if (fit->ismax(point_index))
    maxbranch=this;
} 

bonsai *bonsai::maxbranch;
vertex *bonsai::fit;

bonsai::bonsai(int index,vertex *vfit)
{
  fit=vfit;
  makebranch(index);
  maxbranch=this;
}

bonsai::bonsai(int n,int *index,vertex *vfit)
{
  int i;

  fit=vfit;
  point_index=*index;
  nbranch=n-1;
  branches=new bonsai *[nbranch];
  maxbranch=this;
  for(i=0; i<nbranch; i++)
    branches[i]=new bonsai(index[i+1]);
}

void bonsai::sprout(void)
{
  int    n;
  if (branches!=NULL)
    for(n=0; n<nbranch; n++)
      if (branches[n]!=NULL)
	branches[n]->sprout();
  if (point_index<0) return;

  int    *newi;

  n=fit->surround(point_index,newi);
  if (n==0) return;

  if (branches==NULL)
    {
      nbranch=n;
      branches=new bonsai*[nbranch];
      for(n=0; n<nbranch; n++)
	branches[n]=new bonsai(newi[n]);
      delete newi;
      return;
    }

  int    i;
  bonsai **newbranches;

  newbranches=new bonsai *[nbranch+n];
  for(i=0; i<nbranch; i++) newbranches[i]=branches[i];
  delete branches;
  branches=newbranches;
  for(i=0; i<n; i++)
    branches[nbranch+i]=new bonsai(newi[i]);
  delete newi;
  nbranch+=n;
  return;
}

int bonsai::prune(void)
{
  int i,active,sum=0;

  for (i=0; i<nbranch; i++)
    if (branches[i]!=NULL)
      {
        active=branches[i]->prune();
	if (active==0) killbranch(i); else sum+=active;
      }
  if (sum==0)
    {
      if (branches!=NULL) delete branches;
      branches=NULL;
      nbranch=0;
      if (point_index<0) return(1);
    }
  if (point_index>=0)
    if (fit->toobad(point_index))
      point_index=-point_index-1;
    else
      sum++;
  return(sum);
}

void bonsai::print(int it)
{
  int i,j;

  j=0;
  if (point_index>=0)
    {
      fit->print(point_index);
      j=1;
    }
  for(i=0; i<nbranch; i++)
    if (branches[i]!=NULL)
      {
	if (j) for(j=0; j<it; j++) printf("    ");
	printf("%2d. ",i+1);
	branches[i]->print(it+1);
      }
}
