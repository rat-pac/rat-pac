#ifndef __RAT_PhotonThinning__
#define __RAT_PhotonThinning__

namespace RAT {


class PhotonThinning
{
public:
  static void Init();
  inline static double GetFactor() { return fThinFactor; };
  static void SetFactor(double factor);

protected:
  static double fThinFactor;
};


} // namespace RAT

#endif
