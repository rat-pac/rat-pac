#ifndef __GLG4Gen_h__
#define __GLG4Gen_h__

#include <globals.hh>

class G4Event;
class G4Track;
class G4PrimaryVertex;
class GLG4TimeGen;
class GLG4VertexGen;
class GLG4PosGen;

// Base class for event generators
class GLG4Gen {

public:
  GLG4Gen() { };
  virtual ~GLG4Gen() { };
  virtual void GenerateEvent(G4Event *event)=0;
  virtual double NextTime() const { return nextTime; };
  virtual void SubtractTime(double time) { nextTime -= time; };
  virtual void ResetTime(double offset=0.0)=0;
  virtual bool IsRepeatable() const =0;
  
  virtual void SetState(G4String state)=0;
  virtual G4String GetState() const =0;

  // Optional methods for setting specific aspects of
  // event generator if applicable
  virtual void SetTimeState(G4String /*state*/) { };
  virtual G4String GetTimeState() const { return ""; };
  virtual void SetVertexState(G4String /*state*/) { };
  virtual G4String GetVertexState() const { return ""; };
  virtual void SetPosState(G4String /*state*/) { };
  virtual G4String GetPosState() const { return ""; };
protected:
  double nextTime;
};


// Generic generator formed by combining separate time, vertex, and position
// generators.
class GLG4Gen_Combo : public GLG4Gen {
public:
  GLG4Gen_Combo();
  virtual ~GLG4Gen_Combo();
  virtual void GenerateEvent(G4Event *event);
  virtual void ResetTime(double offset=0.0);
  virtual bool IsRepeatable() const { return true; };

  virtual void SetState(G4String state);
  virtual G4String GetState() const;

  virtual void SetTimeState(G4String state);
  virtual G4String GetTimeState() const;
  virtual void SetVertexState(G4String state);
  virtual G4String GetVertexState() const;
  virtual void SetPosState(G4String state);
  virtual G4String GetPosState() const;


protected:
  G4String stateStr;
  GLG4TimeGen *timeGen;
  GLG4VertexGen *vertexGen;
  GLG4PosGen *posGen;
};


// Creates a one-shot generator used to defer a long-lived track
// to a later event.
class GLG4Gen_DeferTrack : public GLG4Gen {
public:
  GLG4Gen_DeferTrack(const G4Track *track);
  virtual ~GLG4Gen_DeferTrack();
  virtual void GenerateEvent(G4Event *event);
  virtual void ResetTime(double /*offset*/) { };
  virtual bool IsRepeatable() const { return false; };

  virtual void SetState(G4String state);
  virtual G4String GetState() const;
  
protected:
  G4PrimaryVertex *vertex;
};


// Creates events from an ASCII input.  See documentation about
// input format in comments preceding 
// GLG4VertexGen_HEPEvt::GeneratePrimaryVertex
class GLG4Gen_External : public GLG4Gen {
public:
  GLG4Gen_External();
  virtual ~GLG4Gen_External();
  virtual void GenerateEvent(G4Event *event);
  virtual void ResetTime(double offset=0.0);
  virtual bool IsRepeatable() const { return true; };

  virtual void SetState(G4String state);
  virtual G4String GetState() const;

  virtual void SetTimeState(G4String state);
  virtual G4String GetTimeState() const;
  virtual void SetVertexState(G4String state);
  virtual G4String GetVertexState() const;
  virtual void SetPosState(G4String state);
  virtual G4String GetPosState() const;

protected:
  G4String stateStr;
  GLG4TimeGen *timeGen;
  GLG4VertexGen *vertexGen;
  GLG4PosGen *posGen;
};

#endif
