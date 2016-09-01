#include "RAT/BONSAI/bonsai.h"
#include "RAT/BONSAI/pot.h"

namespace BONSAI {
    
    // **********************************************
    // do tree search to fit vertex
    // **********************************************
    class tree: public pot
    {
        bonsai *root;
        float  depthfactor,tfactor;
        
    public:
        tree(void);
        tree(int n_raw,int *cable,int *bad_ch,float *pmt_loc,
             float *tim_raw,float *chg_raw);
        ~tree(void);
        
        inline void set_depth(float f)
        { depthfactor=f; }
        inline void set_time(float t)
        { tfactor=t; }
        void search(void);
        void search(float *vert,float rad,int maxiter);
    };
    
}
