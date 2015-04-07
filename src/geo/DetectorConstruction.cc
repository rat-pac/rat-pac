#include <string>
#include <cstdlib>
#include <TVector3.h>
#include <G4GeometryManager.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4VPhysicalVolume.hh>
#include <G4SolidStore.hh>
#include <G4SDManager.hh>
#include <BWVetGenericChamber.hh>
#include <RAT/DetectorConstruction.hh>
#include <RAT/PhotonThinning.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>
#include <RAT/GeoBuilder.hh>
#include <RAT/Materials.hh>
#include <RAT/GLG4SimpleOpDetSD.hh>
#include <RAT/GeoFactory.hh>

using namespace std;

namespace RAT {

DetectorConstruction* DetectorConstruction::sDetectorConstruction = NULL;

G4VPhysicalVolume* DetectorConstruction::Construct() {
  // Load the DETECTOR table
  DB* db = DB::Get();
  DBLinkPtr ldetector = db->GetLink("DETECTOR");

  string glg4data = "";
  string experiment = "";
  if (getenv("GLG4DATA") != NULL) {
    glg4data = string(getenv("GLG4DATA")) + "/";
  }

  // Load experiment RATDB files before doing anything else
  try {
    experiment = ldetector->GetS("experiment");
    info << "Loading experiment-specific RATDB files for: "
         << experiment << newline;
    db->LoadAll(glg4data + experiment);
  }
  catch (DBNotFoundError& e) {
    info << "No experiment-specific tables loaded." << newline;
  }

  string geo_file = ldetector->GetS("geo_file");
  info << "Loading detector geometry from " << geo_file << newline;
  if (db->Load(geo_file) == 0) {
    Log::Die("DetectorConstruction: Could not open detector geometry");
  }

  info << "Constructing detector materials...\n";
  ConstructMaterials();

  // Delete the old detector if we are constructing a new one
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();

  // Add sensitive volumes here (only veto for now)
  G4SDManager* sdman = G4SDManager::GetSDMpointer();
  G4VSensitiveDetector *veto 
    =  new BWVetGenericChamber("/mydet/veto/genericchamber");  
  sdman->AddNewDetector(veto);

  // Setup photon thinning parameters
  PhotonThinning::Init();

  GeoBuilder geo;
  fWorldPhys = geo.ConstructAll();

  if ( geo.GetBuilderSource()==GeoBuilder::GDMLFILE ) {
    SetupGDMLSD();
  }

  info << "Dump Surface Info...\n";
  G4LogicalBorderSurface::DumpInfo();

  return fWorldPhys;
}


void DetectorConstruction::ConstructMaterials() {
  Materials::ConstructMaterials();
  Materials::LoadOpticalSurfaces();
}


DetectorConstruction* DetectorConstruction::GetDetectorConstruction() {
  if (!sDetectorConstruction) {
    sDetectorConstruction = new DetectorConstruction();
  }
  return sDetectorConstruction;
}

void DetectorConstruction::SetupGDMLSD() {
  DBLinkGroup lgeo = DB::Get()->GetLinkGroup("GEO");

  // find GDML table
  DBLinkGroup::iterator i_table;
  for (i_table = lgeo.begin(); i_table != lgeo.end(); ++i_table) {
    string name = i_table->first;
    DBLinkPtr table = i_table->second;
    string gdmlfilename;
    try {
      gdmlfilename = table->GetS("gdml_file");
    }
    catch (DBNotFoundError &e) {
      continue;
    }
    info << "GeoBuilder used GDML file as source of geometry." << newline;

    // opdet sd name
    G4String opdet_lv_name;
    try {
      opdet_lv_name = table->GetS("opdet_lv_name");
    }
    catch (DBNotFoundError &e) {
      // do nothing
      info << "Did not find 'opdet_lv_name'. Proceeding without OpDetSD assignment." << newline;
    }
    info << "Creating GLG4SimpleOpDetSD. Adding volumes as opdet channels for LVs with name='" << opdet_lv_name << "'" << newline;
    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    GLG4SimpleOpDetSD* opdetsd = new GLG4SimpleOpDetSD( "opdet_lv_name" );
    sdman->AddNewDetector(opdetsd);
    G4PhysicalVolumeStore* pvolumes = G4PhysicalVolumeStore::GetInstance();
    int nopdets = 0;
    for ( G4PhysicalVolumeStore::iterator it=pvolumes->begin(); it!=pvolumes->end(); it++) {
      G4VPhysicalVolume* volume = (*it);
      if ( volume->GetLogicalVolume()->GetName()==opdet_lv_name ) {
	G4String pvname = volume->GetName();
	volume->GetLogicalVolume()->SetSensitiveDetector( opdetsd );
	int channelid;
	size_t numstart,numend;
	try {
	  numstart = pvname.find_first_of("1234567890");
	  numend = pvname.find_first_not_of("1234567890",numstart+1);
	  channelid = atoi( pvname.substr(numstart, numend-numstart).c_str() );
	}
	catch (int e) {
	  Log::Die( "Error parsing OpDet physical volume name for channel ID. Need to place a number in the name." );
	}
	opdetsd->AddOpDetChannel( channelid, volume );
	nopdets += 1;
	//info << "Found OpDet instance. PVname=" << volume->GetName() << " ChannelID=" << channelid << newline;
      }
    }
    info << "Found OpDet " << nopdets << " instances." << newline;
    
    break;
  }
}

void DetectorConstruction::SetupGDMLSurfaces() {
  DBLinkGroup lgeo = DB::Get()->GetLinkGroup("GEO");

  // find GDML table
  DBLinkGroup::iterator i_table;
  for (i_table = lgeo.begin(); i_table != lgeo.end(); ++i_table) {
    string name = i_table->first;
    DBLinkPtr table = i_table->second;
    string type;

    // check for GDML entry
    string gdmlfilename;
    try {
      gdmlfilename = table->GetS("gdml_file");
    }
    catch (DBNotFoundError &e) {
    }
    if ( gdmlfilename!="" )
      continue;

    try {
      type = table->GetS("type");
    } catch (DBNotFoundError &e) {
      Log::Die("GeoBuilder error: volume " + name + " has no type");
    }

    if (type!="border")
      continue;

    try {
      info << "Make border: " << newline;
      GeoFactory::ConstructWithFactory(type, table);
    } catch (GeoFactoryNotFoundError &e) {
      Log::Die("GeoBuilder error: Cannot find factory for volume type "  + type);
    }
    
  }
}


}
