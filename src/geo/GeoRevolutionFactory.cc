#include <RAT/GeoRevolutionFactory.hh>
#include <G4Polycone.hh>
#include <RAT/Log.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {
    
G4VSolid *GeoRevolutionFactory::ConstructSolid(DBLinkPtr table)
{
    
    string volume_name = table->GetIndex();
        
    G4int numZPlanes;
    
    const vector<double> &z = table->GetDArray("z");
    const vector<double> &r_max = table->GetDArray("r_max");
    vector<double> r_min;
    bool solid;
    try{ 
      r_min = table->GetDArray("r_min"); 
      solid = false;
    }
    catch(DBNotFoundError& e){ 
      r_min = vector<double>(z.size(), 0.0); 
      solid = true;
    }
    
    numZPlanes = G4int(z.size());

    if ((z.size() != r_max.size()) || (z.size() != r_min.size()) || (r_max.size() != r_min.size())) 
    {
	Log::Die("GeoRevolutionFactory::ConstructSolid: Tables z, r_max and r_min must all be same size for 'revolve'.");	
    }


    // Optional parameters
    G4double phi_start = 0.0;
    try { phi_start = table->GetD("phi_start") * CLHEP::deg; } 
    catch (DBNotFoundError &e) { };
    G4double phi_delta = CLHEP::twopi;                  
    try { phi_delta = table->GetD("phi_delta") * CLHEP::deg; } 
    catch (DBNotFoundError &e) { };
    
    G4double *z_array;
    G4double *r_array;
    G4double *r_min_array;
    
    z_array = new G4double[numZPlanes];
    r_array = new G4double[numZPlanes];
    r_min_array = new G4double[numZPlanes];
    
    for ( G4int i=0; i < numZPlanes; ++i )
    {
	z_array[i] = z[i] * CLHEP::mm;
	r_array[i] = fabs(r_max[i]) * CLHEP::mm;
	r_min_array[i] = fabs(r_min[i]) * CLHEP::mm;
    }

    if(solid)
      return new G4Polycone(volume_name,
			    phi_start,
			    phi_delta,
			    numZPlanes,
			    r_array,
			    z_array);
    else
      return new G4Polycone(volume_name,                    
			    phi_start,
			    phi_delta,
			    numZPlanes,
			    z_array,
			    r_min_array,
			    r_array);
    
    
}
    
} // namespace RAT
