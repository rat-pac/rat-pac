#ifndef __RAT_DetectorFactory__
#define __RAT_DetectorFactory__

#include <string>
#include <map>

#include <RAT/DB.hh>

namespace RAT {

/// Allows a detector geometry to be based on values that are calculated at 
/// runtime based on some parameters instead of simply being static in a RATDB
/// file while preserving DB functionality in dynamicly built detectors
class DetectorFactory {

    public:
    
        DetectorFactory() { }
        
        virtual ~DetectorFactory() { }
        
        /// Invokes DefineDetector on the DetectorFactory registered to name
        static void DefineWithFactory(const std::string name, DBLinkPtr detector);
        
        /// Registers a DetectorFactory under a name
        static void Register(const std::string name, DetectorFactory *factory);
        
    protected:
    
        /// Map of registered DetectorFactories
        static std::map<const std::string, DetectorFactory*> fFactoryMap;
        
        /// Override in subclasses to set up the DB GEO tables. Typically load 
        /// in a template specifying constants and calculate necessary dynamic
        /// fields on the fly.
        virtual void DefineDetector(DBLinkPtr detector) = 0;
        
};

} //namespace RAT

#endif
