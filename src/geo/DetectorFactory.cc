#include <RAT/DetectorFactory.hh>
#include <RAT/Log.hh>

namespace RAT {
        
    std::map<const std::string, DetectorFactory*> DetectorFactory::fFactoryMap;
    
    void DetectorFactory::DefineWithFactory(const std::string name, DBLinkPtr detector) {
        if (fFactoryMap.find(name) != fFactoryMap.end()) {
            fFactoryMap[name]->DefineDetector(detector);
        } else {
            Log::Die("No detector factory registered as \"" + name + "\"");
        }
    }
    
    void DetectorFactory::Register(const std::string name, DetectorFactory *factory) {
        fFactoryMap[name] = factory;
    }
        
}
