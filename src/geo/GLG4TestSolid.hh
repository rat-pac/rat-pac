// This file is part of the GenericLAND software library.
// $Id: GLG4TestSolid.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
/*
  class GLG4TestSolid

  Encapsulates routines for testing a Geant4 solid for consistency.
  (See GLG4TestSolid.cc for documentation.)

  Author: Glenn Horton-Smith (Tohoku) 1999.12.09
*/

#ifndef __GLG4TESTSOLID_HH__
#define __GLG4TESTSOLID_HH__ 1

#include "globals.hh"
#include "G4ThreeVector.hh"

class G4VSolid;
class G4VisExtent;
class G4VoxelLimits;
class G4AffineTransform;

class GLG4TestSolid {
public:
  static G4String  Test(const G4VSolid &s, G4int npair);
  
  static G4String  TestRay(const G4VSolid &s,
			   const G4ThreeVector &p,
			   const G4ThreeVector &v,
			   G4double checkTolerance);
  
  static G4String  TestAtSurface(const G4VSolid &s,
				 const G4ThreeVector &p1,
				 G4double checkTolerance);
  
  static void      ResetRandomPointSeed(G4int newiseed);
  
  static G4ThreeVector PickRandomPoint(G4VisExtent &ve,
				       G4int itype= -1);
};

#endif
