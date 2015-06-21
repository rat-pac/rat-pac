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

#include <RAT/DetectorFactory.hh>
#include <RAT/WatchmanDetectorFactory.hh>
#include <RAT/TheiaDetectorFactory.hh>

using namespace std;

namespace RAT {

DetectorConstruction* DetectorConstruction::sDetectorConstruction = NULL;

DetectorConstruction::DetectorConstruction() {
    DetectorFactory::Register("Watchman",new WatchmanDetectorFactory());
    DetectorFactory::Register("Theia",new TheiaDetectorFactory());
}

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

  try { 
    string detector_factory = ldetector->GetS("detector_factory");
    info << "Loading detector factory " << detector_factory << newline;
    DetectorFactory::DefineWithFactory(detector_factory,ldetector);
  } catch (DBNotFoundError &e) {
    try {
      string geo_file = ldetector->GetS("geo_file");
      info << "Loading detector geometry from " << geo_file << newline;
      if (db->Load(geo_file) == 0) {
        Log::Die("DetectorConstruction: Could not open detector geometry");
      }
    } catch (DBNotFoundError &e) {
        Log::Die("DetectorConstruction: Could not open geo_file or detector_factory");
    }
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

    // get name of logical volumes which will be sensitive surfaces
    std::vector<std::string> opdet_lv_names;
    try {
      opdet_lv_names = table->GetSArray("opdet_lv_name");
    }
    catch (DBNotFoundError &e) {
      try {
	std::string opdet_lv_name = table->GetS("opdet_lv_name");
	opdet_lv_names.push_back( opdet_lv_name );
      }
      catch (DBNotFoundError &e) {
	// do nothing
	info << "[WARNING] Did not find 'opdet_lv_name'. Proceeding without OpDetSD assignment." << newline;
      }
    }
    info << "Creating GLG4SimpleOpDetSD. Sensitive Logical Volumes include those whose names='";
    for ( std::vector<std::string>::iterator it=opdet_lv_names.begin(); it!=opdet_lv_names.end(); it++ )
      info << *it << " ";
    info << "'" << newline;
    // make sensitive detector
    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    GLG4SimpleOpDetSD* opdetsd = new GLG4SimpleOpDetSD( "opdet_lv_name" );
    sdman->AddNewDetector(opdetsd);

    // assign SD to logical volumes. We also make a list of physical volumes
    G4LogicalVolumeStore* lvolumes = G4LogicalVolumeStore::GetInstance();
    int nopdets = 0;
    for ( std::vector<std::string>::iterator it=opdet_lv_names.begin(); it!=opdet_lv_names.end(); it++ ) {
      G4LogicalVolume* lv = lvolumes->GetVolume( *it );
      if (lv) {
	lv->SetSensitiveDetector( opdetsd );	
	nopdets++;
	//std::cout << "Assigning as OpDetSD: " << lv->GetName() << std::endl;
      }
    }
    info << "Number of sensitive logical volumes found for GLG4SimpleOpDetSD: " << nopdets << newline;

    // get list of strings which we will use to define optical detector channels.
    // they will have to contain in their daughter volumes, a sensitive detector
    std::vector<std::string> opchannel_pv_names;
    try {
      opchannel_pv_names = table->GetSArray("opchannel_pv_name");
    }
    catch (DBNotFoundError &e) {
      try {
	std::string opchannel_pv_name = table->GetS("opchannel_pv_name");
        opchannel_pv_names.push_back( opchannel_pv_name );
      }
      catch (DBNotFoundError &e) {
        info << "[WARNING] Did not find 'opchannel_pv_name'." << newline;
      }
    }
    info << "Number of strings to search for GLG4SimpleOpDetSD Optical Channels: " << opchannel_pv_names.size() << newline;

    G4PhysicalVolumeStore* pvolumes = G4PhysicalVolumeStore::GetInstance();
    int nopchannels = 0;
    for ( G4PhysicalVolumeStore::iterator it=pvolumes->begin(); it!=pvolumes->end(); it++) {
      G4VPhysicalVolume* volume = (*it);
      bool found = false;
      std::string opchan_name;
      for ( std::vector<std::string>::iterator it_pvname=opchannel_pv_names.begin(); it_pvname!=opchannel_pv_names.end(); it_pvname++ )
	if ( volume->GetName().contains( *it_pvname ) ) {
	  opchan_name = *it_pvname;
	  found = true;
	  break;
	}
      
      G4LogicalVolume* lv = volume->GetLogicalVolume();
      if (found) {
	//std::cout << "found opchan pv name match: " << opchan_name << std::endl;
	G4String pvname = volume->GetName();
	// now we have to find a daughter with optical channel
	// these needs to be a recursive function
	bool issensitive = false;
	if ( lv->GetSensitiveDetector()==opdetsd ) {
	  issensitive = true;
	}
	else {
	  for (G4int idaughter=0; idaughter<lv->GetNoDaughters(); idaughter++ ) {
	    G4LogicalVolume* lv_daughter = lv->GetDaughter(idaughter)->GetLogicalVolume();
	    if ( lv_daughter->GetSensitiveDetector()==opdetsd ) {
	      issensitive = true;
	      break;
	    }
	  }
	}
	//std::cout << " is sensitive: " << issensitive << std::endl;

	if ( issensitive ) {
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
	  nopchannels += 1;
	  info << "Found Optical Channel instance. PVname=" << volume->GetName() << " ChannelID=" << channelid << newline;
	}//end of is sensitive
      }//end of if found channel name
    }//loop over physical volumes
    info << "Found " << nopchannels << " Optical Channels." << newline;
  }//loop over geo links
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
