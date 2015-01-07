///////////////////////////////////////////////////////////////////////////////
/// \class RAT::ASDCDetectorFactory
///
/// \brief  Rebuilds the ASDC detector on the fly based on simple parameters
///         
/// \author Benjamin Land <benland100@berkeley.edu>
///
/// REVISION HISTORY:\n
///     2015-01-07 : B Land - Added doxygen header block \n
///
/// \details Uses the geometry template in ASDC/ASDC.geo but positions and 
///          resizes components on the fly according the parameters given in 
///          the table named ASDC_PARAMS. This DetectorFactory is registered as 
///          "ASDC" and is preferred to using the .geo file directly. 
///
///////////////////////////////////////////////////////////////////////////////

#ifndef __RAT_ASDCDetectorFactory__
#define __RAT_ASDCDetectorFactory__

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
