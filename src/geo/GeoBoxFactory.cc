#include <RAT/GeoBoxFactory.hh>
#include <G4Box.hh>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {

G4VSolid *GeoBoxFactory::ConstructSolid(DBLinkPtr table)
{
  string volume_name        = table->GetIndex();
  const vector<double> &size = table->GetDArray("size");
  return new G4Box(volume_name, size[0] * CLHEP::mm, size[1] * CLHEP::mm, size[2] * CLHEP::mm);
}

} // namespace RAT
