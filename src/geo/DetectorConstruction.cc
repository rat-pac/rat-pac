#include <RAT/DetectorConstruction.hh>

#include <string>
#include <RAT/DB.hh>
#include <RAT/Log.hh>
#include <RAT/GeoBuilder.hh>
#include <RAT/Materials.hh>
#include <TVector3.h>

#include <G4GeometryManager.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4LogicalVolumeStore.hh>
#include <G4SolidStore.hh>

#include <G4SDManager.hh>
#include <BWVetGenericChamber.hh>
#include <RAT/PhotonThinning.hh>

using namespace std;

namespace RAT {

DetectorConstruction *DetectorConstruction::fDetectorConstruction = 0;

DetectorConstruction::DetectorConstruction(){
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Load the DETECTOR table
  DB* db = DB::Get();
  DBLinkPtr ldetector = db->GetLink("DETECTOR");

  string glg4data = "";
  string experiment = "";
  if(getenv("GLG4DATA") != NULL)
    glg4data = string(getenv("GLG4DATA")) + "/";
  // Load experiment RATDB files before doing anything else
  try{
    experiment = ldetector->GetS("experiment");
    info << "Loading experiment-specific RATDB files for: "
         << experiment << newline;
    db->LoadAll(glg4data + experiment);
  }
  catch(DBNotFoundError& e){
    info << "No experiment-specific tables loaded." << newline;
  }

  string geo_file = ldetector->GetS("geo_file");
  info << "Loading detector geometry from " << geo_file << newline;
  if(db->Load(geo_file) == 0)
    Log::Die("DetectorConstruction: Could not open detector geometry");

  info << "Constructing detector materials...\n";
  ConstructMaterials();

  // delete the old detector if we are constructing a new one
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
void DetectorConstruction::ConstructMaterials()
{
  Materials::ConstructMaterials();
  Materials::LoadOpticalSurfaces();
}

DetectorConstruction *DetectorConstruction::GetDetectorConstruction()
{
  if (!fDetectorConstruction)
    fDetectorConstruction = new DetectorConstruction();
  return fDetectorConstruction;
}

bool DetectorConstruction::GetPMTPosition(int pmtID, G4ThreeVector &pos){
  // FIXME
  TVector3 v(0.0, 0.0, 0.0);
  //bool val = DetectorConfig::GetDetectorConfig()->GetPMTPosition(pmtID, v);
  pos.setX(v.X());
  pos.setY(v.Y());
  pos.setZ(v.Z());
  return true;
  //return val;
}

vector<G4ThreeVector> DetectorConstruction::GetPMTPositions(){
  //vector<TVector3> tpos = DetectorConfig::GetDetectorConfig()->GetPMTPositions();
  vector<G4ThreeVector> pos;
  //for(unsigned i=0; i<tpos.size(); i++)
  //  pos.push_back(G4ThreeVector(tpos[i].X(), tpos[i].Y(), tpos[i].Z()));
  return pos;
}

bool DetectorConstruction::GetVetoPMTPosition(int vetoPMTID, G4ThreeVector &pos){
  TVector3 v(0.0, 0.0, 0.0);
  //bool val = DetectorConfig::GetDetectorConfig()->GetVetoPMTPosition(vetoPMTID, v);
  pos.setX(v.X());
  pos.setY(v.Y());
  pos.setZ(v.Z());
  return true;
  //return val;
}

vector<G4ThreeVector> DetectorConstruction::GetVetoPMTPositions(){
  //vector<TVector3> tpos = DetectorConfig::GetDetectorConfig()->GetVetoPMTPositions();
  vector<G4ThreeVector> pos;
  //for(unsigned i=0; i<tpos.size(); i++)
  //  pos.push_back(G4ThreeVector(tpos[i].X(), tpos[i].Y(), tpos[i].Z()));
  return pos;
}

}
