#include "hits.h"

// return absolute value, if that value is >1,
//   otherwise return 1/absolute value
inline double hits::absrat(double el)
{
  double ael=fabs(el);

  if (ael>1) return(ael);
  if (ael<1e-10) return(1e10);
  return(1/ael);
}

// **********************************************
// find ``smallest'' matrix element ``above''
// (pass,pass)
// **********************************************
inline double hits::findmin(double *mat,int *rowind,int *colind,int pass)
{
  int    row,col,ind;
  int    saverow=rowind[pass],savecol=colind[pass];
  double test,min=absrat(mat[5*saverow+savecol]);

  for(row=pass; row<3; row++)
    {
      ind=5*rowind[row];
      for(col=pass; col<3; col++)
	if ((row!=pass) || (col!=pass))
	  if ((test=absrat(mat[ind+colind[col]]))
	      <min)
	    {
	      rowind[pass]=rowind[row];
	      colind[pass]=colind[col];
	      min=test;
	    }
    }
  // if the smallest element is not reasonable,
  // attempt column 4 and 5
  for(col=3; (min>=1e10) && (col<5); col++)
    {
      ind=colind[col];
      for(row=pass; row<3; row++)
	if ((test=absrat(mat[5*rowind[row]+ind]))
	    <min)
	  { rowind[pass]=rowind[row];
	    colind[pass]=ind;
	    min=test; }
    }
  // find the pass, where the row/column of this smallest
  // element would have been chosen; replace with the numbers
  // of this pass
  for(row=pass+1; row<3; row++)
    if (rowind[pass]==rowind[row])
      rowind[row]=saverow;
  for(col=pass+1; col<5; col++)
    if (colind[pass]==colind[col])
      colind[col]=savecol;
  return(mat[5*rowind[pass]+colind[pass]]);
}

// **********************************************
// force zeros in a column replacing rows
// by suitable linear combinations of rows
// **********************************************
inline void hits::force_zero(double *mat,int *rowind,int *colind,
			     int pass,int ind)
{
  int    row,col,indr,indc;
  double factor;

  for(row=0; row<3; row++) if (row!=pass)
    {
      indr=5*rowind[row];
      if ((factor=mat[indr+colind[pass]])!=0)
	{
	  for(col=pass+1; col<5; col++)
	    {
	      indc=colind[col];
	      mat[indr+indc]-=factor*mat[ind+indc];
	    }
	}
    }
}

// **********************************************
// invert 3x5 matrix as much as possible
// mat:    matrix
// rowind: row numbers of inverted rows
// colind: column numbers of inverted columns
// **********************************************
int hits::inv35(double *mat,int *rowind,int *colind)
{
  int    pass,col,ind;
  double factor;

  // initialize row numbers and column numbers
  // assuming the ordered case
  for(col=0; col<3; col++)
    { rowind[col]=col;
      colind[col]=col; }
  for(; col<5; col++) colind[col]=col;

  // try to invert up to 3 rows:
  for(pass=0; pass<3; pass++)
    {
      factor=findmin(mat,rowind,colind,pass); // find best element to reduce
      if (factor==0) return(pass);            // if zero already, break off inversion
      // reduce the chosen element to one by dividing the row
      // by the chosen element
      ind=5*rowind[pass];
      factor=1/factor;
      /*mat[ind+colind[pass]]=1;*/
      for(col=pass+1; col<5; col++)
	mat[ind+colind[col]]*=factor;
      // force zeros in this column 
      force_zero(mat,rowind,colind,pass,ind);
#ifdef DEBUG
      printf("*********result of %d**********\n",pass);
      for(row=0; row<3; row++)
	{
	  for(col=0; col<5; col++) printf("%f ",mat[5*row+col]);
	  printf("\n");
	}
	printf("\n");
#endif
    }
  return(pass);
}

// **********************************************
// quick sort algorithm to sort the hit times
// **********************************************
void hits::qsort(short int *list,short int n,short int *first)
{
  if (n<2) return; // no need to sort for 1 or zero hits
  if (n==2)        // 2 hits are easy to sort ...
    {
      if (times[*list]<times[list[1]]) return;
      n=list[1];
      list[1]=*list;
      *list=n;
      return;
    }

  // split list of hits in 2 sublists, sort sublists
  int half=n/2;
  qsort(list,half,first);
  qsort(list+half,n-half,first);

  // merge the two sorted sublists
  short int *second=list+half;
  int       m=n-half,firstp,secondp,nfirst=0;
  short int firstel=*list;

  // firstp is the index of the current element
  // of the first sublist, secondp of the second
  // exit loop, if second list is exhausted, or
  // first list is exhausted and no elements remain
  // in the storage
  for(firstp=secondp=0;
      (secondp<m) && ((firstp<half) || (nfirst!=0));
      firstp++)
    {
      if (times[firstel]<times[second[secondp]])
	{ // take element of the first sublist
	  if (nfirst==0)            // no elements in storage
	    firstel=list[firstp+1]; // next el. of 1. sublist
	  else
	    { // otherwise, first use storage
	      if (firstp<half)              // if necessary,
		first[nfirst]=list[firstp]; // store element
	      else
		nfirst--;      // otherwise decrease storage
	      list[firstp]=firstel; // fill list with el. of 1. sublist
	      firstel=*(++first);   // next el. of 1. sublist from storage
	    }
	}
      else
	{ // take element of the second sublist
	  if (firstp<half)                // if necessary,
	    first[nfirst++]=list[firstp]; // store element
 	  list[firstp]=second[secondp++]; // fill list with el. of 2.
	}
    }
  // empty storage
  for(; nfirst>0; nfirst--)
    list[firstp++]=*(first++);
}


// **********************************************
// * create space for all hits                  *
// **********************************************
inline void hits::create_arrays(int n_raw,int n_hit)
{
  places=new short int[n_raw];
  cables=new int[n_hit];
  times=new float[n_hit];
  charges=new float[n_hit];
  positions=new float[3*n_hit];
}

// **********************************************
// **********************************************
// public functions
// **********************************************
// constructor:
// Stores hit times, charges, cable numbers, and
// positions
// **********************************************
hits::hits(int ns,float *set,float *pmt_loc,int *bad_ch,
	   int n_raw,int *cable,float *tim_raw,float *chg_raw)
{
  int i,j,number,pos,*nh2;

  // count #hits per charge bin and create offset 'pointers'
  nchargeset=ns;
  nhit=new int[nchargeset];
  avcharges=new float[nchargeset];
  for(j=0; j<nchargeset; j++) nhit[j]=0;
  for(i=0; i<n_raw; i++)
    {
      number=cable[i]-1;
      if (bad_ch[number]==0)
	{
	  for(j=0; (j<nchargeset) && (chg_raw[number]>=set[j]); j++);
	  if (--j>=0) nhit[j]++;
	}
    }
  for(j=1; j<nchargeset; j++) nhit[j]+=nhit[j-1];
  create_arrays(n_raw,nhit[nchargeset-1]);

  // sort each hit into its charge set
  nh2=new int[nchargeset];
  for(j=0; j<nchargeset; j++) avcharges[j]=nh2[j]=0;
  for(i=0; i<n_raw; i++)
    {
      number=cable[i]-1;
      if (bad_ch[number]==0)
	{
	  for(j=0; (j<nchargeset) && (chg_raw[number]>=set[j]); j++);
	  if (--j>=0)
	    {
	      if (j>0) pos=nhit[j-1]+nh2[j]; else pos=*nh2;
	      places[i]=pos;
	      cables[pos]=number+1;
	      times[pos]=tim_raw[number];
	      charges[pos]=chg_raw[number];
	      avcharges[j]+=chg_raw[number];
	      positions[3*pos]=pmt_loc[3*number];	  
	      positions[3*pos+1]=pmt_loc[3*number+1];	  
	      positions[3*pos+2]=pmt_loc[3*number+2];
	      nh2[j]++;
	    }
	  else
	    places[i]=-1;
	}
    }
  for(j=0; j<nchargeset; j++)
    if (nh2[j]>0) avcharges[j]/=nh2[j];
  delete(nh2);
}

hits::hits(int ns,float *set,pmt_geometry *geom,int nh,int *cab,float *t,float *q)
{
  int i,j,number,pos,*nh2;

  // count #hits per charge bin and create offset 'pointers'
  nchargeset=ns;
  nhit=new int[nchargeset];
  avcharges=new float[nchargeset];
  for(j=0; j<nchargeset; j++) nhit[j]=0;
  for(i=0; i<nh; i++)
    {
      for(j=0; (j<nchargeset) && (q[i]>=set[j]); j++);
      if (--j>=0) nhit[j]++;
    }
  for(j=1; j<nchargeset; j++) nhit[j]+=nhit[j-1];
  create_arrays(nh,nhit[nchargeset-1]);

  // sort each hit into its charge set
  nh2=new int[nchargeset];
  for(j=0; j<nchargeset; j++) avcharges[j]=nh2[j]=0;
  for(i=0; i<nh; i++)
    {
      for(j=0; (j<nchargeset) && (q[i]>=set[j]); j++);
      if (--j>=0)
	{
	  if (j>0) pos=nhit[j-1]+nh2[j]; else pos=*nh2;
	  places[i]=pos;
	  number=(int)(cab[i]);
	  cables[pos]=number--;
	  times[pos]=t[i];
	  charges[pos]=q[i];
	  avcharges[j]+=charges[pos];
	  geom->position(positions+3*pos,number);
	  nh2[j]++;
	}
      else
	places[i]=-1;
    }
  for(j=0; j<nchargeset; j++)
    if (nh2[j]>0) avcharges[j]/=nh2[j];
  delete(nh2);
}

hits::hits(int ns,float *set,pmt_geometry *geom,comtype2 *itevent)
{
  int i,j,number,pos,*nh2;

  // count #hits per charge bin and create offset 'pointers'
  nchargeset=ns;
  nhit=new int[nchargeset];
  avcharges=new float[nchargeset];
  for(j=0; j<nchargeset; j++) nhit[j]=0;
  ns=itevent->it_index;
  for(i=0; i<ns; i++)
    {
      for(j=0; (j<nchargeset) && (itevent->hits[i][1]>=set[j]); j++);
      if (--j>=0) nhit[j]++;
    }
  for(j=1; j<nchargeset; j++) nhit[j]+=nhit[j-1];
  create_arrays(ns,nhit[nchargeset-1]);

  // sort each hit into its charge set
  nh2=new int[nchargeset];
  for(j=0; j<nchargeset; j++) avcharges[j]=nh2[j]=0;
  for(i=0; i<ns; i++)
    {
      for(j=0; (j<nchargeset) && (itevent->hits[i][1]>=set[j]); j++);
      if (--j>=0)
	{
	  if (j>0) pos=nhit[j-1]+nh2[j]; else pos=*nh2;
	  places[i]=pos;
	  number=(int)fabs(itevent->hits[i][0]);
	  cables[pos]=number--;
	  times[pos]=itevent->hits[i][2];
	  charges[pos]=itevent->hits[i][1];
	  avcharges[j]+=charges[pos];
	  geom->position(positions+3*pos,number);
	  nh2[j]++;
	}
      else
	places[i]=-1;
    }
  for(j=0; j<nchargeset; j++)
    if (nh2[j]>0) avcharges[j]/=nh2[j];
  delete(nh2);
}

// frees hit storage arrays
hits::~hits(void)
{
  delete(nhit);
  delete(places);
  delete(positions);
  delete(times);
  delete(charges);
  delete(avcharges);
  delete(cables);
}


// **********************************************
// average hit times between tmin and tmax
// **********************************************
float hits::time_av(float tmin,float tmax)
{
  int   i,n;
  float t_av;

  if (nhit[nchargeset-1]<=0) return(-1);
  for(t_av=0,n=0,i=0; i<nhit[nchargeset-1]; i++)
    if ((times[i]>tmin) && (times[i]<tmax))
      {
	t_av+=times[i];
	n++;
      }
  if (n==0) return(0.5*(tmin+tmax));
  return(t_av/n);
}

// **********************************************
// find vertex from four-hit combination
// **********************************************
int hits::vertex4(int *hits,double *vert)
{
  double mat[15];
  double sprod,sq1,sq2,root;
  double dist;
  int    nrow,rowind[3],colind[5];
  int    i,j,ii,imin,h;

  // find minimum time, coordinate system uses
  // this hit as origin (x,y,z,t)_min=0
  for(i=1,imin=*hits; i<4; i++)
    if (times[h=hits[i]]<times[imin]) imin=h;
  // create matrix of the other hits:
  // each row is -2 times the contravariant 4 vector
  // followed by the square
  for(i=0,j=0; i<4; i++)
    if ((h=hits[i])!=imin)
      {
	for(sq1=0,ii=0; ii<3; ii++)
	  {
	    sq2=positions[3*imin+ii]-positions[3*h+ii];
	    sq1+=sq2*sq2;
	    mat[5*j+ii]=2*sq2;
	  }
	sq2=NS_TO_CM*(times[h]-times[imin]);
	sq1-=sq2*sq2;
	mat[5*j+3]=2*sq2;
	mat[5*j+4]=sq1;
	j++;
      }
  // invert as large as possible a submatrix of this 3x5 matrix
  // prefer to invert the first three columns; if impossible,
  // use the fourth, then the fifth column
  nrow=inv35(mat,rowind,colind);
  // rowind, colind are the coordinates of `1's, that were
  // created by the transformation
  // sort the column indices
  for(i=0; i<nrow; i++)
    {
      for(ii=i; ii<nrow; ii++)
	if (colind[ii]==i) break;
      if (ii==nrow)
	for(ii=i; ii<nrow; ii++)
	  if (colind[ii]==3) break;
      if (ii==nrow)
	for(ii=i; ii<nrow; ii++)
	  if (colind[ii]==4) break;
      if (i!=ii)
	{
	  j=colind[i];
	  colind[i]=colind[ii];
	  colind[ii]=j;
	  j=rowind[i];
	  rowind[i]=rowind[ii];
	  rowind[ii]=j;
	}
    }
  // at this point, don't report result, if inversion was incomplete
  // since no unique vertex can be found if any at all
  if (nrow<3)
    {
#ifdef DEBUG
      printf("too many solutions! not supported yet! %d %d %d %d: %d\n",
	     hits[0],hits[1],hits[2],hits[3],nrow);
#endif
      return(3);
    }
  // necessarily at least one contradiction
  // of the four space-time constraints
  if (colind[4]!=4) return(-1);
  // form two 3 vectors out of the two non-inverted columns
  for(ii=0; ii<2; ii++)
    for(i=0; i<3; i++)
      vert[4*ii+colind[i]]=mat[5*rowind[i]+colind[ii+3]];
  // compute the scalar products
  // use inverted sign for a time component
  sprod=sq1=sq2=0;
  for(i=0; i<3; i++)
    if ((j=colind[i])==3)
      { sprod-=vert[j]*vert[j+4];
	sq1-=vert[j]*vert[j];
	sq2-=vert[4+j]*vert[4+j]; }
    else
      { sprod+=vert[j]*vert[j+4];
	sq1+=vert[j]*vert[j];
	sq2+=vert[4+j]*vert[4+j]; }
  if ((j=colind[3])==3) sq1-=1; else sq1+=1;
  // add +-1 to the first scalar product: the radicant is
  root=sprod*sprod-sq1*sq2;
  if (root<0) return(0); // no solution
  root=sqrt(root);
  // test each of the two solution, that the time of the
  // vertex is smaller than all PMT times and construct
  // the two vertices
  if (sq1>0)
    { if ((j==3) && (sprod+root<0)) return(0);
      vert[j]=-(root+sprod)/sq1;
      vert[j+4]=(root-sprod)/sq1; }
  else
    { if ((j==3) && (sprod>root)) return(0);
      vert[j]=(root-sprod)/sq1;
      vert[j+4]=-(root+sprod)/sq1; }
  if ((j==3) && (vert[7]>0))
    for(i=0; i<3; i++)
      vert[i]=-(vert[3]*vert[i]+vert[i+4])+positions[3*imin+i];
  else
    for(i=0; i<3; i++)
      {
	ii=colind[i];
	dist=vert[ii];
	vert[ii]=-(vert[j]*vert[ii]+vert[ii+4]);
	vert[ii+4]=-(vert[j+4]*dist+vert[ii+4]);
	if (ii<3)
	  { vert[ii]+=positions[3*imin+ii];
	    vert[ii+4]+=positions[3*imin+ii]; }
	else
	  { vert[3]=CM_TO_NS*vert[3]+times[imin];
	    vert[7]=CM_TO_NS*vert[7]+times[imin]; }
      }
  if (j==3)
    { vert[3]=CM_TO_NS*vert[3]+times[imin];
      vert[7]=CM_TO_NS*vert[7]+times[imin]; }
  else
    { vert[j]+=positions[3*imin+j];
      vert[j+4]+=positions[3*imin+j]; }
  if (vert[7]>times[imin]) return(1); else return(2);
}

// **********************************************
// find vertex from four-hit combination
// **********************************************
int hits::vertex4(int *fourcombo,int *hitlist,int listsize,
		  int &hbegin,int &hend,float tsig,
                  float cyl_radius,float cyl_height,float dwallmin,
                  double *vert,float *gdn)
{
  float rad,height,tmin,tmax;
  int   h,thishit,s,ns=vertex4(fourcombo,vert);

  for(s=0; s<ns; s++)
    {
      gdn[s]=-1;
      rad=sqrt(vert[4*s]*vert[4*s]+vert[4*s+1]*vert[4*s+1]);
      if ((tmin=cyl_radius-rad)<dwallmin) continue;
      height=fabs(vert[4*s+2]);
      if ((tmax=cyl_height-height)<dwallmin) continue;
      if (tmax<tmin) tmin=tmax;

      float vertex[3],t;

      vertex[0]=vert[4*s];
      vertex[1]=vert[4*s+1];
      vertex[2]=vert[4*s+2];
      /*printf("%3d %3d %3d %3d (1): ",fourcombo[0],fourcombo[1],fourcombo[2],fourcombo[3]);
      printf("%8.2lf %8.2lf %8.2lf %6.1lf ",vert[4*s],vert[4*s+1],vert[4*s+2],vert[4*s+3]);
      printf("%8.2f",tmin);*/
      tmin=vert[4*s+3]+tmin*CM_TO_NS-3*tsig;
      rad+=cyl_radius;
      height+=cyl_height;
      tmax=vert[4*s+3]+sqrt(rad*rad+height*height)*CM_TO_NS+3*tsig;
      //printf("(%6.1f) to %8.2f(%6.1f) ",tmin,sqrt(rad*rad+height*height),tmax);
      gdn[s]=4;
      while((hbegin>0) && (times[hitlist[hbegin]]>=tmin)) hbegin--;
      while((hbegin<listsize-1) && (times[hitlist[hbegin]]<tmin)) hbegin++;
      while((hend<listsize-1) && (times[hitlist[hend]]<=tmax)) hend++;
      while((hend>0) && (times[hitlist[hend]]>tmax)) hend--;
      //printf("begin %6.1f end %6.1f ",times[hitlist[hbegin]],times[hitlist[hend]]);
      for(h=hbegin; h<=hend; h++)
	{
	  thishit=hitlist[h];
	  if ((thishit!=fourcombo[0]) && (thishit!=fourcombo[1]) &&
	      (thishit!=fourcombo[2]) && (thishit!=fourcombo[3]))
	    {
	      t=times[thishit]-tof(vertex,thishit)-vert[4*s+3];
	      t/=tsig;
	      t*=-0.5*t;
	      if (t>=-4.5) gdn[s]+=exp(t);
	    }
	}
      //printf("%8.4f\n",gdn[s]);
    }
  return(ns);
}
