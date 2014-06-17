#include <RAT/GeoSolidFactory.hh>
#include <G4Material.hh>
#include <RAT/Materials.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4VisAttributes.hh>

using namespace std;

namespace RAT {
  
  G4VPhysicalVolume *GeoSolidFactory::Construct(DBLinkPtr table)
  {
    detail << "GeoSolidFactory: Constructing volume " << table->GetIndex() << newline;

    G4VSolid *solid = ConstructSolid(table);

    string mother_name = table->GetS("mother");
    G4LogicalVolume *mother;
    if (mother_name == "")
      mother = 0; // World volume has no mother
    else {
      mother = FindMother(mother_name);
      if (mother == 0)
	      Log::Die("Unable to find mother volume \"" + mother_name + "\" for "
		    + table->GetName() + "[" + table->GetIndex() + "]");
    }

    G4LogicalVolume *logi = ConstructLogicalVolume(solid, table);
    SetSensitive(logi, table); // Set sensitive volume if applicable

    bool replicas = false;
    try {
      replicas = table->GetI("replicas") > 1;
    } catch (DBNotFoundError &e) { replicas = false; }

    G4VPhysicalVolume *phys;
    if (replicas)
      phys = ConstructPhysicalReplica(logi, mother, table);
    else
      phys = ConstructPhysicalVolume(logi, mother, table);
    return phys;
  } 

  G4LogicalVolume *GeoSolidFactory::ConstructLogicalVolume(G4VSolid *solid,
  						       DBLinkPtr table)
  {
    string volume_name = table->GetIndex();
    string material_name = table->GetS("material");
    G4LogicalVolume *lv = NULL;
    if(material_name == "G4_Gd"){
      G4NistManager* man = G4NistManager::Instance();   
      lv = new G4LogicalVolume(solid,
  			    man->FindOrBuildMaterial("G4_Gd"),
  			    volume_name);
    }
    else{
      lv = 
      new G4LogicalVolume(solid,
  			G4Material::GetMaterial(material_name),
  			volume_name);
    }

    // Create optional skin surface for volume.  For more complex surface
    // selection you need to write a separate factory
    try {
      string surface_name = table->GetS("surface");
      if (Materials::optical_surface.count(surface_name) == 0)
        Log::Die("GeoSolidFactory: Error building "+volume_name+", surface "
                 + surface_name + " does not exist");
      new G4LogicalSkinSurface(volume_name+"_surface", lv,
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

    // Allow voxel optimization to be selectively disabled
    // GEANT4 default is to always optimize
    try {
      int optimize = table->GetI("optimize");
      if (optimize == 0) {
        lv->SetOptimisation(false);
        info << "GeoSolidFactory: Voxelization disabled for " << table->GetName()
             << "[" << table->GetIndex() << "]" << newline;
      }
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

    try {
      int force_auxedge = table->GetI("force_auxedge");
      vis->SetForceAuxEdgeVisible(force_auxedge == 1);
    } catch (DBNotFoundError &e) { };

    lv->SetVisAttributes(vis);

    // Check for invisible flag last
    try {
      int invisible = table->GetI("invisible");
      if (invisible == 1)
        lv->SetVisAttributes(G4VisAttributes::Invisible);
    } catch (DBNotFoundError &e) { };


    return lv;
  }

  
  
  
  
} // namespace RAT

