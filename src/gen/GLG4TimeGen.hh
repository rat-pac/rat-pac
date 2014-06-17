#ifndef __GLG4TimeGen_h__
#define __GLG4TimeGen_h__

#include <globals.hh>

class GLG4TimeGen {
public:
  GLG4TimeGen() { };
  virtual ~GLG4TimeGen() { };
  virtual double GenerateEventTime(G4double offset=0.0)=0;
  virtual void SetState(G4String state)=0;
  virtual G4String GetState() const =0;
};


class GLG4TimeGen_Uniform : public GLG4TimeGen {
public:
  GLG4TimeGen_Uniform() : rate(1.0) { };
  virtual ~GLG4TimeGen_Uniform() { };
  virtual double GenerateEventTime(G4double offset=0.0) { return 1.0/rate + offset; };
  virtual void SetState(G4String state);
  virtual G4String GetState() const;

protected:
  double rate;
};

class GLG4TimeGen_Poisson : public GLG4TimeGen_Uniform {
public:
  GLG4TimeGen_Poisson() { };
  virtual ~GLG4TimeGen_Poisson() { };
  virtual double GenerateEventTime(G4double offset=0.0);
};


#endif
