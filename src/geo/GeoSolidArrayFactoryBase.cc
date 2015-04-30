#include <GeoSolidArrayFactoryBase.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>
#include <G4Material.hh>
#include <RAT/Materials.hh>
#include <G4VisAttributes.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4SDManager.hh>
#include <G4PVPlacement.hh>
#include <vector>

using namespace std;

namespace RAT {
    
    G4VPhysicalVolume *GeoSolidArrayFactoryBase::Construct(G4VSolid *BaseSolid, DBLinkPtr table)
    {
	
	string volume_name = table->GetIndex();

	G4Material *solid_material = G4Material::GetMaterial(table->GetS("material"));

	G4LogicalVolume *logiSolid = new G4LogicalVolume(BaseSolid, solid_material, volume_name);
	
	// Create optional skin surface for volume. 
	try {
	    string surface_name = table->GetS("surface");
	    if (Materials::optical_surface.count(surface_name) == 0)
		Log::Die("GeoSolidFactory: Error building "+volume_name+", surface "
			 + surface_name + " does not exist");
	    new G4LogicalSkinSurface(volume_name+"_surface", logiSolid,
				     Materials::optical_surface[surface_name]);
	} catch (DBNotFoundError &e) { };

	// Optional visualization parts
	G4VisAttributes *vis = new G4VisAttributes();
	try {
	    const vector<double> &color = table->GetDArray("color");
	    if (color.size() == 3) // RGB
		vis->SetColour(G4Colour(color[0], color[1], color[2]));
	    else if (color.size() == 4) // RGBA
		vis->SetColour(G4Colour(color[0], color[1], color[2], color[3]));
	    else
		warn << "GeoSolidFactory error: " << table->GetName() 
		     << "[" << table->GetIndex() << "].color must have 3 or 4 components"
		     << newline;
	} catch (DBNotFoundError &e) { };	
	try {
	    string drawstyle = table->GetS("drawstyle");
	    if (drawstyle == "wireframe")
		vis->SetForceWireframe(true);
	    else if (drawstyle == "solid")
		vis->SetForceSolid(true);
	    else
		warn << "GeoSolidFactory error: " << table->GetName()
		     << "[" << table->GetIndex() << "].drawstyle must be either \"wireframe\" or \"solid\".";
	} catch (DBNotFoundError &e) { };
	
	// Check for invisible flag last
	try {
	    int invisible = table->GetI("invisible");
	    if (invisible == 1)
		logiSolid->SetVisAttributes(G4VisAttributes::Invisible);
	} catch (DBNotFoundError &e) { };
	
	logiSolid->SetVisAttributes(vis);

	
	//  Set sensitive detector
	try { 
	    string sensitive_detector_name = table->GetS("sensitive_detector");
	    G4SDManager* sdman = G4SDManager::GetSDMpointer();  
	    G4VSensitiveDetector *sd = sdman->FindSensitiveDetector(sensitive_detector_name);
	    if (sd)
		logiSolid->SetSensitiveDetector(sd);
	    else
		Log::Die("GeoFactory error: Sensitive detector "
			 + sensitive_detector_name + " does not exist.\n");
	} catch (DBNotFoundError &e) { };


	return GeoSolidArrayFactoryBase::Construct(logiSolid, table);     
    }


    G4VPhysicalVolume *GeoSolidArrayFactoryBase::Construct(G4LogicalVolume *logiSolid, DBLinkPtr table)
    {
	string volume_name = table->GetIndex();
	
	// Read Solid positions
	string pos_table_name = table->GetS("pos_table");
	DBLinkPtr lpos_table = DB::Get()->GetLink(pos_table_name);
	const vector<double> &pos_x = lpos_table->GetDArray("x");
	const vector<double> &pos_y = lpos_table->GetDArray("y");
	const vector<double> &pos_z = lpos_table->GetDArray("z");


	// read max number of solids to use
	int max_solids = pos_x.size(); // default to read all
	try { max_solids = table->GetI("max_num"); }
	catch (DBNotFoundError &e) { }
	if (max_solids > (int) pos_x.size())
	    max_solids = pos_x.size();

	// read starting number of solids to use
	int start_solid_num = 0; // default to read all
	try {  start_solid_num = table->GetI("start_num"); }
	catch (DBNotFoundError &e) { }
	if (start_solid_num < 0)
	    start_solid_num = 0;

	// Try to see if a sub type has been specified
	int sub_type = -1; // default to read all
	try { sub_type = table->GetI("sub_type"); }
	catch (DBNotFoundError &e) { }
	
	vector<int> sub_type_array;
	for (int i = 0; i<max_solids; i++)
	   sub_type_array.push_back(-1);
	if (sub_type > -1)
	{
	    sub_type_array = lpos_table->GetIArray("sub_type"); 
	}
	
        // Find mother
	string mother_name = table->GetS("mother");

	// direction of individual solids.  Default is that +z is orientation pointing direction
	// optional, default is no rotation
	bool rot_manual = false;
	vector<double> rot_x, rot_y, rot_z;
	try { 
	    string rotate_str = table->GetS("rotate_solids");
	    if (rotate_str == "manual")
		rot_manual = true;
	} catch (DBNotFoundError &e) { }
	if (rot_manual) 
	{
	    rot_x = lpos_table->GetDArray("rot_x");
	    rot_y = lpos_table->GetDArray("rot_y");
	    rot_z = lpos_table->GetDArray("rot_z");
	} 

	// Orientation of Solids
	bool orient_manual = false;
	try { 
	    string orient_str = table->GetS("orientation");
	    if (orient_str == "manual")
		orient_manual = true;
	    else if (orient_str == "point")
		orient_manual = false;
	    else 
		Log::Die("GeoBuilder error: Unknown solid orientation " 
			 + orient_str);
	} catch (DBNotFoundError &e) { }

	vector<double> dir_x, dir_y, dir_z;
	vector<double> orient_point_array;
	G4ThreeVector orient_point;
	if (orient_manual) 
	{
	    dir_x = lpos_table->GetDArray("dir_x");
	    dir_y = lpos_table->GetDArray("dir_y");
	    dir_z = lpos_table->GetDArray("dir_z");
	} 
	else 
	{
	    orient_point_array = table->GetDArray("orient_point");
	    if (orient_point_array.size() != 3)
		Log::Die("GeoBuilder error: orient_point must have 3 values");
	    orient_point.set(orient_point_array[0], orient_point_array[1],
			     orient_point_array[2]);    
	}
	
	// Optionally can rescale Solid radius from mother volume center for
	// case where Solids have spherical layout symmetry
	bool rescale_radius = false;
	double new_radius = 1.0;
	try {
	    new_radius = table->GetD("rescale_radius");
	    rescale_radius = true;
	} catch (DBNotFoundError &e) { }
	
	
	// get pointer to physical mother volume
	G4VPhysicalVolume* phys_mother = GeoFactory::FindPhysMother(mother_name);
	if (phys_mother == 0)
	    Log::Die("GeoBuilder error: Solid mother physical volume " + mother_name
		     +" not found");
	
	for (int solidID = start_solid_num; solidID < max_solids; solidID++) 
	{
	    if ((sub_type == -1) || (sub_type == sub_type_array[solidID]))
	    {
		
		// name
		string solidname = volume_name + "_" + ::to_string(solidID);
		

		// position
		G4ThreeVector solidpos(pos_x[solidID], pos_y[solidID], pos_z[solidID]);
	
		// direction
		G4ThreeVector soliddir;
		if (orient_manual)
		    soliddir.set(dir_x[solidID], dir_y[solidID], dir_z[solidID]);
		else
		    soliddir = orient_point - solidpos;
		soliddir = soliddir.unit();
		
		// rescale
		if (rescale_radius)
		    solidpos.setMag(new_radius);

		// rotation required to point in direction of soliddir
		double angle_y = (-1.0)*atan2(soliddir.x(), soliddir.z());
		double angle_x = atan2(soliddir.y(), sqrt(soliddir.x()*soliddir.x()+soliddir.z()*soliddir.z()));
		double angle_z = atan2(-1*soliddir.y()*soliddir.z(), soliddir.x());
		
		G4RotationMatrix* solidrot = new G4RotationMatrix();
		
		solidrot->rotateY(angle_y);
		solidrot->rotateX(angle_x);
		solidrot->rotateZ(angle_z);
		
		if (rot_manual) 
		{
		    solidrot->rotateZ(rot_z[solidID] * CLHEP::deg);
		    solidrot->rotateY(rot_y[solidID] * CLHEP::deg);    
		    solidrot->rotateX(rot_x[solidID] * CLHEP::deg);
		}
		// ****************************************************************
		// * Use the constructor that specifies the PHYSICAL mother, since
		// * each Solid occurs only once in one physical volume.  This saves
		// * the GeometryManager some work. -GHS.
		// ****************************************************************
		new G4PVPlacement(solidrot,
				  solidpos,
				  solidname,
				  logiSolid,
				  phys_mother,
				  false,
				  solidID);
		
	    } // end loop over solidID
	}
	return 0;
    }
    
} // namespace RAT


