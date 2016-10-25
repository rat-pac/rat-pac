// RAT::FastNeutronMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the FastNeutron parameters via the command line.

#ifndef RAT_FastNeutronMessenger_hh
#define RAT_FastNeutronMessenger_hh

#include "G4UImessenger.hh"
#include "G4String.hh"
#include <RAT/VertexGen_FastNeutron.hh>


// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;

namespace RAT {
    
    // Foward declarations in namespace
    class VertexGen_FastNeutron;
    
    class FastNeutronMessenger: public G4UImessenger
    {
    public:
        FastNeutronMessenger(VertexGen_FastNeutron*);
        ~FastNeutronMessenger();
        
        void SetNewValue(G4UIcommand* command, G4String newValues);
        G4String GetCurrentValue(G4UIcommand* command);
        
    private:
        RAT::VertexGen_FastNeutron* fastneutron;
        
        G4UIcmdWithADouble* DCmd;
        G4UIcmdWithADouble* ECmd;

    };
    
} // namespace RAT

#endif // RAT_FastNeutronMessenger_hh
