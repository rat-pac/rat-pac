// RAT::CoincidenceMessenger
// 16 July 2008 JR Wilson

// Provide user commands to allow the user to change
// the Coincidence parameters via the command line.

#ifndef RAT_CoincidenceMessenger_hh
#define RAT_CoincidenceMessenger_hh

#include <RAT/Coincidence_Gen.hh>
#include "G4UImessenger.hh"
#include "G4String.hh"

// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;
class G4UIcmdWithABool;

namespace RAT {

	// Foward declarations in namespace
	class Coincidence_Gen;

	class CoincidenceMessenger: public G4UImessenger
	{
	public:
		CoincidenceMessenger(Coincidence_Gen*);
		~CoincidenceMessenger();

		void SetNewValue(G4UIcommand* command, G4String newValues);
		G4String GetCurrentValue(G4UIcommand* command);

		private:
		Coincidence_Gen* 	Co_gen;

		G4UIcommand* 		GenAddCmd;
		G4UIcommand* 		VtxSetCmd;
		G4UIcommand* 		PosSetCmd;
		G4UIcommand* 		ERangeCmd;
		G4UIcommand* 		ExpCmd;
		G4UIcmdWithABool* 	ExpForceWinCmd;
		G4UIcmdWithADouble* TimeWinCmd;
	};

} // namespace RAT

#endif // RAT_CoincidenceMessenger_hh
