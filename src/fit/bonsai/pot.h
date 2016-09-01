#include "RAT/BONSAI/vertex.h"

namespace BONSAI {
    
    // **********************************************
    // find starting position for vertex fit
    // **********************************************
    class pot: public vertex
    {
        float  rinit,rcent,rpmt,top;
        
        // **********************************************
        // set radius for 4-hit combination test vertices,
        // PMT positiion vertices, center-of-tank and
        // set the height of the detector
        // **********************************************
        inline void add4vert(short int fast,int *cab,double *ffit);
        
    protected:
        inline void set_radius(float ri,float rp,float rc,float to)
        { rinit=ri;
            rpmt=rp;
            rcent=rc;
            top=to; }
        
    public:
        inline pot():vertex()
        { rinit=rcent=rpmt=top=0; };
        inline pot(int n_raw,int *cable,int *bad_ch,float *pmt_loc,
                   float *tim_raw,float *chg_raw):
        vertex(n_raw,cable,bad_ch,pmt_loc,tim_raw,chg_raw)
        { rinit=rcent=rpmt=top=0; };
        void search();
    };
    
}
