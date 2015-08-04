//
// $Id: BWVetGenericChamberHit.cc,v 1.1 2005/09/28 05:04:23 sekula Exp $
// --------------------------------------------------------------
//

#include <BWVetGenericChamberHit.hh>
#include <G4VVisManager.hh>
#include <G4Circle.hh>
#include <G4Colour.hh>
#include <G4VisAttributes.hh>
#include <G4ios.hh>
#include <CLHEP/Units/SystemOfUnits.h>

namespace RAT {


G4Allocator<BWVetGenericChamberHit> BWVetGenericChamberHitAllocator;

BWVetGenericChamberHit::BWVetGenericChamberHit(G4int i,G4double t)
{
  id = i;
  time = t;
  pLogV = 0;
}

BWVetGenericChamberHit::~BWVetGenericChamberHit()
{;}

BWVetGenericChamberHit::BWVetGenericChamberHit(const BWVetGenericChamberHit &right)
    : G4VHit() {
  id = right.id;
  time = right.time;
  pos = right.pos;
  rot = right.rot;
  pLogV = right.pLogV;
}

const BWVetGenericChamberHit& BWVetGenericChamberHit::operator=(const BWVetGenericChamberHit &right)
{
  id = right.id;
  time = right.time;
  pos = right.pos;
  rot = right.rot;
  pLogV = right.pLogV;
  return *this;
}

int BWVetGenericChamberHit::operator==(const BWVetGenericChamberHit &/*right*/) const
{
  return 0;
}

void BWVetGenericChamberHit::Draw()
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
  {
    G4Transform3D trans(rot.inverse(),pos);
    G4VisAttributes attribs;
    const G4VisAttributes* pVA = pLogV->GetVisAttributes();
    if(pVA) attribs = *pVA;
    G4Colour colour(0.,1.,1.);
    attribs.SetColour(colour);
    attribs.SetForceSolid(true);
    pVVisManager->Draw(*pLogV,attribs,trans);
  }
}

void BWVetGenericChamberHit::Print()
{
  int deb=0;
  if(deb!=0)G4cout << "  BWVetGenericChamber[" << id << "] " << time/CLHEP::ns << " (nsec)" << G4endl;
}


} // namespace RAT
