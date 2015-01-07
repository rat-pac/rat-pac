#ifndef __RAT_WatchmanDetectorFactory__
#define __RAT_WatchmanDetectorFactory__

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
