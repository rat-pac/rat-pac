///////////////////////////////////////////////////////////////////////////////
/// \class RAT::DetectorFactory
///
/// \brief  Pure virtual class for defining parameterized geometries. 
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Extend to create and/or modify database entries related to a 
///          geometry (e.g. based on some parameterization) before the geometry 
///          is built. If the parameterization is stored in a database table,
///          this makes it particularly easy to control the geometry with simple
///          macro commands. Basic implementations should load a .geo file into
///          the database and modify the fields that can't be defined static.
///          DetectorFactory implementations should be registered in 
///          DetectorConstruction.cc
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_DetectorFactory__
#define __RAT_DetectorFactory__

#include <string>
#include <map>

#include <RAT/DB.hh>

namespace RAT {

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
