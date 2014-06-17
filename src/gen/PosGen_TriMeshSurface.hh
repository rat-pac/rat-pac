/* This position generator reads in the TriMeshSurface ratdb table and 
   generates positions uniformly on the surface of the triangle mesh.  
   Multiple triangle meshes can be read using /proc/pos/set multiple times.  
   Optionally, one can extrude the triangle a fixed thickness and generate 
   positions uniformly in that volume.  One can also extrude the volume in 
   the direction of the normal, in the opposite direction, or both where 
   the normal is defined by the counterclockwise orientation of the vertices.
 */

#ifndef __RAT_PosGen_TriMeshSurface__
#define __RAT_PosGen_TriMeshSurface__

#include <RAT/GLG4PosGen.hh>
#include <vector>

using namespace std;

namespace RAT{

  class PosGen_TriMeshSurface : public GLG4PosGen{
  public:
    PosGen_TriMeshSurface(const char* arg_dbname="triMeshSurface") 
      : GLG4PosGen(arg_dbname), totalArea(0.0) {}

    void GeneratePosition(G4ThreeVector &argResult);
    
    void SetState(G4String newValues);

    G4String GetState() const{ return state; };

  protected:
    vector<G4ThreeVector> point; // the first vertex
    vector<G4ThreeVector> edge0; // 2nd vertex minus first vertex
    vector<G4ThreeVector> edge1; // 3rd vertex minus first vertex
    vector<G4ThreeVector> normal;// normal vector with vertices counterclockwise
    vector<double> area;         // area of all triangles up to this one
    double totalArea;            
    vector<double> thickness;    // thickness of volume for each type
    vector<int> direction;       // direction to extrude 
    vector<int> ntriangles;      // number of triangles of this type
    G4String state;
  };

}

#endif
