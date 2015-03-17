/** @file
    For GLG4NeutronDiffusionAndCapture class.
    
    This file is part of the GenericLAND software library.
    $Id: GLG4NeutronDiffusionAndCapture.cc,v 1.1 2005/08/30 19:55:22 volsung Exp $

    @author Glenn Horton-Smith
*/

////////////////////////////////////////////////////////////////////////

#include "GLG4NeutronDiffusionAndCapture.hh"

#include "G4Step.hh"
#include "G4VParticleChange.hh"
#include "G4EnergyLossTables.hh"
#include "G4HadronCaptureProcess.hh"
class G4UImessenger; // for G4ProcessTable.hh
#include "G4ProcessTable.hh"
#include "GLG4PrimaryGeneratorAction.hh"

#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

////////////////////////////////////////////////////////////////

GLG4NeutronDiffusionAndCapture::GLG4NeutronDiffusionAndCapture(const G4String& aName)
  : G4VProcess(aName)
{
   if (verboseLevel>0) {
     G4cout << GetProcessName() << " is created "<< G4endl;
   }
   
   _generator= GLG4PrimaryGeneratorAction::GetTheGLG4PrimaryGeneratorAction();
   if (_generator == 0) {
     G4Exception(__FILE__, "No Generator Action", FatalException, "GLG4NeutronDiffusionAndCapture:: no GLG4PrimaryGeneratorAction instance.");
   }

   _neutronCaptureProcess= (G4HadronCaptureProcess *)
     ( G4ProcessTable::GetProcessTable()->FindProcess("LCapture","neutron") );
   if (_neutronCaptureProcess == 0) {
     G4Exception(__FILE__, "No Neutron Capture", FatalException, "GLG4NeutronDiffusionAndCapture:: could not find neutron capture");
   }
}

GLG4NeutronDiffusionAndCapture::~GLG4NeutronDiffusionAndCapture()
{}

////////////////////////////////////////////////////////////////

GLG4NeutronDiffusionAndCapture::GLG4NeutronDiffusionAndCapture(GLG4NeutronDiffusionAndCapture& right)
  : G4VProcess(right)
{}

////////////////////////////////////////////////////////////////
 
G4double GLG4NeutronDiffusionAndCapture::PostStepGetPhysicalInteractionLength(
                             const G4Track& aTrack,
			     G4double  /* previousStepSize */,
			     G4ForceCondition* condition
			    )
{
  // condition is set to "Not Forced"
  *condition = NotForced;

  // go to diffuse-and-capture ASAP (~1 Bohr radius) if neutron is epithermal
  // and we have the material properties vector to tell us what to do with it;
  // otherwise let it keep tracking.
  if (aTrack.GetKineticEnergy() < 100.*eV) {
    G4MaterialPropertiesTable* materialPropertiesTable=
      aTrack.GetMaterial()->GetMaterialPropertiesTable();
    if ( materialPropertiesTable ) {
      G4MaterialPropertyVector* captureTimeVector=
	materialPropertiesTable->GetProperty("NEUTRON_CAPTURE_TIME");
      if ( captureTimeVector )
	return 0.05*nanometer;
    }
  }

  return DBL_MAX;
}

////////////////////////////////////////////////////////////////

G4VParticleChange* GLG4NeutronDiffusionAndCapture::PostStepDoIt(
			     const G4Track& aTrack,
			     const G4Step& aStep
			    )
{
  // copy "aTrack" to something we can modify.
  G4Track nTrack( aTrack );
  // nTrack.SetTouchable( aTrack.GetTouchable() ); // this is slightly naughty.
  nTrack.SetStep( &aStep );

  // change kinetic energy
  G4double oldEnergy= aTrack.GetKineticEnergy();
  G4Material* material= aTrack.GetMaterial();
  nTrack.SetKineticEnergy(k_Boltzmann*material->GetTemperature());

  // retrieve constants for diffusion
  G4double mean_capture_time= 0.0*microsecond;  // default
  G4double slow_diffusion_const= 0.0*mm*mm/ns;  // default
  G4double fast_diffusion_rms= 0.0*mm;          // default
  G4MaterialPropertiesTable* materialPropertiesTable=
    material->GetMaterialPropertiesTable();
  if ( materialPropertiesTable ) {
    G4MaterialPropertyVector* mpv;
    mpv= materialPropertiesTable->GetProperty("NEUTRON_CAPTURE_TIME");
    if (mpv)
      mean_capture_time= mpv->Value(oldEnergy);
    else
      G4cerr << "Warning, no NEUTRON_CAPTURE_TIME property for "
	     << material->GetName() << G4endl;
    mpv= materialPropertiesTable->GetProperty("NEUTRON_SLOW_DIFFUSION_CONST");
    if (mpv)
      slow_diffusion_const= mpv->Value(oldEnergy);
    else
      G4cerr << "Warning, no NEUTRON_SLOW_DIFFUSION_CONST property for "
	     << material->GetName() << G4endl;
    mpv= materialPropertiesTable->GetProperty("NEUTRON_FAST_DIFFUSION_RMS");
    if (mpv)
      fast_diffusion_rms= mpv->Value(oldEnergy);
    else
      G4cerr << "Warning, no NEUTRON_FAST_DIFFUSION_RMS property for "
	     << material->GetName() << G4endl;
  }
  else {
    G4cerr << "GLG4NeutronDiffusionAndCapture::PostStepDoIt: error, no material "
      " properties table for " << material->GetName() << G4endl;
  }

  // calculate time and position offsets from diffusion
  G4double capture_time= -log( 1.0-G4UniformRand() ) * mean_capture_time;
  G4ThreeVector position_offset( G4RandGauss::shoot(),
				 G4RandGauss::shoot(),
				 G4RandGauss::shoot() );
  position_offset*= sqrt((slow_diffusion_const * capture_time
			  + fast_diffusion_rms*fast_diffusion_rms)/3.0);

  // Apply time and position offsets.
  // This will cause secondaries to have correct time and position.
  // (It will also cause the ParticleChange to be initialized in such a
  // way that the real neutron track (aTrack) is updated.)
  nTrack.SetGlobalTime( nTrack.GetGlobalTime() + capture_time );
  nTrack.SetPosition( nTrack.GetPosition() + position_offset );

  // call neutron decay
  // note: assumes that "aStep" is not used by neutronCaptureProcess!
  //  (True in Geant4.3.2 as of 4-Oct-2001.)
  // FIXME: Why is mfp_cap unused??
  /* G4double mfp_cap= _neutronCaptureProcess->GetMeanFreePath(nTrack, 0.0, NULL);*/
  G4VParticleChange* nChange=
    _neutronCaptureProcess->PostStepDoIt(nTrack, aStep);

  // neutron decay should have stopped and killed neutron track
  // here we defer and stop-and-kill each secondary, if time is outside window
  if ( nTrack.GetGlobalTime() > _generator->GetEventWindow() ) {
    for (int isec= nChange->GetNumberOfSecondaries(); (isec--) > 0; ) {
      G4Track* t= nChange->GetSecondary(isec);
      _generator->DeferTrackToLaterEvent(t);
      t->SetTrackStatus(fStopAndKill);
    }
  }

  // return nChange from neutronCaptureProcess to tell Geant4 what to do
  return nChange;
}

////////////////////////////////////////////////////////////////
