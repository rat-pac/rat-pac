#ifndef __RAT_VertexGen_SN__
#define __RAT_VertexGen_SN__

#include <RAT/SNgen.hh>
#include <RAT/GLG4VertexGen.hh>
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
        
        virtual void GenerateICCVertex( G4Event* argEvent,
                                         G4ThreeVector& dx,
                                         G4double dt,
                                         G4double e_nu);
        
        virtual void GenerateNCVertex( G4Event* argEvent,
                                      G4ThreeVector& dx,
                                      G4double dt,
                                      G4double e_nu);
        
        
        virtual void GenerateINCVertex( G4Event* argEvent,
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
        
        virtual void Eval2BodyKinematicICC(G4double enu,
                                             G4ThreeVector neutrino_dir);
        
        // Choose which interaction and evaluate the neutrino energy from the spectrum
        virtual G4int  ChooseInteraction();
//        virtual G4double  pickEnergyFromSpectrum();
        
        // generates a primary vertex with given particle type, direction, and energy.
        virtual void SetState( G4String newValues );
        // format: dir_x dir_y dir_z
        // If dir_x==dir_y==dir_z==0, the directions are isotropic.
        virtual G4String GetState();
        // returns current state formatted as above
        
        
        
        // Differential cross section for inverse beta decay
//        static double CrossSection(double Enu, double CosThetaLab);
        
        static double FindCosTheta(G4double Enu, G4double target_mass_c2,G4double recoil_mass_c2);
        
    private:
        G4ParticleDefinition *electron, *nu, *n, *p, *eplus,*gamma;
        G4ThreeVector nu_dir;
        
        G4String	_particle;			// name of the particle type
        G4String	_spectrum,specname;			// name of the spectrum to use
        DBLinkPtr	_lspec;				// link to spectrum entry in database
        std::vector<double> spec_E;		// spectrum energy values
        std::vector<double> spec_mag;
        CLHEP::HepLorentzVector primair;
        CLHEP::HepLorentzVector secondaire;
        
        // Separate class to generate the elastic-scattering event.
        // Concrete definition; will invoke the SNgen() constructor.
        SNgen sngen;
        
        // Electron mass
        G4double m_electron;
    };
    
} // namespace RAT

#endif
