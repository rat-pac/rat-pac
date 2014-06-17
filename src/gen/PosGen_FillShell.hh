/** @class PosGen_FillShell
 *  Generate event vertices at random points within a spherical shell and contained in a specific volume. 
 *
 *  Syntax: /generator/pos/set <xyz coordinates of center of spheres> <rmin> <rmax> <volume name>
 */

#ifndef __RAT_PosGen_FillShell__
#define __RAT_PosGen_FillShell__

#include <RAT/GLG4PosGen.hh>
#include <G4VPhysicalVolume.hh>

namespace RAT 
{
  class PosGen_FillShell : public GLG4PosGen 
  {
  public:
    PosGen_FillShell(const char* arg_dbname = "fillshell");
    virtual void GeneratePosition(G4ThreeVector& argResult);
    void SetState(G4String newValues);
    G4String GetState() const;
    
  protected:
    G4VPhysicalVolume* FindPhysVolume(const std::string vol_name);
    G4ThreeVector pos;
    G4VPhysicalVolume* pVolume;

    double ri;
    double ro;
    int max_iterations;
  }; 
} // namespace RAT

#endif

