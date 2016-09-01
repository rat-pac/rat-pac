#include "RAT/BONSAI/likelihood.h"

namespace BONSAI {
    
    // *************************************************************
    // * calculate likelihood with and without timing constraint   *
    // *************************************************************
    float likelihood::quality(float *vertex)
    {
        verfit[0]=vertex[0];
        verfit[1]=vertex[1];
        verfit[2]=vertex[2];
        like0=fittime(1,verfit,dirfit,dt);
        dev=dirfit[3]-cang0;
        if (dev>0)
            if (plusdang==FIT_PARAM_NONE)
                like=like0;
            else
                like=like0-dev*dev*plusdang;
            else
                if (minusdang==FIT_PARAM_NONE)
                    like=like0;
                else
                    like=like0-dev*dev*minusdang;
        if (nlike++==0) set_worst(like); else check_worst(like);
        return(like);
    }
    
}
