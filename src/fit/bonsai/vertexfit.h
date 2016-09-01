#include "RAT/BONSAI/tree.h"

namespace BONSAI {
    
    // **********************************************
    // load and adapt vertexfit to detector geometry
    // **********************************************
    class vertexfit: public tree
    {
        int   npmt;
        float *pmts;
        float rmax,zmax;
        
        void loadgeom(void);
        void set(float resolution);
        
    public:
        vertexfit(float resolution);
        vertexfit(int np,float *ps,float resolution);
        vertexfit(float resolution,
                  int n_raw,int *cable,int *bad_ch,
                  float *tim_raw,float *chg_raw);
        ~vertexfit(void);
        inline void loadhits(int n_raw,int *cable,int *bad_ch,
                             float *tim_raw,float *chg_raw)
        { vertex::loadhits(n_raw,cable,bad_ch,pmts,tim_raw,chg_raw);}
        inline int npmts(void)
        { return(npmt); }
        inline void get_positions(float *array)
        { int i;
            for(i=0; i<3*npmt; i++) array[i]=pmts[i]; };
    };
    
}
