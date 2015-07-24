#include "TApplication.h"
#include "TROOT.h"
#include "TF1.h"
#include "TH1.h"
#include "TMath.h"
#include "TVector3.h"
#include "TRandom.h"
#include "TRandom3.h"

#include <RAT/ESgen_2.hh>
#include <RAT/ESgenMessenger_2.hh>
#include <RAT/DB.hh>

#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>

#include <cmath>

namespace RAT {
 
    ESgen_2::ESgen_2(){
        
        // Create a messenger to allow the user to change some ES parameters.
        messenger = new ESgenMessenger_2(this);
      
        DefaultValues();
      
        m_e     = 0.51099891;
        sigma_0 = 88.06*(pow(10,-46));
        g_1     = 0.23;
        g_2     = 0.73;
        pi	    = 3.141592653589793238;
  }


    ESgen_2::~ESgen_2(){

        if ( messenger != 0 ){
            delete messenger;
            messenger = 0;
        }
    }


    CLHEP::HepLorentzVector ESgen_2::GenerateEvent(const G4ThreeVector& theNeutrino) {
    
        G4double E_nu		    = (GetAntiNuEnergy()) * MeV;
        G4double E_e		    = (GetElectronEnergy(E_nu)) * MeV;
        G4ThreeVector direction = GetScatteringAngle(E_nu, E_e);
      
        CLHEP::HepLorentzVector theElectron;
        theElectron.setPx(E_e*direction.x());
        theElectron.setPy(E_e*direction.y());
        theElectron.setPz(E_e*direction.z());
        theElectron.setE(E_e);

        return theElectron;
    }

    
    void ESgen_2::DefaultValues(){
        
        SetFissionFractions(0.496, 0.087, 0.351, 0.066);
    }
    
    
    void ESgen_2::SetFissionFractions(G4double u235, G4double u238, G4double pu239, G4double pu241) {
        
        if (u235 + u238 + pu239 + pu241 == 1.0){
            U235fraction  = u235;
            U238fraction  = u238;
            Pu239fraction = pu239;
            Pu241fraction = pu241;
        }
        
        else {
            
           // cout << "Your inputted fission fractions did not add up to one. I will use the default values (......blah......)\n";
           
            U235fraction  = 0.496;
            U238fraction  = 0.087;
            Pu239fraction = 0.351;
            Pu241fraction = 0.066;
            
        }
    }
    
    
    
    G4double ESgen_2::GetAntiNuEnergy() {
        
        G4double E_neutrino;
        
        TF1 * foldedenergyspectrum = new TF1("energyspectrum", "(7.8*pow(10,-45)*0.511*x)*(((((([0]*exp(0.870+(-0.160*x)+(-0.0910*x*x)))+([1]*exp(0.896+(-0.239*x)+(-0.0981*x*x)))+([2]*exp(0.793+(-0.080*x)+(-0.1085*x*x)))+([3]*exp(0.976+(-0.162*x)+(-0.0790*x*x))))))))",0,8);
        
        foldedenergyspectrum->SetNpx(10000);
        
        foldedenergyspectrum->SetParameter(0,U235fraction);
        foldedenergyspectrum->SetParameter(1,Pu239fraction);
        foldedenergyspectrum->SetParameter(2,Pu241fraction);
        foldedenergyspectrum->SetParameter(3,U238fraction);
        
        E_neutrino = foldedenergyspectrum->GetRandom();
        
        return E_neutrino;
    }
    
    
    G4double ESgen_2::GetElectronEnergy(G4double enu){
        
        G4double E_electron;
        
        G4double TeMax = ((2*(pow(enu,2)))/(m_e+(2*enu)));

        TF1 *sigma_Te = new TF1("sigma_Te","([2]/[1])*(pow([3],2)+(pow([4],2)*pow(1-(x/[0]),2))-(([3]*[4]*[1]*x)/pow([0],2)))",0,TeMax);
        
        sigma_Te->SetNpx(10000);
        
        sigma_Te->SetParameter(0,enu);
        sigma_Te->SetParameter(1,m_e);
        sigma_Te->SetParameter(2,sigma_0);
        sigma_Te->SetParameter(3,g_1);
        sigma_Te->SetParameter(4,g_2);

        E_electron = sigma_Te->GetRandom();
        
        return E_electron;
    }
 
    
    G4ThreeVector ESgen_2::GetScatteringAngle(G4double enu, G4double eelectron) {
        
        G4double theta = acos((sqrt(((eelectron*(pow((m_e+enu),2)))/((2*m_e*(pow(enu,2)))+((pow(enu,2))*eelectron))))));
        
        G4double phi      = G4UniformRand()*(2*pi);
        G4ThreeVector dir = G4ThreeVector(sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta));
        
        // This direction vector is assuming that the antineutrinos are coming from the -Z direction and
        // and travelling in the +Z direction.
        // This was done because the sampling in spherical coordinates was much easier.
        // This can be changed later.
        
        return(dir);
    }
    
    
    
    

} // namespace RAT
