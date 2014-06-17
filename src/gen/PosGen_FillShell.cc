#include <RAT/PosGen_FillShell.hh>
#include <RAT/Log.hh>
#include <RAT/StringUtil.hh>
#include <GLG4VertexGen.hh>

#include <G4TransportationManager.hh>
#include <G4Navigator.hh>
#include <G4PhysicalVolumeStore.hh>
#include <Randomize.hh>

#include <TMath.h>

#include <sstream>

namespace RAT 
{
  PosGen_FillShell::PosGen_FillShell(const char *arg_dbname)
    : GLG4PosGen(arg_dbname), pos(0.,0.,0.), pVolume(0), max_iterations(10000000) { }

  // In order to get a GEANT4 physical volume by name, we must search through
  // the list in the G4PhysicalVolumeStore.  
  G4VPhysicalVolume* PosGen_FillShell::FindPhysVolume(const std::string volume_name)
  {
    G4PhysicalVolumeStore* store = G4PhysicalVolumeStore::GetInstance();
    Log::Assert(store, "PosGen_FillShell::FindPhysVolume: Unable to get GEANT4 physical volume store.");
    for (std::vector<G4VPhysicalVolume*>::iterator i_volume = store->begin();
         i_volume != store->end(); i_volume++) {
      G4VPhysicalVolume* testvolume = *i_volume;
      // Cast to G4String to avoid ambiguious overload
      if (testvolume->GetName() == G4String(volume_name))
        return testvolume;
    }
    return 0;
  }

  // PosGen_FillShell::SetState parses a string of generator parameters
  // to set the appropriate local variables.
  void PosGen_FillShell::SetState(G4String newValues)
  {
    std::vector<std::string> params = split(newValues, " ");

    // it is a GLG4 convention that SetState with a null string argument
    // should print usage information
    if (newValues.length() == 0) 
    {
      G4cout << "Current state of this GLG4PosGen_PointPaintFill:\n"
             << " \"" << GetState() << "\"\n" << G4endl;
      G4cout << "Usage: x_mm y_mm z_mm r_in r_out volname" << G4endl;
      return;
    }

    // for backwards-compatibility, handle two input cases
    double x, y, z;
    double vx, vy, vz;
    std::string volname;
    G4Navigator* gn;
    G4ThreeVector vpos;
    switch(params.size())
    {
      // newValues is "x y z ri ro volname"
      case 6:
        x = to_double(params[0]);
        y = to_double(params[1]);
        z = to_double(params[2]);
        ri = to_double(params[3]);
        ro = to_double(params[4]);
        volname = params[5];

        pVolume = FindPhysVolume(volname);
        Log::Assert(pVolume, "PosGen_FillShell::SetState: No physical volume found with name " + volname);
        break;

      // newValues is "vx vy vz x y z ri ro"
      // where vi is a point in the containing volume
      case 8:
        vx = to_double(params[0]);
        vy = to_double(params[1]);
        vz = to_double(params[2]);
        x = to_double(params[3]);
        y = to_double(params[4]);
        z = to_double(params[5]);
        ri = to_double(params[6]);
        ro = to_double(params[7]);

        vpos = G4ThreeVector(vx, vy, vz);
        gn = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
        pVolume = gn->LocateGlobalPointAndSetup(vpos, 0, false);
        Log::Assert(pVolume, "PosGen_FillShell::SetState: No physical volume found at given point");
        break;

      default:
        Log::Die("PosGen_FillShell::SetState: Error parsing input string.");
        return;
    }

    pos = G4ThreeVector(x, y, z);

    if(ri > ro) {
      info << "Inner radius is greater than outer radius, flipping them.\n";
      double radius_temp = ro;
      ro = ri;
      ri = radius_temp;
    }

  }

  // PosGen_FillShell::GetState returns a G4String containing the values
  // of all generator parameters, formatted like the input string
  G4String PosGen_FillShell::GetState() const
  {
    std::string rv = dformat("%d %d %d %d %d %s", pos.x(), pos.y(), pos.z(), ri, ro, pVolume->GetName().c_str());
    return G4String(rv);  
  }

  // PosGen_FillShell::GeneratePosition sets argResult to a point uniformly
  // sampled from a spherical shell centered at the given origin (pos) and
  // within the specified physical volume (volname).
  void PosGen_FillShell::GeneratePosition(G4ThreeVector &argResult)
  {
    G4Navigator* gNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
    G4ThreeVector rpos;
    int iterations = 0;
    do {
      // a point is found by sampling uniformly in r^3, cos(theta), and phi
      double r = pow(G4UniformRand() * (pow(ro, 3.0) - pow(ri, 3.0)) + pow(ri, 3.0), 1.0/3);
      double phi = TMath::Pi() * G4UniformRand();
      double theta = acos(2.0 * G4UniformRand() - 1);
      if(G4UniformRand() > 0.5)
        theta = -theta;

      double x = r * sin(theta) * cos(phi);
      double y = r * sin(theta) * sin(phi);
      double z = r * cos(theta);

      rpos = G4ThreeVector(x, y, z);
      rpos += pos;

      iterations++;
      Log::Assert(iterations<max_iterations, "PosGen_FillShell::GeneratePosition: Failed to find a point within volume " + pVolume->GetName() + " in " + ::to_string(max_iterations) + " tries.");
      
    } while(!(gNavigator->LocateGlobalPointAndSetup(rpos, 0, true)->GetName() == pVolume->GetName()));

    debug << "PosGen_FillShell::GeneratePosition: Point in volume " << pVolume->GetName()  << " found in " << iterations << " tries.\n";

    argResult = rpos;
  }
 
} // namespace RAT

