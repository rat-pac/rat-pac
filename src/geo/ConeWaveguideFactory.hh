#ifndef __RAT_ConeWaveguideFactory__
#define __RAT_ConeWaveguideFactory__

#include <RAT/WaveguideFactory.hh>
#include <G4OpticalSurface.hh>

namespace RAT {

class ConeWaveguideFactory : public WaveguideFactory {
public:
  ConeWaveguideFactory() : WaveguideFactory() { };
  virtual void SetTable(std::string table, std::string index="");
  virtual G4LogicalVolume *Construct(const std::string &name,
                                     G4LogicalVolume *mother, bool invisible=false);
  virtual G4ThreeVector GetPlacementOffset();
  virtual double GetZTop();
  virtual double GetRadius();
protected:
  double fZTop, fZBottom;
  double fRadiusTop, fRadiusBottom;
  double fInnerRadiusTop, fInnerRadiusBottom;
  G4Material *fMaterial;
  G4OpticalSurface *fSurface;
};


} // namespace RAT

#endif
