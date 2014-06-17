// RAT::CfGen
// 10-Jan-2006 WGS

// Implements a GLG4Sim-style generator for the spontaneous fission of
// Californium.  The class we use is CfSource, prepared by Matt Worcester.

// To use this generator, the command is:

// /generator/add cf ISOTOPE:POSITION[:TIME]

// (that is, the TIME is optional).  ISOTOPE should be 252, since that
// is the only one currently supported.  POSITION and TIME are the
// same as for the combo generator.  For example:

// /generator/add cf252:fill

// Note that there is also no "defering" of any tracks of emitted
// particles into other events.  "TIME" refers to the t0 of the
// fission; the default is flat time distribution with a rate of 1 ns.

#ifndef __RAT_CfGen__
#define __RAT_CfGen__

#include <GLG4Gen.hh>

#include <globals.hh>

class G4Event;
class G4ParticleDefinition;
class GLG4TimeGen;
class GLG4PosGen;

namespace RAT {

  class CfGen : public GLG4Gen {
  public:
    CfGen();
    virtual ~CfGen();
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

    // Californium isotope.
    int isotope;

    // The time and position generators specified by the user.
    GLG4TimeGen* timeGen;
    GLG4PosGen* posGen;

    // The CfSource event model only generate neutrons and photons.
    G4ParticleDefinition* neutron;
    G4ParticleDefinition* gamma;
  };

} // namespace RAT

#endif // RAT_CfGen_h
