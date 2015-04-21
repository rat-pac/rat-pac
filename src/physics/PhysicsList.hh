/**
 * @class RAT::PhysicsList
 * @brief Defines the physics processes active in the simulation
 *
 * @author A. Mastbaum <mastbaum@hep.upenn.edu>
 *
 * @detail This physics list extends the Shielding list included with Geant4
 * to add optical processes and custom processes overridden in RAT.
 */

#ifndef __RAT_PhysicsList__
#define __RAT_PhysicsList__

#include <G4VUserPhysicsList.hh>
#include <Shielding.hh>

namespace RAT {

class PhysicsList : public Shielding {
public:
  PhysicsList();

  ~PhysicsList();

  // Instantiate desired Particles
  void ConstructParticle();

  // Instantiate desired Processes
  void ConstructProcess();

private:
  // Construct and register optical processes
  void ConstructOpticalProcesses();

  // Register opticalphotons with the PMT G4FastSimulationManagerProcess
  void AddParameterization();
};

}  // namespace RAT

#endif  // __RAT_PhysicsList__

