#define FOURLOG10 9.2103404

#define NLIKE   56   /* # searched bins for quick search */
#define NHIST   70   /* # bins for histogram for t0 presearch */

#define BINS1   30.  /* histogram #1 bin size in nsec*/
#define BINS2    6.  /* histogram #2 bin size in nsec*/
#define HRANGE  31

#define FACT1    (1/BINS1)
#define FACT2    (1/BINS2)

#include "RAT/BONSAI/timefit.h"
#include "RAT/BONSAI/centroid.h"
#include <stdio.h>
#include <stdlib.h>
#include "RAT/BONSAI/binfile.h"

namespace BONSAI {

// **********************************************
// fits the vertex time, assuming a given vertex
// calculates the likelihood for the timing dist.
// **********************************************

// **********************************************
// delete arrays
// **********************************************
void timefit::delete_dist(void)
{
  if (offset!=NULL) delete(offset);
  if (loglike!=NULL) delete(loglike);
  if (pdf!=NULL) delete(pdf);
  if (pdf_sum!=NULL) delete(pdf_sum);
  if (lmax!=NULL) delete(lmax);
  if (pdf_max!=NULL) delete(pdf_max);
  if (sig!=NULL) delete(sig);
  offset=NULL;
  loglike=NULL;
  pdf=NULL;
  pdf_sum=NULL;
  nneg=NULL;
  lmax=NULL;
  pdf_max=NULL;
  qminmax=NULL;
  sig=NULL;
}

// **********************************************
// create arrays
// **********************************************
void timefit::create_dist(void)
{
  loglike=new unsigned int[5*offset[nlike-1]];
  pdf=new float[5*offset[nlike-1]];
  pdf_sum=new float[5*offset[nlike-1]];
  lmax=new unsigned int[nlike];
  pdf_max=new float[2*nlike];
  sig=new float[nlike];
}

// **********************************************
// calculate the integrated pdfs
// **********************************************
void timefit::integrate_dist(void)
{
  int i,k,set,part=offset[nlike-1];

  for(set=i=0; set<(int)nlike; set++)
    {
      for(lmax[set]=loglike[offset[set]-1]; i<(int)offset[set]-1; i++)
	{
	  if (loglike[i]>lmax[set])
	    lmax[set]=loglike[i];
	  for(k=1; k<5; k++)
	    loglike[k*part+i]=((5-k)*loglike[i]+k*loglike[i+1])/5;
	}
      for(k=1; k<5; k++)
	loglike[k*part+i]=(5-k)*loglike[i]/5;
      i++;
    }
  // calculate the log(likelihood) values from the integers
  for(i=0; i<5*part; i++)
    pdf[i]=exp(FOURLOG10*(loglike[i]*1E-5-1));
  // sum up the log(likelihood) for each charge bin
  for(set=i=0; set<(int)nlike; set++)
    {
      pdf_sum[i]=pdf[i];
      for(k=1; k<5; k++)
	pdf_sum[k*part+i]=pdf_sum[(k-1)*part+i]+pdf[k*part+i];
      for(i++; i<(int)offset[set]; i++)
	{
	  pdf_sum[i]=pdf[i]+pdf_sum[4*part+i-1];
	  for(k=1; k<5; k++)
	    pdf_sum[k*part+i]=pdf[k*part+i]+pdf_sum[(k-1)*part+i];
	}
      pdf_max[nlike+set]=exp(FOURLOG10*(1e-5*lmax[set]-1));
    }
  for(set=0; set<(int)nlike-1; set++)
    printf("pdf for %5.2fpe<=q<%5.2fpe: zero at %d total=%fns\n",
	   qminmax[set],qminmax[set+1],nneg[set],
	   0.2*TBIN*pdf_sum[4*part+offset[set]-1]);
  printf("pdf for %5.2fpe<=q: zero at %d total=%fns\n",
	 qminmax[set],nneg[set],0.2*TBIN*pdf_sum[4*part+offset[set]-1]);
}

// **********************************************
// load timing distribution from disk
// **********************************************
void timefit::load_dist(void)
{
  unsigned int *distp;
  int          i,*sizes,*numbers;
  void         **starts;

  delete_dist();

  // ********************** load pdfs from file **********************
  printf("Loading pdfs...\n");
  binfile      bf((char*)"like.bin",'r');

  if (bf.read(sizes,numbers,starts)!=2)
    {
      printf("Not a valid likelihood binary file!\n");
      exit(1);
    }
  nlike=numbers[0]/3;
  bgfit=((int *)(starts[0]))[nlike-1];
  if (bgfit) printf("Turn on background fitting\n");
  offset=(unsigned int *)(starts[0]);
  offset[nlike-1]=numbers[1];
  nneg=offset+nlike;
  qminmax=(float *)(starts[0])+2*nlike;
  create_dist();
  distp=(unsigned int *)(starts[1]);
  for(i=0; i<numbers[1]; i++) loglike[i]=distp[i];
  delete(distp);
  delete(numbers);
  delete(sizes);
  delete(starts);

  integrate_dist(); // integrate pdfs'
  // ********** define constants for the log L peak fitting **********
  nbin=3;
  s0=2*nbin+1;
  sx2=nbin*(nbin+1)*s0/3;
  sx4=nbin*(nbin*nbin*nbin+
	    (2*nbin*nbin*nbin*nbin+3)/5)+
            2*nbin*(nbin*nbin-1)/3;
  det=sx4*s0-sx2*sx2;
}

// **********************************************
// calculate time-of-flight subtracted hit times
// **********************************************
void timefit::maketof(float *vertex)
{
  int i,nh=event_hits->ntot();

  if (event_hits==NULL) return;
  if (tof!=NULL) delete(tof);
  tof=new float[nh];
  for(i=0; i<nh; i++)
    tof[i]=event_hits->hittime(i)-event_hits->tof(vertex,i);
}

// **********************************************
// calculate time-of-flight subtracted hit times
// **********************************************
void timefit::makedirtof(float *vertex)
{
  int i,nh=event_hits->ntot();

  if (event_hits==NULL) return;
  if (tof!=NULL) delete(tof);
  if (dir!=NULL) delete(dir);
  tof=new float[nh];
  dir=new float[3*nh];
  for(i=0; i<nh; i++)
    tof[i]=event_hits->hittime(i)-event_hits->tof(vertex,dir+3*i,i);
}


// **********************************************
// do a histogram search for the peak time in
// the tof-subtracted timing distribution
// **********************************************
inline float timefit::histo_t0(void)
{
  int          i,number,max;
  unsigned int *lp;
  float        tstart,time;

  // histogram #1: coarse search
  for(lp=time_histo,i=0; i<NHIST; i++) *(lp++)=0;
  for(i=0; i<event_hits->ntot(); i++)
    if ((time=tof[i])>0)
      if ((number=(int) (time*FACT1))<NTIME)
	time_histo[number]++;
  for(max=0,i=1; i<NHIST; i++)
    if (time_histo[i]>time_histo[max]) max=i;
  tstart=(max+0.5)*BINS1-(NHIST/2)*BINS2;

  // histogram #2: fine search
  for(lp=time_histo,i=0; i<NHIST; i++) *(lp++)=0;
  for(i=0; i<event_hits->ntot(); i++)
    if ((time=tof[i]-tstart)>0)
      if ((number=(int) (time*FACT2))<NTIME)
	time_histo[number]++;
  for(max=0,i=1; i<NHIST; i++)
    if (time_histo[i]>time_histo[max]) max=i;
  return(tstart+(max+0.5)*BINS2-HRANGE*TBIN);
}

// **********************************************
// quickly sum up the log(likelihood) for all hits
// **********************************************
float timefit::fastaddloglik(float &t0,float &dt,int &max)
{
  int   sub;
  float peak;

  sub=(int) (histo_t0()*BINPERNS);
  max=addloglik(sub,NLIKE);
  if (max==0)
    {
      dt=-1.;
      t0=TBIN*sub;
      return(4*(time_histo[max]*1E-5-event_hits->ntot()));
    }
  if (max==NLIKE-1)
    {
      dt=-1.;
      t0=(sub+max)*TBIN;
      return(4*(time_histo[max]*1E-5-event_hits->ntot()));
    }
  peak=peakfit(max,t0,dt);
  t0+=TBIN*sub;
  return(peak);
}

// **********************************************
// sum up the log(likelihood) for all hits,
// if peak time is already approx. known
// **********************************************
float timefit::addloglik(float &t0,float &dt,int &max,float tmi,float tpl)
{
  int   sub,nlik;
  float peak;

  sub=(int) ((t0-tmi)*BINPERNS-2.5);
  nlik=(int) (5+(tmi+tpl)*BINPERNS);
  //printf("2sub=%f %d %d %f\n",(t0-tmi)*BINPERNS-2.5,sub,nlik,5+(tmi+tpl)*BINPERNS);
  max=addloglik(sub,nlik);
  if (max==0)
    {
      t0-=2*tpl;
      return(addloglik(t0,dt,max,2*tmi,2*tpl));
    }
  if (max==nlik-1)
    {
      t0+=2*tmi;
      return(addloglik(t0,dt,max,2*tmi,2*tpl));
    }
  if (max==0)
    {
      dt=-1.;
      t0=TBIN*sub;
      return(4*(time_histo[max]*1E-5-event_hits->ntot()));
    }
  if (max==nlik-1)
    {
      dt=-1.;
      t0=(sub+max)*TBIN;
      return(4*(time_histo[max]*1E-5-event_hits->ntot()));
    }
  peak=peakfit(max,t0,dt);
  t0+=TBIN*sub;
  return(peak);
}

// **********************************************
// interpolate with a parabola between the three
// largest likelihoods
// **********************************************
inline float timefit::maxloglike(int max)
{
  float interpol,sum=time_histo[max-1]+time_histo[max+1];
  float diff2=(float)time_histo[max+1]-(float)time_histo[max-1];

  diff2=diff2*diff2;
  interpol=time_histo[max]+0.125*diff2/(1.6*time_histo[max]-0.8*sum);

  return(4*(time_histo[max]*1E-5-event_hits->ntot()));
  return(4*(interpol*1E-5-event_hits->ntot()));
}

// **********************************************
// fit a parabola to the likelihood function;
// calculate vertex time and error
// **********************************************
float timefit::peakfit(int max,float &tpeak,float &dt)
{
  int          i,number;
  unsigned int *lp;
  float        sy,sxy,sx2y,loglikev,curv,b;

  sy=sxy=sx2y=0;
  if (max<=nbin)
    lp=time_histo;
  else
    lp=time_histo+max-nbin;
  if (max>=nbin)
    if (max+nbin<NTIME)
      number=nbin;
    else
      number=NTIME-max-1;
  else
    number=max;
  for(i=-nbin; i<=number; i++)
    {
      loglikev=4*(*(lp++)*1E-5-event_hits->ntot());
      sy+=loglikev;
      sxy+=i*loglikev;
      sx2y+=i*i*loglikev;
    }
  curv=sx2*sy-sx2y*s0;
  b=sxy/sx2;
  /* c=(sx4*sy-sx2*sx2y)/det;*/
  if (fabs(curv)<1e-10)
    {
      curv=det*5e9;
      tpeak=TBIN*(max+1);
    }
  else
    {
      curv=det/(2*curv);
      tpeak=TBIN*(max+1+b*curv);
    }
  if (curv<0)
    {
      dt=-sqrt(-curv);
      tpeak=TBIN*(max+1);
    }
  else
    dt=TBIN*sqrt(curv);
  return(maxloglike(max));
}


// **********************************************
// shift the hit times by the vertex time;
// find minimum and maximum
// **********************************************
inline void timefit::tofminmax(float &tofmin,float &tofmax)
{
  int i;

  tofmin=tofmax=*tof;
  for(i=1; i<event_hits->ntot(); i++)
    {
      if (tof[i]<tofmin) tofmin=tof[i];
      if (tof[i]>tofmax) tofmax=tof[i];
    }
}

// **********************************************
// define a signal range for each charge bin
// around the vertex time t0=0;
// calculate the normalization of
// the likelihood function
// **********************************************
inline void timefit::sigrange(float tofmin,float tofmax,
                              float *norm_fact,float *time_range)
{
  int   size=offset[nlike-1];
  int   set,offs,min,max,mins,maxs,smins,smaxs;

  max= (int) (0.5-tofmin*BINPERNS);
  min=-(int) (0.5+tofmax*BINPERNS);
  if ((smaxs=+(int) (0.5-5*tofmin*BINPERNS)-5*max)<0)
    {
      max--;
      smaxs+=5;
    }
  if ((smins=-(int) (0.5+5*tofmax*BINPERNS)-5*min)<0)
    {
      min--;
      smins+=5;
    }
  for(offs=set=0; set<(int)nlike; offs=offset[set++])
    {
      maxs=max+offs+nneg[set];
      mins=min+offs+nneg[set];
      if (maxs+0.2*smaxs<offset[set])
        {
	  norm_fact[set]=pdf_sum[maxs+size*smaxs-1];
	  time_range[set]=maxs+0.2*smaxs;
        }
      else
        {
	  norm_fact[set]=pdf_sum[offset[set]-1+4*size];
          time_range[set]=offset[set];
        }
      if (mins-1+0.2*smins>offs)
        {
	  norm_fact[set]-=pdf_sum[mins+size*smins-1];
          time_range[set]-=mins+0.2*smins;
        }
      else
        time_range[set]-=offs;
      norm_fact[set]=5*BINPERNS/norm_fact[set];
      time_range[set]*=TBIN;
    }
}

// **********************************************
// calculate the likelihood for each event;
// estimate dark noise background and add to
// likelihood normalize signal likelihood
// **********************************************
float timefit::makelike(float t0)
{
  float tofmin,tofmax;
  float bg=0,llmax,time,norm;
  int   offs,set,i,j,shift,shift2,nbg;

  if (tof==NULL) return(-1e10);
  tofminmax(tofmin,tofmax);
  tofmin-=t0;
  tofmax-=t0;
  float *norm_fact = new float [nlike], *time_range = new float[nlike];
  sigrange(tofmin,tofmax,norm_fact,time_range);
  if (like!=NULL) delete(like);
  like=new float[event_hits->ntot()];
  llmax=0;
  for(nbg=set=offs=i=0; set<(int)nlike; offs=offset[set++])
    {
      j=i;
      for(nbg=0; i<event_hits->beyond_last_hit(set); i++)
	{
	  time=(tof[i]-t0)*BINPERNS;
	  if (time>0)
	    { shift=-(int) (0.5+time); shift2=-(int) (0.5+5*time)-5*shift; }
	  else
	    { shift=+(int) (0.5-time); shift2=+(int) (0.5-5*time)-5*shift; }
	  if (shift2<0)
	    {
	      shift--;
	      shift2+=5;
	    }
	  shift+=offs+nneg[set];
	  if ((shift<offs) || (shift>=(int)offset[set]))
	    {
	      like[i]=0.0;
	      nbg++;
	    }
	  else
	    like[i]=norm_fact[set]*pdf[shift+shift2*offset[nlike-1]];
	}

      // 17-MAY-2008 by y.t.  to avoid divided by 0 (as a test)
//      sig[set]=event_hits->nhits(set)-nbg*(tofmax-tofmin)/
//	            (tofmax-tofmin-time_range[set]);
      if ((tofmax-tofmin-time_range[set])>0) 
	  sig[set]=event_hits->nhits(set)-nbg*(tofmax-tofmin)/
	      (tofmax-tofmin-time_range[set]);
      else {
        delete [] norm_fact;
        delete [] time_range;
	    return(-1e10);
	  }

      if (sig[set]<0) sig[set]=0;
      bg=event_hits->nhits(set)-sig[set];
      bg/=tofmax-tofmin;
      pdf_max[set]=bg+sig[set]*norm_fact[set]*pdf_max[set+nlike];
      norm=1/pdf_max[set];
      time=norm*bg;
      for(i=j; i<event_hits->beyond_last_hit(set); i++)
        if (like[i]==0)
	  like[i]=time;
	else
	  {
	    like[i]=like[i]*sig[set]+bg;
	    llmax+=log(like[i]);
	    like[i]*=norm;
	  }
    }
//  llmax+=nbg*log(bg); // 17-MAY-2008 fixed by M.Smy/Y.Takeuchi 
  delete [] norm_fact;
  delete [] time_range;
  if ((bg<0) || ((bg==0) && (nbg!=0))) return(-1e10);
  if (nbg>0) llmax+=nbg*log(bg);
  return(llmax);
}
// **********************************************
// calculate the likelihood for each event;
// estimate dark noise background and add to
// likelihood normalize signal likelihood, don't include bg
// **********************************************
float timefit::makesiglike(float t0,float *bg)
{
  float tofmin,tofmax;
  float llmax,time,norm;
  int   offs,set,i,j,shift,shift2,nbg;

  if (tof==NULL) return(-1e10);
  tofminmax(tofmin,tofmax);
  tofmin-=t0;
  tofmax-=t0;
  float *norm_fact = new float [nlike], *time_range = new float[nlike];
  sigrange(tofmin,tofmax,norm_fact,time_range);
  if (like!=NULL) delete(like);
  like=new float[event_hits->ntot()];
  llmax=0;
  for(nbg=set=offs=i=0; set<(int)nlike; offs=offset[set++])
    {
      j=i;
      for(nbg=0; i<event_hits->beyond_last_hit(set); i++)
	{
	  time=(tof[i]-t0)*BINPERNS;
	  if (time>0)
	    { shift=-(int) (0.5+time); shift2=-(int) (0.5+5*time)-5*shift; }
	  else
	    { shift=+(int) (0.5-time); shift2=+(int) (0.5-5*time)-5*shift; }
	  if (shift2<0)
	    {
	      shift--;
	      shift2+=5;
	    }
	  shift+=offs+nneg[set];
	  if ((shift<offs) || (shift>=(int)offset[set]))
	    {
	      like[i]=0.0;
	      nbg++;
	    }
	  else
	    like[i]=norm_fact[set]*pdf[shift+shift2*offset[nlike-1]];
	}
      sig[set]=event_hits->nhits(set)-nbg*(tofmax-tofmin)/
	            (tofmax-tofmin-time_range[set]);
      if (sig[set]<0) sig[set]=0;
      bg[set]=event_hits->nhits(set)-sig[set];
      bg[set]/=tofmax-tofmin;
      pdf_max[set]=bg[set]+sig[set]*norm_fact[set]*pdf_max[set+nlike];
      norm=1/pdf_max[set];
      for(i=j; i<event_hits->beyond_last_hit(set); i++)
        if (like[i]*sig[set]>bg[set])
	  {
	    like[i]=like[i]*sig[set]+bg[set];
	    llmax+=log(like[i]);
	    like[i]*=norm;
	  }
        else
          like[i]=-1;
      bg[set]*=norm;
    }
  delete [] norm_fact;
  delete [] time_range;
  return(llmax);
}

// **********************************************
// calculate likelihood given a background rate
// **********************************************
inline float timefit::makelike(int set,float &bg,float range)
{
  float llmax;
  int   i,nbg;

  if ((!sig) || (sig[set]>event_hits->nhits(set))) return(-1e10);
  bg=(event_hits->nhits(set)-sig[set])/range;
  nbg=0;
  for(llmax=0,i=event_hits->first_hit(set); i<event_hits->beyond_last_hit(set); i++)
    if (like[i]==0.0)
      nbg++;
    else
      llmax+=log(like[i]*sig[set]+bg);
  llmax+=nbg*log(bg);
  return(llmax);
}

// **********************************************
// initialize arrays
// **********************************************
timefit::timefit(void)
{
  bgfit=0;
  nlike=0;
  time_histo=new unsigned int[NTIME];
  offset=NULL;
  loglike=NULL;
  pdf=NULL;
  pdf_sum=NULL;
  nneg=NULL;
  lmax=NULL;
  pdf_max=NULL;
  qminmax=NULL;
  tof=NULL;
  dir=NULL;
  like=NULL;
  sig=NULL;
  load_dist();
  event_hits=NULL;
}

// **********************************************
// initialize arrays and create hitsel object
// **********************************************
timefit::timefit(hitsel *hs)
{
  nlike=0;
  time_histo=new unsigned int[NTIME];
  offset=NULL;
  loglike=NULL;
  pdf=NULL;
  pdf_sum=NULL;
  nneg=NULL;
  lmax=NULL;
  qminmax=NULL;
  tof=NULL;
  dir=NULL;
  like=NULL;
  sig=NULL;
  load_dist();
  event_hits=hs;
}

// **********************************************
// free arrays and hitsel object
// **********************************************
timefit::~timefit(void)
{
  delete_dist();
  delete(time_histo);
  if (event_hits!=NULL) delete(event_hits);
  event_hits=NULL;
  if (tof!=NULL) delete(tof);
  tof=NULL;
  if (dir!=NULL) delete(dir);
  dir=NULL;
  if (like!=NULL) delete(like);
  like=NULL;
}

// **********************************************
// fit the vertex time and direction
// **********************************************
inline void timefit::fitdir(float *vertex,float &dt,
			     centroid *&centr,float &mucang)
{
  float  *weight,time,histo[201];
  double direct[4];
  int    set,offs,ind,bin,bin2;

  weight=new float[event_hits->ntot()];
  for(set=offs=ind=0; set<(int)nlike; offs=offset[set++])
    for(; ind<event_hits->beyond_last_hit(set); ind++)
      {
	time=tof[ind]-vertex[3];
	if (time>0)
	  weight[ind]=-0.04*time*time;
	else
	  weight[ind]=-0.125*time*time;
	if (weight[ind]>-10)
	  {
	    time*=BINPERNS;
	    if (time>0)
	      { bin=-(int) (0.5+time); bin2=-(int) (0.5+5*time)-5*bin; }
	    else
	      { bin=+(int) (0.5-time); bin2=+(int) (0.5-5*time)-5*bin; }
	    if (bin2<0)
	      {
		bin--;
		bin2+=5;
	      }
	    bin+=offs+nneg[set];
	    if ((bin>=offs) && (bin<(int)offset[set]))
	      weight[ind]=pdf[bin+bin2*offset[nlike-1]]*exp(weight[ind]);
	    else
	      weight[ind]=0;
	  }
	else
	  weight[ind]=0;
      }
  centr=new centroid(event_hits->ntot(),dir,weight);
  centr->centre(direct);
  direct[3]=sqrt(*direct*(*direct)+direct[1]*direct[1]+direct[2]*direct[2]);
  if (direct[3]>0)
    {
      *direct/=direct[3];
      direct[1]/=direct[3];
      direct[2]/=direct[3];
    }
  else
    {
      mucang=0;
      delete(weight);
      return;
    }
  for(set=0; ((set<(int)nlike) && (event_hits->sumhits(set)>30)); offs=offset[set++])
    if (qminmax[set]>=9) break;
  for(bin=0; bin<=200; bin++)
    histo[bin]=0;
  for(ind=event_hits->first_hit(set); set<(int)nlike; set++)
    for(; ind<event_hits->beyond_last_hit(set); ind++)
      {
	bin=(int)
	  (200*(*direct*dir[3*ind]+direct[1]*dir[3*ind+1]+direct[2]*dir[3*ind+2])/
	   (direct[3]));
	if (bin<0) bin=0;
	if (bin>200) bin=200;
	histo[bin]+=weight[ind];
      }
  for(bin=1; bin<=200; bin++)
    histo[bin]+=histo[bin-1];
  for(bin=199; ((bin>=0) && (histo[bin]>0.05*histo[200])); bin--);
  mucang=(bin+0.5)*(direct[3])/200;
  delete(weight);
  return;
}

void timefit::fittime(int ndt,float *dts,float &tpeak,float &dt)
{
  int          i,j,shift,shift2,number,max;
  float        time,sy,sxy,sx2y,loglikev,curv,b;
  unsigned int *lp,*distp,*start;

  for(lp=time_histo,i=0; i<NTIME; i++) *(lp++)=0;
  for(i=0; i<ndt; i++)
    {
      time=(dts[i]+800)*BINPERNS;
      if (time>0)
	{
	  shift=(int) (time+0.5); shift2=5*shift-(int) (5*time+0.5);
	  if (shift2<0)
	    {
	      shift++;
	      shift2+=5;
	    }
	  distp=loglike+shift2*offset[nlike-1];
	  shift-=nneg[0];
	  number=NTIME-shift;
	  max=offset[0];
	  if (number>max) number=max;
	  if (shift>0)
	    start=time_histo+shift;
	  else
	    {
	      start=time_histo;
	      distp-=shift;
	      number+=shift;
	    }
 	  for(j=0; j<number; j++)
 	    *(start++)+=*(distp++);
	}
    }
  for(max=0,i=1; i<NTIME; i++)
    if (time_histo[i]>time_histo[max]) max=i;
  sy=sxy=sx2y=0;
  if (max<=nbin)
    lp=time_histo;
  else
    lp=time_histo+max-nbin;
  if (max>=nbin)
    if (max+nbin<NTIME)
      number=nbin;
    else
      number=NTIME-max-1;
  else
    number=max;
  for(i=-nbin; i<=number; i++)
    {
      loglikev=4*(*(lp++)*1E-5-ndt);
      sy+=loglikev;
      sxy+=i*loglikev;
      sx2y+=i*i*loglikev;
    }
  curv=sx2*sy-sx2y*s0;
  b=sxy/sx2;
  /* c=(sx4*sy-sx2*sx2y)/det;*/
  if (fabs(curv)<1e-10)
    {
      curv=det*5e9;
      tpeak=TBIN*(max+1);
    }
  else
    {
      curv=det/(2*curv);
      tpeak=TBIN*(max+1+b*curv);
    }
  if (curv<0)
    {
      dt=-sqrt(-curv);
      tpeak=TBIN*(max+1);
    }
  else
    dt=TBIN*sqrt(curv);
  tpeak-=800;
  return;
}

// **********************************************
// fit the vertex time and direction, calculate ellipticity
// **********************************************
float timefit::fittime(short int fast,float *vertex,float *direct,float &dt)
{
  int      maxprod;
  axes     ax;
  centroid *centr;
  float    cdiff,ll;

  makedirtof(vertex);
  if (fast)
    ll=fastaddloglik(vertex[3],dt,maxprod);
  else
    ll=addloglik(vertex[3],dt,maxprod);
  fitdir(vertex,dt,centr,direct[4]);
  if (centr->trace()<0.001)
    {
      double dir[4];

      centr->centre(dir);
      dir[3]=*dir*(*dir)+dir[1]*dir[1]+dir[2]*dir[2];
      if (dir[3]>0)
        {
	  direct[3]=dir[3]=sqrt(dir[3]);
	  *direct=acos(dir[2]/dir[3]);
	}
      else
        {
          *direct=acos(dir[2]);
          direct[3]=0;
	}
      direct[1]=atan2(dir[1],dir[2]);
      direct[2]=direct[4]=0;
    }
  else
    {
      centr->eigen(ax);
      maxprod=ax.align(*centr,direct);
      cdiff=direct[3]-direct[4];
      if (cdiff<0.05)
        direct[3]=direct[4];
      else if (cdiff<0.15)
        direct[3]=direct[4]+cdiff*(10*cdiff-0.5);
      if (maxprod==0)
        {
          direct[4]=(centr->yy()+centr->zz());
	  if (direct[4]>0) direct[4]=fabs(centr->yy()-centr->zz())/direct[4];
        }
      else if (maxprod==1)
        {
          direct[4]=(centr->xx()+centr->zz());
          if (direct[4]>0) direct[4]=fabs(centr->xx()-centr->zz())/direct[4];
	}
      else if (maxprod==2)
        {
          direct[4]=(centr->xx()+centr->yy());
          if (direct[4]>0) direct[4]=fabs(centr->xx()-centr->yy())/direct[4];
	}
    }
  delete(centr);
  if (bgfit)
    return(makelike(vertex[3]));
  return(ll);
}

// **********************************************
// fit the vertex time and direction, calculate ellipticity,
// if time is already approximately known
// **********************************************
float timefit::fittime(float *vertex,float *direct,float &dt,float tmi,float tpl)
{
  int      maxprod;
  axes     ax;
  centroid *centr;
  float    cdiff,ll;

  makedirtof(vertex);
  ll=addloglik(vertex[3],dt,maxprod,tmi,tpl);
  fitdir(vertex,dt,centr,direct[4]);
  if (centr->trace()<0.001)
    {
      double dir[4];

      centr->centre(dir);
      dir[3]=*dir*(*dir)+dir[1]*dir[1]+dir[2]*dir[2];
      if (dir[3]>0)
        {
	  direct[3]=dir[3]=sqrt(dir[3]);
	  *direct=acos(dir[2]/dir[3]);
	}
      else
        {
          *direct=acos(dir[2]);
          direct[3]=0;
	}
      direct[1]=atan2(dir[1],dir[2]);
      direct[2]=direct[4]=0;
    }
  else
    {
      centr->eigen(ax);
      maxprod=ax.align(*centr,direct);
      cdiff=direct[3]-direct[4];
      if (cdiff<0.05)
        direct[3]=direct[4];
      else if (cdiff<0.15)
        direct[3]=direct[4]+cdiff*(10*cdiff-0.5);
      if (maxprod==0)
        {
          direct[4]=(centr->yy()+centr->zz());
	  if (direct[4]>0) direct[4]=fabs(centr->yy()-centr->zz())/direct[4];
        }
      else if (maxprod==1)
        {
          direct[4]=(centr->xx()+centr->zz());
          if (direct[4]>0) direct[4]=fabs(centr->xx()-centr->zz())/direct[4];
	}
      else if (maxprod==2)
        {
          direct[4]=(centr->xx()+centr->yy());
          if (direct[4]>0) direct[4]=fabs(centr->xx()-centr->yy())/direct[4];
	}
    }
  delete(centr);
  if (bgfit)
    return(makelike(vertex[3]));
  return(ll);
}


inline void timefit::corfunc(int *&bin,float *&cor,double *direct,double cang)
{
  float cbin[4],sprod;
  int   i,j;

  bin=new int[event_hits->ntot()];
  cor=new float[event_hits->ntot()];
  cbin[3]=0.5*(1+cang);
  cbin[2]=cang;
  cbin[1]=1.5*cang-0.5;
  cbin[0]=2*cang-1;
  for(i=0; i<event_hits->ntot(); i++)
    {
      sprod=dir[3*i]*direct[0]+dir[3*i+1]*direct[1]+dir[3*i+2]*direct[2];
      for(j=0; ((j<4) && (sprod>cbin[j])); j++);
      bin[i]=j;
      switch(j)
	{
	case 0: cor[i]=0;    break;
	case 1: cor[i]=-1.5; break;
	case 2:
	case 3: sprod=2*(sprod-cang)/(1-cang);
                cor[i]=1.5*sprod*(1-sprod*sprod/3);
		break;
	case 4: cor[i]=1.5; break;
	  }
    }
}

inline void timefit::lcor(float &alpha,float &t0,float &tout,int *bin,float *cor)
{
  float        costh[5];
  float        time,t0bin,dt;
  float        sum1,sumf,sumf2,sumt,sumft,dt2;
  int          i,j,offs,set,shift,shift2,number,max;
  unsigned int *distp,*start,count[5];

  for(j=0; j<5; j++) costh[j]=count[j]=0;
  costh[0]=costh[1]=-1;
  costh[4]=1;
  for(j=0; j<500; j++) time_histo[j]=0;
  for(i=offs=set=0; set<(int)nlike; offs=offset[set++])
    {
      for(; i<event_hits->beyond_last_hit(set); i++)
        {
          time=(100+tof[i]-t0)*BINPERNS;
          if (time>0)
            {

	      shift=(int) (time+0.5); shift2=5*shift-(int) (5*time+0.5);
	      if (shift2<0)
		{
		  shift++;
		  shift2+=5;
		}
	      distp=loglike+offs+shift2*offset[nlike-1];
	      shift-=200+nneg[set];
	      number=100-shift;
	      max=offset[set]-offs;
	      if (number>max) number=max;
	      if (shift>0)
		start=time_histo+shift;
	      else
		{
		  start=time_histo+100*bin[i];
		  distp-=shift;
		  number+=shift;
		}
	      if (number>0)
		{
		  j=bin[i];
		  if ((j>1) && (j<4)) costh[j]+=cor[i];
		  count[j]++;
		  for(j=0; j<number; j++)
		    *(start++)+=*(distp++);
		}
            }
        }
    }
  sum1=sumf=sumf2=sumt=sumft=0;
  for(i=0; i<5; i++)
    if (count[i]>0)
      {
	if ((i>1) && (i<4)) costh[i]/=count[i];
	for(max=100*i,j=100*i+1; j<100*(i+1); j++)
	  if (time_histo[j]>time_histo[max]) max=j;
	if ((max>100*i) && (max<100*(i+1)))
	  {
	    peakfit(max,t0bin,dt);
	    t0bin-=20+40*i;
	    //printf("lcor: %6.3f %5.2f %4.2f ",costh[i],t0bin,dt);
	    if (i==0)
	      if (t0bin>0.5*dt) tout=t0bin; else tout=0;
	    else
	      {
		dt2=dt*dt;
		sumf+=costh[i]/dt2;
		sumf2+=costh[i]*costh[i]/dt2;
		sumt+=t0bin/dt2;
		sumft+=costh[i]*t0bin/dt2;
		sum1+=1/dt2;
		if (i==3) { if (fabs(t0)>dt) t0=t0bin; else t0=0; }
	      }
	  }
      }
  alpha=sumf2*sum1-sumf*sumf;
  if (sumf2>0) alpha=(sumft*sum1-sumf*sumt)/alpha; else alpha=0;
}

inline void timefit::maketof(float alpha,float t0,float tout,float *origtof,int *bin,float *cor)
{
  int i;

  for(i=0; i<event_hits->ntot(); i++)
    if (bin[i]==0)
      tof[i]=origtof[i]-(t0+tout);
    else
      tof[i]=origtof[i]-(t0+alpha*cor[i]);
}

inline float timefit::addloglik(float *vertex,double *direct,double cang,float ll0)
{
  float *cor,*copytof,ll,alpha,t0,tout=0,dt;
  int   *bin,sub=(int) (vertex[3]*BINPERNS)-NLIKE/2;

  corfunc(bin,cor,direct,cang);
  t0=vertex[3];
  lcor(alpha,t0,tout,bin,cor);
  if (alpha>0)
    {
      if (tout<0) return(-1e10);
      alpha=0;
    }
  copytof=tof;
  tof=new float[event_hits->ntot()];
  maketof(alpha,t0,tout,copytof,bin,cor);
  ll=peakfit(addloglik(sub,NLIKE),vertex[3],dt);
  vertex[3]+=TBIN*sub;
  delete(tof);
  delete(bin);
  delete(cor);
  tof=copytof;
  if (ll>ll0) return(ll); else return(ll0);
}

// **********************************************
// fit the vertex time and direction
// **********************************************
float timefit::fittime_lcor(short int fast,
                            float *vertex,float *direct,float &dt)
{
  int      maxprod;
  double   cen[3],cang;
  float    ll0,ll,mucang,cdiff;
  axes     ax;
  centroid *centr;

  makedirtof(vertex);
  if (fast)
    ll0=fastaddloglik(vertex[3],dt,maxprod);
  else
    ll0=addloglik(vertex[3],dt,maxprod);
  fitdir(vertex,dt,centr,mucang);
  if (bgfit) ll0=makelike(vertex[3]);
  centr->centre(cen);
  centr->eigen(ax);
  maxprod=ax.align(*centr,direct);
  if (maxprod==0)
    direct[4]=fabs(centr->yy()-centr->zz())/(centr->yy()+centr->zz());
  else if (maxprod==1)
    direct[4]=fabs(centr->xx()-centr->zz())/(centr->xx()+centr->zz());
  else if (maxprod==2)
  direct[4]=fabs(centr->xx()-centr->yy())/(centr->xx()+centr->yy());
  delete(centr);
  cang=sqrt(cen[0]*cen[0]+cen[1]*cen[1]+cen[2]*cen[2]);
  if (cang>0)
    {
      cen[0]/=cang;
      cen[1]/=cang;
      cen[2]/=cang;
    }
  ll=addloglik(vertex,cen,cang,ll0);
  cdiff=cang-mucang;
  if (cdiff<0.05)
    direct[3]=mucang;
  else if (cdiff<0.25)
    direct[3]=mucang+cdiff*(cdiff-0.05)/.2;
  else
    direct[3]=cang;
  if (ll<ll0) return(ll0);
  if (bgfit) return(makelike(vertex[3]));
  return(ll);
}
// **********************************************
// fit the vertex time and direction
// **********************************************
float timefit::chargetest(float *vertex,float *direct,
			  float &qin,float &qoutplus,float *qoutminus)
{
  float dt,cang=direct[3],dx,dy,dz,sum;
  int   i;

  fittime_lcor(0,vertex,direct,dt);
  dz=cos(*direct);
  dx=sin(*direct);
  dy=dx*sin(direct[1]);
  dx*=cos(direct[1]);
  if (direct[3]<cang) direct[3]=cang;
  qin=qoutplus=*qoutminus=qoutminus[1]=sum=0;
  for(i=0; i<event_hits->ntot(); i++)
    {
      cang=dx*dir[3*i]+dy*dir[3*i+1]+dz*dir[3*i+2];
      if ((cang<direct[3]) && (tof[i]<vertex[3]))
	sum+=event_hits->hitcharge(i)*(0.67*(direct[3]-cang)+0.02*(vertex[3]-tof[i]));
      //if ((cang<direct[3]) && (tof[i]<vertex[3]))
      //printf("%f: %f %f %f\n",event_hits->hitcharge(i),direct[3]-cang,vertex[3]-tof[i],
      //       0.67*(direct[3]-cang)+0.02*(vertex[3]-tof[i]));
      if (cang<0.5*(1+direct[3]))
	{
	  if (cang>direct[3]-0.06)
	    qin+=event_hits->hitcharge(i);
	  else
	    if (tof[i]>vertex[3]-4)
	      qoutplus+=event_hits->hitcharge(i);
	    else if ((cang>direct[3]-0.09) || (tof[i]>vertex[3]-8))
	      *qoutminus+=event_hits->hitcharge(i);
	    else
	      qoutminus[1]+=event_hits->hitcharge(i);
	}
    }
  //printf("sum=%f %f\n",sum,qin+qoutplus+*qoutminus+qoutminus[1]);
  return(sum/(qin+qoutplus+*qoutminus+qoutminus[1]));
}

float timefit::maxconecharge(float *vert,float *direct)
{
  float dx,dy=sin(*direct),dz=cos(*direct),cang,max=-1e10;
  int   i;

  dx=dy*cos(direct[1]);
  dy*=sin(direct[1]);
  makedirtof(vert);
  for(i=0; i<event_hits->ntot(); i++)
    {
      cang=dx*dir[3*i]+dy*dir[3*i+1]+dz*dir[3*i+2];
      if (cang>direct[3])
	if (event_hits->hitcharge(i)>max)
	  max=event_hits->hitcharge(i);
    }
  return(max);
}

// **********************************************
// find the vertex time, and fit signal
// **********************************************
float timefit::fittime(float *vertex,float &dt,float &signal)
{
  int   set,offs,ind,nbg,shift,i;
  float tofmin,tofmax,range;
  float bg,llmax=0,time;
  float sear[3],lsear[3],fac;

  maketof(vertex);
  addloglik(vertex[3],dt,ind);
  if (tof==NULL) return(-1e10);
  if (like!=NULL) delete(like);
  like=new float[event_hits->ntot()];
  tofminmax(tofmin,tofmax);
  tofmin-=vertex[3];
  tofmax-=vertex[3];
  float *norm_fact = new float [nlike], *time_range = new float[nlike];
  sigrange(tofmin,tofmax,norm_fact,time_range);
  range=tofmax-tofmin;
  signal=0;
  for(set=offs=ind=0; set<(int)nlike; offs=offset[set++])
    {
      for(nbg=0; ind<event_hits->beyond_last_hit(set); ind++)
	{
	  time=tof[ind]-vertex[3];
	  if (time>0)
	    shift=offs+nneg[set]-((int) (0.5+time*BINPERNS));
	  else
	    shift=offs+nneg[set]+((int) (0.5-time*BINPERNS));
	  if ((shift<offs) || (shift>=(int)offset[set]))
	    {
	      like[ind]=0.0;
	      nbg++;
	    }
	  else
	    like[ind]=norm_fact[set]*pdf[shift];
	}
      sig[set]=event_hits->nhits(set)-nbg*range/(range-time_range[set]);
      if (sig[set]<0) sig[set]=0;
      fac=1.03;
      sear[0]=sig[set]/fac;
      sear[1]=sig[set];
      sear[2]=sig[set]*fac;
      for(i=0; i<3; i++)
	{
	  sig[set]=sear[i];
	  lsear[i]=makelike(set,bg,range);
	}
      while (fac>1.0001)
	{
	  while (*lsear>lsear[1])
	    {
	      sear[2]=sear[1];
	      lsear[2]=lsear[1];
	      sear[1]=*sear;
	      lsear[1]=*lsear;
	      *sear=sear[1]/fac;
	      sig[set]=*sear; *lsear=makelike(set,bg,range);
	    }
	  while (lsear[2]>lsear[1])
	    {
	      *sear=sear[1];
	      *lsear=lsear[1];
	      sear[1]=sear[2];
	      lsear[1]=lsear[2];
	      sear[2]=sear[1]*fac;
	      sig[set]=sear[2]; lsear[2]=makelike(set,bg,range);
	    }
	  fac=1+0.5*(fac-1);
	  *sear=sear[1]/fac;
	  sear[2]=sear[1]*fac;
	  sig[set]=*sear;   *lsear  =makelike(set,bg,range);
	  sig[set]=sear[2]; lsear[2]=makelike(set,bg,range);
	}
      sig[set]=sear[1];
      signal+=sig[set];
      llmax+=lsear[1];
    }
  delete [] norm_fact;
  delete [] time_range;
  return(llmax);
}

// **********************************************
// test the shape of the timing distributions
// **********************************************
float timefit::goodness(float &ll,float *vertex,float *gdnset)
{
  float gdn,ttof,dt,denom,denall;
  int   set,offs,ind,shift,shift2;

  maketof(vertex);
  if (tof==NULL) return(-1e10);
  ll=addloglik(vertex[3],dt,ind);
  if (bgfit)
    {
      ll=makelike(vertex[3]);
      if (like==NULL) return(-1e10);
    }
  for(denall=gdn=0,set=offs=ind=0; set<(int)nlike; offs=offset[set++])
    {
      if (bgfit) denom=1; else denom=lmax[set];
      for(gdnset[set]=0; ind<event_hits->beyond_last_hit(set); ind++)
	if (bgfit)
	  gdnset[set]+=1+0.25*log10(like[ind]);
	else
          {
	    ttof=(tof[ind]-vertex[3])*BINPERNS;
	    if (ttof>0)
	      { shift=-(int)(0.5+ttof);shift2=-(int)(0.5+5*ttof)-5*shift; }
	    else
	      { shift=+(int)(0.5-ttof);shift2=+(int)(0.5-5*ttof)-5*shift; }
	    if (shift2<0)
	      {
		shift--;
		shift2+=5;
	      }
	    shift+=offs+nneg[set];
	    if ((shift>=offs) && (shift<(int)offset[set]))
	      gdnset[set]+=loglike[shift+shift2*offset[nlike-1]];
	  }
      if (event_hits->nhits(set)>0)
        {
          gdnset[set]/=(float)event_hits->nhits(set);
	  denall+=denom;
        }
      gdn+=gdnset[set];
      gdnset[set]/=denom;
    }
  return(gdn/denall);
}

int timefit::nwind(float *vertex,float tmin,float tmax)
{
  int   ind,nw;
  float dt;

  maketof(vertex);
  addloglik(vertex[3],dt,ind);
  for(nw=ind=0; ind<event_hits->ntot(); ind++)
    {
      dt=tof[ind]-vertex[3];
      if ((dt>=tmin) && (dt<=tmax))
	nw++;
    }
  return(nw);
}

int timefit::nwind(float *vertex,float tmin,float tmax,float *ttof,int *cab)
{
  int   ind,nw;
  float dt;

  maketof(vertex);
  addloglik(vertex[3],dt,ind);
  for(nw=ind=0; ind<event_hits->ntot(); ind++)
    {
      dt=tof[ind]-vertex[3];
      if ((dt>=tmin) && (dt<=tmax))
	{
	  ttof[nw]=dt;
	  cab[nw++]=event_hits->hitcable(ind);
	}
    }
  return(nw);
}

int timefit::nwind(float *vertex,float tmin,
		   float tmax,float *ttof,float *d,int *cab)
{
  int   ind,nw;
  float dt;

  makedirtof(vertex);
  addloglik(vertex[3],dt,ind);
  for(nw=ind=0; ind<event_hits->ntot(); ind++)
    {
      dt=tof[ind]-vertex[3];
      if ((dt>=tmin) && (dt<=tmax))
	{
	  ttof[nw]=dt;
	  d[3*nw]=dir[3*ind];
	  d[3*nw+1]=dir[3*ind+1];
	  d[3*nw+2]=dir[3*ind+2];
	  cab[nw++]=event_hits->hitcable(ind);
	}
    }
  return(nw);
}

float timefit::tgood(float *vertex,float bgrate,float &guncor)
{
  int   ind;
  float g,w,dt,count;

  maketof(vertex);
  addloglik(vertex[3],dt,ind);
  for(g=count=ind=0; ind<event_hits->ntot(); ind++)
    {
      dt=tof[ind]-vertex[3];
      dt*=0.04*dt;
      w=0.0035*dt;
      if (w<=18) w=exp(-w); else continue;
      count+=w;
      dt*=0.5;
      if (dt<=50) g+=w*exp(-dt);
    }
  if (count==0) guncor=0; else guncor=g/count;
  g-=12.533141*bgrate;
  count-=149.7997*bgrate;
  if (count<=0) return(0); else return(g/count);
}

// **********************************************
// test the shape of the timing distributions
// **********************************************
float timefit::shapetest(void)
{
  /*  int   i,*histo,nhist,binmin,binmax;
  float tofmin,tofmax,bg=event_hits->ntot()-sig,dev,d;

  if (tof==NULL) return(1e10);
  tofmin=-*tof;
  tofmax=tofmin;
  for(i=1; i<event_hits->ntot(); i++)
    {
      if (-tof[i]>tofmax)
	tofmax=-tof[i];
      if (-tof[i]<tofmin)
	tofmin=-tof[i];
    }
  if (tofmin<0)
    binmin=-(int) (0.5-tofmin*BINPERNS);
  else
    binmin=(int) (0.5+tofmin*BINPERNS);
  if (tofmax<0)
    binmax=-(int) (0.5-tofmax*BINPERNS);
  else
    binmax=(int) (0.5+tofmax*BINPERNS);
  tofmin=TBIN*binmin;
  tofmax=TBIN*binmax;
  nhist=binmax-binmin+1;
  histo=new int[nhist+1];
  for(i=0; i<nhist; i++) histo[i]=0;
  for(i=0; i<event_hits->ntot(); i++)
    histo[(int)(0.5-(tof[i]+tofmin)*BINPERNS)]++;
  for(i=1; i<nhist; i++) histo[i]+=histo[i-1];
  dev=-1e10;
  for(i=0; (i<nhist) && (NNEG+i+binmin<1); i++)
    if ((d=fabs((i+0.5)*bg/nhist-histo[i]))>dev) dev=d;
  for(; (i<nhist) && (NNEG+i+binmin-1<NDIST); i++)
    if ((d=fabs(sig*pdf_sum[NNEG+i+binmin-1]/pdf_sum[NDIST-1]+
		(i+0.5)*bg/nhist-histo[i]))>dev) dev=d;
  for(; i<nhist; i++)
    if ((d=fabs(sig+(i+0.5)*bg/nhist-histo[i]))>dev) dev=d;
  delete histo;
  dev/=sqrt((float)event_hits->ntot());
  return(dev);*/
  return(-1);
}

}
