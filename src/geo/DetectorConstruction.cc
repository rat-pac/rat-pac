#include <string>
#include <TVector3.h>
#include <G4GeometryManager.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4LogicalVolumeStore.hh>
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

}

