#include <RAT/PosGen_TriMeshSurface.hh>
#include <RAT/StringUtil.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>
#include <Randomize.hh>
#include <sstream>

namespace RAT{

  void PosGen_TriMeshSurface::GeneratePosition(G4ThreeVector &argResult){
    // choose a triangle
    double rndm = G4UniformRand() * totalArea;
    int itr = (int) area.size() - 1;
    while(area[itr-1] > rndm && itr > 0)
      itr --;
    double a0 = G4UniformRand();
    double a1 = G4UniformRand();
    if(a0 + a1 > 1){
      a0 = 1 - a0;
      a1 = 1 - a1;
    }
    argResult = point[itr] + a0*edge0[itr] + a1*edge1[itr];
    // figure out which type of triangle this is
    int itype = ntriangles.size() - 1;
    while(ntriangles[itype-1] > itr && itype > 0)
      itype --;
     // if this type has a thickness, add the offset parallel to the normal
    if(thickness[itype] > 0.0){
      rndm = G4UniformRand() * thickness[itype];
      if(direction[itype] == -1)
	rndm *= -1.0;
      else if(direction[itype] == 0)
	if(G4UniformRand() < 0.5)
	  rndm *= -1.0;
      argResult += rndm * normal[itr];
    }
  }

  void PosGen_TriMeshSurface::SetState(G4String newValues){
     state += newValues;
    // complain if there are no arguments
    newValues = strip_default(newValues);
    if(newValues.length() == 0){
      G4cout << "Current state of this PosGen_TriMeshSurface:\n"
	     << "\"" << GetState() << "\"\n" << G4endl;
      G4cout << "Format of arguments to run PosGen_TriMeshSurface: \n"
	"\"tableIndex optionalThickness_mm optionalDirection\"" << G4endl;
      return;
    }
    // read the new values
    istringstream is(newValues.c_str());
    string index;
    is >> index;
    if(is.fail())
      Log::Die("PosGen_TriMeshSurface: could not parse table index");
    double t = 0.0;
    int dir = 0;
    is >> t >> dir;
    if(t < 0.0)
      Log::Die("PosGen_TriMeshSurface: thickness must be greater than 0");
    thickness.push_back(t);
    if(dir < -1 || dir > 1)
      Log::Die("PosGen_TriMeshSurface: direction must be -1, 0, or 1");
    direction.push_back(dir);
    // get the vertices 
    DBLinkPtr ltable = DB::Get()->GetLink("TriMeshSurface", index);
    const vector<double> &x = ltable->GetDArray("x"); 
    const vector<double> &y = ltable->GetDArray("y"); 
    const vector<double> &z = ltable->GetDArray("z");
    // check for consistency
    if(x.size() != y.size() || y.size() != z.size())
      Log::Die("PosGen_TriMeshSurface: x, y, and z not the same length");
    else if((int) x.size() % 3 != 0)
      Log::Die("PosGen_TriMeshSurface: array length must be multiple of 3");
    else if(x.size() == 0)
      Log::Die("PosGen_TriMeshSurface: array has zero length");
    if(ntriangles.size() == 0)
      ntriangles.push_back((int) (x.size() / 3));
    else{
      int n = ntriangles[ntriangles.size() - 1];
      ntriangles.push_back(n + (int) x.size() / 3);
    }
    // add the triangles to the array
    for(unsigned i=0; i<x.size(); i+=3){
      G4ThreeVector p0(x[i], y[i], z[i]);
      G4ThreeVector p1(x[i+1], y[i+1], z[i+1]);
      G4ThreeVector p2(x[i+2], y[i+2], z[i+2]);
      G4ThreeVector e0 = p1 - p0;
      G4ThreeVector e1 = p2 - p0;
      G4ThreeVector n = e0.cross(e1);
      double a = 0.5 * n.mag();
      point.push_back(p0);
      edge0.push_back(e0);
      edge1.push_back(e1);
      normal.push_back(n.unit());
      area.push_back(totalArea + a);
      totalArea += a;
    }
  }

}
