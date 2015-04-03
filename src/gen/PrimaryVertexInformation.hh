#ifndef __RAT__PrimaryVertexInformation__
#define __RAT__PrimaryVertexInformation__

#include <G4VUserPrimaryVertexInformation.hh>
#include <G4PrimaryParticle.hh>
#include <vector>

namespace RAT {

  class PrimaryVertexInformation : public G4VUserPrimaryVertexInformation
  {
    public:
      PrimaryVertexInformation() {};
      ~PrimaryVertexInformation() {};

      void Print() const {}; // required for G4VUserPrimaryVertexInformation

      /** Parent particles of interaction */
      G4PrimaryParticle* GetParentParticle(Int_t i) { return parents[i]; }
      int GetParentParticleCount() const { return parents.size(); }
      void AddNewParentParticle(G4PrimaryParticle* part) {
        parents.push_back(part);
      }

    private:
      std::vector<G4PrimaryParticle*> parents;
  };
} // namespace RAT

#endif
