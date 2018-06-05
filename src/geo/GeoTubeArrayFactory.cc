#include <RAT/GeoTubeArrayFactory.hh>
#include <RAT/Log.hh>

#include <G4SubtractionSolid.hh>
#include <G4Tubs.hh>
#include <G4Orb.hh>
#include <vector>

using namespace std;

namespace RAT {
    
    G4VPhysicalVolume *GeoTubeArrayFactory::Construct(DBLinkPtr table)
    {
	
	string volume_name = table->GetIndex();
	G4double r_max = table->GetD("r_max") * CLHEP::mm;
	G4double size_z = table->GetD("size_z") * CLHEP::mm;   // this is the lenght of the LG face to the PMT closest point
	
	// Optional parameters
	G4double r_min = 0.0;
	try { r_min = table->GetD("r_min") * CLHEP::mm; } 
	catch (DBNotFoundError &e) { };
	G4double phi_start = 0.0;
	try { phi_start = table->GetD("phi_start") * CLHEP::deg; } 
	catch (DBNotFoundError &e) { };
	G4double phi_delta = CLHEP::twopi;
	try { phi_delta = table->GetD("phi_delta") * CLHEP::deg; } 
	catch (DBNotFoundError &e) { };

	// can cut out a sphereical region from all the solids of 
	// radius sphere_cut_r.
        // requires that rescale_r be set.
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
	
	G4VSolid* BaseSolid =  new G4Tubs(volume_name, r_min, r_max, size_z,phi_start, phi_delta);
	
	if ((s_cut_r > 0) && (rescale_r > 0))
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



