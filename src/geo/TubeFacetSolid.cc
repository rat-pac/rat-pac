
#include "TubeFacetSolid.hh"

#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4Cons.hh>
#include <G4Sphere.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>


using namespace std;

namespace RAT {
    
    G4VSolid *MakeFacetSolid(const G4String&           pName,
			     std::vector<G4TwoVector>  polygon,
			     G4double                  scale,
			     G4double                  hz,
			     G4double                  angle,
			     G4VSolid*                 basesolid)
    {
	
	G4int Nfac = polygon.size();
	G4double poly_max = 0.0 * CLHEP::mm;
	G4double poly_max_tmp = 0.0 * CLHEP::mm;
	for (G4int i = 0; i < Nfac; i++)
	{
	    poly_max_tmp = sqrt((polygon[i].x() * CLHEP::mm * polygon[i].x() * CLHEP::mm) + (polygon[i].y() * CLHEP::mm * polygon[i].y() * CLHEP::mm));
	    if (poly_max_tmp >= poly_max)
		poly_max = poly_max_tmp;
	}
	poly_max = poly_max * scale;	
	
	G4double l = poly_max;
	if (hz > l)
	    l = hz;
	
	l*=2;
    
	G4VSolid* cutter1 = new G4Box("temp", l, l, l);
    
	for (G4int i = 0; i < Nfac; i++)
	{
	    G4double x1 = polygon[i].x()*scale;
	    G4double y1 = polygon[i].y()*scale;
	    G4double x0, y0;
	    if (i==0)   // loop back to last element of polygon
	    {
		x0 = polygon[Nfac-1].x()*scale;
		y0 = polygon[Nfac-1].y()*scale; 
	    }
	    else        // use previous element of polygon
	    {
		x0 = polygon[i-1].x()*scale;
		y0 = polygon[i-1].y()*scale;
	    }
	    G4double x01 = (x0+x1)/2.0;
	    G4double y01 = (y0+y1)/2.0;

	    G4double theta2 = (CLHEP::pi/2+atan2(y1-y0,x1-x0));

	    G4double xbox = (x01+l/cos(angle)*cos(theta2));
	    G4double ybox = (y01+l/cos(angle)*sin(theta2));

	    G4double zbox = -hz;

	    G4ThreeVector boxtrans(xbox, ybox, zbox);	

	    G4RotationMatrix* boxrot = new G4RotationMatrix();
	    boxrot->rotateZ(CLHEP::pi-atan2(y1-y0,x1-x0));
	    boxrot->rotateX(-angle);

	    basesolid = new G4SubtractionSolid(pName, basesolid, cutter1, boxrot, boxtrans);
	} 
	return basesolid;
    }

    G4VSolid *MakeFacetSphereSolid(const G4String&           pName,
				   std::vector<G4TwoVector>  polygon,
				   std::vector<G4double>     angles,
				   G4double                  OR,
				   G4VSolid*                 basesolid)
    {
	G4VSolid* cutter1 = new G4Box("temp", 2*OR, 2*OR, 2*OR);
	G4int Nfac = polygon.size();
	for (G4int i = 0; i < Nfac; i++)
	{
	    G4double x1 = polygon[i].x();
	    G4double y1 = polygon[i].y();
	    G4double x0, y0;
	    if (i==0)   // loop back to last element of polygon
	    {
		x0 = polygon[Nfac-1].x();
		y0 = polygon[Nfac-1].y(); 
	    }
	    else        // use previous element of polygon
	    {
		x0 = polygon[i-1].x();
		y0 = polygon[i-1].y();
	    }

	    G4double theta2 = (CLHEP::pi/2+atan2(y1-y0,x1-x0));

	    G4double xbox = (2*OR/cos(angles[i])*cos(theta2));
	    G4double ybox = (2*OR/cos(angles[i])*sin(theta2));
	    G4double zbox = 0;

	    G4ThreeVector boxtrans(xbox, ybox, zbox);	

	    G4RotationMatrix* boxrot = new G4RotationMatrix();
	    boxrot->rotateZ(CLHEP::pi-atan2(y1-y0,x1-x0));
	    boxrot->rotateX(-angles[i]);

	    basesolid = new G4SubtractionSolid(pName, basesolid, cutter1, boxrot, boxtrans);
	} 
	return basesolid;
    }
      
    G4VSolid *MakeTubeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_min,
				 G4double                  r_max)
    {
	G4VSolid* basesolid = new G4Tubs(pName, r_min, r_max, hz, 0, CLHEP::twopi); 
    
	return MakeFacetSolid(pName, polygon, scale, hz, angle, basesolid);
    }



    G4VSolid *MakeTubeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_max)
    {
	return MakeTubeFacetSolid(pName, polygon, scale, hz, angle, 0, r_max);
    }
    
    G4VSolid *MakeConeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_min_1,
				 G4double                  r_max_1,
				 G4double                  r_min_2,
				 G4double                  r_max_2)
    {
        //G4double r_max = r_max_1;
        //if (r_max_2 > r_max_1)
        //    r_max = r_max_2;
	G4VSolid* basesolid = new G4Cons(pName, r_min_1, r_max_1, r_min_2, r_max_2, hz, 0, CLHEP::twopi); 
	
	return MakeFacetSolid(pName, polygon, scale, hz, angle, basesolid); 
    }
    
    G4VSolid *MakeConeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_max_1,
				 G4double                  r_max_2)
    {   
	return MakeConeFacetSolid(pName, polygon, scale, hz, angle, 0, r_max_1, 0, r_max_2); 
    }
    
    G4VSolid *MakeShellFacetSolid(const G4String&           pName,
				  std::vector<G4TwoVector>  polygon,
				  std::vector<G4double>     angles,
				  G4double                  IR,
				  G4double                  OR,
				  G4double                  dTheta)
    {
	G4VSolid* basesolid = new G4Sphere(pName, IR, OR, 0, CLHEP::twopi, 0, dTheta);
	return MakeFacetSphereSolid(pName, polygon, angles, OR, basesolid);
    }



    G4VSolid *MakeSphereFacetSolid(const G4String&           pName,
				   std::vector<G4TwoVector>  polygon,
				   std::vector<G4double>     angles,
				   G4double                  OR,
				   G4double                  dTheta)
    {
	return MakeShellFacetSolid(pName, polygon, angles, 0.0, OR, dTheta);
    }


    

} // namespace RAT
