#include <RAT/GeoConvexLensFactory.hh>
#include <G4Sphere.hh>
#include <G4Tubs.hh>
#include <G4SubtractionSolid.hh>
#include <RAT/Log.hh>
#include "G4IntersectionSolid.hh"
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {

G4VSolid *GeoConvexLensFactory::ConstructSolid(DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  G4double R1 = table->GetD("R1") * CLHEP::mm;
  G4double thickness = table->GetD("thickness") * CLHEP::mm;

  // Optional parameters
  G4double R2 = R1;
  try {R2=table->GetD("R2")*CLHEP::mm;} 
  catch (DBNotFoundError &e){};
	G4double diameter=R1;
	if(R2>R1)
		diameter=R2;
  try {diameter=table->GetD("diameter")*CLHEP::mm;} 
  catch (DBNotFoundError &e){};
  // end optional parms
  
  
  if ((R1<thickness) || (R2<thickness)) 
    Log::Die("GeoBuilder error (from Convex Lens Factory): curvature Radii must be > thickness");

  //ensure median plane of the lens to be at x=0
  double f=.5*(2*R1-thickness)/(R2+R1-thickness);
  G4VSolid* left=new G4DisplacedSolid("left",new G4Sphere("leftsp",0.,R1,0.,360.*CLHEP::deg,0.,180.*CLHEP::deg),0,G4ThreeVector(0,0,f*thickness-R1));
  G4VSolid* right=new G4DisplacedSolid("right",new G4Sphere("rightsp",0.,R2,0.,360.*CLHEP::deg,0.,180.*CLHEP::deg),0,G4ThreeVector(0,0,R2-(1-f)*thickness));
  G4VSolid* lens=new G4IntersectionSolid(volume_name,left,right);
	lens=new G4SubtractionSolid("lens",lens,new G4Tubs("cutborder",diameter*.5,R1+R2,R1+R2,0,360.*CLHEP::deg));

  return lens;

}

} // namespace RAT
