#include <GeoWaterBoxArrayFactory.hh>
#include <RAT/WaterBoxConstruction.hh>

#include <RAT/DB.hh>
#include <RAT/Log.hh>

#include <G4PVPlacement.hh>

#include <RAT/Materials.hh>


using namespace std;

namespace RAT {

  /// \brief Makes an array of waterbox physical volumes
  ///
  /// makes an array of waterbox physical volumes
  G4VPhysicalVolume *GeoWaterBoxArrayFactory::Construct(DBLinkPtr table) {
    info << "GeoWaterBoxArrayFactory: Constructing volume "+ table->GetIndex()  
         << newline;
   
    string volume_name = table->GetIndex();
    WaterBoxConstruction waterBox;
    G4LogicalVolume* waterBox_log = waterBox.ConstructLogicalVolume(table);
    
    string mother_name = table->GetS("mother");
    G4LogicalVolume *mother = FindMother(mother_name);
    if (mother == 0)
      Log::Die("Unable to find mother volume " + mother_name + " for "
	       + volume_name);
    G4VPhysicalVolume* phys_mother = FindPhysMother(mother_name);
    if (phys_mother == 0)
      Log::Die("GeoBuilder error: PMT mother physical volume " + mother_name
	       +" not found");
    
    string pos_table_name = table->GetS("waterbox_locations");
    DBLinkPtr lpos_table = DB::Get()->GetLink(pos_table_name);
    const vector<double> &box_x = lpos_table->GetDArray("x");
    const vector<double> &box_y = lpos_table->GetDArray("y");
    const vector<double> &box_z = lpos_table->GetDArray("z");

    unsigned nboxes = box_x.size();
    if( box_y.size() != nboxes ) {
      Log::Die("GeoWaterBoxArrayFactory: Table " + pos_table_name +
	       " has bad table of positions. Different number of x and y positions.");
    }
    if( box_z.size() != nboxes ) {
      Log::Die("GeoWaterBoxArrayFactory: Table " + pos_table_name +
	       " has bad table of positions. Different number of x and z positions.");
    }

    G4ThreeVector position;
    G4String pvp_string;
    for(unsigned i=0 ; i<nboxes ; i++ ) {
      pvp_string = "waterbox_" + ::to_string(i);
      position.setX(box_x[i]);
      position.setY(box_y[i]);
      position.setZ(box_z[i]);
      new G4PVPlacement(0, // no rotation
			position,
			waterBox_log, // the water box logical volume
			pvp_string, // a name for Geant 4, unique
			mother, // logical mother volume
			0, // currently not used according to docs
			i, // pCopyNo, the first should be (and is) 0
			0);
    }


    return NULL;

  }



} // namespace rat
