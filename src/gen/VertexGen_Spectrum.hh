// RAT::VertexGen_Spectrum
//18-July-2008 JR Wilson

/** @class RAT::VertexGen_Spectrum
*
* Vertex Generator - events of given particle (or ion) type, with energies sampled from spectrum via DB
*
* @author Jeanne Wilson <j.wilson2@physics.ox.ac.uk>
*
* first version : 18-July-2008
*
* This vertex generator (to be used in composite generators like combo of coincidence) produces isotropic particles
* or ions with kinetic energies sampled from a data-base defined distribution. An option to limit the range 
* of the energy distribution is available
*/
#ifndef __RAT_VertexGen_Spectrum__
#define __RAT_VertexGen_Spectrum__

#include <RAT/GLG4VertexGen.hh>
#include "RAT/DB.hh"

#include <G4Event.hh>
#include <G4ThreeVector.hh>
#include <globals.hh>
#include <vector>

namespace RAT {

	class VertexGen_Spectrum : public GLG4VertexGen {
		public:

		VertexGen_Spectrum(const char *arg_dbname="SPECTRUM");
		virtual 		~VertexGen_Spectrum();
		/** generate and add new vertex to this event.  Position and
  		* time of vertex are offset from 0 by dx and dt.  (These
  		* are usually derived from GLG4PosGen and GLG4TimeGen.)
		*/
		virtual void 	GeneratePrimaryVertex( G4Event* argEvent,
						G4ThreeVector& dx,
						G4double dt);
		/** set the state for generator. Format: pname specname
		* where pname is the particle name, and specname is the database spectrum name
		*/
		virtual void 	SetState( G4String newValues );
		/** return current state */
		virtual 		G4String GetState();
		
		/** Set up the spectrum array in memory - create normalised cumulative magnitude for easy sampling*/
		virtual void	InitialiseSpectrum();
		
		/** Sample energy from spectrum between bounds Elim_lo and Elim_hi */
		virtual	float 	SampleEnergy();

		/** For this generator, it is possible to limit the output energies */
		virtual bool 	ELimitable() { return true; };
		
		/** Set the limits on the generated energy */
		virtual void	LimitEnergies(float Elo, float Ehi);
		
		/** Return the maximum possible energy */
		virtual float	EMaximum();
		
		/** Return the minimum possible energy */
		virtual float	EMinimum();
		
		private:
		G4String	_particle;			// name of the particle type
		G4ParticleDefinition* _pDef;	// particle definition
   		G4String	_spectrum;			// name of the spectrum to use
		DBLinkPtr	_lspec;				// link to spectrum entry in database
		float	    _emin;				// valid range for spectrum in MeV
		float	    _emax;
		std::vector<double> spec_E;		// spectrum energy values
            	std::vector<double> spec_mag;		// spectrum magnitude values
		std::vector<double> spec_cummag;	// spectrum cumulative non-normalised magnitude values
		float		Elim_Ulo;			// user applied universal lower limit to energy range
		float		Elim_Uhi;			// user applied universal higher limit to energy range
		float		Elim_Tlo;			// temporary lower energy limit	(applies to one event)
		float		Elim_Thi;			// temporary higher energy limit (applies to one event)
	};

} // namespace RAT

#endif
