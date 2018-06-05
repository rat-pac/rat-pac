#ifndef __RAT_VertexGen_ES__
#define __RAT_VertexGen_ES__

#include <RAT/ESgen.hh>
#include <RAT/GLG4VertexGen.hh>

#include <G4Event.hh>
#include <G4ThreeVector.hh>
#include <globals.hh>

////////////////////////////////////////////////////////////////////
///\class RAT::VertexGen_ES
///\author Nuno Barros <nfbarros@hep.upenn.edu> -- contact person
///\date 18-Feb-2011
///
/// \brief Vertex generator for neutrino-electron elastic scattering.
///
/// \details
/// This class generates a vertex of a neutrino-electron elastic scattering.
/// It is strongly based in an initial implementation by Joe Formaggio and an adaptation for a beam by Bill Seligman.
/// The event generation for solar neutrinos is a bit more complicated and therefore the whole class was virtually re-written since then.
/// The ES cross-section is now determined in a separate class. This class is responsible for performing the kinematic propagation.
///
/// REVISION HISTORY:
/// - 02/09/2005 J. Formaggio (UW):
///     - Original implementation
/// - 21-Jan-2006 Bill Seligman:
///   - Converted to Geant4+GLG4Sim+RAT
/// - 18-Feb-2011 Nuno Barros:
///   - Updated to accommodate the new solar generator, which is now used by default.
///   - Completely refactored to improve the cross-section calculation on a different class (imported from QSNO).
///   - Prepare ground to return both the electron in the neutrino in the generated event (only electron for now).
/// - 02-Jul-2012 N. Barros:
///   - Removed the usage of Geant4 default streamers (G4cout) to use RAT Log mechanism instead.
/// - 02-Aug-2012 N. Barros:
///   - Reintroduced the compatibility with the combo generator, for simulation of simple cases.
///   - The messenger options are available or not depending on who is the parent generator (disabled if it is solar).
///   - Changed the "simple" data types from G4XXX to C++ core types.
/// - 14-Aug-2012 N. Barros:
///   - Solved some issues with the compatibility with the combo generator.
///   - If the direction is set to 0,0,0 the generator now produces a random direction at each event.
/// - 26-Nov-2014: Matt Strait: Doxygen fixes.
/// - 17-Jul-2017: Modified for RAT-PAC
///
////////////////////////////////////////////////////////////////////

namespace RAT {

  class VertexGen_ES : public GLG4VertexGen {
  public:

    // Note that the database named is "ibd" by default in the
    // constructor.  In other words, we assume the anti-neutrino flux
    // is the same for both inverse beta-decay (IBD) and elastic
    // scattering (ES)... at least for now.

    VertexGen_ES(const char* arg_dbname="solar");
    virtual ~VertexGen_ES();
    virtual void GeneratePrimaryVertex( G4Event* argEvent,
					G4ThreeVector& dx,
					G4double dt);
    // generates a primary vertex with given particle type, direction, and energy.
    virtual void SetState( G4String newValues );
    // format: dir_x dir_y dir_z
    // If dir_x==dir_y==dir_z==0, the directions are isotropic.
    virtual G4String GetState();
    // returns current state formatted as above
    
    /**
     * Auxiliary method to set the flux to be generated.
     *
     * The flux can be any entry existing in the database defined in the constructor.
     * For the case of solar it should be one of the solar fluxes (<tt>pp,pep,hep,be7,b8,n13,o15,f17</tt>).
     * It looks up the corresponding spectrum in the SOLAR.ratdb file.
     * @param flux Key to the database entry. In case of solar it should be the corresponding flux.
     */
    void SetFlux(const G4String flux);

    /**
     * Return the flux being generated.
     * @return Database key of the flux being used in this instance of the generator.
     */
    G4String GetFlux() {return fFlux;};

    /**
     * Auxiliary method to set the neutrino flavor being generated.
     *
     * @param flavor Flavor key passed to the RAT::ESCrossSec class. It can be one of (<tt>nue,numu,nuebar,numubar</tt>).
     */
    void SetNuFlavor(const G4String flavor);

    /** Returns the neutrino flavor being generated. */
    G4String GetNuFlavor() {return fNuFlavor;};

    /**
     *  @return the helper elastic scattering generator object.
     *  */
    ESgen *GetHelper() {return fESgen;};

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
    /** Definitions of the involved particles. */
    G4ParticleDefinition* fElectron, *fNue, *fNumu;

    /** Incoming neutrino direction. */
    G4ThreeVector fNuDir;

    /** Database key for the spectrum being generated. */
    G4String fFlux;

    /** Neutrino flavor being generated to initialize the cross section calculation. */
    G4String fNuFlavor;

    /**
     * Class responsible for the kinematic calculations.
     *
     * \todo Merge both classes.
     */
    ESgen *fESgen;

    // Electron mass
    double fElectronMass;

    /** Name of the database entry to read the input spectrum from.
     * Defaults to IBD.
     */
    G4String fDBName;

    /** Failsafe flag in case of direction with zero amplitude.*/
    bool fRandomDir;

  };

} // namespace RAT

#endif
