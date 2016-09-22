#include <RAT/DS/EV.hh>
#include <RAT/DS/Root.hh>
#include <RAT/Processor.hh>

#include <RAT/BONSAI/pmt_geometry.h>
#include <RAT/BONSAI/goodness.h>
#include <RAT/BONSAI/likelihood.h>
#include <RAT/BONSAI/bonsaifit.h>

#include <vector>
#include <map>
#include <TRandom.h>

namespace RAT
{
    
    class FitBonsaiProc : public Processor
    {
    public:
        FitBonsaiProc();
        virtual ~FitBonsaiProc();
        
        virtual Processor::Result Event(DS::Root *ds, DS::EV *ev);
        
    protected:
        TRandom rnd;
        pmt_geometry *bonsai_geometry;
        goodness *bonsai_hits;
        likelihood *bonsai_likelihood;
        bonsaifit *bonsai_fit;
        float bonsai_vtxfit[4],bonsai_dirfit[5],goodn[2],maxlike[2],*gdn;
        int   nsel[3];
        int maxnhit;
    };
    
}
