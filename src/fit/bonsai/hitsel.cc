#include <stdlib.h>

#include "BONSAI/centroid.h"
#define MINPRESEL  10         /* desired minimum number of preselected hits */
#define MINHIT      3         /* required minimum number of hits            */
#define MAXRAW   2000         /* maximum number of raw hits                 */
#define MAXSTOSIZE 500000000  /* maximal storage size for clusters*/
#define ALLOWED_SIZE_FRACTION 1.0

#define ARRAYSIZE MAXRAW*(MAXRAW-1)/2

// **********************************************
// does hit selection, wenn needed
// **********************************************

// **********************************************
// free hit selection array
// **********************************************
hitsel::~hitsel(void)
{
  delete [] selected;
}

// -------------------------------------------------------------------
// make table of causally related PMTs
// -------------------------------------------------------------------

short int hitsel::make_causal_table(short int *&related,
				    short int *&relations,
				    float twin,
				    float resolution,
				    float tcoincidence)
{
  short int row,column,pmt,pmt2,n_cl,high,low;
  short int *nr_rel,curnrel,*new_index,*select;
  short int *relp,*relp1,*relp2;
  short int removed_pmt;

  /*-------------------------------------------------------------------
    mark all relations as valid
    -------------------------------------------------------------------*/
  related=new short int[nsel*(nsel-1)/2];
  nr_rel=new short int[nsel];
  for(row=0,relp=related; row<nsel; row++)
    {
      for(column=row+1; column<nsel; column++)
	*(relp++)=0;
      nr_rel[row]=0;
    }
  /*-------------------------------------------------------------------
    establish possible relationships between individual hits
    -------------------------------------------------------------------*/
  for(row=0,relp=related; row<nsel; row++)
    for(column=row+1; column<nsel; column++)
      if (((*relp++)=2*causal(selected[row],selected[column],
			      twin,resolution,tcoincidence)))
	{
	  nr_rel[row]++;
	  nr_rel[column]++;
	}
  /*-------------------------------------------------------------------
    remove all PMT's which have a small number of relations
    -------------------------------------------------------------------*/
  do
    {
      removed_pmt=0;
      for(row=0; row<nsel; row++)
	{
	  if ((nr_rel[row]<MINHIT) && (nr_rel[row]>0))
	    {
	      removed_pmt=1;
	      if (row>0)
		{
		  relp=related+row-1;
		  for(column=0; column<row; relp+=nsel-2-(column++))
		    if (*relp)
		      {
			*(relp)=0;
			nr_rel[row]--;
			nr_rel[column]--;
		      }
		}
	      relp=related+row*nsel-row*(row+1)/2;
	      for(column=row+1; column<nsel; column++,relp++)
		if (*(relp))
		  {
		    *(relp)=0;
		    nr_rel[row]--;
		    nr_rel[column]--;
		  }
	    }
	}
    } while(removed_pmt);
  /* -------------------------------------------------------------------
     compactify the relation matrix and sort hits by number of relations
     -------------------------------------------------------------------*/
  select=new short int[nsel];
  relations=new short int[nsel];
  for(row=0,n_cl=0; row<nsel; row++)
    if ((curnrel=nr_rel[row])>=MINHIT)
      {
        pmt=selected[row];
        if (n_cl==0)
	   pmt2=n_cl;
	else
	   {
	     if (   (curnrel>*relations) ||
	            ( (curnrel==*relations) &&
                      (hitcharge(pmt)>hitcharge(*select)) )   )
	       pmt2=0;
	     else
	       {
	         if (   (curnrel<relations[n_cl-1]) ||
	                ( (curnrel==relations[n_cl-1]) &&
                          (hitcharge(pmt)<hitcharge(select[n_cl-1])) )   )
		   pmt2=n_cl;
		 else
		   {
		     pmt2=n_cl/2;
		     for(low=0,high=n_cl; high-low>1; pmt2=(low+high)/2)
	               if (   (curnrel>relations[pmt2]) ||
	                      ( (curnrel==relations[pmt2]) &&
                                (hitcharge(pmt)>hitcharge(select[pmt2])) )   )
			 high=pmt2;
		       else
			 low=pmt2;
		     pmt2=high;
		   }
	       }
	   }
        for(high=n_cl-1; high>=pmt2; high--)
	  {
	    select[high+1]=select[high];
	    relations[high+1]=relations[high];
	  }
	select[pmt2]=pmt;
	relations[pmt2]=curnrel;
	n_cl++;
      }
  delete [] nr_rel;
  delete [] relations;
  if (n_cl<=MINHIT)
    {
      delete [] select;
      delete [] related;
      return(-2); // error: too few clean hits
    }
  new_index=new short int[nsel];
  for(pmt=0; pmt<nsel; pmt++)
    for(new_index[pmt]=-1,pmt2=0; pmt2<n_cl; pmt2++)
      if (selected[pmt]==select[pmt2])
        {
	  new_index[pmt]=pmt2;
	  break;
        }
  delete [] selected;
  selected=new short int[n_cl];
  for(pmt=0; pmt<n_cl; pmt++)
    selected[pmt]=select[pmt];
  delete [] select;
  relp1=related;
  relp2=related;

  /* -------------------------------------------------------------------
     rebuild relation matrix and create lists of related hits
     -------------------------------------------------------------------*/
  related=new short int[n_cl*n_cl];
  relations=new short int[n_cl*(n_cl+2)];
  relp=related;
  for(row=0; row<n_cl; row++)
    for(relations[(n_cl+2)*row]=column=0; column<n_cl; column++)
      *(relp++)=0;
  for(row=0; row<nsel; row++)
    {
      pmt=new_index[row];
      for(column=row+1; column<nsel; column++,relp2++)
	if ((pmt>=0) && (*relp2))
	  {
	    pmt2=new_index[column];
	    if (pmt2>=0)
	      {
		related[pmt*n_cl+pmt2]=*(relp2);
		related[pmt+n_cl*pmt2]=*(relp2);
	      }
	  }
    }
  delete [] new_index;
  delete [] relp1;
  relp=related;
  nsel=n_cl+2;
  for(row=0; row<n_cl; row++)
    for(relp+=column=row+1,relp1=relations+nsel*row; column<n_cl; column++)
      if (*relp++)
        {
	  relp2=relations+nsel*column;
	  relp1[2+(relp1[0])++]=column;
	  relp2[2+(relp2[0])++]=row;
        }
  for(row=0; row<n_cl; row++)
    for(column=0,relp=relations+nsel*row+2,relp[-1]=relp[-2]; column<relp[-2]; column++)
      if (relp[column]>row)
        {
	  relp[-1]=column;
	  break;
	}
  nsel=n_cl;
  //for(row=0; row<n_cl; row++,printf("\n"))
  //  for(printf("%3d:",selected[row]),column=0; column<n_cl; column++)
  //    printf(" %d",causal(selected[row],selected[column],twin,resolution,tcoincidence));
  return(n_cl);
}

/*-------------------------------------------------------------------
  reduce multiple occurences of numbers from a list and return
  reduced list and number of occurence for each number in the list
  -------------------------------------------------------------------*/
short int hitsel::reduce(int       size,
			 short int *source,
			 short int *destination,
			 short int *occurence)
{
  int       nr,i,j;
  short int copy;

  nr=0;
  for(i=0; i<size; i++,source++)
    {
      copy=1;
      for (j=0; j<nr && copy; j++)
	if (*source==destination[j])
	    copy=0;
      if (copy)
	{
	  destination[nr]=*source;
	  occurence[nr++]=1;
	}
      else
	occurence[--j]++;
    }
  return(nr);
}


// **********************************************
// create selection array and do hit selection
// **********************************************
void hitsel::select(float dlim,float tlim,
		    float twin,float resolution,float tcoincidence)
{
  int       s,i;
  short int n;

  for(s=nset()-1; (s>=0) && (sumhits(s)<=MINPRESEL); s--);
  if (s<0) s=0;
  do
    {
      nsel=sumhits(s);
      selected=new short int[nsel];
      for(i=0; i<nsel; i++)
	selected[i]=first_hit(s)+i;
      s--;
    }
  while (((n=clus_sel(dlim,tlim,twin,resolution,
                      tcoincidence)) <= 0) && (s>=0));
  if (n<=0)
    {
      nsel=ntot();
      selected=new short int[nsel];
      for(i=0; i<nsel; i++)
	selected[i]=i;
      hits::qsort(selected,nsel);
    }
}

// -------------------------------------------------------------------
// find centroid of PMT hits and the three main axes
// -------------------------------------------------------------------
void hitsel::pmtcentroid(int n,float *ctrd,float *axes,float rmax,float zmax)
{
  double fac;
  int    i;

  for(i=0; i<9; i++) axes[i]=0;
  for(i=0; i<3; i++) ctrd[i]=0;
  axes[0]=axes[4]=axes[8]=1;
  if (n<3) return;

  centroid c(this);

  c.eigen(axes);
  fac=c.r();
  if (fac<rmax)
    {
      *ctrd=c.x();
      ctrd[1]=c.y();
    }
  else
    {
      fac=rmax/fac;
      *ctrd=fac*c.x();
      ctrd[1]=fac*c.y();
    }
  fac=fabs(c.z());
  if (fac<zmax) ctrd[2]=c.z(); else ctrd[2]=c.z()*zmax/fac;
  return;
}


// -------------------------------------------------------------------
// reject temporally and spatially isolated hits
// -------------------------------------------------------------------

short int hitsel::mrclean(float dislimit,float tlimit)
{
  float     dislimit2=dislimit*dislimit;
  short int row,column,pmt,n_cl;
  short int *is_selected;

  if (nsel<MINHIT)
    {
      delete [] selected;
      return(-1); // error: too few raw hits
    }

   /*-------------------------------------------------------------------
     get list of good hits: the Mr. Clean algorithm rejects isolated hits
     -------------------------------------------------------------------*/
  is_selected=new short int[nsel];
  for(row=0; row<nsel; row++) is_selected[row]=0;
  for(row=0; row<nsel; row++)
    for(column=0; (column<nsel) && (!is_selected[row]); column++)
      if (clean(selected[row],selected[column],dislimit2,tlimit))
	{   // select the non-isolated hit
	  is_selected[row]=1;
	  is_selected[column]=1;
	}
   /*-------------------------------------------------------------------
     add all selected hits to the list
     -------------------------------------------------------------------*/
  for(n_cl=pmt=0; pmt<nsel; pmt++)
    if (is_selected[pmt])
      is_selected[n_cl++]=selected[pmt];
  delete [] selected;
  selected=new short int[n_cl];
  if (n_cl<MINHIT)
    {
      delete [] is_selected;
      delete [] selected;
      return(-2);                // error: too few clean hits
    }
  for(pmt=0; pmt<n_cl; pmt++)
    selected[pmt]=is_selected[pmt];
  delete [] is_selected;
  nsel=n_cl;
  return(n_cl);
}

// -------------------------------------------------------------------
// find list of PMTs, each pair of them could be hit by
// direct light from a common vertex
// -------------------------------------------------------------------
short int hitsel::clus_sel(float dlim,         // spatial limit of Mr. Clean
			   float tlim,         // temporal limit of Mr. Clean
			   float twin,         // maximum possible t difference
			   float resolution,   // time resol. of PMTs
			   float tcoincidence) // maximum time difference
                                               // of perfect coincidence
     /* -> error status for the selection
	>0: selection o.k.; # of selected hits
	-1: too few raw hits
	-2: too few preselected hits
	-3: too few selected hits
	-5: no cluster found
        -6: time sorting error
	-7: double hit error
        -8: cluster storage overflow
	-9: too many raw hits */

     /*===================================================================
       function begin:
       -------------------------------------------------------------------*/
{
  short int row,column,pmt1,pmt2,pmt1_index,pmt2_index;
  int       n_gd,ntest,n_bd,n_clus,min_size,max_size,clus_size,size;
  int       max_index;
  short int new_pmt;
  short int *related,*relations,*cluster,**clusterp,*joined,*occur;
  short int *max_clus=NULL,*clus,*max_end,*select;
  short int *relp,*relp1,*relp2;

  if (nsel>MAXRAW)
    {
      delete [] selected;
      return(-9); // error: too many raw hits
    }
  if (nsel<MINHIT)
    {
      delete [] selected;
      return(-1); // error: too few raw hits
    }
  if ((dlim>0) && (tlim>0))
    if ((n_gd=mrclean(dlim,tlim))<0) return(n_gd);
  n_gd=make_causal_table(related,relations,twin,resolution,tcoincidence);
  if (n_gd<0) return(n_gd);
  /*-------------------------------------------------------------------
    cluster finding algorithm
    -------------------------------------------------------------------*/
  max_size=n_gd*(n_gd-1);
  clusterp=new short int *[max_size];
  if (n_gd<=1000)
    max_size=max_size/2*n_gd;
  else
    max_size=MAXSTOSIZE;
  cluster=new short int[max_size];
  clus=cluster;
  max_end=cluster+max_size;
  n_clus=0;
  min_size=MINHIT;
  max_size=0;
  /*-------------------------------------------------------------------
    now loop through the entire matrix of active relations, using each
    relation as a seed for a cluster
    -------------------------------------------------------------------*/
  relp=related;
  max_index=0;
  for(ntest=n_gd,row=0; row<ntest; row++)
    for(column=row+1,relp+=column,relp1=relations+(n_gd+2)*row+2;
        column<n_gd; column++)
      if (*relp++)
	{
	  relp2=relations+(n_gd+2)*column+2;
  /*-------------------------------------------------------------------
    storing seed */ 
	  clusterp[n_clus]=clus;
	  *(clus)=row;
	  clus[1]=column;
	  clus_size=2;
  /*-------------------------------------------------------------------
    forming a list of candidates by combining the list of relations of
    both seed hits: a hit is a candidate if related to both seed hits */ 
	  pmt1_index=pmt2_index=0;
	  while((pmt1_index<relp1[-2]) && (pmt2_index<relp2[-2]))
	    if ((pmt1=relp1[pmt1_index])==(pmt2=relp2[pmt2_index]))
	      {
		clus[clus_size++]=pmt1;
		pmt1_index++;
		pmt2_index++;
	      }
	    else
	      if (pmt1<pmt2) pmt1_index++; else pmt2_index++;
	  //for(new_pmt=2; (clus_size>=min_size) && (new_pmt<clus_size);
	  //    new_pmt++)
	  for(new_pmt=2; new_pmt<clus_size; new_pmt++)
	    {
  /*-------------------------------------------------------------------
    deactivate all its relations to the cluster pmts */
	      pmt1=clus[new_pmt];
	      relp2=related+pmt1*n_gd;
	      if ((pmt2=*clus)<pmt1) related[pmt2*n_gd+pmt1]=0; else relp2[pmt2]=0;
	      if ((pmt2=clus[1])<pmt1) related[pmt2*n_gd+pmt1]=0; else relp2[pmt2]=0;
	      for(pmt2=2,relp2=related+pmt1; pmt2<new_pmt; pmt2++)
	        relp2[n_gd*clus[pmt2]]=0;
  /*-------------------------------------------------------------------
    if the number of relations of the last hit is equal to
    the hits in the cluster so far, there cannot be any
    more hits to the cluster */
	      relp2=relations+pmt1*(n_gd+2)+2;	      
	      if (relp2[-2]==new_pmt)
	        {
		  clus_size=new_pmt+1;
		  break;
		}
 /*-------------------------------------------------------------------
    amend the list by removing hits not related to the new hit */
	      pmt2_index=relp2[-1];
	      size=clus_size;
	      for(pmt1_index=new_pmt+1; /* lists are identical so far */
	          (pmt1_index<size) && (pmt2_index<relp2[-2]) &&
		  (clus[pmt1_index]==relp2[pmt2_index]); pmt1_index++)
		pmt2_index++;
	      clus_size=pmt1_index;
	      while((pmt1_index<size) && (pmt2_index<relp2[-2]))
	        if ((pmt1=clus[pmt1_index])==(pmt2=relp2[pmt2_index]))
	          {
		    clus[clus_size++]=pmt1;
		    pmt1_index++;
		    pmt2_index++;
	          }
	        else
	          if (pmt1<pmt2) pmt1_index++; else pmt2_index++;
	    }
   /*-------------------------------------------------------------------
    see, if cluster is large enough to be stored; store if so */
	  if (clus_size>=min_size)
	    {
	      /*short int temp[clus_size],i,j;
	      for(i=0; i<clus_size; i++)
	        temp[i]=selected[clus[i]];
	      printf("seed %3d=%5d %3d=%5d: cluster of size %d\n",
	             temp[0],hitcable(temp[0]),temp[1],hitcable(temp[1]),clus_size);
	      for(i=0; i<clus_size; i++,printf("\n"))
	        for(j=0; j<clus_size; j++)
		  printf(" %d",causal(temp[i],temp[j],twin,resolution,tcoincidence));
	      printset(clus_size,temp);*/
  /* -------------------------------------------------------------------
     see, if cluster is larger than largest cluster so far. If so,
     adjust minimum allowed size and remove hits with two few
     relations as seed candidate*/
	      if (clus_size>max_size)
		{
		  max_size=clus_size;
		  max_clus=clusterp[n_clus];
		  max_index=n_clus;
		  min_size=(short int) ALLOWED_SIZE_FRACTION*max_size;
		  while(relations[(ntest-1)*(n_gd+2)]+1<min_size) ntest--;
		}
	      clus+=clus_size;
	      n_clus++;
	    }
	}
  delete [] relations;
  delete [] related;
  delete [] clusterp;
  /*-------------------------------------------------------------------
    unify all found clusters in one, note, how often a PMT occurs
    -------------------------------------------------------------------*/
  if (n_clus<=0)
    {
      delete [] cluster;
      delete [] selected;
      return(-5); // error: no cluster found
    }
  n_bd=clus-max_clus;
  joined=new short int[n_bd];
  occur=new short int[n_bd];
  n_bd=reduce(n_bd,max_clus,joined,occur);
  delete [] cluster;

  /*-------------------------------------------------------------------
    take all PMTs with high occurence, take all PMTs with medium
    occurence, if their charges are `reasonable'
    -------------------------------------------------------------------*/
  max_size=1+2*(n_clus-max_index-1)/3;
  //min_size=1+(n_clus-max_index-1)/3;
  select=new short int[n_gd];
  for(pmt1=0; pmt1<n_gd; pmt1++)
    select[pmt1]=0;
  for(pmt1=0; pmt1<n_bd; pmt1++)
    select[joined[pmt1]]=(occur[pmt1]>=max_size);/* ||
			   ((occur[pmt1]>=min_size) &&
	                   (chg[pmt1]>=0.5) && (chg[pmt1]<=2.))*/
  nsel=0;
  for(nsel=pmt1=0; pmt1<n_gd; pmt1++)
    if (select[pmt1]) select[nsel++]=selected[pmt1];
  if (nsel<MINHIT)
    {
      for(pmt1=0; pmt1<n_gd; pmt1++)
        select[pmt1]=0;
      for(pmt1=0; pmt1<n_bd; pmt1++)
        select[joined[pmt1]]=1;
      nsel=0;
      for(nsel=pmt1=0; pmt1<n_gd; pmt1++)
        if (select[pmt1]) select[nsel++]=selected[pmt1];
    }
  delete [] selected;
  selected=new short int[nsel];
  for(pmt1=0; pmt1<nsel; pmt1++)
    selected[pmt1]=select[pmt1];

  hits::qsort(selected,nsel);
  delete [] occur;
  pmt2=*selected;
  for(row=1; row<nsel; row++)
    {
      pmt1=selected[row];
      if (hittime(pmt1)<hittime(pmt2))
	{
	  printf("Time sorting error: PMT %5d (%9.2f)< PMT %5d (%9.2f)\n",
		 hitcable(pmt1),hittime(pmt1),hitcable(pmt2),hittime(pmt2));
	  delete [] select;
	  delete [] joined;
	  delete [] selected;
	  return(-6); /* time sorting error */
	}
      if (hitcable(pmt1)==hitcable(pmt2))
	{
	  printf("Double hit PMT %5d error\n",hitcable(pmt1));
	  delete [] select;
	  delete [] joined;
	  delete [] selected;
	  return(-7); /* double hit error */
	}
      pmt2=pmt1;
    }
  delete [] select;
  delete [] joined;
  if (nsel<MINHIT)
    {
      delete [] selected;
      return(-3); /* error: too few clean hits */
    }
  //printf("final selected hits: %d\n",nsel);
  //printset(nsel,selected);
  return(nsel); /* selection is o.k. */
}

void hitsel::printset(int nset,short int *set)
{
  int       m;
  short int s[nset];

  for(m=0; m<nset; m++) s[m]=set[m];
  hits::qsort(s,nset);
  for(m=0; m<nset; m++)
    printf("%3d %5d %10.4f %8.2f\n",m,
           hitcable(s[m]),hittime(s[m]),hitcharge(s[m]));
}
