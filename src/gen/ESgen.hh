#ifndef __RAT_ESgen__
#define __RAT_ESgen__

////////////////////////////////////////////////////////////////////
///
///         Code contributed by the SNO+ collaboration
///
/// \class RAT::ESgen
/// \author Nuno Barros <nfbarros@hep.upenn.edu> -- contact person
/// \date 15-Dec-2010
///
/// \brief Implements the generation of a neutrino-electron elastic scattering event.
///
/// This class is the workhorse of the generator, separating the Geant4 specific methods from a more physics oriented structure.
/// It is based on the original implementation by Joe Formaggio and the following adaptation by Bill Seligman.
/// However the whole class was re-written later to adapt it for the final use of solar neutrino generation.
///
/// REVISION HISTORY:\n
///     - 02-Sep-2005 J. Formaggio (UW):
///         - Original implementation.
///         - Generates an neutrino-elastic scattering event, based on the cross-section as function of neutrino energy and the electron's recoil energy.  Allow for variations in the weak mixing angle and the possibility of a neutrino magnetic moment.
///     - 07-Feb-2006 Bill Seligman:
///         - Converted to Geant4+GLG4Sim+RAT.
///         - I'm following the code structure of the IBD classes: RATVertexGen_ES handles the G4-related tasks of constructing an event, while this class deals with the physics of the cross-section.  Some of the code (the flux in particular) is copied from IBDgen.
///     - 15-Dec-2010 N. Barros (LIP)
///         - Strongly re-wrote this class to accommodate a solar neutrino generator. (solar). The original implementation was generating ES events from reactor anti-nus (ibd).
///         - The code has been strongly refactored so that the original code is still running. The choice is passed through the macro file or the command line.
///     - 18-Feb-2011 N. Barros:
///         - Improved the code for speed.
///         - Solved a minor bug in the calculation of the direction of the outgoing electron.
///         - Moved Geant4-related calculations into the parent vertex generator RAT::VertexGen_ES.
///     - 14-Dec-2011 N. Barros:
///         - Improved usage of the incoming neutrino direction and corresponding calculation of outgoing electron.
///     - 10-Jan-2012 N. Barros:
///         - Solved a problem with the rotation applied to get the outgoing electron direction.
///         - Improved speed in the determination of the electron momentum.
///     - 22-Jun-2012   N. Barros:
///         - Refactored code and cleaned up to prepare for pull request.
///         - Improved speed on random sampling over the spectrum and cross sections.
///     - 02-Jul-2012 N. Barros:
///         - Removed now superfluous SetXSecMax(). Now using CLHEP random sampler.
///         - Solved problem in the determination of the rater per target for continuous spectra.
///         - Removed dependency on ESMessenger. Its functionality was moved to upper level classes.
///     - 14-Aug-2012 N. Barros:
///         - Solved some issues with the compatibility with the combo generator.
///         - If the direction is set to 0,0,0 the generator now produces a random direction at each event.
///         - Added a new flag to customize the name of the database to draw the spectrum from (allows ad-hoc entries).
///     - 17-Jul-2017 R. Bonventre:
///         - Modified for RAT-PAC
////////////////////////////////////////////////////////////////////

#include <RAT/LinearInterp.hh>
#include <G4ThreeVector.hh>
#include <G4LorentzVector.hh>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Random/RandGeneral.h>

/// Forward declarations.
class TGraph;
class RandGeneral;

namespace RAT {

  /// Forward declarations within the namespace
  class ESCrossSec;

  class ESgen {
  public:
    ESgen();
    ~ESgen();
  
    // Generate random event vectors
    //    Pass in the neutrino direction (unit vector)
    //    Returns 4-momentum vectors for resulting electron.
		/**
		 * Generate random event vectors.
		 *
		 * Pass in the neutrino direction (unit vector).
		 * \param[in] nu_dir Incoming neutrino direction (lab coordinates).
		 * \param[out] neutrino Outgoing neutrino direction (lab coordinates. Not used).
		 * \param[out] electron Outgoing electron direction (lab coordinates).
		 * \return 4-momentum vectors for resulting electron.
		 */
    void GenerateEvent(const G4ThreeVector& nu_dir,
				G4LorentzVector &neutrino,
				G4LorentzVector &electron);

		/**
		 * Setter for the flux to use.
		 *
		 * \param[in] nutype Key to the database to load the flux.
		 */
		void SetNuType(const G4String &nutype);

		/** Getter for the spectrum being used */
		inline const G4String & GetNuType() const { return fNuFlavor;};

		/**
		 * Setter for the neutrino flavor being generated.
		 *
		 * This parameter is passed down into the cross-section to calculate the correct shape.
		 *
		 * @param nuflavor Flavor of the neutrino being calculated. Can be one of (<tt>nue,numu,nuebar,numubar</tt>).
		 * \attention \f$ \sigma_{\mu} = \sigma_{\tau}\f$
		 *
		 */
		void SetNuFlavor(const G4String &nuflavor);
		/** Getter for neutrino flavor */
		inline const G4String & GetNuFlavor() const { return fNuFlavor;};

		/**
		 * @brief Getter for the total neutrino flux.
		 * @return total neutrino flux in \f$ s^{-1} cm^{-2} \f$
		 */
		inline G4double GetTotalFlux() {
			// Return the neutrino flux as the value loaded from the database
			return fTotalFlux;
		}

		/**
     * Getter of the DB entry to input the spectrum from.
     * @return name of the DB name.
     */
    const G4String GetDBName() const  {return fDBName;}

    /**
     * Setter of the DB name. Defaults to \'SOLAR\'
     * @param[in] name of the database entry to look at.
     */
    void SetDBName(const G4String name);

	private:

		/** Private member for load the database and cross-section data. */
		void LoadGenerator();

		/** Generate the interaction given the neutrino energy and the recoil angle.*/
		void GenInteraction(double &Enu, double &CosThetaLab);

		/** Resets internal vectors. To be removed. */
		void Reset();
		/** Show internal state of calculations. To be removed.*/
		void Show();

		/** Sampler of neutrino energy from the spectrum. */
		G4double SampleNuEnergy();

		/** Sampler of recoil electron energy from the differential cross section. */
		G4double SampleRecoilEnergy(G4double Enu);

	protected:

		/** Private method to check if generator is loaded. */
		inline G4bool GetGenLoaded() {return fGenLoaded;};


		/** Generator type */
		G4String fGenType;
		/** Neutrino type */
		G4String fNuType;

		/** Neutrino flavor */
		G4String fNuFlavor;

		/** Instance of cross-section class */
		ESCrossSec *fXS;

		/**
		 *  @brief Spectrum shape to be sampled
		 *
		 *  Using ROOT TGraph to make use of it's nice evaluator.
		 */
		TGraph *fNuSpectrum;

		/** vector of neutrino energy points in the neutrino spectrum shape. */
		std::vector<double> fEnuTbl;

		/** Normalized flux in the neutrino spectrum shape. */
		std::vector<double> fFluxTbl;

		/** Recoil upper limit for the electron.*/
		G4float fEmax;
		/** Recoil lower limit for the electron.*/
		G4float  fEmin;

		/** Recoil upper limit for the electron.*/
		G4float fEnuMax;
		/** Recoil lower limit for the electron.*/
		G4float  fEnuMin;

		/** Maximum flux in spectrum shape */
		G4double fFluxMax;

		/** Generator loaded flag. */
		G4bool   fGenLoaded;

		/** electron mass */
		G4double fMassElectron;

		/** Total neutrino flux */
		G4double fTotalFlux;

		/** Random number generator for the nu spectrum sampler */
		CLHEP::RandGeneral *fSpectrumRndm;

		/** Name of the database entry to read the input spectrum from.
		 * Defaults to SOLAR.
		 */
		G4String fDBName;

	};

} // namespace RAT

#endif

