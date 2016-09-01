#ifndef TIMEFIT
#define TIMEFIT
#include "RAT/BONSAI/hitsel.h"
#include "RAT/BONSAI/centroid.h"
// **********************************************
// fits the vertex time, assuming a given vertex
// calculates the likelihood for the timing dist.
// **********************************************

namespace BONSAI {
    
    class timefit
    {
        short    int bgfit;
        unsigned int nlike,*offset,*nneg,*lmax;
        unsigned int *loglike,*time_histo;
        float        *qminmax,*pdf,*pdf_sum,*pdf_max;
        int          nbin,s0,sx4,sx2,det;
        float        *tof,*dir,*like,*sig;
        
        void  delete_dist(void);
        void  create_dist(void);
        void  integrate_dist(void);
        void  load_dist(void);
        void  maketof(float *vertex);
        void  makedirtof(float *vertex);
        inline float histo_t0(void);
        inline int   addloglik(int sub,int ndist);
        inline float addloglik(float &tpeak,float &dt,int &max);
        float fastaddloglik(float &tpeak,float &dt,int &max);
        float addloglik(float &t0,float &dt,int &max,float tmi,float tpl);
        inline float maxloglike(int max);
        float peakfit(int max,float &tpeak,float &dt);
        inline void  tofminmax(float &tofmin,float &tofmax);
        inline void  sigrange(float tofmin,float tofmax,float *norm_fact,float *time_range);
        float makelike(float t0);
        float makesiglike(float t0,float *bg);
        inline float makelike(int set,float &bg,float range);
        inline void  fitdir(float *vertex,float &dt,
                            centroid *&centr,float &mucang);
        inline void  corfunc(int *&bin,float *&cor,double *direct,double cang);
        inline void  lcor(float &alpha,float &t0,float &tout,int *bin,float *cor);
        inline void  maketof(float alpha,float t0,float tout,float *origtof,int *bin,float *cor);
        inline float addloglik(float *vertex,double *direct,double cang,float ll0);
        
    protected:
        hitsel       *event_hits;
        
    public:
        timefit(void);
        timefit(hitsel *hs);
        virtual ~timefit();
        
        inline float fittime(short int fast,float *vertex,float &dt0);
        void  fittime(int ndt,float *dts,float &t0,float &dt);
        float fittime(short int fast,float *vertex,float *direct,float &dt);
        float fittime(float *vertex,float *direct,float &dt,float tmi,float tpl);
        float fittime_lcor(short int fast,float *vertex,float *direct,float &dt);
        float chargetest(float *vertex,float *direct,
                         float &qin,float &qoutplus,float *qoutminus);
        float maxconecharge(float *vert,float *direct);
        float fittime(float *vertex,float &dt0,float &signal);
        float goodness(float &ll,float *vertex,float *gdnset);
        int   nwind(float *vertex,float tmin,float tmax);
        int   nwind(float *vertex,float tmin,float tmax,float *ttof,int *cab);
        int   nwind(float *vertex,float tmin,float tmax,
                    float *ttof,float *d,int *cab);
        float tgood(float *vertex,float bgrate,float &guncor);
        float shapetest(void);
        inline int sets(void);
        inline int gethits(short int fast,float *vertex,
                           int *cabs,float *tofs,float *dirs,float *weight);
        inline void getdist(int set,float t0,int nbin,
                            float tmin,float tmax,float *cont);
        inline float *chargebins(void);
    };
    
#define NTIME 4500   /* # bins for t0 search */
#define TBIN     0.4 /* time bin size */
#define BINPERNS 2.5 /* 1/TBIN */
    
    
    // **********************************************
    // sum up the log(likelihood) for all hits
    // **********************************************
    inline float timefit::addloglik(float &t0,float &dt,int &max)
    {
        max=addloglik(0,NTIME);
        return(peakfit(max,t0,dt));
    }
    
    // **********************************************
    // sum up the log(likelihood) for all hits,
    // given offset sub and array size ndist
    // **********************************************
    inline int timefit::addloglik(int sub,int ndist)
    {
        int           set,offs,i,j,shift,shift2,number,max;
        unsigned int *lp,*distp,*start;
        float        *timep,time;
        
        for(lp=time_histo,i=0; i<ndist; i++) *(lp++)=0;
        for(offs=set=0,timep=tof; set<(int)nlike; offs=offset[set++])
            for(i=0; i<event_hits->nhits(set); i++)
            {
                time=*(timep++)*BINPERNS;
                if (time>0)
                {
                    shift=(int) (time+0.5); shift2=5*shift-(int) (5*time+0.5);
                    if (shift2<0)
                    {
                        shift++;
                        shift2+=5;
                    }
                    distp=loglike+offs+shift2*offset[nlike-1];
                    shift-=sub+nneg[set];
                    number=ndist-shift;
                    max=offset[set]-offs;
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
        for(max=0,i=1; i<ndist; i++)
            if (time_histo[i]>time_histo[max]) max=i;
        return(max);
    }
    
    // **********************************************
    // fit the vertex time
    // **********************************************
    inline float timefit::fittime(short int fast,float *vertex,float &dt)
    {
        int ind;
        
        maketof(vertex);
        if (!bgfit)
        {
            if (fast)
                fastaddloglik(vertex[3],dt,ind);
            else
                addloglik(vertex[3],dt,ind);
            return(4*(time_histo[ind]*1E-5-event_hits->ntot()));
        }
        if (fast)
            fastaddloglik(vertex[3],dt,ind);
        else
            addloglik(vertex[3],dt,ind);
        return(makelike(vertex[3]));
    }
    inline int timefit::sets(void)
    {
        return(nlike);
    }
    inline float *timefit::chargebins(void)
    {
        return(qminmax);
    }
    
    // **********************************************
    // get cable number, time residual, direction and
    // direction fit weight of each hit
    // **********************************************
    inline int timefit::gethits(short int fast,float *vertex,
                                int *cabs,float *tofs,float *dirs,float *weight)
    {
        int    set,offs,ind,bin,bin2;
        float   time,dt;
        
        makedirtof(vertex);
        if (fast)
            fastaddloglik(vertex[3],dt,ind);
        else
            addloglik(vertex[3],dt,ind);
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
        for(ind=0; ind<event_hits->ntot(); ind++)
        {
            cabs[ind]=event_hits->hitcable(ind);
            tofs[ind]=tof[ind]-vertex[3];
        }
        for(ind=0; ind<3*event_hits->ntot(); ind++)
            dirs[ind]=dir[ind];
        return(event_hits->ntot());
    }
    
    // **********************************************
    // get a binned distribution of a likelihood function
    // **********************************************
    inline void timefit::getdist(int set,float t0,int nbin,
                                 float tmin,float tmax,float *cont)
    {
        float t,time,dt;
        int   offs,shift,shift2;
        
        if (set<0) return;
        if (set>=(int) nlike) return;
        if (set>1) offs=offset[set-1]; else offs=0;
        dt=(tmax-tmin)/nbin;
        for(t=tmin+0.5*dt; t<tmax; t+=dt)
        {
            time=(t-t0)*BINPERNS;
            if (time>0)
            { shift=-(int)(0.5+time);shift2=-(int)(0.5+5*time)-5*shift; }
            else
            { shift=+(int)(0.5-time);shift2=+(int)(0.5-5*time)-5*shift; }
            if (shift2<0)
            {
                shift--;
                shift2+=5;
            }
            shift+=offs+nneg[set];
            if ((shift>=offs) && (shift<(int)offset[set]))
                *cont++=pdf[shift+shift2*offset[nlike-1]];
            else
                *cont++=0;
        }
    }
    
}
#endif
