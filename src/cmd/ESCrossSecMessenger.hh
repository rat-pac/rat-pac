////////////////////////////////////////////////////////////////////
/// Code contributed by the SNO+ collaboration
///
/// \class RAT::ESCrossSecMessenger
///
/// \brief Messenger class to control cross section options.
///
/// \author Nuno Barros <nfbarros@hep.upenn.edu> -- contact person
///
/// REVISION HISTORY:\n
/// 10-FEB-2006 - Bill Seligman
///				Provide user commands to allow the user to change the ESgen parameters via the command line.
///
/// 18-FEB-2010 - Nuno Barros
///				Updated to pass the options to the cross-section object directly to provide independence
///             to the cross section class.
///
/// 22-JUN-2012 - Nuno Barros
///				Cleaned up code, cleaned documentation and renamed conflicting variables.
///
///
/// \details Controls user customizable options for the cross section class.
///			At the moment the customizations are:
/// 		- Control over the weak mixing angle.
///			- Control over the cross section calculation strategy. \see RAT::ESCrossSec for more details.
///
///
////////////////////////////////////////////////////////////////////

#ifndef RAT_ESCrossSectionMessenger_hh
#define RAT_ESCrossSectionMessenger_hh

#include <G4UImessenger.hh>
#include <G4String.hh>

// Forward declarations
class G4UIcommand;
class G4UIcmdWithADouble;
class G4UIcmdWithAnInteger;

namespace RAT {

  // Forward declarations in namespace
  class ESCrossSec;

  class ESCrossSecMessenger: public G4UImessenger
  {
  public:
    ESCrossSecMessenger(ESCrossSec*);
    ~ESCrossSecMessenger();

    void SetNewValue(G4UIcommand* command, G4String newValues);
    G4String GetCurrentValue(G4UIcommand* command);

  private:
    ESCrossSec* fESXS;

    G4UIcmdWithADouble* fWmaCmd;
    G4UIcmdWithAnInteger* fStratCmd;
  };

} // namespace RAT

#endif // RAT_ESCrossSecMessenger_hh
