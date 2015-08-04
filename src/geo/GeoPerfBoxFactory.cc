#include <RAT/GeoPerfBoxFactory.hh>
#include <RAT/Log.hh>
#include <G4SubtractionSolid.hh>
#include <G4RotationMatrix.hh>
#include <G4ThreeVector.hh>
#include <G4Tubs.hh>
#include <G4Box.hh>
#include <cmath>
#include <sstream>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT{

  G4VSolid* GeoPerfBoxFactory::ConstructSolid(DBLinkPtr table){
    string volumeName = table->GetIndex();

    // get the base box
    vector<double> size = table->GetDArray("size");
    double thickness = 0.0;
    try{ thickness = table->GetD("thickness") * CLHEP::mm; }
    catch(DBNotFoundError& e){}
    for(unsigned i=0; i<size.size(); i++){
      size[i] *= CLHEP::mm;
      if(size[i] < thickness)
	Log::Die("GeoPerfBoxFactory: thickness larger than dimension");
    }
    G4VSolid* box = new G4Box(volumeName, size[0], size[1], size[2]);
    if(thickness > 0.0){
      G4VSolid* cut = new G4Box(volumeName + "_cut", size[0] - thickness, 
				size[1] - thickness, size[2] - thickness);
      box = new G4SubtractionSolid(volumeName, box, cut, 
				   0, G4ThreeVector(0.0, 0.0, 0.0));
    }

    // cut out the holes
    vector<double> rhole = table->GetDArray("r_hole");
    vector<double> xhole = table->GetDArray("x_hole");
    vector<double> yhole = table->GetDArray("y_hole");
    if(rhole.size() != xhole.size() || rhole.size() != yhole.size())
      Log::Die("GetPerfBoxFactory: hole parameter arrays not same size");
    for(unsigned i=0; i<rhole.size(); i++){
      rhole[i] *= CLHEP::mm;
      xhole[i] *= CLHEP::mm;
      yhole[i] *= CLHEP::mm;
      stringstream ss;
      ss << i;
      G4VSolid* tube = new G4Tubs(volumeName + ss.str(),
				  0.0, rhole[i], size[2]*1.01, 0.0, CLHEP::twopi);
      box = new G4SubtractionSolid(volumeName, box, tube,
				   0, G4ThreeVector(xhole[i], yhole[i], 0.0));
    }

    return box;
  }

}
      
    
