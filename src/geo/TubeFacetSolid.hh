#ifndef __RAT_TubeFacetSolid__
#define __RAT_TubeFacetSolid__

#include <G4VSolid.hh>
#include <G4TwoVector.hh>
#include <G4String.hh>
#include <string>
#include <vector>

namespace RAT {
    G4VSolid *MakeFacetSolid(const G4String&           pName,
			     std::vector<G4TwoVector>  polygon,
			     G4double                  scale,
			     G4double                  hz,
			     G4double                  angle,
			     G4VSolid*                 basesolid);
    
    G4VSolid *MakeFacetSphereSolid(const G4String&           pName,
				   std::vector<G4TwoVector>  polygon,
				   std::vector<G4double>     angles,
				   G4double                  OR,
				   G4VSolid*                 basesolid);

    G4VSolid *MakeTubeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_max);
    
    G4VSolid *MakeTubeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_min,
				 G4double                  r_max);
    
    G4VSolid *MakeConeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_max_1,
				 G4double                  r_max_2);

    G4VSolid *MakeConeFacetSolid(const G4String&           pName,
				 std::vector<G4TwoVector>  polygon,
				 G4double                  scale,
				 G4double                  hz,
				 G4double                  angle,
				 G4double                  r_min_1,
				 G4double                  r_max_1,
				 G4double                  r_min_2,
				 G4double                  r_max_2);
    
    G4VSolid *MakeShellFacetSolid(const G4String&           pName,
				  std::vector<G4TwoVector>  polygon,
				  std::vector<G4double>     angles,
				  G4double                  IR,
				  G4double                  OR,
				  G4double                  dTheta);
    
    G4VSolid *MakeSphereFacetSolid(const G4String&           pName,
				   std::vector<G4TwoVector>  polygon,
				   std::vector<G4double>     angles,
				   G4double                  OR,
				   G4double                  dTheta);
    
} // namespace RAT

#endif
