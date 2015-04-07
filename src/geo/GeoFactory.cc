#include <RAT/GeoFactory.hh>
#include <RAT/Log.hh>

#include <vector>

#include <G4SDManager.hh>

#include <G4PVPlacement.hh>
#include <G4PVReplica.hh>
#include <G4PhysicalVolumeStore.hh>

#include <G4LogicalVolume.hh>
#include <G4LogicalVolumeStore.hh>

using namespace std;


namespace RAT {


std::map<std::string, GeoFactory *> GeoFactory::fFactoryMap;

void GeoFactory::Register(const std::string &name, GeoFactory *factory)
{
  fFactoryMap[name] = factory;
}

G4VPhysicalVolume *GeoFactory::ConstructWithFactory(const std::string &name, DBLinkPtr table)
{
  if (fFactoryMap.count(name) == 0) throw GeoFactoryNotFoundError(name);
  else return fFactoryMap[name]->Construct(table);
}

GeoFactory::GeoFactory(const std::string &name)
{
  cout << "Registering " << name << endl;
  GeoFactory::Register(name, this);
}


void GeoFactory::SetSensitive(G4LogicalVolume *logi, DBLinkPtr table)
{
  try {
    string sensitive_detector = table->GetS("sensitive_detector");
    G4SDManager* sdman = G4SDManager::GetSDMpointer();
    G4VSensitiveDetector *sd = sdman->FindSensitiveDetector(sensitive_detector);
    if (sd)
      logi->SetSensitiveDetector(sd);
    else
      Log::Die("GeoFactory error: Sensitive detector "
               + sensitive_detector + " does not exist.\n");
  } catch (DBNotFoundError &e) { }
}


G4LogicalVolume *GeoFactory::FindMother(const std::string mother_name)
{
  G4LogicalVolumeStore* store = G4LogicalVolumeStore::GetInstance();

  for (vector<G4LogicalVolume*>::iterator i_volume = store->begin();
       i_volume != store->end(); ++i_volume) {
    G4LogicalVolume* testvolume = *i_volume;
    // Cast to G4String to avoid ambiguious overload
    if (testvolume->GetName() == G4String(mother_name)) {
      return testvolume;
    }
  }

  return 0;
}

G4VPhysicalVolume *GeoFactory::FindPhysMother(const std::string mother_name)
{
  G4PhysicalVolumeStore* store = G4PhysicalVolumeStore::GetInstance();

  for (vector<G4VPhysicalVolume*>::iterator i_volume = store->begin();
       i_volume != store->end(); ++i_volume) {
    G4VPhysicalVolume* testvolume = *i_volume;
    // Cast to G4String to avoid ambiguious overload
    if (testvolume->GetName() == G4String(mother_name))
      return testvolume;
  }

  return 0;
}


G4VPhysicalVolume *
GeoFactory::ConstructPhysicalVolume(G4LogicalVolume *logi,
                                    G4LogicalVolume *mother,
                                    DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  G4VPhysicalVolume *pv;

  // optional orienation and rotation, default is neither
  G4RotationMatrix *rotation = new G4RotationMatrix();
  try {
    const vector<double> &orientvector = table->GetDArray("orientation");
    G4ThreeVector soliddir;
    double angle_y = 0;
    double angle_x = 0;
    double angle_z = 0;
    soliddir.set(orientvector[0], orientvector[1], orientvector[2]);
    soliddir = soliddir.unit();
    angle_y = (-1.0)*atan2(soliddir.x(), soliddir.z());
    angle_x = atan2(soliddir.y(), sqrt(soliddir.x()*soliddir.x()+soliddir.z()*soliddir.z()));
    angle_z = atan2(-1*soliddir.y()*soliddir.z(), soliddir.x());
    rotation->rotateY(angle_y);
    rotation->rotateX(angle_x);
    rotation->rotateZ(angle_z);
  } catch (DBNotFoundError &e) { };

  try {
    const vector<double> &rotvector = table->GetDArray("rotation");
    rotation->rotateX(rotvector[0] * deg);
    rotation->rotateY(rotvector[1] * deg);
    rotation->rotateZ(rotvector[2] * deg);
  } catch (DBNotFoundError &e) { };

  // optional, default is position at center
  G4ThreeVector position(0.0, 0.0, 0.0);
  try {
    const vector<double> &posvector = table->GetDArray("position");
    position.setX(posvector[0] * mm);
    position.setY(posvector[1] * mm);
    position.setZ(posvector[2] * mm);
  } catch (DBNotFoundError &e) { };

  pv = new G4PVPlacement(rotation, position, logi, volume_name,
                         mother, false /*?*/, 0 /*?*/);

  return pv;
}

G4VPhysicalVolume *
GeoFactory::ConstructPhysicalReplica(G4LogicalVolume *logi,
                                     G4LogicalVolume *mother,
                                     DBLinkPtr table)
{
  string volume_name = table->GetIndex();
  G4VPhysicalVolume *pv;

  int replicas = table->GetI("replicas");
  string axis_str = table->GetS("replica_axis");
  EAxis axis = kXAxis;

  if (axis_str == "x")
    axis = kXAxis;
  else if (axis_str == "y")
    axis = kYAxis;
  else if (axis_str == "z")
    axis = kZAxis;
  else if (axis_str == "rho")
    axis = kRho;
  else if (axis_str == "phi")
    axis = kPhi;
  else
    Log::Die("GeoFactory error: Unknown replica axis " + axis_str);

  G4double replica_spacing = table->GetD("replica_spacing") * mm;

  pv = new G4PVReplica(volume_name,
                       logi,
                       mother,
                       axis,
                       replicas,
                       replica_spacing);

  return pv;
}

} // namespace RAT
