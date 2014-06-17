#include "RAT/PolygonOrientation.hh"

#include <vector>
#include <G4TwoVector.hh>
#include <assert.h>

namespace RAT {

    int CheckOrientation(std::vector<G4TwoVector> &G4Polygon)
    {
	int NVertices = G4Polygon.size();

	//Check at least 3 vertices
	assert(NVertices>=3);

	//Find vectors defining surface
	G4TwoVector v1 = G4Polygon[2]-G4Polygon[0];
	G4TwoVector v2 = G4Polygon[2]-G4Polygon[1];

	//Find determinant
	double determinant = v1.x()*v2.y()-v1.y()*v2.x();

	if (determinant > 0)
	{
	    std::vector<G4TwoVector> temp(NVertices);
	    for (int i=0;i<NVertices;i++)
	    {
		temp[i]=G4Polygon[NVertices-1-i];
	    }
	    G4Polygon = temp;
	}

	return 1;
    }


} // namespace RAT
