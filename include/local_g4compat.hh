#ifndef __local_g4compat__
#define __local_g4compat__ 1
// establish some compatibility between versions of Geant4

#ifndef G4VERSION_NUMBER
#include "G4Version.hh"
#endif

#if (G4VERSION_NUMBER < 500)
#define G4State_PreInit	    PreInit	    
#define G4State_Init	    Init	    
#define G4State_Idle	    Idle	    
#define G4State_GeomClosed  GeomClosed  
#define G4State_EventProc   EventProc   
#define G4State_Quit	    Quit	    
#define G4State_Abort       Abort
#endif

#if (G4VERSION_NUMBER >= 600)
#define G4std  std
#endif

#endif /* __local_g4compat__ */
