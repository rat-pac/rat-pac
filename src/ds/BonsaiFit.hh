/**
 *  @class DS::BonsaiFit
 *
 *  Data Structure: Position and time fit using BONSAI
 */
#ifndef __RAT_DS_BonsaiFit__
#define __RAT_DS_BonsaiFit__

#include <RAT/DS/PosFit.hh>
#include <TVector3.h>

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
                
                /* Direction fit by BONSAI */
                virtual const TVector3 &GetDirection() const { return dir; }
                virtual void SetDirection(TVector3 _dir) { dir = _dir; }

                ClassDef(BonsaiFit, 1)

            protected:
                double time;
                TVector3 dir;
        };
        
    } // namespace DS
    
} // namespace RAT

#endif

