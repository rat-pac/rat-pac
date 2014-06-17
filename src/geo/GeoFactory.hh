#ifndef __RAT_GeoFactory__
#define __RAT_GeoFactory__

#include <string>
#include <G4VPhysicalVolume.hh>
#include <G4LogicalVolume.hh>
#include <G4VSolid.hh>
#include <RAT/DB.hh>


namespace RAT {
  
  class GeoFactoryNotFoundError {
  public:
    GeoFactoryNotFoundError(const std::string &name) : fName(name) { };
    std::string fName;
  };
  
  class GeoFactory {
  public:
    static void Register(const std::string &name, GeoFactory *factory);
    static G4VPhysicalVolume *ConstructWithFactory(const std::string &name, DBLinkPtr table);
  protected:
    static std::map<std::string, GeoFactory *> fFactoryMap;


  public:
    GeoFactory(const std::string &name);    
    virtual G4VPhysicalVolume *Construct(DBLinkPtr table) = 0;
    
    static G4LogicalVolume *FindMother(const std::string mother_name);
    static G4VPhysicalVolume *FindPhysMother(std::string mother_name);
    
  protected:
    // Utility methods
    virtual void SetSensitive(G4LogicalVolume *logi, DBLinkPtr table);

    virtual G4VPhysicalVolume *ConstructPhysicalVolume(G4LogicalVolume *logi,
  					     G4LogicalVolume *mother,
  					     DBLinkPtr table);
    virtual G4VPhysicalVolume *ConstructPhysicalReplica(G4LogicalVolume *logi,
  					      G4LogicalVolume *mother,
  					      DBLinkPtr table);
  };
  
  
} // namespace RAT

#endif
