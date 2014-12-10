#ifndef HITS
#define HITS
#include <stdio.h>
#include <math.h>
//#define NS_TO_CM   21.58333 /* speed of light (group speed) */
//#define NS_TO_CM    21.98
#define NS_TO_CM      21.77

#define CM_TO_NS   (1./NS_TO_CM)
#define CM_TO_NS2 1./(NS_TO_CM*NS_TO_CM)
#include "RAT/BONSAI/pmt_geometry.h"

namespace BONSAI {
    
    typedef struct{
        float hits[20000][3];  /*RECALL:C array's 0-n, f77 1-n  */
        int   it_index;
    } comtype2;
    
    // **********************************************
    // Stores hit times, cable numbers and positions
    // calculates time-of-flights and distances
    // **********************************************
    
    class hits
    {
        int       nchargeset,*nhit;       //# of charge sets, # of hits for each set
        short int *places;                // array position of nth hit
        int   *cables;                    // cable #'s, times, charges, and
        float *times,*charges,*positions; // locations for each hit
        float *avcharges;                 // average charge for each charge set
        
        inline double absrat(double el);
        inline double findmin(double *mat,int *rowind,int *colind,int pass);
        inline void force_zero(double *mat,int *rowind,int *colind,int pass,int ind);
        int  inv35(double *mat,int *rowind,int *colind);
        void qsort(short int *list,short int n,short int *first);
        inline void create_arrays(int n_raw,int n_hit);
        
    protected:
        inline short int position(int i);
        inline int hits_in_set(int set,int i);
        inline int invalid_hit(int i);
        inline void swap(int i,int j);
        inline float tdiff(int i,int j);
        inline float dist2(int i,int j);
        
    public:
        hits(int ns,float *set,float *pmt_loc,int *bad_ch,
             int n_raw,int *cable,float *tim_raw,float *chg_raw);
        hits(int ns,float *set,pmt_geometry *geom,int nh,int *cab,float *t,float *q);
        hits(int ns,float *set,pmt_geometry *geom,comtype2 *itevent);
        ~hits(void);
        
        inline int nset();
        inline int first_hit(int set);
        inline int beyond_last_hit(int set);
        inline int nhits(int set);
        inline float charge(int set);
        inline int sumhits(int set);
        inline int hitcable(int hit);
        inline float x(int i);
        inline float y(int i);
        inline float z(int i);
        inline float hittime(int hit);
        inline float hitcharge(int i);
        inline void printhit(int hit);
        inline int ntot(void);
        inline void frontof(float *pos,int i,float top);
        inline float tof(float *pos,float *dir,int hit);
        inline float tof(float *pos,int hit);
        inline float gaussweight(float t0,float tmi,float tpl,int hit);
        inline void addhitloc(double *loc,int hit,double weight);
        inline void print_cables(int n);
        inline void qsort(short int *list,short int n);
        inline void qsort(short int *list);
        float time_av(float tmin,float tmax);
        int vertex4(int *hits,double *vert);
        int vertex4(int *fourcombo,int *hitlist,int listsize,
                    int &hbegin,int &hend,float tsig,
                    float cyl_radius,float cyl_height,float dwallmin,
                    double *vert,float *gdn);
    };//class hits
    // **********************************************
    // protected functions
    // **********************************************
    // return the postion of hit i in hit arrays
    // **********************************************
    inline short int hits::position(int i)
    {
        return(places[i]);
    }
    
    // **********************************************
    // return number of hits in a set
    // **********************************************
    inline int hits::hits_in_set(int set,int i)
    {
        if ((set<0) || (set>=nchargeset)) return(-1);
        if ((i<0) || (i>=nhit[set])) return(-1);
        if (set==0) return(i);
        return(i+nhit[set-1]);
    }
    // **********************************************
    // test valid hit number
    // **********************************************
    inline int hits::invalid_hit(int hit)
    {
        return((hit<0) || (hit>=nhit[nchargeset-1]));
    }
    // **********************************************
    // swap two hits
    // **********************************************
    inline void hits::swap(int i,int j)
    {
        float hf;
        int   hi,ti=3*i,tj=3*j;
        
        if (i==j) return;
        /*if (invalid_hit(i) || invalid_hit(j)) return;*/
        hi=cables[i];     cables[i]=cables[j];           cables[j]=hi;
        hi=places[i];     places[i]=places[j];           places[j]=hi;
        hf=times[i];      times[i]=times[j];             times[j]=hf;
        hf=charges[i];    charges[i]=charges[j];         charges[j]=hf;
        hf=positions[ti]; positions[ti++]=positions[tj]; positions[tj++]=hf;
        hf=positions[ti]; positions[ti++]=positions[tj]; positions[tj++]=hf;
        hf=positions[ti]; positions[ti++]=positions[tj]; positions[tj++]=hf;
    }
    // **********************************************
    // return time difference between two hits
    // **********************************************
    inline float hits::tdiff(int i,int j)
    {
        //if (i==j) return(0);
        //if (invalid_hit(i) || invalid_hit(j)) return(-1);
        
        if (times[i]>times[j])
            return(times[i]-times[j]);
        else
            return(times[j]-times[i]);
    }
    // **********************************************
    // return distance^2 between two hits
    // **********************************************
    inline float hits::dist2(int i,int j)
    {
        int   ti=3*i,tj=3*j;
        float delta_x,delta_y,delta_z;
        
        //if (i==j) return(0);
        //if (invalid_hit(i) || invalid_hit(j)) return(-1);
        delta_x=positions[ti++]-positions[tj++];
        delta_y=positions[ti++]-positions[tj++];
        delta_z=positions[ti]  -positions[tj];
        return(delta_x*delta_x+delta_y*delta_y+delta_z*delta_z);
    }
    
    
    // **********************************************
    // public functions
    // **********************************************
    // return # of charge sets
    // **********************************************
    inline int hits::nset()
    {
        return(nchargeset);
    }
    // **********************************************
    // return hit # of first hit in a set
    // **********************************************
    inline int hits::first_hit(int set)
    {
        if ((set<0) || (set>=nchargeset)) return(-1);
        if (set==0) return(0);
        return(nhit[set-1]);
    }
    // **********************************************
    // return hit # of last hit in a set +1
    // **********************************************
    inline int hits::beyond_last_hit(int set)
    {
        if ((set<0) || (set>=nchargeset)) return(-1);
        return(nhit[set]);
    }
    // **********************************************
    // return # of hits in a set
    // **********************************************
    inline int hits::nhits(int set)
    {
        if ((set<0) || (set>=nchargeset)) return(-1);
        if (set==0) return(*nhit);
        return(nhit[set]-nhit[set-1]);
    }
    // **********************************************
    // return # of hits in a set
    // **********************************************
    inline float hits::charge(int set)
    {
        if ((set<0) || (set>=nchargeset)) return(-1);
        if (set==0) {
            if (*nhit==0) {
                return(-1);
            } else {
                return(*avcharges);
            }
        }
        if (nhit[set]==nhit[set-1]) return(-1); else return(avcharges[set]);
    }
    // **********************************************
    // return sum of hits for this set and higher charge sets
    // **********************************************
    inline int hits::sumhits(int set)
    {
        if ((set<0) || (set>=nchargeset)) return(-1);
        if (set==0) return(nhit[nchargeset-1]);
        return(nhit[nchargeset-1]-nhit[set-1]);
    }
    // **********************************************
    // return cable of a hit PMT
    // **********************************************
    inline int hits::hitcable(int hit)
    {
        return(cables[hit]);
    }
    // **********************************************
    // return time of hit
    // **********************************************
    inline float hits::x(int hit)
    {
        return(positions[3*hit]);
    }
    // **********************************************
    // return time of hit
    // **********************************************
    inline float hits::y(int hit)
    {
        return(positions[3*hit+1]);
    }
    // **********************************************
    // return time of hit
    // **********************************************
    inline float hits::z(int hit)
    {
        return(positions[3*hit+2]);
    }
    // **********************************************
    // return time of hit PMT
    // **********************************************
    inline float hits::hittime(int hit)
    {
        return(times[hit]);
    }
    // **********************************************
    // return charge of hit
    // **********************************************
    inline float hits::hitcharge(int hit)
    {
        return(charges[hit]);
    }
    // **********************************************
    // print out a hit
    // **********************************************
    inline void hits::printhit(int hit)
    {
        printf("%5d %8.2f %8.2f %8.2f %11.5f %6.2f\n",
               cables[hit],positions[3*hit],positions[3*hit+1],
               positions[3*hit+2],times[hit],charges[hit]);
    }
    // **********************************************
    // return total number of hits
    // **********************************************
    inline int hits::ntot(void)
    {
        return(nhit[nchargeset-1]);
    }
    // **********************************************
    // return position in front of a hit PMT
    // **********************************************
    inline void hits::frontof(float *pos,int i,float top)
    {
        if (fabs(positions[3*i+2])<top)
        {
            *pos=positions[3*i]*0.9882;
            pos[1]=positions[3*i+1]*0.9882;
            pos[2]=positions[3*i+2];
        }
        else
        {
            *pos=positions[3*i];
            pos[1]=positions[3*i+1];
            pos[2]=positions[3*i+2]*0.989;
        }
    }
    // **********************************************
    // return time-of-flight of a hit PMT
    // (assuming direct light)
    // **********************************************
    inline float hits::tof(float *pos,float *dir,int hit)
    {
        float *pmt=positions+3*hit,dis,invdis;
        
        *dir=*pmt++-*pos++;
        dir[1]=*pmt++-*pos++;
        dir[2]=*pmt-*pos;
        dis=*dir*(*dir)+dir[1]*dir[1]+dir[2]*dir[2];
        if (dis==0) return(0);
        dis=sqrt(dis);
        invdis=1/dis;
        *dir*=invdis;
        dir[1]*=invdis;
        dir[2]*=invdis;
        return(CM_TO_NS*dis);
    }
    inline float hits::tof(float *pos,int hit)
    {
        float *pmt=positions+3*hit;
        float dx=*pos-*pmt;
        float dy=pos[1]-pmt[1];
        float dz=pos[2]-pmt[2];
        
        return(CM_TO_NS*sqrt(dx*dx+dy*dy+dz*dz));
    }
    // **********************************************
    // generated Gaussian weight based on absolut timing
    // **********************************************
    inline float hits::gaussweight(float t0,float tmi,float tpl,int hit)
    {
        float dt=times[hit]-t0,weight;
        
        if (dt<0)
            weight=dt*dt/(tmi*tmi);
        else
            weight=dt*dt/(tpl*tpl);
        if (weight<30) return(exp(-0.5*weight)); else return(0);
    }
    // **********************************************
    // add hit position to array (to form centroid of hits)
    // **********************************************
    inline void hits::addhitloc(double *loc,int hit,double weight)
    {
        float  *pmt=positions+3*hit;
        double x=*pmt,y=pmt[1],z=pmt[2];
        
        *loc+=x*weight;
        loc[1]+=y*weight;
        loc[2]+=z*weight;
        loc[3]+=x*x*weight;
        loc[4]+=y*y*weight;
        loc[5]+=z*z*weight;
        loc[6]+=x*y*weight;
        loc[7]+=y*z*weight;
        loc[8]+=x*z*weight;
    }
    // **********************************************
    // print cable number
    // **********************************************
    inline void hits::print_cables(int n)
    {
        int i;
        
        for(i=0; i<n; i++)
            printf("%d ",cables[i]);
        printf("\n");
    }
    // **********************************************
    // sort hits by time
    // **********************************************
    inline void hits::qsort(short int *list,short int n)
    {
        short int *buffer=new short int[n];
        
        qsort(list,n,buffer);
        delete(buffer);
    }
    // **********************************************
    // sort hits by time
    // **********************************************
    inline void hits::qsort(short int *list)
    {
        short int          *buffer=new short int[nhit[nchargeset-1]];
        unsigned short int set,hit,offs;
        short int *splaces;                  // array position of nth hit
        int   *scables;                      // cable #'s, times, charges, and
        float *stimes,*scharges,*spositions; // locations for each hit
        
        
        // sort in time
        for(set=hit=offs=0; set<nchargeset; set++)
        {
            for(; hit<nhit[set]; hit++)
                list[hit]=hit;
            qsort(list+offs,nhit[set]-offs,buffer);
            offs=nhit[set];
        }
        // reshuffle according to sort list
        splaces=places; scables=cables;
        stimes=times; scharges=charges; spositions=positions;
        set=nhit[nchargeset-1];
        places=new short int[set]; cables=new int[set]; times=new float[set];
        charges=new float[set]; positions=new float[3*set];
        for(hit=0; hit<set; hit++)
        {
            offs=buffer[hit]=list[hit];
            places[hit]=splaces[offs];
            cables[hit]=scables[offs];
            times[hit]=stimes[offs];
            charges[hit]=scharges[offs];
            offs*=3;
            positions[3*hit]=spositions[offs++];
            positions[3*hit+1]=spositions[offs++];
            positions[3*hit+2]=spositions[offs];
        }
        // remember original places
        for(hit=0; hit<set; hit++)
            list[buffer[hit]]=hit;
        delete(splaces);
        delete(scables);
        delete(stimes);
        delete(scharges);
        delete(spositions);
        delete(buffer);
    }
}//end namespace bonsai
#endif
