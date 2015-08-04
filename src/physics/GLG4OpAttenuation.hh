// This file is part of the GenericLAND software library.
// $Id: GLG4OpAttenuation.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
//  "Attenuation" (absorption or scattering) of optical photons
//
//   GenericLAND Simulation
//
//   Original: Glenn Horton-Smith, Dec 2001
//
// GLG4OpAttenuation.hh
// 

#ifndef GLG4OpAttenuation_h
#define GLG4OpAttenuation_h 1

/////////////
// Includes
/////////////

#include "G4OpAbsorption.hh"
#include "RAT/DummyProcess.hh"

/////////////////////
// Class Definition
/////////////////////

class GLG4OpAttenuation : public G4OpAbsorption
{

private:

        //////////////
        // Operators
        //////////////

        // GLG4OpAttenuation& operator=(const GLG4OpAttenuation &right);

        static DummyProcess fgAttenuation;
        static DummyProcess fgScattering;

public: // Without description

        ////////////////////////////////
        // Constructors and Destructor
        ////////////////////////////////

        GLG4OpAttenuation(const G4String& processName = "Attenuation");

        // GLG4OpAttenuation(const GLG4OpAttenuation &right);

	~GLG4OpAttenuation();

	////////////
	// Methods
        ////////////

        // G4bool IsApplicable(const G4ParticleDefinition& aParticleType);
        // Returns true -> 'is applicable' only for an optical photon.

	// G4double GetMeanFreePath(const G4Track& aTrack,
	// 			    G4double ,
	// 			    G4ForceCondition* );
        // Returns the absorption length for bulk absorption of optical
        // photons in media with a specified attenuation length. 

public: // With description

	G4VParticleChange* PostStepDoIt(const G4Track& aTrack,
 				        const G4Step&  aStep);
        // This is the method implementing attenuation of optical 
        // photons.  Fraction of photons scattered or absorbed is
        // determined by the MaterialProperyVector "OPSCATFRAC".

};

#endif /* GLG4OpAttenuation_h */
