//
// $Id: BWVetGenericChamberHit.hh,v 1.1 2005/09/28 05:04:23 sekula Exp $
// --------------------------------------------------------------
//
#ifndef __RAT_BWVetGenericChamberHit__
#define __RAT_BWVetGenericChamberHit__

#include <G4VHit.hh>
#include <G4THitsCollection.hh>
#include <G4Allocator.hh>
#include <G4ThreeVector.hh>
#include <G4LogicalVolume.hh>
#include <G4Transform3D.hh>
#include <G4RotationMatrix.hh>

namespace RAT {


class BWVetGenericChamberHit : public G4VHit
{
  public:

      BWVetGenericChamberHit(G4int i,G4double t);
      virtual ~BWVetGenericChamberHit();
      BWVetGenericChamberHit(const BWVetGenericChamberHit &right);
      const BWVetGenericChamberHit& operator=(const BWVetGenericChamberHit &right);
      int operator==(const BWVetGenericChamberHit &right) const;


      inline void *operator new(size_t);
      inline void operator delete(void*aHit);

      void Draw();
      void Print();

  private:
      G4int id;
      G4double time;
      G4ThreeVector pos;
      G4RotationMatrix rot;
      const G4LogicalVolume* pLogV;

  public:
      inline G4int GetID() const { return id; }
      inline G4double GetTime() const { return time; }
      inline void SetTime(G4double val) { time = val; }
      inline void SetPos(G4ThreeVector xyz) { pos = xyz; }
      inline G4ThreeVector GetPos() const { return pos; }
      inline void SetRot(G4RotationMatrix rmat) { rot = rmat; }
      inline G4RotationMatrix GetRot() const { return rot; }
      inline void SetLogV(G4LogicalVolume* val) { pLogV = val; }
      inline const G4LogicalVolume* GetLogV() const { return pLogV; }

};

typedef G4THitsCollection<BWVetGenericChamberHit> BWVetGenericChamberHitsCollection;

extern G4Allocator<BWVetGenericChamberHit> BWVetGenericChamberHitAllocator;

inline void* BWVetGenericChamberHit::operator new(size_t)
{
  void* aHit;
  aHit = (void*)BWVetGenericChamberHitAllocator.MallocSingle();
  return aHit;
}

inline void BWVetGenericChamberHit::operator delete(void*aHit)
{
  BWVetGenericChamberHitAllocator.FreeSingle((BWVetGenericChamberHit*) aHit);
}


} // namespace RAT

#endif


