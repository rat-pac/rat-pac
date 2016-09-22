#ifndef __RAT_VertexGen_SN__
#define __RAT_VertexGen_SN__

#include <RAT/ESgen.hh>
#include <RAT/GLG4VertexGen.hh>
#include <RAT/IBDgen.hh>
#include "RAT/DB.hh"



#include <G4Event.hh>
#include <G4ThreeVector.hh>
#include <globals.hh>

// Calculates the cross-section for neutrino-elastic scattering
// as function of neutrino energy and the electron's recoil energy.
// Allow for variations in the weak mixing angle and the possibility
// of a neutrino magnetic moment
//
// J. Formaggio (UW) -02/09/2005

// Converted to Geant4+GLG4Sim+RAT by Bill Seligman (21-Jan-2006).
// Converted to a Supernova generator Marc Bergevin, 2016

namespace RAT {
    
    class VertexGen_SN : public GLG4VertexGen {
    public:
        
        // Note that the database named is "ibd" by default in the
        // constructor.  In other words, we assume the anti-neutrino flux
        // is the same for both inverse beta-decay (IBD) and elastic
        // scattering (ES)... at least for now.
        
        VertexGen_SN(const char* arg_dbname="supernova");
        virtual ~VertexGen_SN();
        virtual void GeneratePrimaryVertex( G4Event* argEvent,
                                           G4ThreeVector& dx,
                                           G4double dt);
        
        // generates a primary vertex with given particle type, direction, and energy.
        virtual void SetState( G4String newValues );
        // format: dir_x dir_y dir_z
        // If dir_x==dir_y==dir_z==0, the directions are isotropic.
        virtual G4String GetState();
        // returns current state formatted as above
        
        
        // Choose between the different type of interaction available
        // IBD: Inverse Beta Decay, ES: Elastic Scattering, CC: Charge Current, NC: Neutral Current
        // Apply the correct kinematics
        virtual void GenerateIBDVertex( G4Event* argEvent,
                                       G4ThreeVector& dx,
                                       G4double dt,
                                       G4double e_nu);
        
        virtual void GenerateESVertex( G4Event* argEvent,
                                      G4ThreeVector& dx,
                                      G4double dt,
                                      G4double e_nu);
        
        virtual void GenerateCCVertex( G4Event* argEvent,
                                      G4ThreeVector& dx,
                                      G4double dt,
                                      G4double e_nu);
        
        virtual void GenerateNCVertex( G4Event* argEvent,
                                      G4ThreeVector& dx,
                                      G4double dt,
                                      G4double e_nu);
        
        virtual  CLHEP::HepLorentzVector GetEmomentum(G4double e_nu,
                                                      G4double eelectron,
                                                      G4ThreeVector neutrino_dir);
        virtual  G4double GetElectronEnergy(G4double nu_energy);
        
        virtual void Eval2BodyKinematicIBD(G4double enu,
                                        G4ThreeVector neutrino_dir);
        
        
        virtual void Eval2BodyKinematicCC(G4double enu,
                                           G4ThreeVector neutrino_dir);
        
        virtual void Eval2BodyKinematicCCbar(G4double enu,
                                          G4ThreeVector neutrino_dir);
        
        // Choose which interaction and evaluate the neutrino energy from the spectrum
        virtual G4int  ChooseInteraction(int model);
        virtual G4double  pickEnergyFromSpectrum();
        
        
        
    private:
        
        G4ParticleDefinition *electron, *nu, *n, *eplus;
        G4ThreeVector nu_dir;
        
        G4double m_electron;
        G4String	_particle;			// name of the particle type
        G4String	_spectrum,specname;			// name of the spectrum to use
        DBLinkPtr	_lspec;				// link to spectrum entry in database
        std::vector<double> spec_E;		// spectrum energy values
        std::vector<double> spec_mag;		// spectrum magnitude values
        
        
        CLHEP::HepLorentzVector positron;
        CLHEP::HepLorentzVector neutron;
//        
//        G4double targetMass;
//        G4double recoilMass;
        
        // Separate class to generate the elastic-scattering event.
        // Concrete definition; will invoke the ESgen() constructor.
        // Electron mass
        
//        IBDgen ibd;
//        ESgen esgen;
        
    };
    
} // namespace RAT

#endif
