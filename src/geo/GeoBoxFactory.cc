#include <RAT/GeoBoxFactory.hh>
#include <G4Box.hh>

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

using namespace std;

namespace RAT {

G4VSolid *GeoBoxFactory::ConstructSolid(DBLinkPtr table)
{
  string volume_name        = table->GetIndex();
  const vector<double> &size = table->GetDArray("size");
  return new G4Box(volume_name, size[0] * mm, size[1] * mm, size[2] * mm);
}

} // namespace RAT
