#include <math.h>
#include "RAT/BONSAI/bonsaifit.h"

namespace BONSAI {
    
    // *************************************************************
    // * find deviation to fiducial volume edge, adjust search rad *
    // *************************************************************
    inline void bonsaifit::fiducial_dev(float &devx,float &devy,float &devz,float &dev)
    {
        // edge of fiducial volume is in vertex[3],vertex[4],vertex[5]
        // calculate deviation to search center
        devx=vertex[3]-vertex[0];
        devy=vertex[4]-vertex[1];
        devz=vertex[5]-vertex[2];
        dev=sqrt(devx*devx+devy*devy+devz*devz);
        // set rad to half of min(rad,dev): ensures, that search
        // doesn't trail off outside the fiducial volume
        if (dev<rad)
            rad=dev*0.5;
        else
            rad*=0.5;
#ifdef DEBUG_TWO
        printf("->(%8.2f,%8.2f,%8.2f) radius=%6.4f\n",
               vertex[3],vertex[4],vertex[5],rad);
#endif
    }
    
    // *************************************************************
    // * set position out of vertex array and set end radius       *
    // *************************************************************
    inline void bonsaifit::end_search(float *point,int indx,int indy,int indz,float rmax)
    { // store best fit point at zero position
        point[0]=vertex[indx];
        point[1]=vertex[indy];
        point[2]=vertex[indz];
        if (rad<0.3*rmax)
            point[STORERADIUS]=0.3*rmax;
        else
            point[STORERADIUS]=rad;
#ifdef DEBUG
        printf("-> (%8.2f,%8.2f,%8.2f,%6.1f) q=%8.6f radius=%6.2f<%6.2f\n",
               point[0],point[1],point[2],point[STORERES],
               point[STOREQ],point[STORERADIUS],rmax);
#endif
        return;
    }
    
    // *************************************************************
    // * search for maximum from start with radius rmax            *
    // *************************************************************
    void bonsaifit::search(float *start,float rmax)
    {
        if (start[STORERADIUS]<0.01) return;
        
        int   max;
        float inter[4],dev;
        
        // set centre point and dodecahedran centre to current point
        vertex[0]=vertex[3]=start[0];
        vertex[1]=vertex[4]=start[1];
        vertex[2]=vertex[5]=start[2];
        rad=start[STORERADIUS];
#ifdef DEBUG
        printf("search %6.2fcm (%8.2f,%8.2f,%8.2f,%6.1f)",
               rad,start[0],start[1],start[2],start[STORERES]);
#ifdef DEBUG_TWO
        printf("\n");
#endif
#endif
        while(1)
        {
            // calculate dodecahedran of search radius and all thirteen qualitites
#ifdef DEBUG_TWO
            printf("check around (%8.2f,%8.2f,%8.2f) with radius=%6.2f\n",
                   vertex[0],vertex[1],vertex[2],rad);
#endif
            qual->check_around(vertex,start+STORERES,rad,q,max);
            // if the maximum qualities is outside, end the search there
            if (!qual->fit_volume(vertex+3*max,vertex+3,dev))
            {
#ifdef DEBUG_TWO
                printf("max outside: (%8.2f,%8.2f,%8.2f)",
                       vertex[3*max],vertex[3*max+1],vertex[3*max+2]);
#endif
                fiducial_dev(inter[0],inter[1],inter[2],dev);
                if (rad<=rmax)
                {
                    start[STOREQ]=qual->quality(vertex+3);
                    qual->get_result(start+STORERES);
                    end_search(start,3,4,5,rmax);
                    return;
                }
                vertex[0]=vertex[3]; vertex[1]=vertex[4]; vertex[2]=vertex[5];
                continue;
            }
            if (max!=0)
            {
                // if maximum is not in the dodecahedran center, move center to the maximum
                vertex[0]=vertex[3]=vertex[3*max];
                vertex[1]=vertex[4]=vertex[3*max+1];
                vertex[2]=vertex[5]=vertex[3*max+2];
                q[0]=q[max];
                continue;
            }
            // calculate quality at interpolated point
            qual->interpolate(vertex,rad,q,inter);
            // if that quality is the maximum, set center to this
            // point and the search radius to the distance between
            // old and new center; otherwise cut radius in half
            if (q[(int)qual->ncheck()]>q[0])
            {
                if (!qual->fit_volume(vertex+3*qual->ncheck(),vertex+3,dev))
                {
#ifdef DEBUG_TWO
                    printf("interpolation outside: (%8.2f,%8.2f,%8.2f)",
                           vertex[3*qual->ncheck()],vertex[3*qual->ncheck()+1],
                           vertex[3*qual->ncheck()+2]);
#endif
                    fiducial_dev(inter[0],inter[1],inter[2],dev);
                    if (rad<=rmax)
                    {
                        start[STOREQ]=qual->quality(vertex+3);
                        qual->get_result(start+STORERES);
                        end_search(start,3,4,5,rmax);
                        return;
                    }
                    vertex[0]=vertex[3]; vertex[1]=vertex[4]; vertex[2]=vertex[5];
                    continue;
                }
                // move center and best point to interpolated point
                vertex[0]=vertex[3]=vertex[3*qual->ncheck()];
                vertex[1]=vertex[4]=vertex[3*qual->ncheck()+1];
                vertex[2]=vertex[5]=vertex[3*qual->ncheck()+2];
                if (inter[3]<1) rad*=inter[3];
                q[0]=q[(int)qual->ncheck()];
                qual->get_result(start+STORERES);
            }
            else
                rad*=0.5;
            // if radius is below cut-off, finish search
            if (rad<=rmax)
            {
                start[STOREQ]=q[0];
                end_search(start,0,1,2,rmax);
                return;
            }
        }
    }
    
}
