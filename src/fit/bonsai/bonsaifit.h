#ifndef BONSAIFIT
#define BONSAIFIT
#include "RAT/BONSAI/fitquality.h"
#include "RAT/BONSAI/searchgrid.h"

namespace BONSAI {
    
    //#define DEBUG
    //#define DEBUG_TWO
    
    // *************************************************************
    // * maximize the vertex quality function  provided by qual    *
    // *************************************************************
    class bonsaifit
    {
        fitquality   *qual;           // quality criterion to be maximized
        float        rmax,rmax2,zmax; // maximum allowed x^2+y^2, |z|
        short int    nbranch,current; // number of branches and current br.
        short int    best,worst;      // best and worst fit branch
        float        *branches;       // positions, radii, qualities,...
        float        rad;             // active radius
        float        *vertex,*q;      // active search positons, qualities
        
        // find deviation to fiducial volume edge, adjust search rad
        inline void      fiducial_dev(float &inter_x,float &inter_y,
                                      float &inter_z,float &dev);
        // set position out of vertex array and set end radius
        inline void      end_search(float *point,int indx,int indy,int indz,
                                    float rmax);
        // find minimum and maximum quality
        inline void      find_extreme(void);
        // de-activate all branches with quality<minqual
        inline void      ream(float delta_qual);
        // if the range of qualities is above reamthr, ream off the
        // upper reamfrac portion of the qualities
        inline void      ream(float reamthr,float reamfrac);
        // combine searches that are close to each other
        inline void      sphere(void);
        
    public:
        // set number of branches to zero
        inline bonsaifit(fitquality *qu);
        // destroy branches
        inline ~bonsaifit(void);
        // search for maximum from start with radius rmax
        void      search(float *start,float rmax);
        // calculate the qualities for the initial points; set search radii
        inline void search(float radius,searchgrid *grid,int set);
        // if the quality range between best and worst fit>reamthr, keep
        // active only reamfrac of the branches; then find the best for each
        // each active branch with cut-off radius rmax
        inline void search(float reamthr,float reamfrac,float rmax);
        // if the quality range between best and worst fit>reamthr, keep
        // active only reamfrac of the branches; then average close points
        // together and find the best fit for each remaining branch (after
        // increasing search radii<rmin) with cut-off radius rmax
        inline void search(float radius,float rmax,float *point);
        // calculate the quality for one initial point, set search
        // radius to radius, optimize until search radius<rmax
        inline void search(float reamthr,float reamfrac,
                           float rmin,float rmax);
        // print out all currently active branches
        inline void print_branch_list(void);
        // find best fit at the end of the search
        inline void search(float reamthr,float reamfrac);
        inline float xfit(void);     // return best x position
        inline float yfit(void);     // return best y position
        inline float zfit(void);     // return best z position
        inline float maxq(void);     // return quality of best fit position
        inline float fitrad(void);   // return radius of best fit position
        inline void  fitresult(void);// return best fit result
    };
    
#define VSIZE       (qual->nresult()+5)
#define STORERADIUS 3
#define STOREQ      4
#define STORERES    5
    
    // private
    
    // *************************************************************
    // * find minimum and maximum quality                          *
    // *************************************************************
    inline void bonsaifit::find_extreme(void)
    {
        // find first valid branch
        for(current=0; current<VSIZE*nbranch; current+=VSIZE)
            if (branches[current+STORERADIUS]>0) break;
        if (current==VSIZE*nbranch) // no valid branch at all
        {
            best=worst=-1;
            return;
        }
        // go through all valid, fiducial branches
        for(best=worst=current; current<VSIZE*nbranch; current+=VSIZE)
            if ((branches[current+STORERADIUS]>0) &&
                qual->fit_volume(branches+current))
            {
                // set best and worst
                if (branches[current+STOREQ]>branches[best+STOREQ]) best=current;
                if (branches[current+STOREQ]<branches[worst+STOREQ]) worst=current;
            }
    }
    
    // *************************************************************
    // * de-activate all branches with quality<minqual             *
    // *************************************************************
    inline void bonsaifit::ream(float minqual)
    {
        for(current=0; current<VSIZE*nbranch; current+=VSIZE)
            if (branches[current+STORERADIUS]>0)
                if (branches[current+STOREQ]<minqual) // quality too small?
                    branches[current+STORERADIUS]=0;    // de-activate branch
    }
    
    // *************************************************************
    // * if the range of qualities is above reamthr, ream off the  *
    // * upper reamfrac portion of the qualities                   *
    // *************************************************************
    inline void bonsaifit::ream(float reamthr,float reamfrac)
    {
        // required quality threshold is q_th(f)=f*q_worst+(1-f)*q_best
        if (branches[best+STOREQ]>branches[worst+STOREQ]+reamthr)
            ream(reamfrac*branches[worst+STOREQ]+(1-reamfrac)*branches[best+STOREQ]);
#ifdef DEBUG
        printf("quality range %8.6f<q<%8.6f",
               branches[worst+STOREQ],branches[best+STOREQ]);
        if (branches[best+STOREQ]>branches[worst+STOREQ]+reamthr)
        {
            printf(": cut at %6.4frange: %8.6f\n",reamfrac,
                   reamfrac*branches[worst+STOREQ]+(1-reamfrac)*branches[best+STOREQ]);
            print_branch_list();
        }
        else
            printf(" below threshold %6.4f\n",reamthr);
#endif
    }
    
    
    // *************************************************************
    // * combine searches that are close to each other             *
    // *************************************************************
    inline void bonsaifit::sphere(void)
    {
        float     *cent,dx,dy,dz,cx,cy,cz,cr2,g;
        
        while(1)
        {
            // find largest active quality and set sphere center to that point
            for(cent=NULL,current=0; current<VSIZE*nbranch; current+=VSIZE)
                if (branches[current+STORERADIUS]>0)
                    if ((cent==NULL) || (branches[current+STOREQ]>cent[STOREQ]))
                        cent=branches+current;
            // if there's none, stop
            if (cent==NULL) return;
            // define a sphere of 3 times the search radius; don't use less than 10cm
            // average all points weighted by quality that are within the sphere
            cr2=3*cent[STORERADIUS];
            if (cr2<10) cr2=10;
            cx=cent[0];  cy=cent[1];  cz=cent[2];  g=cent[STOREQ]-qual->worstquality();
#ifdef DEBUG_TWO
            printf("around  (%8.2f,%8.2f,%8.2f) with weight %8.6f and radius %6.4f\n",
                   cx,cy,cz,g,cr2);
#endif
            cr2*=cr2;
            cent[0]*=g;
            cent[1]*=g;
            cent[2]*=g;
            cent[STOREQ]=g;
            cent[STORERADIUS]=-cent[STORERADIUS]; // de-activate the branch for this function
            for(current=0; current<VSIZE*nbranch; current+=VSIZE)
                if (branches[current+STORERADIUS]>0)
                    if ((g=branches[current+STOREQ]-qual->worstquality())>0)
                        if (cent[STOREQ]>g)
                        {
                            dx=branches[current]-cx;
                            dy=branches[current+1]-cy;
                            dz=branches[current+2]-cz;
                            if (dx*dx+dy*dy+dz*dz<cr2)
                            {
#ifdef DEBUG_TWO
                                printf("average (%8.2f,%8.2f,%8.2f) with weight %8.6f %f\n",
                                       branches[current],branches[current+1],branches[current+2],g,
                                       sqrt(dx*dx+dy*dy+dz*dz));
#endif
                                cent[0]+=g*branches[current];
                                cent[1]+=g*branches[current+1];
                                cent[2]+=g*branches[current+2];
                                cent[STOREQ]+=g;
                                branches[current+STORERADIUS]=0; // de-activate
                            }
                        }
            if ((g=cent[STOREQ])>0)
            {
                g=1/g;
                cent[0]*=g;
                cent[1]*=g;
                cent[2]*=g;
            }
#ifdef DEBUG_TWO
            printf("results (%8.2f,%8.2f,%8.2f)\n",cent[0],cent[1],cent[2]);
#endif
        }
    }
    
    // public
    
    
    // *************************************************************
    // * set number of branches to zero                            *
    // *************************************************************
    inline bonsaifit::bonsaifit(fitquality *qu)
    {
        qual=qu;
        nbranch=0;
        branches=NULL;
        vertex=new float[3*(qual->ncheck()+1)];
        q=new float[qual->ncheck()+1];
    }
    
    // *************************************************************
    // * destroy branches                                          *
    // *************************************************************
    inline bonsaifit::~bonsaifit(void)
    {
        if (branches!=NULL) delete(branches);
        delete(vertex);
        delete(q);
    }
    
    // *************************************************************
    // * calculate the qualities for the initial points, set       *
    // * search radii to radius                                    *
    // *************************************************************
    inline void bonsaifit::search(float radius,searchgrid *grid,int set)
    {
        if (branches!=NULL) delete(branches);
        nbranch=grid->size(set)+grid->size(0);
        branches=new float[VSIZE*nbranch];
        grid->copy_points(0,branches,VSIZE,0,1,2);
        grid->copy_points(set,branches+VSIZE*grid->size(0),VSIZE,0,1,2);
        // calculate goodness for initial points; find best and worst fit
        for(best=worst=current=0; current<nbranch; current++)
        {
            qual->set_branch(current);
            branches[VSIZE*current+STORERADIUS]=radius;
            branches[VSIZE*current+STOREQ]=qual->quality(branches+VSIZE*current);
            qual->get_result(branches+VSIZE*current+STORERES);
            if (qual->fit_volume(branches+VSIZE*current))
            {
                if (branches[VSIZE*current+STOREQ]>branches[best+STOREQ])
                    best=VSIZE*current;
                if (branches[VSIZE*current+STOREQ]<branches[worst+STOREQ])
                    worst=VSIZE*current;
            }
#ifdef DEBUG
            printf("%4d (%8.2f,%8.2f,%8.2f,%6.1f): q=%8.6f, radius=%6.2f\n",
                   current,branches[VSIZE*current],branches[VSIZE*current+1],
                   branches[VSIZE*current+2],branches[VSIZE*current+STORERES],
                   branches[VSIZE*current+STOREQ],
                   branches[VSIZE*current+STORERADIUS]);
#endif
        }
    }
    
    // *************************************************************
    // * if the quality range between best and worst fit>reamthr,  *
    // * keep active only reamfrac of the branches; then find the  *
    // * best for each active branch with cut-off radius rmax      *
    // *************************************************************
    inline void bonsaifit::search(float reamthr,float reamfrac,float rmax)
    {
        // ream off the upper reamfrac portion of goodnesses
        ream(reamthr,reamfrac);
        // search for best fit of each remaining branch
        for(current=0; current<nbranch; current++)
        {
            qual->set_branch(current);
            search(branches+VSIZE*current,rmax);
        }
    }
    
    // *************************************************************
    // * calculate the quality for one initial point, set search   *
    // * radius to radius, optimize until search radius<rmax       *
    // *************************************************************
    inline void bonsaifit::search(float radius,float rmax,float *point)
    {
        if (!qual->fit_volume(point)) return;
        if (branches!=NULL) delete(branches);
        nbranch=1;
        branches=new float[VSIZE*nbranch];
        branches[0]=point[0];
        branches[1]=point[1];
        branches[2]=point[2];
        // calculate goodness for initial points; find best and worst fit
        best=worst=current=0;
        qual->set_branch(current);
        branches[VSIZE*current+STORERADIUS]=radius;
        branches[VSIZE*current+STOREQ]=qual->quality(branches+VSIZE*current);
        qual->get_result(branches+VSIZE*current+STORERES);
#ifdef DEBUG
        printf("%4d (%8.2f,%8.2f,%8.2f,%6.1f): q=%8.6f, radius=%6.2f\n",
               current,branches[VSIZE*current],branches[VSIZE*current+1],
               branches[VSIZE*current+2],branches[VSIZE*current+STORERES],
               branches[VSIZE*current+STOREQ],
               branches[VSIZE*current+STORERADIUS]);
#endif
        search(branches,rmax);
    }
    
    // *************************************************************
    // * if the quality range between best and worst fit>reamthr,  *
    // * keep active only reamfrac of the branches; then average   *
    // * close points together and find the best fit for each      *
    // * remaining branch (after increasing search radii<rmin)     *
    // * with cut-off radius rmax                                  *
    // *************************************************************
    inline void bonsaifit::search(float reamthr,float reamfrac,
                                  float rmin,float rmax)
    {
        // ream off the upper reamfrac portion of goodnesses
        find_extreme();
        ream(reamthr,reamfrac);
        sphere();
        for(current=0; current<nbranch; current++)
            if (branches[current*VSIZE+STORERADIUS]!=0)
            {
                // re-activated temporarily de-activated branches
                if (branches[current*VSIZE+STORERADIUS]<0)
                    branches[current*VSIZE+STORERADIUS]=
                    -branches[current*VSIZE+STORERADIUS];
                if (branches[current*VSIZE+STORERADIUS]<rmin)
                    branches[current*VSIZE+STORERADIUS]=rmin;
                qual->set_branch(current);
                search(branches+current*VSIZE,rmax);
            }
#ifdef DEBUG
        printf("Merged searches: ");
        print_branch_list();
#endif
    }
    
    // *************************************************************
    // * find best fit at the end of the search                    *
    // *************************************************************
    inline void bonsaifit::search(float reamthr,float reamfrac)
    {
        find_extreme();
        ream(reamthr,reamfrac);
    }
    // *************************************************************
    // * print out all currently active branches                   *
    // *************************************************************
    inline void bonsaifit::print_branch_list(void)
    {
        printf("Current list of active branches:\n");
        for(current=0; current<nbranch; current++)
            if (branches[current*VSIZE+STORERADIUS]>0)
                printf("%4d (%8.2f,%8.2f,%8.2f,%6.1f): q=%8.6f, radius=%6.2f\n",
                       current,branches[current*VSIZE],branches[current*VSIZE+1],
                       branches[current*VSIZE+2],branches[current*VSIZE+STORERES],
                       branches[current*VSIZE+STOREQ],branches[current*VSIZE+STORERADIUS]);
    }
    
    // *************************************************************
    // * return best x position                                    *
    // *************************************************************
    inline float bonsaifit::xfit(void)
    {
        return(branches[best]);
    }
    
    // *************************************************************
    // * return best y position                                    *
    // *************************************************************
    inline float bonsaifit::yfit(void)
    {
        return(branches[best+1]);
    }
    
    // *************************************************************
    // * return best z position                                    *
    // *************************************************************
    inline float bonsaifit::zfit(void)
    {
        return(branches[best+2]);
    }
    
    // *************************************************************
    // * return quality of best fit position                       *
    // *************************************************************
    inline float bonsaifit::maxq(void)
    {
        return(branches[best+STOREQ]);
    }
    
    // *************************************************************
    // * return radius of best fit position                        *
    // *************************************************************
    inline float bonsaifit::fitrad(void)
    {
        return(branches[best+STORERADIUS]);
    }
    
    // *************************************************************
    // * return best fit result to fitter                          *
    // *************************************************************
    inline void bonsaifit::fitresult(void)
    {
        qual->set_result(branches+best+STORERES);
    }
    
}
#endif
