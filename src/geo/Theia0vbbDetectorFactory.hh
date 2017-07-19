///////////////////////////////////////////////////////////////////////////////
/// \class RAT::Theia0vbbDetectorFactory
///
/// \brief  Rebuilds the Theia detector on the fly based on simple parameters
///         now with a 0vbb isotope loaded balloon
///         
/// \original author Benjamin Land <benland100@berkeley.edu>
///
///
/// REVISION HISTORY:\n
///     2017-07-11 : C Grant - Used B Land's TheiaDetector factory to create Theia0vbbDetectorFactory
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Uses the geometry template in Theia_0vbb/Theia_0vbb.geo but positions and 
///          resizes components on the fly according the parameters given in 
///          the table named Theia_PARAMS. This DetectorFactory is registered as 
///          "Theia_0vbb" and is preferred to using the .geo file directly. 
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_Theia0vbbDetectorFactory__
#define __RAT_Theia0vbbDetectorFactory__

#include <RAT/DetectorFactory.hh>

namespace RAT {

class Theia0vbbDetectorFactory : public DetectorFactory {

    public:
        Theia0vbbDetectorFactory() { }
        virtual ~Theia0vbbDetectorFactory() { }
        
    protected:
        virtual void DefineDetector(DBLinkPtr detector);

};

} //namespace RAT

#endif
