// RAT::DecayChain_Gen
// 10-Jan-2006 WGS

// Implements a GLG4Sim-style generator for a chain of radioactive
// decays.  The class we use is DecayChain, prepared by Joe Formaggio
// and Jason Detwiler.  This class is based on code in
// GLG4Sim/GLG4Gen.

// To use this generator, the command is:

// /generator/add decaychain ISOTOPE:POSITION[:TIME]

// (that is, the TIME is optional).  ISOTOPE comes from the list in
// data/beta_decays.dat; POSITION and TIME are the same as for the
// combo generator.  For example:

// /generator/add decaychain 208Tl:fill

// For now, the implicit assumption is that all the isotopes in the
// decay chain remain at the fixed point determined by the position
// generator (no drifting during decays).  There is also no "defering"
// of any tracks of emitted particles into other events.  "TIME"
// refers to the start of the selected decay chain; as with the combo
// generator, the default is a poisson.

// The location of the file path data/beta_decays.dat is assumed to be
// relative to the directory in the RATROOT enviornment variable.  If you have
// to start RAT from a different location, want to use a different
// beta_decays.dat file, etc., you can change the file path by setting
// the variable $RATDecayDataDir in your shell environment.

#ifndef __RAT_DecayChain_Gen__
#define __RAT_DecayChain_Gen__

#include <globals.hh>
#include <GLG4Gen.hh>

class G4Event;
class GLG4TimeGen;
class GLG4PosGen;

namespace RAT {

  // Forward declarations in RAT namespace
  class DecayChain;

  class DecayChain_Gen : public GLG4Gen {
  public:
    DecayChain_Gen();
    virtual ~DecayChain_Gen();
    virtual void GenerateEvent(G4Event *event);
    virtual void ResetTime(double offset=0.0);
    virtual bool IsRepeatable() const { return true; };

    virtual void SetState(G4String state);
    virtual G4String GetState() const;

    virtual void SetTimeState(G4String state);
    virtual G4String GetTimeState() const;
    virtual void SetPosState(G4String state);
    virtual G4String GetPosState() const;

  protected:

    // Generator initialization, specified by the user.
    G4String stateStr;

    // The time and position generators specified by the user.
    GLG4TimeGen* timeGen;
    GLG4PosGen* posGen;

    // The decay chain for the isotope selected by the user.
    DecayChain* fDecayChain;
  };

} // namespace RAT

#endif
