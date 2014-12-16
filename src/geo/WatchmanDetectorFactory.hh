#ifndef __WATCHMANDETECTORFACTORY__
#define __WATCHMANDETECTORFACTORY__

#include <RAT/DetectorFactory.hh>

namespace RAT {

class WatchmanDetectorFactory : public DetectorFactory {

    public:
        WatchmanDetectorFactory() { }
        virtual ~WatchmanDetectorFactory() { }
        
    protected:
        virtual void DefineDetector(DBLinkPtr detector);

};

} //namespace RAT

#endif
