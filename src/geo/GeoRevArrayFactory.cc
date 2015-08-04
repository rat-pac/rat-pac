#include <RAT/GeoRevArrayFactory.hh>
#include <RAT/PMTConstruction.hh>
#include <RAT/GeoPMTParser.hh>
#include <RAT/Log.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <G4SubtractionSolid.hh>
#include <G4Polycone.hh>
#include <G4Orb.hh>
#include <vector>

using namespace std;

namespace RAT {
    
    G4VPhysicalVolume *GeoRevArrayFactory::Construct(DBLinkPtr table)
    {
	
	string volume_name = table->GetIndex();

	G4int numZPlanes;

	const vector<double> &z = table->GetDArray("z");
	const vector<double> &r_max = table->GetDArray("r_max");
	const vector<double> &r_min = table->GetDArray("r_min");

	numZPlanes = G4int(z.size());

	if ((z.size() != r_max.size()) || (z.size() != r_min.size()) || (r_max.size() != r_min.size()))
	{
        	Log::Die("GeoRevArrayFactory::ConstructSolid: Tables z, r_max and r_min must all be same size for 'revolve'.");
	}
	
	// Optional parameters
	G4double phi_start = 0.0;
	try { phi_start = table->GetD("phi_start") * CLHEP::deg; }
	catch (DBNotFoundError &e) { };
	G4double phi_delta = CLHEP::twopi;
	try { phi_delta = table->GetD("phi_delta") * CLHEP::deg; }
	catch (DBNotFoundError &e) { };

	// can cut out a sphereical region from all the solids of 
	// radius sphere_cut_r
	G4double s_cut_r = -1.0;
	try { s_cut_r = table->GetD("sphere_cut_r") * CLHEP::mm; } 
	catch (DBNotFoundError &e) { };
	
	// can rescale Solid radius from mother volume center for
	// case where Solids have spherical layout symmetry
	G4double rescale_r = -1.0;
	try { rescale_r = table->GetD("rescale_radius") * CLHEP::mm; } 
	catch (DBNotFoundError &e) { };

	int preflip=0;
	try{preflip=table->GetI("preflip");}
	catch(DBNotFoundError &e){};

	// End optional parameters

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

    	G4VSolid *BaseSolid = new G4Polycone("temp_lg_1", phi_start, phi_delta, numZPlanes, z_array, r_min_array, r_array);

	if ((s_cut_r > 0)&&(rescale_r > 0))
	{
	    G4VSolid* sphere_cutter = new G4Orb("temp_sphere", s_cut_r);     // This is the cut out piece
	
	    G4RotationMatrix* sphererot = new G4RotationMatrix();

	    G4ThreeVector spherepos(0.0, 0.0, -1*rescale_r);
				      
	    BaseSolid = new G4SubtractionSolid(volume_name, BaseSolid, sphere_cutter, sphererot, spherepos);
	}

  if(preflip){
    G4RotationMatrix* fliprot=new G4RotationMatrix(G4ThreeVector(1,0,0),CLHEP::pi);
    BaseSolid=new G4DisplacedSolid(volume_name+"flipped",BaseSolid,fliprot,G4ThreeVector(0,0,0));
  }

	return GeoSolidArrayFactoryBase::Construct(BaseSolid, table);     
    }
    
} // namespace RAT



