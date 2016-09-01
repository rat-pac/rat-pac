#ifndef VERTEX
#define VERTEX
#include "RAT/BONSAI/timefit.h"
#include "RAT/BONSAI/plato.h"

namespace BONSAI {
    
    // **********************************************
    // manage an array of tested vertices
    // **********************************************
    class vertex: public timefit
    {
        float        *vert,*bestfit,*worstfit;
        float        rpmt,zpmt,r2cut,zcut,cang0,plusdang,minusdang;
        float        dislimit,timlimit,twin,tres,tclose;
        int          nvert,maxvert;
        dodecahedron plat;
        
        inline int closest_dist2(float *v,float lim);
        inline void set(float x,float y,float z,float r);
        inline void set(float x,float y,float z,float t,float r);
        inline void set(float ll);
        inline float test(short int fast);
        inline float test(float tmi,float tpl);
        
    protected:
        float  timefactor;
        void loadhits(int n_raw,int *cable,int *bad_ch,
                      float *pmt_loc,float *tim_raw,float *chg_raw);
        inline void set_cylinder(float rp,float zp,float res);
        inline void set_hitsel(float dlim,float tlim,float tw,float tr,float tc);
        inline void checksize(int newsize);
        inline void addsize(int add);
        inline void new_vertex(short int fast,float x,float y,float z,float r);
        inline void new_vertex(float x,float y,float z,float t,float tmi,float tpl,float r);
        inline void add_pmt_vertices(short int fast,float rpmt,float rcent,float zmax);
        inline void reset(void);
        
    public:
        vertex(void);
        vertex(int n_raw,int *cable,int *bad_ch,float *pmt_loc,
               float *tim_raw,float *chg_raw);
        ~vertex(void);
        
        inline void set_cang(float cang,float pang,float mang);
        inline int toobad(int index);
        inline int nsel();
        inline int ismax(int index);
        int surround(int index,int *&newi);
        float bestwall(float dwall,float rmin,float *wallv);
        
        inline void print(int index);
        inline int bestid(void);
        inline void bestvertex(float *v);
        inline void getvertex(float *v,int id);
        inline float bestradius();
        inline float maxll(void);
        inline float maxll(int id);
        inline float depth(void);
        inline int ntest(void);
        inline int max(void);
        inline int inside_cylinder(float x,float y,float z);
        inline void addvertex(float *v,float rad);
        void purge(void);
    };
    
#define NVERTFLOAT 11 // size of a test vertex entry
    
#define STORERAD   1  // place to store radius of vertex
#define STOREX     2  // place to store x,y,z and time of vertex
#define STOREY     3  // must be in sequence! (x,y,z,t)
#define STOREZ     4
#define STORET     5
#define STORETHETA 6  // place to store direction of vertex
#define STOREPHI   7  // must be in sequence
#define STOREALPHA 8  // (theta,phi,alpha,opening angle,
#define STORECANG  9  //  ellipticity)
#define STOREELL  10
    
    // private
    
    // **********************************************
    // sets the position of a new vertes
    // **********************************************
    inline void vertex::set(float x,float y,float z,float r)
    {
        vert[nvert+STORERAD]=r;
        vert[nvert+STOREX]=x;
        vert[nvert+STOREY]=y;
        vert[nvert+STOREZ]=z;
    }
    // **********************************************
    // sets the position of a new vertes
    // **********************************************
    inline void vertex::set(float x,float y,float z,float t,float r)
    {
        vert[nvert+STORERAD]=r;
        vert[nvert+STOREX]=x;
        vert[nvert+STOREY]=y;
        vert[nvert+STOREZ]=z;
        vert[nvert+STORET]=t;
    }
    // **********************************************
    // sets a new likelihood and compares with best
    // and worst fit
    // **********************************************
    inline void vertex::set(float ll)
    {
        if ((vert[nvert]=ll)>*bestfit)
            bestfit=vert+nvert;
        if (vert[nvert]<*worstfit)
            worstfit=vert+nvert;
        nvert+=NVERTFLOAT;
    }
    
    // **********************************************
    // gets likelihood and direction from a new test vertex
    // **********************************************
    inline float vertex::test(short int fast)
    {
        float dt,ret,dev;
        
        if (cang0>0)
        {
            ret=fittime(fast,vert+nvert+STOREX,
                        vert+nvert+STORETHETA,dt);
            dev=vert[nvert+STORECANG]-cang0;
        }
        else
        {
            ret=fittime_lcor(fast,vert+nvert+STOREX,
                             vert+nvert+STORETHETA,dt);
            dev=vert[nvert+STORECANG]+cang0;
        }
        if (dev>0)
        {
            if (plusdang>0)
                ret-=dev*dev*plusdang;
        }
        else
        {
            if (minusdang>0)
                ret-=dev*dev*minusdang;
        }
        return(ret);
    }
    
    // **********************************************
    // gets likelihood and direction from a new test vertex
    // **********************************************
    inline float vertex::test(float tmi,float tpl)
    {
        float dt,ret,dev;
        
        if (cang0>0)
        {
            ret=fittime(vert+nvert+STOREX,
                        vert+nvert+STORETHETA,dt,tmi,tpl);
            dev=vert[nvert+STORECANG]-cang0;
        }
        else
        {
            ret=fittime_lcor(1,vert+nvert+STOREX,
                             vert+nvert+STORETHETA,dt);
            dev=vert[nvert+STORECANG]+cang0;
        }
        if (dev>0)
        {
            if (plusdang>0)
                ret-=dev*dev*plusdang;
        }
        else
        {
            if (minusdang>0)
                ret-=dev*dev*minusdang;
        }
        return(ret);
    }
    
    // protected
    
    // **********************************************
    // define allowed cylinder for fit vertices
    // **********************************************
    inline void vertex::set_cylinder(float rp,float zp,float res)
    {
        rpmt=rp;
        zpmt=zp;
        r2cut=rp+res;
        r2cut*=r2cut;
        zcut=zp+res;
    }
    
    // **********************************************
    // define hit selection parameter
    // **********************************************
    inline void vertex::set_hitsel(float dlim,float tlim,
                                   float tw,float tr,float tc)
    {
        dislimit=dlim; timlimit=tlim;
        twin=tw; tres=tr; tclose=tc;
    }
    
    
    // **********************************************
    // checks the memory size and increases it,
    // if necessary
    // **********************************************
    inline void vertex::checksize(int newsize)
    {
        if (maxvert<NVERTFLOAT*newsize)
        {
            if (maxvert>0) delete vert;
            vert=new float[NVERTFLOAT*newsize];
            maxvert=NVERTFLOAT*newsize;
        }
        nvert=0;
        bestfit=worstfit=vert;
    }
    
    // **********************************************
    // increases the memory size
    // **********************************************
    inline void vertex::addsize(int add)
    {
        printf("Allocate space for an extra %d test vertices: %d\n",
               add,maxvert/NVERTFLOAT+add);
        if (vert==NULL)
        {
            vert=new float[NVERTFLOAT*add];
            maxvert=NVERTFLOAT*add;
        }
        else
        {
            float *new_vert=new float[maxvert+NVERTFLOAT*add];
            int i;
            
            for(i=0; i<nvert; i++)
                new_vert[i]=vert[i];
            bestfit+=new_vert-vert;
            worstfit+=new_vert-vert;
            delete vert;
            vert=new_vert;
            maxvert+=NVERTFLOAT*add;
        }
    }
    
    
    // **********************************************
    // adds a new vertex position and tests it
    // **********************************************
    inline void vertex::new_vertex(short int fast,float x,float y,float z,float r)
    {
        if (!inside_cylinder(x,y,z)) return;
        set(x,y,z,r);
        set(test(fast));
    }
    
    // **********************************************
    // adds a new vertex position and tests it
    // **********************************************
    inline void vertex::new_vertex(float x,float y,float z,float t,float tmi,float tpl,float r)
    {
        float t1,t2,l2;
        if (!inside_cylinder(x,y,z)) return;
        set(x,y,z,t,r);
        set(test(tmi,tpl));
        return;
        set(x,y,z,r);
        test(1);
        t1=vert[nvert+STORET];
        set(x,y,z,t,r);
        //set(test(tmi,tpl));
        l2=test(tmi,tpl);
        t2=vert[nvert+STORET];
        printf("%8.2f %8.2f %8.2f %10.5f %10.5f %10.5f\n",x,y,z,t,t1,t2);
        set(l2);
    }
    
    // **********************************************
    // adds test vertices in front of each selected PMT
    // **********************************************
    inline void vertex::add_pmt_vertices(short int fast,float rpmt,float rcent,float zmax)
    {
        if (nvert>=NVERTFLOAT)
        {
            int ind;
            
            for(ind=0; ind<nsel(); ind++)
            {
                vert[nvert+STORERAD]=rpmt;
                event_hits->frontof(vert+nvert+STOREX,event_hits->sel(ind),zmax);
                set(test(fast));
            }
        }
        else
        {
            nvert=0;
            vert[STOREX]=vert[STOREY]=vert[STOREZ]=0;
            *vert=test(fast);
            vert[STORERAD]=rcent;
            bestfit=worstfit=vert;
            nvert=NVERTFLOAT;
        }
    }
    
    // **********************************************
    // resets the number of tested vertices to zero
    // **********************************************
    inline void vertex::reset(void)
    {
        nvert=0;
    }
    
    
    // public
    
    // **********************************************
    // define Cherenkov angle constraint
    // **********************************************
    inline void vertex::set_cang(float cang,float pang,float mang)
    {
        cang0=cang;
        if (pang<0)
            plusdang=pang;
        else
            plusdang=0.5/(pang*pang);
        if (mang<0)
            minusdang=mang;
        else
            minusdang=0.5/(mang*mang);
        printf("Cherenkov Constraint: %f %f %f\n",cang0,plusdang,minusdang);
    }
    
    // **********************************************
    // returns, if likelihood is acceptable
    // **********************************************
    inline int vertex::toobad(int index)
    {
        if (nvert>10000*NVERTFLOAT) timefactor=0;
        return(vert[index]<*bestfit-timefactor);
    }
    
    // **********************************************
    // returns the number of selected PMTs
    // **********************************************
    inline int vertex::nsel()
    {
        return(event_hits->nselected());
    }
    
    
    // **********************************************
    // prints the vertex of ID number index
    // **********************************************
    inline void vertex::print(int index)
    {
        printf("%12.6f for (%8.2f %8.2f %8.2f %8.2f) with %6.1f\n",
               vert[index],
               vert[index+STOREX],vert[index+STOREY],
               vert[index+STOREZ],vert[index+STORET],
               vert[index+STORERAD]);
    }
    
    // **********************************************
    // checks, if ID index is the best fit point
    // **********************************************
    inline int vertex::ismax(int index)
    {
        return(bestfit==vert+index);
    }
    
    // **********************************************
    // returns the ID number of the best fit point
    // **********************************************
    inline int vertex::bestid(void)
    {
        return(bestfit-vert);
    }
    
    // **********************************************
    // returns the vertex of the best fit point
    // **********************************************
    inline void vertex::bestvertex(float *v)
    {
        if (bestfit==NULL) return;
        *v=bestfit[STOREX];
        v[1]=bestfit[STOREY];
        v[2]=bestfit[STOREZ];
        v[3]=bestfit[STORET];
    }
    // **********************************************
    // returns the vertex of a point
    // **********************************************
    inline void vertex::getvertex(float *v,int id)
    {
        int base=id*NVERTFLOAT;
        
        if (base>=nvert) return;
        *v=vert[base+STOREX];
        v[1]=vert[base+STOREY];
        v[2]=vert[base+STOREZ];
        v[3]=vert[base+STORET];
    }
    
    // **********************************************
    // returns the radius of the best fit point
    // **********************************************
    inline float vertex::bestradius()
    {
        return(bestfit[STORERAD]);
    }
    
    // **********************************************
    // returns the best likelihood
    // **********************************************
    inline float vertex::maxll(void)
    {
        return(*bestfit);
    }
    inline float vertex::maxll(int id)
    {
        int base=id*NVERTFLOAT;
        if (base>=nvert) return(-1e10);
        
        return(vert[base]);
    }
    
    // **********************************************
    // returns the difference in log(likelihood)
    // between best and worst fit
    // **********************************************
    inline float vertex::depth(void)
    {
        return(*bestfit-*worstfit);
    }
    
    // **********************************************
    // returns the number of tested vertices
    // **********************************************
    inline int vertex::ntest(void)
    {
        return(nvert/NVERTFLOAT);
    }
    
    // **********************************************
    // returns the number of tested vertices
    // **********************************************
    inline int vertex::max(void)
    {
        return(maxvert/NVERTFLOAT);
    }
    
    // **********************************************
    // returns if a point is inside a cylinder
    // **********************************************
    inline int vertex::inside_cylinder(float x,float y,float z)
    {
        return((fabs(z)<=zcut) && (x*x+y*y<=r2cut));
    }
    
    inline void vertex::addvertex(float *v,float rad)
    {
        if (!inside_cylinder(v[0],v[1],v[2])) return;
        if (nvert+NVERTFLOAT>=maxvert)
        {
            int add=maxvert/(500*NVERTFLOAT);
            
            if (add<1) add=500; else add*=500;
            addsize(add);
        }
        set(v[0],v[1],v[2],rad);
        vert[nvert]=test(1);
        if (nvert==0)
        {
            bestfit=worstfit=vert;
            nvert+=NVERTFLOAT;
            return;
        }
        if (vert[nvert]>*bestfit) bestfit=vert+nvert;
        if (vert[nvert]<*worstfit) worstfit=vert+nvert;
        return;
    }
}   
#endif
