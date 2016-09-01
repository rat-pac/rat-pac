#ifndef HITSEL
#define HITSEL
#include "RAT/BONSAI/hits.h"

namespace BONSAI {
    
    // **********************************************
    // does  hit selection, when needed
    // **********************************************
    class hitsel: public hits
    {
        short int *selected;
        int       nsel;
        
        short int make_causal_table(short int *&related,short int *&relations,
                                    float twin,float resolution,float tcoincidence);
        short int reduce(int size,short int *source,
                         short int *destination,
                         short int *occurence);
        void select(float dlim,float tlim,
                    float twin,float resolution,float tcoincidence);
        
    public:
        inline hitsel(float dlim,float tlim,
                      float twin,float resolution,float tcoincidence,
                      int ns,float *set,float *pmt_loc,int *bad_ch,
                      int n_raw,int *cable,float *tim_raw,float *chg_raw);
        inline hitsel(float dlim,float tlim,
                      float twin,float resolution,float tcoincidence,
                      int ns,float *set,pmt_geometry *geom,
                      int n_raw,int *cable,float *tim_raw,float *chg_raw);
        inline hitsel(float dlim,float tlim,
                      float twin,float resolution,float tcoincidence,
                      int ns,float *set,pmt_geometry *geom,comtype2 *itevent);
        inline hitsel(int ns,float *set,pmt_geometry *geom,comtype2 *itevent);
        ~hitsel(void);
        inline int nselected(void);
        inline short int sel(int i);
        inline float tpeak(void);
        inline int clean(int i,int j,float d2lim,float tlim);
        inline int causal(int i,int j,float twin,float resolution,float tcoincidence);
        inline int vertex4(int *hits,double *vert);
        inline void eigen(double *ctrd,float *axes);
        void pmtcentroid(int n,float *ctrd,float *axes,float rmax,float zmax);
        
        short int mrclean(float dlim,float tlim);
        short int clus_sel(float dlim,float tlim,
                           float twin,float resolution,float tcoincidence);
        inline void qsort(void);
        void printset(int nset,short int *set);
    };
    
    // **********************************************
    // initialize according to base class hits;
    // then create hit selection array
    // **********************************************
    inline hitsel::hitsel(float dlim,float tlim,
                          float twin,float resolution,float tcoincidence,
                          int ns,float *set,float *pmt_loc,int *bad_ch,
                          int n_raw,int *cable,float *tim_raw,float *chg_raw):
    hits(ns,set,pmt_loc,bad_ch,n_raw,cable,tim_raw,chg_raw)
    {
        select(dlim,tlim,twin,resolution,tcoincidence);
    }
    
    inline hitsel::hitsel(float dlim,float tlim,
                          float twin,float resolution,float tcoincidence,
                          int ns,float *set,pmt_geometry *geom,
                          int n_raw,int *cable,float *tim_raw,float *chg_raw):
    hits(ns,set,geom,n_raw,cable,tim_raw,chg_raw)
    {
        select(dlim,tlim,twin,resolution,tcoincidence);
    }
    
    inline hitsel::hitsel(float dlim,float tlim,
                          float twin,float resolution,float tcoincidence,
                          int ns,float *set,pmt_geometry *geom,comtype2 *itevent):
    hits(ns,set,geom,itevent)
    {
        int   sel;
        float sel_cable;
        
        select(dlim,tlim,twin,resolution,tcoincidence);
        for(sel=0; sel<nsel; sel++)
        {
            sel_cable=(float) hitcable(selected[sel]);
            for(ns=0; ns<itevent->it_index; ns++)
                if (sel_cable==itevent->hits[ns][0])
                {
                    itevent->hits[ns][0]=-sel_cable;
                    break;
                }
        }
    }
    
    inline hitsel::hitsel(int ns,float *set,pmt_geometry *geom,comtype2 *itevent):
    hits(ns,set,geom,itevent)
    {
        int hit;
        
        for(nsel=hit=0; hit<itevent->it_index; hit++)
            if (itevent->hits[hit][0]<0) nsel++;
        if (nsel==0)
            selected=new short int[1];
        else
            selected=new short int[nsel];
        for(nsel=hit=0; hit<itevent->it_index; hit++)
            if (itevent->hits[hit][0]<0)
                if (position(hit)>=0)
                    selected[nsel++]=position(hit);
        hits::qsort(selected,nsel);
    }
    
    // **********************************************
    // return number of selected hits
    // **********************************************
    inline int hitsel::nselected(void)
    {
        return(nsel);
    }
    // **********************************************
    // return number of selected hits
    // **********************************************
    inline short int hitsel::sel(int i)
    {
        if ((i>=0) && (i<nsel))
            return(selected[i]);
        return(-1);
    }
    // **********************************************
    // does simple peak finding of absolute timing
    // **********************************************
    inline float hitsel::tpeak(void)
    {
        float t0=time_av(-1e10,1e10);
        
        t0=time_av(10,t0+220);
        t0=time_av(t0-70,t0+70);
        t0=time_av(t0-40,t0+70);
        t0=time_av(t0-40,t0+70);
        return(t0);
    }
    // **********************************************
    // returns, if two hits are sepearted in space and time
    // **********************************************
    inline int hitsel::clean(int i,int j,float d2lim,float tlim)
    {
        if (i==j) return(0);
        if (invalid_hit(i) || invalid_hit(j)) return(-3);
        return((tdiff(i,j)<tlim) && (dist2(i,j)<d2lim));
    }
    // **********************************************
    // returns, if two hits could originate from a common vertex
    // **********************************************
    inline int hitsel::causal(int i,int j,
                              float twin,float resolution,float tcoincidence)
    {
        if (i==j) return(0);
        
        float delta_t=tdiff(i,j)-resolution;
        
        if (delta_t<tcoincidence) return(1);
        if (delta_t>=twin) return(0);
        return(delta_t*delta_t<dist2(i,j)*CM_TO_NS2);
    }
    // **********************************************
    // does vertex for combination of four selected hits
    // **********************************************
    inline int hitsel::vertex4(int *hits,double *vert)
    {
        int sel[4];
        
        *sel=selected[*hits];     sel[1]=selected[hits[1]];
        sel[2]=selected[hits[2]]; sel[3]=selected[hits[3]];
        return(hits::vertex4(sel,vert));
    }
    
    inline void hitsel::qsort(void)
    {
        short int *list=new short int[ntot()],hit,*sselected;
        
        hits::qsort(list);
        sselected=selected;
        selected=new short int[nsel];
        for(hit=0; hit<nsel; hit++)
            selected[hit]=list[sselected[hit]];
        delete(sselected);
        delete(list);
    }
    
}
#endif
