#include "GLG4TimeGen.hh"
#include <Randomize.hh>
#include "GLG4StringUtil.hh"
#include "CLHEP/Units/SystemOfUnits.h"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

void GLG4TimeGen_Uniform::SetState(G4String state)
{
  state = util_strip_default(state);
  if (state.length() == 0) {
    // print help and current state
    G4cout << "Current state of this GLG4TimeGen:\n"
	   << " \"" << GetState() << "\"\n" << G4endl;
    G4cout << "Format of argument to GLG4TimeGen::SetState: \n"
      " \"rate\"\n"
      " where rate is in 1/sec."
	   << G4endl;
    return;
  }

  rate = util_to_double(state) / s;
}

G4String GLG4TimeGen_Uniform::GetState() const
{
  return util_dformat("%lf", rate);
}

// *****************************************************

double GLG4TimeGen_Poisson::GenerateEventTime(double offset)
{
  return -log(1.0-G4UniformRand())/rate + offset;
}
