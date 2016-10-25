/**
 *  @class DS::BonsaiFit
 *
 *  Data Structure: Position and time fit using BONSAI
 */
#ifndef __RAT_DS_BonsaiFit__
#define __RAT_DS_BonsaiFit__

#include <RAT/DS/PosFit.hh>

namespace RAT
{
    
    namespace DS
    {
        
        class BonsaiFit : public TObject, public PosFit
        {
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
            
            virtual double GetLogLike() const { return loglike[1]; }
            virtual void SetLogLike(double _LogLike) { loglike[1] = _LogLike; }
            
            virtual double GetLogLike0() const { return loglike[0]; }
            virtual void SetLogLike0(double _LogLike0) { loglike[0] = _LogLike0; }
            
            virtual double GetGoodness() const { return goodness[0]; }
            virtual void SetGoodness(double _goodness) { goodness[0] = _goodness; }
            
            virtual double GetShapeGoodness() const { return goodness[1]; }
            virtual void SetShapeGoodness(double _shapegoodness)
            { goodness[1] = _shapegoodness; }
            
            virtual int GetNfit() const { return nsel[2]; }
            virtual void SetNfit(int _nfit) { nsel[2] = _nfit; }
            
            virtual int GetNsel() const { return nsel[0]; }
            virtual void SetNsel(int _nsel) { nsel[0] = _nsel; }
            
            virtual int GetN9() const { return nsel[1]; }
            virtual void SetN9(int _n9) { nsel[1] = _n9; }
            
            virtual void GetBonsaiDir(float *bd)
            {
                bd[0] = bonsai_dir[0]; bd[1] = bonsai_dir[1];
                bd[2] = bonsai_dir[2]; bd[3] = bonsai_dir[3]; bd[4] = bonsai_dir[4];
            }
            virtual void SetBonsaiDir(float *bd)
            {
                bonsai_dir[0] = bd[0];	bonsai_dir[1] = bd[1];
                bonsai_dir[2] = bd[2];	bonsai_dir[3] = bd[3];	bonsai_dir[4] = bd[4];
            }
            
            virtual double GetDirGoodness() const { return goodness[2]; }
            virtual void SetDirGoodness(double _goodness) { goodness[2] = _goodness; }
            
            //Since rat-pac does not separate the inner-detector and outer-detector (ID and OD) pmts
            //in an intuitive fashion, record it here to highlight actual IDhits bonsai will use. 
            
            virtual int GetIDHit() const { return IDHit; }
            virtual void SetIDHit(int _idHit) { IDHit = _idHit; }
            
            virtual int GetODHit() const { return ODHit; }
            virtual void SetODHit(int _odHit) { ODHit = _odHit; }
            
            
            virtual double GetIDCharge() const { return IDCharge; }
            virtual void SetIDCharge(double _idCharge) { IDCharge = _idCharge; }
            
            virtual double GetODCharge() const { return ODCharge; }
            virtual void SetODCharge(double _odCharge) { ODCharge = _odCharge; }
            
            ClassDef(BonsaiFit, 3)
            
        protected:
            TVector3 dir;
            double time,goodness[3],loglike[2];
            float  bonsai_dir[5];
            int    nsel[3];
            int  IDHit,ODHit;
            double IDCharge,ODCharge;
        };
        
    } // namespace DS
    
} // namespace RAT

#endif

