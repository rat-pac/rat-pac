#ifndef __Gen_RandomTrigger_h__
#define __Gen_RandomTrigger_h__

#include <globals.hh>
#include <RAT/GLG4Gen.hh>

// Generates triggers spaced uniformly in time that are uncorrelated 
// with actual physics events
class Gen_RandomTrigger : public GLG4Gen {
public:
  Gen_RandomTrigger();
  virtual ~Gen_RandomTrigger();
  virtual void GenerateEvent(G4Event *event);
  virtual void ResetTime(double offset=0.0);
  virtual bool IsRepeatable() const { return true; };

  virtual void SetState(G4String /*state*/) { };
  virtual G4String GetState() const { return ""; };

  virtual void SetTimeState(G4String state);
  virtual G4String GetTimeState() const;

protected:
  G4String stateStr;
  GLG4TimeGen *timeGen;
};

#endif
