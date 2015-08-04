#include <RAT/GeoTubeIntersectionFactory.hh>
#include <RAT/Log.hh>
#include <G4SubtractionSolid.hh>
#include <G4UnionSolid.hh>
#include <G4RotationMatrix.hh>
#include <G4ThreeVector.hh>
#include <G4Tubs.hh>
#include <G4Polycone.hh>
#include <cmath>
#include <sstream>

using namespace std;

namespace RAT{

  G4VSolid* GeoTubeIntersectionFactory::ConstructSolid(DBLinkPtr table){
    string volumeName = table->GetIndex();
    
    // parameters for the base tube
    vector<double> posz = table->GetDArray("pos_z");
    vector<double> rmax = table->GetDArray("r_max");
    vector<double> rmin;
    bool zeror;
    try{ 
      rmin = table->GetDArray("r_min"); 
      zeror = false;
    }
    catch(DBNotFoundError& e){ 
      rmin = vector<double>(posz.size(), 0.0); 
      zeror = true;
    }    
    double phiStart = 0.0;
    try{phiStart = table->GetD("phi_start") * CLHEP::deg;}
    catch(DBNotFoundError &e){};
    double phiDelta = CLHEP::twopi;
    try{phiDelta = table->GetD("phi_delta") * CLHEP::deg;}
    catch(DBNotFoundError &e){};
    if(posz.size() != rmax.size() || posz.size() != rmin.size())
      Log::Die("GeoTubeIntersectionFactory: base tube arrays"
	       " do not have equal lengths");
    if(posz.size() < 2)
      Log::Die("GeoTubeIntersectionFactory: tube arrays must have length > 1");
    for(unsigned i=0; i<posz.size(); i++){
      posz[i] *= CLHEP::mm;
      rmax[i] *= CLHEP::mm;
      rmin[i] *= CLHEP::mm;
    }

    // parameters for the intersecting tubes
    vector<double> poszInter = table->GetDArray("pos_z_inter");
    vector<double> phiInter = table->GetDArray("phi_inter");
    if(poszInter.size() != phiInter.size())
      Log::Die("GeoTubeIntersectionFactory: intersecton parameter arrays"
	       " do not have same lengths");
    vector<double> sizezInter = table->GetDArray("size_z_inter");
    vector<double> rmaxInter = table->GetDArray("r_max_inter");
    vector<double> rminInter(sizezInter.size(), 0.0);
    try{rminInter = table->GetDArray("r_min_inter");}
    catch(DBNotFoundError& e){}
    vector<double> dispInter(sizezInter.size(), 0.0);
    try{dispInter = table->GetDArray("disp_inter");}
    catch(DBNotFoundError& e){}
    if(poszInter.size() != poszInter.size() || 
       rmaxInter.size() != poszInter.size() ||
       rminInter.size() != poszInter.size() ||
       dispInter.size() != poszInter.size())
      Log::Die("GeoTubeIntersectionFactory: intersecton parameter arrays"
	       " do not have equal lengths");

    vector<bool> inter = vector<bool>(poszInter.size(), true);
    try{ 
      vector<int> includeInter = table->GetIArray("include_inter"); 
      if(poszInter.size() != includeInter.size())
	Log::Die("GeoTubeIntersectionFactory: intersecton parameter arrays"
		 " do not have equal lengths");
      for(unsigned i=0; i<includeInter.size(); i++)
	inter[i] = includeInter[i];
    }
    catch(DBNotFoundError& e){}
    
    // construct the base revolved solid
    G4double* poszArray = new G4double[posz.size()];
    G4double* rmaxArray = new G4double[rmax.size()];
    G4double* rminArray = new G4double[rmin.size()];
    for(unsigned i=0; i<posz.size(); i++){
      poszArray[i] = posz[i];
      rmaxArray[i] = rmax[i];
      rminArray[i] = rmin[i];
    }

    G4VSolid* baseTube;
    if(posz.size() == 2)
      baseTube = new G4Tubs(volumeName, rmin[0], rmax[0],
			    abs(posz[1]-posz[0])/2, phiStart, phiDelta);
    else if(zeror)
	baseTube = new G4Polycone(volumeName, phiStart, phiDelta,
				 posz.size(), rmaxArray, poszArray);
    else
      baseTube= new G4Polycone(volumeName, phiStart, phiDelta,
			       posz.size(), poszArray, rminArray, rmaxArray);

    for(unsigned i=0; i<poszInter.size(); i++){
      sizezInter[i] *= CLHEP::mm;
      rmaxInter[i] *= CLHEP::mm;
      rminInter[i] *= CLHEP::mm;
      poszInter[i] *= CLHEP::mm;
      dispInter[i] *= CLHEP::mm;
      phiInter[i] *= CLHEP::deg;

      int j1 = -1;
      int j2 = -1;
      for(int j=0; j<(int)posz.size()-1; j++){
	if(poszInter[i] >= posz[j] && poszInter[i] < posz[j+1]){
	  j1 = j;
	  j2 = j+1;
	  break;
	}
	else if(poszInter[i] < posz[j] && poszInter[i] >= posz[j+1]){
	  j1 = j+1;
	  j2 = j;
	  break;
	}
      }
      if(j1 == -1 || j2 == -1)
	Log::Die("GeoTubeIntersectionFactory: intersection point out of "
		 "base sold's z range");
      
      double rcenter = (rmax[j2]-rmax[j1]) * (poszInter[i]-posz[j1]);
      rcenter /= posz[j2] - posz[j1];
      rcenter += rmax[j1] + dispInter[i];

      double minr = (rmin[j2]-rmin[j1]) * (poszInter[i]-rmaxInter[i]-posz[j1]);
      minr /= posz[j2] - posz[j1];
      minr += rmin[j1] + dispInter[i];
      if(minr == 0.0)
	minr = rcenter;

      double dr =  minr * (1 - sqrt(1 - pow(fmin(rmaxInter[i]/minr, 1.0), 2)));
      dr += rcenter - minr;
      sizezInter[i] += dr / 2;

      stringstream ss;
      ss << i;
      G4VSolid* interTube = new G4Tubs(volumeName + "_inter_" + ss.str(),
				      rminInter[i],rmaxInter[i], sizezInter[i],
				      0.0, CLHEP::twopi);
      double cutz = sizezInter[i];
      if(dispInter[i] == 0.0)
	cutz *= 1.0 + 1e-6;
      G4VSolid* cutTube = new G4Tubs(volumeName + "_cut_" + ss.str(),
				     0.0, rmaxInter[i], cutz,
				     0.0, CLHEP::twopi);
				
      // put the inter on the x-axis, then rotate
      G4RotationMatrix* rotate = new G4RotationMatrix();
      rotate->rotateZ(-phiInter[i]);
      rotate->rotateY(90 * CLHEP::deg);
      G4ThreeVector translate((rcenter+sizezInter[i]-dr) * cos(phiInter[i]), 
			      (rcenter+sizezInter[i]-dr) * sin(phiInter[i]),
			      poszInter[i]);
      
      if(minr < rcenter)
	baseTube = new G4SubtractionSolid(volumeName, baseTube, cutTube,
					  rotate, translate);
      if(inter[i]){
	baseTube = new G4UnionSolid(volumeName, baseTube, interTube,
				    rotate, translate);
	if(minr < rcenter){
	  G4VSolid* innerTube;
	  if(posz.size() == 2)
	    innerTube = new G4Tubs(volumeName + "_inner",
				   0.0, rmin[0], abs(posz[1]-posz[0])/2, 
				   phiStart, phiDelta);
	  else
	    innerTube = new G4Polycone(volumeName + "_inner", 
				       phiStart, phiDelta,
				       posz.size(),  rminArray, poszArray);
	  baseTube = new G4SubtractionSolid(volumeName + "_inter" + ss.str(),
					    baseTube, innerTube,
					    0, G4ThreeVector(0.0, 0.0, 0.0));
	}
      }
    }

    return baseTube;
  }

}
    

