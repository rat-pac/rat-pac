#ifndef __RAT_ConeWaveguideConstruction__
#define __RAT_ConeWaveguideConstruction__

#include <G4OpticalSurface.hh>
#include <G4VSolid.hh>
#include <G4Material.hh>

namespace RAT {

class ConeWaveguideConstruction {
public:
  ConeWaveguideConstruction(std::string table, std::string index);
  virtual G4VSolid *NewBodySolid(const std::string &name, G4VSolid* fPMTBody);
  virtual double GetHeight();
  virtual G4ThreeVector GetPlacementOffset();
protected:
  double fZTop, fZBottom;
  double fRadiusTop, fRadiusBottom;
  double fInnerRadiusTop, fInnerRadiusBottom;
  G4Material *fMaterial;
  G4OpticalSurface *fSurface;
};


} // namespace RAT

#endif
