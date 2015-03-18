/**
 *  @class DS::BonsaiFit
 *
 *  Data Structure: Position and time fit using BONSAI
 */
#ifndef __RAT_DS_BonsaiFit__
#define __RAT_DS_BonsaiFit__

#include <RAT/DS/PosFit.hh>

namespace RAT {
    
    namespace DS {

        class BonsaiFit : public TObject, public PosFit {
            public:
                BonsaiFit() : TObject(), PosFit("BONSAI") {}
                virtual ~BonsaiFit() {}

                /* Position attributes inherited from PosFit */

                /* Time fit by BONSAI */
                virtual double GetTime() const { return time; }
                virtual void SetTime(double _time) { time = _time; }

                ClassDef(BonsaiFit, 1)

            protected:
                double time;
        };
        
    } // namespace DS
    
} // namespace RAT

#endif

