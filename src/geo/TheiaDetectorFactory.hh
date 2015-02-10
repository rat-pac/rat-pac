///////////////////////////////////////////////////////////////////////////////
/// \class RAT::TheiaDetectorFactory
///
/// \brief  Rebuilds the Theia detector on the fly based on simple parameters
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Uses the geometry template in Theia/Theia.geo but positions and 
///          resizes components on the fly according the parameters given in 
///          the table named Theia_PARAMS. This DetectorFactory is registered as 
///          "Theia" and is preferred to using the .geo file directly. 
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_TheiaDetectorFactory__
#define __RAT_TheiaDetectorFactory__

#include <RAT/DetectorFactory.hh>

namespace RAT {

class TheiaDetectorFactory : public DetectorFactory {

    public:
        TheiaDetectorFactory() { }
        virtual ~TheiaDetectorFactory() { }
        
    protected:
        virtual void DefineDetector(DBLinkPtr detector);

};

} //namespace RAT

#endif
