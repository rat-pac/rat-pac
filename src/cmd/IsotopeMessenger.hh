// RAT::IsotopeMessenger
// 10-Feb-2006 Bill Seligman

// Provide user commands to allow the user to change
// the Isotope parameters via the command line.

#ifndef RAT_IsotopeMessenger_hh
#define RAT_IsotopeMessenger_hh

#include "G4UImessenger.hh"
#include "G4String.hh"
#include <RAT/VertexGen_Isotope.hh>


// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;

namespace RAT {
    
    // Foward declarations in namespace
    class VertexGen_Isotope;
    
    class IsotopeMessenger: public G4UImessenger
    {
    public:
        IsotopeMessenger(VertexGen_Isotope*);
        ~IsotopeMessenger();
        
        void SetNewValue(G4UIcommand* command, G4String newValues);
        G4String GetCurrentValue(G4UIcommand* command);
        
    private:
        RAT::VertexGen_Isotope* isotope;
        
        G4UIcmdWithADouble* ACmd;
        G4UIcmdWithADouble* ZCmd;
        G4UIcmdWithADouble* ECmd;

    };
    
} // namespace RAT

#endif // RAT_IsotopeMessenger_hh
