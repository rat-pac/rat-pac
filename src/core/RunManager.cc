#include <RAT/RunManager.hh>
#include <RAT/ProcBlock.hh>
#include <RAT/Gsim.hh>

// GLG4sim headers
#include <GLG4PhysicsList.hh>

#include <G4RunManager.hh>

#ifdef G4VIS_USE
#include <GLG4VisManager.hh>
#endif

#include <cstdlib>

namespace RAT {


RunManager::RunManager()
{
    mainBlock = 0;
    Init();
}

RunManager::RunManager(ProcBlock* theMainBlock)
{
    mainBlock = theMainBlock;
    Init();
}
 

void RunManager::Init()
{
    theRunManager = new G4RunManager;  // Manages GEANT4 simulation process
    
    // Particle transport and interactions.  Note that this has to be
    // created outside of Gsim, since the physics list must be
    // initialized before the user tracking action.
    theRunManager -> SetUserInitialization( new GLG4PhysicsList );

    // Create the  simulation manager.
    ratGsim = new Gsim( mainBlock );
    
    // Visualization, only if you choose to have it!
#ifdef G4VIS_USE
    theVisManager = new GLG4VisManager();
    theVisManager -> Initialize();
#endif
}


RunManager::~RunManager()
{
  delete ratGsim;
#ifdef G4VIS_USE
    delete theVisManager;
#endif
}


} // namespace RAT
