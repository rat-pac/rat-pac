// RATRunManager
// 31-Jul-2006 William Seligman

// Set up the G4RunManager environment, before initializing the
// main RAT simulation in RATGsim.

#ifndef __RAT_RunManager__
#define __RAT_RunManager__

class G4RunManager;
#ifdef G4VIS_USE
class G4VisManager;
#endif

namespace RAT {

// Forward declarations
class Gsim;
class ProcBlock;

class RunManager {
public:
  RunManager();
  RunManager(ProcBlock* theMainBlock);
  virtual ~RunManager();

protected:
  void Init(); // the real constructor

  G4RunManager* theRunManager;
  ProcBlock* mainBlock;
  Gsim*      ratGsim;

#ifdef G4VIS_USE
  G4VisManager* theVisManager;
#endif
};


} // namespace RAT

#endif // __RAT_RunManager__
