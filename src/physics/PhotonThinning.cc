#include <RAT/DB.hh>
#include <RAT/PhotonThinning.hh>
#include <RAT/Log.hh>

namespace RAT {


double PhotonThinning::fThinFactor;

void PhotonThinning::Init()
{
  SetFactor(DB::Get()->GetLink("MC")->GetD("thin_factor"));
}

void PhotonThinning::SetFactor(double factor)
{
  if (factor < 1.0) {
    Log::Die(dformat("Cannot set photon thinning factor %f which is less than 1.0", factor));
  }
  fThinFactor = factor;
}


} // namespace RAT
