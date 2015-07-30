// ========================================================================================
//
// Reactor antineutrino-electron scattering generator
//      - developed by Daniel Hellfeld 07/2015
//
//  The generator takes in the user specified fission fractions for U235, U238, Pu239, and Pu241 and constructs the antineutrino
//    energy spectrum. It then folds this distribution with the scattering cross-section and randomly samples an energy.
//    With this energy, we construct the differential scattering cross-section with respect to the scattered electron energy. We
//    then randomly sample an electron energy from this distribution. Using the antineutrino energy and the electron energy, we
//    calculate the scattering angle using kinematics. We then rotate the incident antineutrino direction vector by this angle.
//    We then set up the electron with its generated energy and direction and send it off to the simulation.
//
// ========================================================================================

// ROOT includes (I use root to randomly sample the neutrino energy spectrum and differential scattering cross-section)
#include "TApplication.h"
#include "TROOT.h"
#include "TF1.h"
#include "TH1.h"
#include "TMath.h"
#include "TVector3.h"
#include "TRandom.h"
#include "TRandom3.h"

// RAT includes
#include <RAT/ReactorESgen.hh>
#include <RAT/ReactorESgenMessenger.hh>
#include <RAT/DB.hh>

// G4 includes
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>

// Other includes
#include <cmath>

namespace RAT {
    
//#define DEBUG
 
    ReactorESgen::ReactorESgen(){
        
        // Create a messenger to allow the user to change some ES parameters.
        messenger = new ReactorESgenMessenger(this);
      
		// Function title is fairly straightforward
        SetDefaultFissionFractions();
      
		// Define some variables we use throughout
        m_e     = 0.51099891;
        sigma_0 = 88.06*(pow(10,-46));
        g_1     = 0.23;
        g_2     = 0.73;
        pi	    = 3.141592653589793238;
  }


    ReactorESgen::~ReactorESgen(){
		
		// Delete messenger pointer
        if ( messenger != 0 ){
            delete messenger;
            messenger = 0;
        }
    }


    CLHEP::HepLorentzVector ReactorESgen::GenerateEvent(const G4ThreeVector& theNeutrino) {
    
		// Get a random antineutrino energy from the observable energy spectrum
        G4double E_nu = (GetAntiNuEnergy()) * MeV;
		
		// Use antineutrino energy to randomly sample an electron energy
        G4double E_e = (GetElectronEnergy(E_nu)) * MeV;
		
		// Use antineutrino energy, electron energy, and original antineutrino direction to construct the scattered electrons momentum vector
        CLHEP::HepLorentzVector theElectron = GetEmomentum(E_nu, E_e, theNeutrino);
	
        return theElectron;
    }

    
    void ReactorESgen::SetDefaultFissionFractions(){
        
		// Values are taken from G. Zacek, et. al., Physical Review D 34 (2621) 1986.
        U235fraction = 0.496; U238fraction = 0.087; Pu239fraction = 0.351; Pu241fraction = 0.066;
    }
    
	void ReactorESgen::SetU235FissionFrac (G4double u235) {
		U235fraction  = u235;
		
		#ifdef DEBUG
			G4cout << "U235 fission fraction = " << U235fraction << "\n";
		#endif
	}
	
	void ReactorESgen::SetU238FissionFrac (G4double u238) {
		U238fraction  = u238;
		
		#ifdef DEBUG
			G4cout << "U238 fission fraction = " << U238fraction << "\n";
		#endif
	}
	
	void ReactorESgen::SetPu239FissionFrac(G4double pu239){
		Pu239fraction = pu239;
		
		#ifdef DEBUG
			G4cout << "Pu239 fission fraction = " << Pu239fraction << "\n";
		#endif
	}
	
	void ReactorESgen::SetPu241FissionFrac(G4double pu241){
		Pu241fraction = pu241;
		
		#ifdef DEBUG
			G4cout << "Pu241 fission fraction = " << Pu241fraction << "\n";
		#endif
	}
	
	G4double ReactorESgen::GetU235FissionFrac() {return U235fraction;}
	G4double ReactorESgen::GetU238FissionFrac() {return U238fraction;}
	G4double ReactorESgen::GetPu239FissionFrac(){return Pu239fraction;}
	G4double ReactorESgen::GetPu241FissionFrac(){return Pu241fraction;}
	
	
    void ReactorESgen::CheckFissionFractions() {
        
        if (U235fraction + U238fraction + Pu239fraction + Pu241fraction != 1.0){
			
			// We check to see that the user inputted fractions add to 1. If they do not, we go back to the default values.
			G4cout << "\n\n******* WARNING *********\nYour inputted fission fractions did not add up to one. I will instead use the default values (49.6% U235, 8.7% U238, 35.1% Pu239, 6.6% Pu241\n\n";
           
            SetDefaultFissionFractions();
        }
    }
    
	
    G4double ReactorESgen::GetAntiNuEnergy() {

		// We add up the energy spectra for U235, U238, Pu239, and Pu241, with their respective fractions...then multiply all by the scattering cross section.
		// The parameter values used in the exponential polynomials for each isotope are taken from P. Vogel, J. Engel, Physical Review D 39 (3378) 1989.
        TF1 * foldedenergyspectrum = new TF1("energyspectrum", "(7.8*pow(10,-45)*0.511*x)*(((((([0]*exp(0.870+(-0.160*x)+(-0.0910*x*x)))+([1]*exp(0.896+(-0.239*x)+(-0.0981*x*x)))+([2]*exp(0.793+(-0.080*x)+(-0.1085*x*x)))+([3]*exp(0.976+(-0.162*x)+(-0.0790*x*x))))))))",0,8);
        
		// Set parameters
        foldedenergyspectrum->SetNpx(10000);
        foldedenergyspectrum->SetParameter(0,U235fraction);
        foldedenergyspectrum->SetParameter(1,Pu239fraction);
        foldedenergyspectrum->SetParameter(2,Pu241fraction);
        foldedenergyspectrum->SetParameter(3,U238fraction);
        
		// Randomly sample from the distribution
        G4double E_neutrino = foldedenergyspectrum->GetRandom();
        
        #ifdef DEBUG
            G4cout << "-----------------------------------------\n";
            G4cout << "Antineutrino energy = " << E_neutrino << " MeV\n";
        #endif
        
        return E_neutrino;
    }
    
    
    G4double ReactorESgen::GetElectronEnergy(G4double enu){

		// Define the maximum allowable scattered electron energy based off the antineutrino energy
        G4double TeMax = ((2*(pow(enu,2)))/(m_e+(2*enu)));

		// Differential cross-section with respect to the scattered electron energy
		// Taken from C. Giunti, C. Kim, Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press, Oxford, 2007.
        TF1 *sigma_Te = new TF1("sigma_Te","([2]/[1])*(pow([3],2)+(pow([4],2)*pow(1-(x/[0]),2))-(([3]*[4]*[1]*x)/pow([0],2)))",0,TeMax);
        
		// Set parameters
        sigma_Te->SetNpx(10000);
        sigma_Te->SetParameter(0,enu);
        sigma_Te->SetParameter(1,m_e);
        sigma_Te->SetParameter(2,sigma_0);
        sigma_Te->SetParameter(3,g_1);
        sigma_Te->SetParameter(4,g_2);

		// Randomly sample from the distribution
        G4double E_electron = sigma_Te->GetRandom();
        
        #ifdef DEBUG
            G4cout << "Electron energy = " << E_electron << " MeV\n";
        #endif
        
        return E_electron;
    }
 
    
    CLHEP::HepLorentzVector ReactorESgen::GetEmomentum(G4double enu, G4double eelectron, G4ThreeVector neutrino_dir) {
        
		// Calculate scattering angle from neutrino and scattered electron energy
        G4double theta = acos((sqrt(((eelectron*(pow((m_e+enu),2)))/((2*m_e*(pow(enu,2)))+((pow(enu,2))*eelectron))))));
		
        #ifdef DEBUG
            G4cout << "Neutrino vector = {" << neutrino_dir.x() << ", " << neutrino_dir.y() << ", " << neutrino_dir.z() << "}\n";
            G4cout << "Cosine scattering angle (cos(theta)) = " << cos(theta) << "\n";
        #endif
        
		// Randomly sample phi from 0 to 2pi
        G4double phi = G4UniformRand()*(2*pi);
		
		// Construct electron vector by rotating the neutrino vector
		G4ThreeVector rotation_axis = neutrino_dir.orthogonal();
		rotation_axis.rotate(phi, neutrino_dir);
		G4ThreeVector e_direction = neutrino_dir.rotate(theta, rotation_axis);
		
		// Set up electron momentum vector to generate event
		CLHEP::HepLorentzVector e_momentum;
		e_momentum.setPx(eelectron * e_direction.x());
        e_momentum.setPy(eelectron * e_direction.y());
        e_momentum.setPz(eelectron * e_direction.z());
        e_momentum.setE(eelectron);
        
        #ifdef DEBUG
            G4cout << "Electron vector = {" << e_direction.x() << ", " << e_direction.y() << ", " << e_direction.z() << "}\n";
            G4cout << "-----------------------------------------\n";
        #endif
        
        return(e_momentum);
    }

} // namespace RAT
