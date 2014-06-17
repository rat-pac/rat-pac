#ifndef __RAT_WaveguideFactory__
#define __RAT_WaveguideFactory__

#include <G4LogicalVolume.hh>
#include <RAT/DB.hh>

namespace RAT {

class WaveguideFactory {
public:
  WaveguideFactory() : fPMTBody(0) { };
  virtual void SetTable(std::string table, std::string index="") { 
    fTable = DB::Get()->GetLink(table, index); 
  };
  virtual void SetPMTBodySolid(G4VSolid *pmtBody) { fPMTBody = pmtBody; };
  virtual G4LogicalVolume *Construct(const std::string &name,
                                     G4LogicalVolume *mother, bool invisible=false) = 0;
  virtual G4ThreeVector GetPlacementOffset() { return G4ThreeVector(0.,0.,0.); };
  virtual double GetZTop() = 0;
  virtual double GetRadius() = 0;
protected:
  G4VSolid *fPMTBody;
  DBLinkPtr fTable;
};


} // namespace RAT

#endif
