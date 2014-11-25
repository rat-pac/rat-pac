#include <RAT/DS/EV.hh>
#include <RAT/DS/Root.hh>
#include <RAT/Processor.hh>

#include <RAT/BONSAI/pmt_geometry.h>
#include <RAT/BONSAI/likelihood.h>
#include <RAT/BONSAI/bonsaifit.h>

#include <vector>
#include <map>

namespace RAT {

    class FitBonsaiProc : public Processor {
        public:
            FitBonsaiProc();
            virtual ~FitBonsaiProc();
            
            virtual Processor::Result Event(DS::Root *ds, DS::EV *ev);
            
        protected:
            BONSAI::pmt_geometry *bonsai_geometry;
            BONSAI::likelihood *bonsai_likelihood;
            BONSAI::bonsaifit *bonsai_fit;
            
            std::map<int,int> pmtmap; // pmtid -> bonsai_idx
            
            std::vector<float> goodness;
            std::vector<float> hit_time, hit_charge;
            std::vector<int> hit_pmtid;
    };

}

