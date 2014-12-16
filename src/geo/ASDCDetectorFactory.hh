#ifndef __ASDCDETECTORFACTORY__
#define __ASDCDETECTORFACTORY__

#include <RAT/DetectorFactory.hh>

namespace RAT {

class ASDCDetectorFactory : public DetectorFactory {

    public:
        ASDCDetectorFactory() { }
        virtual ~ASDCDetectorFactory() { }
        
    protected:
        virtual void DefineDetector(DBLinkPtr detector);

};

} //namespace RAT

#endif
