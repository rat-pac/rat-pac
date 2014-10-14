#include <stdio.h>
#include "vertexfit.h"
#include "pmt_geometry.h"
#include "fourhitgrid.h"
#include "goodness.h"
#include "likelihood.h"
#include "binfile.h"

vertexfit *vf;
likelihood *tf;
pmt_geometry *geom;
binfile    *bsfile;

#define MINHIT 4

extern "C"
{
  void bsloadinit_(float *res);
  void bsinit_(int *np,float *ps,float *res);
  void bsexit_(void);
  void bsloadhits_(int *nhit,int *cab,int *badch,float *t,float *q);
  float bstimedirfit_(int *fast,float *vertex,float *direct,float *dt);
  void bssetcang_(float *cang0,float *pang,float *mang);
  void bssetfactor_(float *d,float *t);
  float bsbestvertex_(float *vertex);
  float bsvertexfit_(float *vertex);
  float bsfitvertex_(float *vertex,float *rad,int *maxiter);
  void bsaddvertex_(float *vertex);
  float bsvertexlinefit_(float *vertex,float *v,float *d,float *grid);
  float bsgood_(float *vertex,float *bgrate,float *guncor);
  int   bsnwin1_(float *vertex,float *tmin,float *tmax);
  int   bsnwin2_(float *vertex,float *tmin,float *tmax,float *ttof,int *cab);
  int   bsnwin3_(float *vertex,float *tmin,float *tmax,
		 float *ttof,float *d,int *cab);
  float bsbestwall_(float *dwall,float *rmin,float *wallv);
#include "bscalls.h"
}

void bsloadinit_(float *res)
{
  vf=new vertexfit(*res);
  //gri=new gringo(vf);
}

void bsinit_(int *np,float *ps,float *res)
{
  vf=new vertexfit(*np,ps,*res);
  //gri=new gringo(vf);
}

void bsexit_(void)
{
  //delete(gri);
  delete(vf);
}

void bsloadhits_(int *nhit,int *cab,int *badch,float *t,float *q)
{
  vf->loadhits(*nhit,cab,badch,t,q);
}

float bstimedirfit_(int *fast,float *vertex,float *direct,float *dt)
{
  if (*fast>1)
    return(vf->fittime_lcor((*fast)-2,vertex,direct,*dt));
  else
    return(vf->fittime(*fast,vertex,direct,*dt));
}

void bssetcang_(float *cang0,float *pang,float *mang)
{
  vf->set_cang(*cang0,*pang,*mang);
}

void bssetfactor_(float *d,float *t)
{
  vf->set_depth(*d);
  vf->set_time(*t);
}

float bsbestvertex_(float *vertex)
{
  float gdn[vf->sets()],ll;

  vf->bestvertex(vertex);
  return(vf->goodness(ll,vertex,gdn));
}

float bsvertexfit_(float *vertex)
{
  float gdn[vf->sets()],ll;

  vf->search();
  vf->bestvertex(vertex);
  return(vf->goodness(ll,vertex,gdn));
}

float bsfitvertex_(float *vertex,float *rad,int *maxiter)
{
  float gdn[vf->sets()],ll;

  vf->tree::search(vertex,*rad,*maxiter);
  vf->bestvertex(vertex);
  return(vf->goodness(ll,vertex,gdn));
}

void bsaddvertex_(float *vertex)
{
  vf->addvertex(vertex,50.);
}

float bsgood_(float *vertex,float *bgrate,float *guncor)
{
  return(vf->tgood(vertex,*bgrate,*guncor));
}

int bsnwin1_(float *vertex,float *tmin,float *tmax)
{
  return(vf->nwind(vertex,*tmin,*tmax));
}

int bsnwin2_(float *vertex,float *tmin,float *tmax,float *ttof,int *cab)
{
  return(vf->nwind(vertex,*tmin,*tmax,ttof,cab));
}

int bsnwin3_(float *vertex,float *tmin,float *tmax,
	     float *ttof,float *d,int *cab)
{
  return(vf->nwind(vertex,*tmin,*tmax,ttof,d,cab));
}

float bsbestwall_(float *dwall,float *rmin,float *wallv)
{
  return(vf->bestwall(*dwall,*rmin,wallv));
}

void itfitter_init(comtype *itgeom)
{
  if (itgeom==NULL)
    geom=new pmt_geometry();
  else
    geom=new pmt_geometry(itgeom);
  tf=new likelihood(geom->cylinder_radius(),geom->cylinder_height());
}

void qsort(float *cabs,short int *list,short int n,short int *first);
// **********************************************
// quick sort algorithm to sort the hit times
// **********************************************
void qsort(float *cabs,short int *list,short int n,short int *first)
{
  if (n<2) return; // no need to sort for 1 or zero hits
  if (n==2)        // 2 hits are easy to sort ...
    {
      if (cabs[*list]<cabs[list[1]]) return;
      n=list[1];
      list[1]=*list;
      *list=n;
      return;
    }

  // split list of hits in 2 sublists, sort sublists
  int half=n/2;
  qsort(cabs,list,half,first);
  qsort(cabs,list+half,n-half,first);

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
      if (cabs[firstel]<cabs[second[secondp]])
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

void it_sort(comtype2 *itevent)
{
  short int sortlist[2*itevent->it_index],n;
  float     cabs[itevent->it_index];
  float     d1[itevent->it_index],d2[itevent->it_index];
  for(n=0; n<itevent->it_index; n++)
    {
      cabs[n]=itevent->hits[n][0];
      d1[n]=itevent->hits[n][1];
      d2[n]=itevent->hits[n][2];
      sortlist[n]=n;
    }
  qsort(cabs,sortlist,n,sortlist+n);
  for(n=0; n<itevent->it_index; n++)
    {
      itevent->hits[n][0]=cabs[sortlist[n]];
      itevent->hits[n][1]=d1[sortlist[n]];
      itevent->hits[n][2]=d2[sortlist[n]];
    }
  return;
}

int it_clusfit(float *vert,float *result,float *maxgood,int *nsel,
	       comtype2 *itevent,void *gridbuffer,short int maxsize)
{
  *vert=vert[1]=vert[2]=0; *maxgood=vert[3]=-1;
  goodness    good(tf->sets(),tf->chargebins(),geom,itevent);
  *nsel=good.nselected();
  if (good.nselected()<MINHIT) return(0);
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&good);
  gr.packset(gridbuffer,maxsize);
  bonsaifit   cf(&good);

  good.maximize(&cf,&gr);
  if (good.nfit()==0) return(0);
  *maxgood=cf.maxq();
  *vert=cf.xfit();
  vert[1]=cf.yfit();
  vert[2]=cf.zfit();
  cf.fitresult();
  vert[3]=good.get_zero();
  *result=good.get_theta();
  result[1]=good.get_phi();
  result[2]=good.get_cos_theta();
  result[3]=good.get_gdn0();
  return(good.nfit());
}

void itfitter_exit(void)
{
  tf->set_hits(NULL);
  delete(tf);
  delete(geom);
}

int it_bsfit(float *vertex,float *result,float *maxlike,int *nsel,
	     comtype2 *itevent,void *gridbuffer)
{
  *vertex=vertex[1]=vertex[2]=*maxlike=0; vertex[3]=-1;
  hitsel ithits(tf->sets(),tf->chargebins(),geom,itevent);
  *nsel=ithits.nselected();
  if (ithits.nselected()<MINHIT) return(0);
  fourhitgrid gr(gridbuffer,geom->cylinder_radius(),
		 geom->cylinder_height(),&ithits);
  bonsaifit   cf(tf);
  int         nfit;

  tf->set_hits(&ithits);
  tf->maximize(&cf,&gr);
  if ((nfit=tf->nfit())==0) return(0);
  *maxlike=cf.maxq();
  *vertex=cf.xfit();
  vertex[1]=cf.yfit();
  vertex[2]=cf.zfit();
  tf->tgood(vertex,0,maxlike[1]);
  nsel[1]=tf->nwind(vertex,-6,12);
  cf.fitresult();
  vertex[3]=tf->get_zero();
  tf->get_dir(result);
  result[5]=tf->get_ll0();
  tf->set_hits(NULL);
  return(nfit);
}

void it_clus_and_bonsai_fit(float *clusvert,float *bonsaivert,
			    float *clusresult,float *bonsairesult,
			    float *maxgood,float *maxlike,
			    int *nsel,int *nclusfit,int *nbonsaifit,
			    float cluswallcut,comtype2 *itevent)
{
  *clusvert=clusvert[1]=clusvert[2]=0; *maxgood=clusvert[3]=-1;
  goodness    good(tf->sets(),tf->chargebins(),geom,itevent);
  *nsel=good.nselected();
  *nclusfit=*nbonsaifit=0;
  if (good.nselected()<MINHIT) return;
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&good);
  bonsaifit   cf(&good);
  bonsaifit   bf(tf);

  *bonsaivert=0;
  bonsaivert[1]=0;
  bonsaivert[2]=0;
  bonsaivert[3]=-1;
  nsel[1]=-1;
  *bonsairesult=0;
  bonsairesult[1]=0;
  bonsairesult[2]=0;
  bonsairesult[3]=0;
  bonsairesult[4]=0;
  bonsairesult[5]=0;
  *maxlike=0;
  maxlike[1]=-1;
  *nbonsaifit=-1;
  good.maximize(&cf,&gr);
  if ((*nclusfit=good.nfit())!=0)
    {
      *maxgood=cf.maxq();
      *clusvert=cf.xfit();
      clusvert[1]=cf.yfit();
      clusvert[2]=cf.zfit();
      cf.fitresult();
      clusvert[3]=good.get_zero();
      *clusresult=good.get_theta();
      clusresult[1]=good.get_phi();
      clusresult[2]=good.get_cos_theta();
      clusresult[3]=good.get_gdn0();
      if (good.walldist(cf.xfit(),cf.yfit(),cf.zfit())<cluswallcut) return;
    }
  tf->set_hits(&good);
  tf->maximize(&bf,&gr);
  if ((*nbonsaifit=tf->nfit())!=0)
    {
      *maxlike=bf.maxq();
      *bonsaivert=bf.xfit();
      bonsaivert[1]=bf.yfit();
      bonsaivert[2]=bf.zfit();
      tf->tgood(bonsaivert,0,maxlike[1]);
      nsel[1]=tf->nwind(bonsaivert,-6,12);
      bf.fitresult();
      bonsaivert[3]=tf->get_zero();
      tf->get_dir(bonsairesult);
      bonsairesult[5]=tf->get_ll0();
    }
  tf->set_hits(NULL);
}

void it_clus_bonsai_fit(float *clusvert,float *bonsaivert,
			float *clusresult,float *bonsairesult,
			float *maxgood,float *maxlike,
			int *nsel,int *nclusfit,int *nbonsaifit,
			comtype2 *itevent)
{
  *clusvert=clusvert[1]=clusvert[2]=0; *maxgood=clusvert[3]=-1;
  goodness    good(tf->sets(),tf->chargebins(),geom,itevent);
  *nsel=good.nselected();
  *nclusfit=*nbonsaifit=0;
  if (good.nselected()<MINHIT) return;
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&good);
  bonsaifit   cf(&good);
  bonsaifit   bf(tf);

  tf->set_hits(&good);
  *bonsaivert=0;
  bonsaivert[1]=0;
  bonsaivert[2]=0;
  bonsaivert[3]=-1;
  nsel[1]=-1;
  *bonsairesult=0;
  bonsairesult[1]=0;
  bonsairesult[2]=0;
  bonsairesult[3]=0;
  bonsairesult[4]=0;
  bonsairesult[5]=0;
  *maxlike=0;
  maxlike[1]=-1;
  *nbonsaifit=-1;
  good.maximize(&cf,&bf,tf,&gr);
  if ((*nclusfit=good.nfit())!=0)
    {
      *maxgood=cf.maxq();
      *clusvert=cf.xfit();
      clusvert[1]=cf.yfit();
      clusvert[2]=cf.zfit();
      cf.fitresult();
      clusvert[3]=good.get_zero();
      *clusresult=good.get_theta();
      clusresult[1]=good.get_phi();
      clusresult[2]=good.get_cos_theta();
      clusresult[3]=good.get_gdn0();
      if ((*nbonsaifit=tf->nfit())!=0)
        {
          *maxlike=bf.maxq();
          *bonsaivert=bf.xfit();
          bonsaivert[1]=bf.yfit();
          bonsaivert[2]=bf.zfit();
	  tf->tgood(bonsaivert,0,maxlike[1]);
	  nsel[1]=tf->nwind(bonsaivert,-6,12);
          bf.fitresult();
          bonsaivert[3]=tf->get_zero();
          tf->get_dir(bonsairesult);
          bonsairesult[5]=tf->get_ll0();
        }
      else
        {
          *bonsaivert=*clusvert;
	  bonsaivert[1]=clusvert[1];
	  bonsaivert[2]=clusvert[2];
	  tf->tgood(bonsaivert,0,maxlike[1]);
	  nsel[1]=tf->nwind(bonsaivert,-6,12);
        }
    }
  tf->set_hits(NULL);
}

void it_vfwrite(FILE *itvw,unsigned int event_number,int trigger_type,
		int keep,int flag,unsigned short clock48_high,
		unsigned short clock48_middle,unsigned short clock48_low,
		unsigned int fit_tubes,
		unsigned int cfit,unsigned int cvertex_tubes,
		float *cvtx,float *cdir,float cgof,
		unsigned int bfit,unsigned int bvertex_tubes,
		float *bvtx,float *bdir,float bgof,float bll,float bll0)
{
  fprintf(itvw,"%8d %2d %1d %8d %5d %5d %5d %3d ",
	  event_number,trigger_type,keep,flag,clock48_high,
	  clock48_middle,clock48_low,fit_tubes);
  fprintf(itvw,"%4d %3d %8.2f %8.2f %8.2f %6.1f %6.4f %7.4f %6.4f %6.4f ",
	  cfit,cvertex_tubes,cvtx[0],cvtx[1],cvtx[2],cvtx[3],
	  cdir[0],cdir[1],cdir[2],cgof);
  fprintf(itvw,"%4d %3d %8.2f %8.2f %8.2f %6.1f %6.4f %7.4f %7.4f %6.4f %6.4f %6.4f %8.3f %8.3f\n",
	  bfit,bvertex_tubes,bvtx[0],bvtx[1],bvtx[2],bvtx[3],      
	  bdir[0],bdir[1],bdir[2],bdir[3],bdir[4],bgof,bll,bll0);
}

short int it_vfread(FILE *itvr,unsigned int *event_number,int *trigger_type,
	       int *keep,int *flag,unsigned short *clock48_high,
	       unsigned short *clock48_middle,unsigned short *clock48_low,
	       unsigned int *fit_tubes,
	       unsigned int *cfit,unsigned int *cvertex_tubes,
	       float *cvtx,float *cdir,float *cgof,
	       unsigned int *bfit,unsigned int *bvertex_tubes,
	       float *bvtx,float *bdir,float *bgof,float *bll,float *bll0)
{
  char line[256];

  fgets(line,255,itvr);
  if (feof(itvr)) return(-1);
  sscanf(line,"%d %d %d %d %hd %hd %hd %d %d %d %f %f %f %f %f %f %f %f %d %d %f %f %f %f %f %f %f %f %f %f %f %f",
	 event_number,trigger_type,keep,flag,clock48_high,
	 clock48_middle,clock48_low,fit_tubes,
	 cfit,cvertex_tubes,cvtx,cvtx+1,cvtx+2,cvtx+3,
	 cdir,cdir+1,cdir+2,cgof,
	 bfit,bvertex_tubes,bvtx,bvtx+1,bvtx+2,bvtx+3,
	 bdir,bdir+1,bdir+2,bdir+3,bdir+4,bgof,bll,bll0);
  return(0);
}

void cfbsloadinit_(void)
{
  geom=new pmt_geometry();
  tf=new likelihood(geom->cylinder_radius(),geom->cylinder_height());
}

void cfbsinit_(int *np,float *ps)
{
  geom=new pmt_geometry(*np,ps);
  tf=new likelihood(geom->cylinder_radius(),geom->cylinder_height());
}

void cfbsexit_(void)
{
  tf->set_hits(NULL);
  delete(tf);
  delete(geom);
}

void storetest_(int *nhit,int *cab,float *t,float *q,float *tsig,float *dwallmin,
		int *nsel,float *goodn)
{
  goodness    good(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);
  if ((*nsel=good.nselected())<MINHIT)
    {
      *goodn=-1;
      return;
    }
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&good,
		 *tsig,*dwallmin,*goodn);
}

int clusfit_(float *vert,float *result,float *maxgood,int *nsel,
	     int *nhit,int *cab,float *t,float *q)
{
  goodness    good(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);
  *nsel=good.nselected();
  if (good.nselected()<MINHIT) return(0);
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&good);
  bonsaifit   cf(&good);

  good.maximize(&cf,&gr);
  if (good.nfit()==0) return(0);
  *maxgood=cf.maxq();
  *vert=cf.xfit();
  vert[1]=cf.yfit();
  vert[2]=cf.zfit();
  cf.fitresult();
  vert[3]=good.get_zero();
  *result=good.get_theta();
  result[1]=good.get_phi();
  result[2]=good.get_cos_theta();
  result[3]=good.get_gdn0();
  return(good.nfit());
}

int bonsaifit_(float *vert,float *result,float *maxlike,int *nsel,
	       int *nhit,int *cab,float *t,float *q)
{
  goodness    bshits(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);
  *nsel=bshits.nselected();
  if (bshits.nselected()<MINHIT) return(0);
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&bshits);
  bonsaifit   cf(tf);
  int         nfit;
  float       gdn[tf->sets()];

  tf->set_hits(&bshits);
  tf->maximize(&cf,&gr);
  if ((nfit=tf->nfit())==0) return(0);
  *vert=cf.xfit();
  vert[1]=cf.yfit();
  vert[2]=cf.zfit();
  maxlike[2]=tf->goodness(*maxlike,vert,gdn);
  tf->tgood(vert,0,maxlike[1]);
  nsel[1]=tf->nwind(vert,-6,12);
  *maxlike=cf.maxq();
  cf.fitresult();
  vert[3]=tf->get_zero();
  tf->get_dir(result);
  result[5]=tf->get_ll0();
  tf->set_hits(NULL);
  return(nfit);
}

int startbonsaifit_(float *vert,float *result,float *maxlike,int *nsel,
	            int *nhit,int *cab,float *t,float *q)
{
  goodness    bshits(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);
  *nsel=bshits.nselected();
  if (bshits.nselected()<MINHIT) return(0);
  bonsaifit   cf(tf);
  int         nfit;
  float       gdn[tf->sets()];

  tf->set_hits(&bshits);
  tf->maximize(&cf,vert);
  if ((nfit=tf->nfit())==0) return(0);
  *vert=cf.xfit();
  vert[1]=cf.yfit();
  vert[2]=cf.zfit();
  maxlike[2]=tf->goodness(*maxlike,vert,gdn);
  tf->tgood(vert,0,maxlike[1]);
  *maxlike=cf.maxq();
  cf.fitresult();
  vert[3]=tf->get_zero();
  tf->get_dir(result);
  result[5]=tf->get_ll0();
  tf->set_hits(NULL);
  return(nfit);
}

void clusbonsaifit_(float *clusvert,float *bonsaivert,
		    float *clusresult,float *bonsairesult,
		    float *maxgood,float *maxlike,
		    int *nsel,int *nclusfit,int *nbonsaifit,
		    int *nhit,int *cab,float *t,float *q)
{
  goodness    good(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);
  *nsel=good.nselected();
  *nclusfit=*nbonsaifit=0;
  if (good.nselected()<MINHIT) return;
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&good);
  bonsaifit   cf(&good);
  bonsaifit   bf(tf);
  float       gdn[tf->sets()];

  tf->set_hits(&good);
  *bonsaivert=0;
  bonsaivert[1]=0;
  bonsaivert[2]=0;
  bonsaivert[3]=-1;
  *bonsairesult=0;
  bonsairesult[1]=0;
  bonsairesult[2]=0;
  bonsairesult[3]=0;
  bonsairesult[4]=0;
  bonsairesult[5]=0;
  *maxlike=0;
  maxlike[1]=maxlike[2]=-1;
  *nbonsaifit=-1;
  good.maximize(&cf,&bf,tf,&gr);
  if ((*nclusfit=good.nfit())!=0)
    {
      *maxgood=cf.maxq();
      *clusvert=cf.xfit();
      clusvert[1]=cf.yfit();
      clusvert[2]=cf.zfit();
      cf.fitresult();
      clusvert[3]=good.get_zero();
      *clusresult=good.get_theta();
      clusresult[1]=good.get_phi();
      clusresult[2]=good.get_cos_theta();
      clusresult[3]=good.get_gdn0();
      if ((*nbonsaifit=tf->nfit())!=0)
        {
          *bonsaivert=bf.xfit();
          bonsaivert[1]=bf.yfit();
          bonsaivert[2]=bf.zfit();
        }
      else
        {
          *bonsaivert=*clusvert;
	  bonsaivert[1]=clusvert[1];
	  bonsaivert[2]=clusvert[2];
        }
      maxlike[2]=tf->goodness(*maxlike,bonsaivert,gdn);
      tf->tgood(bonsaivert,0,maxlike[1]);
      if ((*nbonsaifit=tf->nfit())!=0)
        {
          *maxlike=bf.maxq();
          bf.fitresult();
          bonsaivert[3]=tf->get_zero();
          tf->get_dir(bonsairesult);
          bonsairesult[5]=tf->get_ll0();
        }
   }
  tf->set_hits(NULL);
}

void clusandbonsaifit_(float *clusvert,float *bonsaivert,
		       float *clusresult,float *bonsairesult,
		       float *maxgood,float *maxlike,
		       int *nsel,int *nclusfit,int *nbonsaifit,
		       float *cluswallcut,
		       int *nhit,int *cab,float *t,float *q)
{
  goodness    good(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);
  *nsel=good.nselected();
  *nclusfit=*nbonsaifit=0;
  if (good.nselected()<MINHIT) return;
  fourhitgrid gr(geom->cylinder_radius(),geom->cylinder_height(),&good);
  bonsaifit   cf(&good);
  bonsaifit   bf(tf);
  float       gdn[tf->sets()];

  *bonsaivert=0;
  bonsaivert[1]=0;
  bonsaivert[2]=0;
  bonsaivert[3]=-1;
  *bonsairesult=0;
  bonsairesult[1]=0;
  bonsairesult[2]=0;
  bonsairesult[3]=0;
  bonsairesult[4]=0;
  bonsairesult[5]=0;
  *maxlike=0;
  maxlike[1]=maxlike[2]=-1;
  *nbonsaifit=-1;
  good.maximize(&cf,&gr);
  if ((*nclusfit=good.nfit())!=0)
    {
      float rwall,zwall;

      *maxgood=cf.maxq();
      *clusvert=cf.xfit();
      clusvert[1]=cf.yfit();
      clusvert[2]=cf.zfit();
      cf.fitresult();
      clusvert[3]=good.get_zero();
      *clusresult=good.get_theta();
      clusresult[1]=good.get_phi();
      clusresult[2]=good.get_cos_theta();
      clusresult[3]=good.get_gdn0();
      rwall=good.returnrvol()-sqrt(cf.xfit()*cf.xfit()+cf.yfit()*cf.yfit());
      zwall=good.returnzvol()-fabs(cf.zfit());
      if (rwall<zwall)
	{
	  if (rwall<*cluswallcut) return;
	}
      else
	{
	  if (zwall<*cluswallcut) return;
	}
    }
  tf->set_hits(&good);
  tf->maximize(&bf,&gr);
  if ((*nbonsaifit=tf->nfit())!=0)
    {
     *bonsaivert=bf.xfit();
      bonsaivert[1]=bf.yfit();
      bonsaivert[2]=bf.zfit();
      maxlike[2]=tf->goodness(*maxlike,bonsaivert,gdn);
      tf->tgood(bonsaivert,0,maxlike[1]);
      *maxlike=bf.maxq();
      bf.fitresult();
      bonsaivert[3]=tf->get_zero();
      tf->get_dir(bonsairesult);
      bonsairesult[5]=tf->get_ll0();
    }
  tf->set_hits(NULL);
}

void goodness_(float *guncor,float *vertex,int *nhit,int *cab,float *t,float *q)
{
  //hitsel event_hits(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);
  hitsel event_hits(0,0,0,0,0,tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);

  tf->set_hits(&event_hits);
  tf->tgood(vertex,0,*guncor);
}

int nwin1_(float *vertex,float *tmin,float *tmax,
	   int *nhit,int *cab,float *t,float *q)
{
  goodness       good(tf->sets(),tf->chargebins(),geom,*nhit,cab,t,q);

  tf->set_hits(&good);
  return(tf->nwind(vertex,*tmin,*tmax));
}

int nwin2_(float *vertex,float *tmin,float *tmax,float *ttof,int *cab)
{
  return(tf->nwind(vertex,*tmin,*tmax,ttof,cab));
}

int nwin3_(float *vertex,float *tmin,float *tmax,
	   float *ttof,float *d,int *cab)
{
  return(tf->nwind(vertex,*tmin,*tmax,ttof,d,cab));
}

void bsfilereadopen_(char *name,int length)
{
  char cname[length+1];
  int i;

  for(i=0; i<length; i++)
    cname[i]=name[i];
  cname[i]=0;
  printf("Opening BONSAI binary file *%s* to read\n",cname);
  bsfile=new binfile(cname,'r');
}

void bsfilewriteopen_(char *name,int length)
{
  char cname[length+1];
  int i;

  for(i=0; i<length; i++)
    cname[i]=name[i];
  cname[i]=0;
  printf("Opening BONSAI binary file *%s* to write\n",cname);
  bsfile=new binfile(cname,'w');
}

void bsfileclose_(void)
{
  delete bsfile;
}

char bsfileitread(unsigned int *eventnr,comtype2 *itevent)
{
  int       *sizes,*numbers,nhit;
  void      **starts;
  short int *cables;
  float     *tq;

  if (bsfile->read(sizes,numbers,starts)!=2) return(1);
  if ((sizes[0]!=2) || (sizes[1]!=4))
    {
      delete(sizes);
      delete(numbers);
      delete(starts);
      return(1);
    }
  nhit=numbers[0];
  cables=(short int *)starts[0];
  tq=(float *)starts[1];
  delete(sizes);
  delete(numbers);
  delete(starts);
  if (numbers[1]!=2*nhit+1) return(1);
  for(itevent->it_index=0; itevent->it_index<nhit; itevent->it_index++)
    {
      itevent->hits[itevent->it_index][0]=cables[itevent->it_index];
      itevent->hits[itevent->it_index][2]=tq[itevent->it_index];
      itevent->hits[itevent->it_index][1]=tq[itevent->it_index+nhit];
    }
  *eventnr=*((unsigned int *)(tq+2*nhit));
  delete(cables);
  delete(tq);
  return(0);
}

void bsfileitwrite(unsigned int eventnr,comtype2 *itevent)
{
   int       nhit=itevent->it_index,i,sizes[3],numbers[3];
   void      *starts[2];
   short int cabs[nhit];
   float     tq[2*nhit+1];

   for(i=0; i<itevent->it_index; i++)
     {
       cabs[i]=(short int)fabs(itevent->hits[i][0]);
       tq[i]=itevent->hits[i][2];
       tq[nhit+i]=itevent->hits[i][1];
       *(unsigned int *) (tq+2*nhit)=eventnr;
     }
   sizes[0]=2;
   sizes[1]=4;
   sizes[2]=-1;
   numbers[0]=nhit;
   numbers[1]=2*nhit+1;
   numbers[2]=-1;
   starts[0]=cabs;
   starts[1]=tq;
   bsfile->write(sizes,numbers,starts);
}
