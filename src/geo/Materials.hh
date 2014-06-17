#ifndef __RAT_Materials__
#define __RAT_Materials__

#include <RAT/DB.hh>
#include <RAT/Log.hh>
#include <G4ios.hh>
#include <G4OpticalSurface.hh>
#include <map>
#include <string>
#include <G4Material.hh>
#include <G4Element.hh>
#include <G4Isotope.hh> 
#include <G4OpticalSurface.hh>
#include <Randomize.hh>
#include <G4PhysicalVolumeStore.hh>
#include <G4NistManager.hh> 

namespace RAT {

class Materials {
public:
  // Load all materials into memory
  static void LoadMaterials();

  // GEANT4 has no global store of optical surface information,
  // so we need to keep it here
  static void LoadOpticalSurfaces();

  static void ConstructMaterials();
  static void ReadPropertyTable();

  static std::map<std::string, G4OpticalSurface*> optical_surface;

private:
  inline static bool BuildMaterial(std::string name, DBLinkPtr ptr);

  // Load all entries from OPTICS
  static void LoadOptics();

  // Create a MaterialPropertiesTable from an OPTICS index
  static void BuildMaterialPropertiesTable(G4Material* material, DBLinkPtr table);

  // Load a single material property vector from an OPTICS index by name
  // This handles energy vs. wavelength basis, etc.
  static G4MaterialPropertyVector* LoadProperty(DBLinkPtr table, std::string name);
};

}  // namespace RAT

#endif  // __RAT_Materials__

