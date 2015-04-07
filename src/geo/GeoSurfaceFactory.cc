#include <RAT/GeoSurfaceFactory.hh>
#include <G4Material.hh>
#include <RAT/Materials.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4VisAttributes.hh>
#include <sstream>
#include <iostream>

using namespace std;

namespace RAT {

string ConvertIntToString(int i)
{
  stringstream sstream;
  sstream << i;
  return sstream.str();
}

G4VPhysicalVolume *GeoSurfaceFactory::Construct(DBLinkPtr table) {
  detail << "GeoSurfaceFactory: Constructing border " << table->GetIndex() << newline;

  string border_name = table->GetIndex();
  string volume1_name, volume2_name;
  
  bool isArray1=false, isArray2=false;
  try {
    volume1_name = table->GetS("volume1");
  } catch (DBNotFoundError &e) {
    Log::Die("Unable to find the first volume");
  };

  RAT::DBLinkPtr table1;
  string type1;
  try {
    table1=DB::Get()->GetLink("GEO",volume1_name);  
    type1=table1->GetS("type");
    if (type1.find("array") !=string::npos) isArray1=true;
  } catch(DBNotFoundError &e) {
  }

  try {
    volume2_name = table->GetS("volume2");
  } catch (DBNotFoundError &e) {
    Log::Die("Unable to find the second volume");
  };

  DBLinkPtr table2;
  string type2;
  try {
    table2 =DB::Get()->GetLink("GEO",volume2_name);
    type2 = table2->GetS("type");
    if (type2.find("array") !=string::npos) isArray2=true;
  }catch(DBNotFoundError &e) {
  }

  G4VPhysicalVolume* Phys1, *Phys2;
  G4int counter =0;
  do {
    Phys1=NULL;
    Phys2=NULL;
    
    /// looking for the first volume (FindPhysMother is misnamed)
    Phys1 = FindPhysMother(volume1_name + ((isArray1)? "_"+ConvertIntToString(counter) : "") );

    if (!Phys1){
      if (isArray1){ 
        if (counter==0) Log::Die(volume1_name+" not found");
        else break;
      }
      else Log::Die(volume1_name+" not found");
    }
    
    /// looking for the second volume (FindPhysMother is misnamed)
    Phys2 = FindPhysMother(volume2_name + ((isArray2)? "_"+ConvertIntToString(counter) : ""));
    if (!Phys2){
      if (isArray2){ 
        if (counter==0) Log::Die(volume2_name+" not found");
        else break;
      }
      else Log::Die(volume2_name+" not found");
    }
//     if (!Phys2) Log::Die(volume2_name+" not found");
  
    try {
      string surface_name = table->GetS("surface");
      if (Materials::optical_surface.count(surface_name) == 0)
        Log::Die("GeoSurfaceFactory: Error building "+border_name+", surface "
                + surface_name + " does not exist");
      new G4LogicalBorderSurface("interface"+volume1_name+"_"+volume2_name,
				 Phys1,Phys2,
				 Materials::optical_surface[surface_name]);
      int reverse=0;
      try {reverse=table->GetI("reverse");
      } catch (DBNotFoundError &e) {};
      if(reverse)
        new G4LogicalBorderSurface("interface"+volume2_name+"_"+volume1_name,
				   Phys2,Phys1,
				   Materials::optical_surface[surface_name]);
    } catch (DBNotFoundError &e) {
      Log::Die("No surface name defined");
    };
    counter++;

  } while (isArray1 || isArray2);
  return 0;
}

} // namespace RAT

