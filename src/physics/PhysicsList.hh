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

#include <string>
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

  // Set the WLS model by name
  void SetOpWLSModel(std::string model);

  // Get the WLS model name
  std::string GetOpWLSModelName() { return this->wlsModelName; }

private:
  // Construct and register optical processes
  void ConstructOpticalProcesses();

  // Adjust the neutron elastic scattering process so that it uses a more
  // precise treatment in the thermal region (below 4 eV)
  // Based on the technique shown here: http://hypernews.slac.stanford.edu/HyperNews/geant4/get/hadronprocess/1471.html
  // See https://indico.cern.ch/event/245281/contributions/1564676/attachments/420136/583408/thermal_physics_validation_argarcia.pdf
  // for more details.
  void EnableThermalNeutronScattering();

  // Register opticalphotons with the PMT G4FastSimulationManagerProcess
  void AddParameterization();

  std::string wlsModelName;  // The name of the WLS model
  G4VPhysicsConstructor* wlsModel;  // The WLS model constructor
};

}  // namespace RAT

#endif  // __RAT_PhysicsList__

