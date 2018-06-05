#include <RAT/GeoBubbleFactory.hh>
#include <G4VSolid.hh>
#include <G4PVPlacement.hh>
#include <G4LogicalVolume.hh>
#include <G4Sphere.hh>
#include <G4Material.hh>
#include <RAT/Log.hh>
#include <Randomize.hh>
#include <G4UserLimits.hh>
#include <string>
#include <vector>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {
  bool check_intersect(G4ThreeVector newpos, vector<G4ThreeVector> &pos, float radius) {
    for (unsigned i = 0; i < pos.size(); i++) {
      if ( (newpos - pos[i]).mag() < (2*radius) )
        return true; //info << "Intersect " << i << " " << j << newline;
    }
    
    return false;
  }
  
  G4VPhysicalVolume* GeoBubbleFactory::Construct(DBLinkPtr table)
  {
    string name = table->GetIndex();
    info << "GeoBubbleFactory: Constructing volume " << name << " (yay bubbles!)"
         << newline;

    // Mother volume properties
    string mother_name = table->GetS("mother");
    G4LogicalVolume *logi_mother = FindMother(mother_name);

    if (logi_mother == 0)
      Log::Die("Unable to find mother volume \"" + mother_name +"\" for " + table->GetName() 
               + "[" + table->GetIndex() + "]");
    G4VSolid *solid_mother = logi_mother->GetSolid(); 
    
    // Bubble properties
    G4Material *material = G4Material::GetMaterial(table->GetS("material"));
    int count = table->GetI("count");
    float radius_mean  = table->GetD("radius_mean");
    float radius_sigma = table->GetD("radius_sigma");

    vector<G4ThreeVector> pos;
    
    for (int i=0; i < count; i++) {
      string bubble_name = name + dformat("%d", i);
      float radius = G4RandGauss::shoot(radius_mean, radius_sigma) * CLHEP::mm;
      if (radius < 0)
        radius = 0.001 * radius_mean * CLHEP::mm;
        
      G4VSolid *solid_bubble = new G4Sphere(bubble_name, 0, radius, /*phi*/ 0, CLHEP::twopi, /*theta*/ 0, CLHEP::pi);
      G4LogicalVolume *logi_bubble = new G4LogicalVolume(solid_bubble, material, bubble_name);
      
      // Pick location for bubble touching surface of volume
      G4ThreeVector bubble_point;
      do {
        G4ThreeVector surface_point = solid_mother->GetPointOnSurface();
        G4ThreeVector surface_normal = solid_mother->SurfaceNormal(surface_point);
        bubble_point = surface_point - radius * surface_normal;
      } while (check_intersect(bubble_point, pos, radius_mean + 3*radius_sigma));

      pos.push_back(bubble_point);
      
      detail << "Bubble loc: " << bubble_point.x() << " " << bubble_point.y()
            << " " << bubble_point.z() << " " << bubble_point.mag() << newline;
      
      new G4PVPlacement(0, bubble_point, logi_bubble, bubble_name, logi_mother, false,
                        /* copy number */ i);
    }
        
    return 0; // many bubbles, so no surface volume
  }
  
  
} // namespace RAT

