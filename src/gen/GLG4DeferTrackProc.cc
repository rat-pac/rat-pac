// This file is part of the GenericLAND software library.
// $Id: GLG4DeferTrackProc.cc,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
// Process to limit step length to stay within event time
// and defer long tracks (AND tracks which start after event time) using
// defered particle "generator".
//
// Written: G. Horton-Smith, 29-Oct-2001
//
////////////////////////////////////////////////////////////////////////

#include "GLG4DeferTrackProc.hh"

#include "G4Step.hh"
#include "G4VParticleChange.hh"
#include "G4EnergyLossTables.hh"
#include "G4GeometryTolerance.hh"
class G4UImessenger; // for G4ProcessTable.hh
#include "G4ProcessTable.hh"
#include "GLG4PrimaryGeneratorAction.hh"
#include <CLHEP/Units/PhysicalConstants.h>


////////////////////////////////////////////////////////////////

GLG4DeferTrackProc::GLG4DeferTrackProc(const G4String& aName)
  : G4VProcess(aName)
{
   if (verboseLevel>0) {
     G4cout << GetProcessName() << " is created "<< G4endl;
   }
   
   _generator= GLG4PrimaryGeneratorAction::GetTheGLG4PrimaryGeneratorAction();
   if (_generator == 0) {
     G4Exception(__FILE__, "No Primary Generator Action", FatalException, "GLG4DeferTrackProc:: no GLG4PrimaryGeneratorAction instance.");
   }
}

GLG4DeferTrackProc::~GLG4DeferTrackProc()
{}

////////////////////////////////////////////////////////////////

GLG4DeferTrackProc::GLG4DeferTrackProc(GLG4DeferTrackProc& right)
  : G4VProcess(right)
{}

////////////////////////////////////////////////////////////////
 
G4double GLG4DeferTrackProc::PostStepGetPhysicalInteractionLength(
                             const G4Track& aTrack,
			     G4double   /* previousStepSize */,
			     G4ForceCondition* condition
			    )
{
  // condition is set to "Not Forced"
  *condition = NotForced;

  // apply maximum time limit
  G4double dTime= (_generator->GetEventWindow() - aTrack.GetGlobalTime());
  if (dTime <= 0.0) {
    return G4GeometryTolerance::GetInstance()->GetSurfaceTolerance();
  }
  G4double beta = (aTrack.GetDynamicParticle()->GetTotalMomentum())/(aTrack.GetTotalEnergy());
  return beta*CLHEP::c_light*dTime;
}

////////////////////////////////////////////////////////////////

G4VParticleChange* GLG4DeferTrackProc::PostStepDoIt(
			     const G4Track& aTrack,
			     const G4Step& /* aStep */
			    )
{
  _generator->DeferTrackToLaterEvent(&aTrack);
  aParticleChange.Initialize(aTrack);
  aParticleChange.ProposeTrackStatus(fStopAndKill);
  return &aParticleChange;
}

////////////////////////////////////////////////////////////////
