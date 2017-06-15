#ifndef __RAT_LessSimpleDAQ2Proc__
#define __RAT_LessSimpleDAQ2Proc__
#include <RAT/Processor.hh>

namespace RAT {

    class LessSimpleDAQ2Proc : public Processor {
    public:
        LessSimpleDAQ2Proc();
        virtual ~LessSimpleDAQ2Proc() { };
        virtual Processor::Result DSEvent(DS::Root *ds);
    protected:
        int fEventCounter;
        std::vector<double> fSPECharge;
    };
}

#endif
