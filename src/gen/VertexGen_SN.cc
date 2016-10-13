// Calculates the cross-section for neutrino-elastic scattering
// as function of neutrino energy and the electron's recoil energy.
// Allow for variations in the weak mixing angle and the possibility
// of a neutrino magnetic moment
//
// J. Formaggio (UW) -02/09/2005

#include <RAT/VertexGen_SN.hh>
#include <RAT/SNgen.hh>

#include <RAT/GLG4PosGen.hh>
#include <RAT/GLG4StringUtil.hh>

#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4Event.hh>
#include <Randomize.hh>
#include <globals.hh>

#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Units/PhysicalConstants.h>

#include <sstream>
#include <cmath>


#include "TF1.h"
#include "TGraph.h"
#include "G4IonTable.hh"


namespace RAT {
    
    VertexGen_SN::VertexGen_SN(const char *arg_dbname)
    : GLG4VertexGen(arg_dbname), nu_dir(0.,0.,0.)
    {
        
        electron = G4ParticleTable::GetParticleTable()->FindParticle("e-");
        m_electron = electron->GetPDGMass();
        nu = G4ParticleTable::GetParticleTable()->FindParticle("anti_nu_e");
        eplus = G4ParticleTable::GetParticleTable()->FindParticle("e+");
        n = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        p  = G4ParticleTable::GetParticleTable()->FindParticle("proton");
        gamma  = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
        
        _lspec    = DB::Get()->GetLink("SN_SPECTRUM", _spectrum);	// default
        sngen.LoadSpectra();
        
    }
    
    VertexGen_SN::~VertexGen_SN() {}
    
    
    void VertexGen_SN::
    GeneratePrimaryVertex(G4Event* argEvent,
                          G4ThreeVector& dx,
                          G4double dt)
    {
        //        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        if (ev_nu_dir.mag2() == 0.0) {
            // Pick isotropic direction
            G4double theta = acos(2.0 * G4UniformRand() - 1.0);
            G4double phi = 2.0 * G4UniformRand() * CLHEP::pi;
            nu_dir.setRThetaPhi(1.0, theta, phi);//Changed such that nu_dir is changed, not e_nu_dir
            G4cout << "You have chosen an isotropic configuration, for supernovae this does not make sense.\n Picking a new direction for you. (" << theta << " " << phi << ")"<< G4endl;
        }
        
        //Pick an interaction for the supernova and then randomly pick a neutrino energy
        G4int interactionCode         =  ChooseInteraction();// Fill in specname
        G4double neutrinoEnergy,gammaEnergy;
        
        if (        interactionCode ==1) {
            neutrinoEnergy = sngen.GetIBDRandomEnergy();
            GenerateIBDVertex(argEvent,dx,dt,neutrinoEnergy);
            
        }else if (  interactionCode ==2 ) {
            neutrinoEnergy = sngen.GetESRandomEnergy();
            GenerateESVertex(argEvent,dx,dt,neutrinoEnergy);
            
        }else if(   interactionCode ==3){
            neutrinoEnergy = sngen.GetCCRandomEnergy();
            GenerateCCVertex(argEvent,dx,dt,neutrinoEnergy);
            
        }else if(   interactionCode ==4){
            neutrinoEnergy = sngen.GetICCRandomEnergy();
            GenerateICCVertex(argEvent,dx,dt,neutrinoEnergy);
            
        }else if(interactionCode ==5){
     
            if (G4UniformRand() < 0.31941) { //1.76/(1.76+3.75) Langanke
                gammaEnergy = sngen.GetNCRandomEnergy();
                GenerateNCVertex(argEvent,dx,dt,gammaEnergy);
            }else{
                gammaEnergy = sngen.GetINCRandomEnergy();
                GenerateINCVertex(argEvent,dx,dt,gammaEnergy);
            }
        
        }else{
            G4cout << "No interactions was chosen, something is wrong with the code" << G4endl;
            G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
            argEvent->AddPrimaryVertex(vertex);
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void VertexGen_SN::SetState(G4String newValues)
    {
        newValues = util_strip_default(newValues); // from GLG4StringUtil
        if (newValues.length() == 0) {
            // print help and current state
            G4cout << "Current state of this VertexGen_SN:\n"
            << " \"" << GetState() << "\"\n" << G4endl;
            G4cout << "Format of argument to VertexGen_SN::SetState: \n"
            " \"nu_dir_x nu_dir_y nu_dir_z\"\n"
            " where nu_dir is the initial direction of the reactor antineutrino.\n"
            " Does not have to be normalized.  Set to \"0. 0. 0.\" for isotropic\n"
            " neutrino direction."
            << G4endl;
            return;
        }
        
        std::istringstream is(newValues.c_str());
        double x, y, z;
        is >> x >> y >> z;
        if (is.fail())
            return;
        
        if ( x == 0. && y == 0. && z == 0. )
            nu_dir.set(0., 0., 0.);
        else
            nu_dir = G4ThreeVector(x, y, z).unit();
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    G4String VertexGen_SN::GetState()
    {
        std::ostringstream os;
        
        os << nu_dir.x() << "\t" << nu_dir.y() << "\t" << nu_dir.z() << std::ends;
        
        G4String rv(os.str());
        return rv;
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    int VertexGen_SN::ChooseInteraction(){
        
        double rand = G4UniformRand();
        
        double a1 = sngen.GetIBDAmplitude() ;
        double a2 = a1 + sngen.GetESAmplitude() ;
        double a3 = a2 + sngen.GetCCAmplitude() ;
        double a4 = a3 + sngen.GetICCAmplitude() ;
        double a5 = a4 + sngen.GetNCAmplitude() ;
        if (std::abs(a5-1.0)>0.01){
            G4cout << "Something is wrong " << a1 << " " << a2 << " " << a3 << " " << a4<< " " << a5 <<   G4endl;
        }
        if (rand < a1){
            return 1;
        }else if(rand < a2){
            return 2;
        }else if (rand< a3){
            return 3;
        }else if (rand< a4){
            return 4;
        }else{
            return 5;
        }
        
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void VertexGen_SN::GenerateIBDVertex( G4Event* argEvent,G4ThreeVector& dx,G4double dt,G4double e_nu){
        
        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        Eval2BodyKinematicIBD(e_nu, ev_nu_dir);
        
#ifdef DEBUG
        G4cout << "Neutrino " << ev_nu_dir << "" << G4endl;
        G4cout << "Positron " << primair << "" << G4endl;
        G4cout << "Neutron "  << secondaire << "" << G4endl;
#endif
        
        // -- Create particles
        // FIXME: Should I also add the neutrino and make these daughters of it?
        // positron
        G4PrimaryParticle* eplus_particle =
        new G4PrimaryParticle(eplus,              // particle code
                              primair.px(),     // x component of momentum
                              primair.py(),     // y component of momentum
                              primair.pz());    // z component of momentum
        eplus_particle->SetMass(eplus->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( eplus_particle );
        
        // neutron
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(n,                  // particle code
                              secondaire.px(),         // x component of momentum
                              secondaire.py(),         // y component of momentum
                              secondaire.pz());        // z component of momentum
        n_particle->SetMass(n->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        
        
        //Adding neutrino to see what happens M.B.
        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(nu,                  // particle code
                              e_nu*ev_nu_dir.x(),         // x component of momentum
                              e_nu*ev_nu_dir.y(),         // y component of momentum
                              e_nu*ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
        
        
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void VertexGen_SN::GenerateESVertex( G4Event* argEvent,G4ThreeVector& dx,G4double dt,G4double e_nu){
        
        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        G4double energyElectron = GetElectronEnergy(e_nu);
        CLHEP::HepLorentzVector mom_electron = GetEmomentum(e_nu,energyElectron,ev_nu_dir);
        
#ifdef DEBUG
        G4cout << "Energies (" << e_nu << ", " << energyElectron << ")" << G4endl;
        G4cout << "Momentum (" << mom_electron2 << ")" << G4endl;
#endif
        
        // Generate elastic-scattering interaction using ESgen.
        
        // -- Create particle at vertex
        G4PrimaryParticle* electron_particle =
        new G4PrimaryParticle(electron,           // particle code
                              mom_electron.px(),     // x component of momentum
                              mom_electron.py(),     // y component of momentum
                              mom_electron.pz());    // z component of momentum
        electron_particle->SetMass(m_electron); // This seems to help in VertexGen_IBD
        vertex->SetPrimary( electron_particle );
        
        
        //Adding neutrino to see what happens M.B.
        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(nu,                  // particle code
                              e_nu*ev_nu_dir.x(),         // x component of momentum
                              e_nu*ev_nu_dir.y(),         // y component of momentum
                              e_nu*ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
        
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void VertexGen_SN::GenerateCCVertex( G4Event* argEvent,G4ThreeVector& dx,G4double dt,G4double e_nu){
        
        
        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        Eval2BodyKinematicCC(e_nu, ev_nu_dir);
        
#ifdef DEBUG
        G4cout << "Neutrino " << ev_nu_dir << "" << G4endl;
        G4cout << "Positron " << primair << "" << G4endl;
        G4cout << "Neutron "  << secondaire << "" << G4endl;
#endif
        
        // -- Create particles
        // FIXME: Should I also add the neutrino and make these daughters of it?
        // positron
        G4PrimaryParticle* eplus_particle =
        new G4PrimaryParticle(electron,              // particle code
                              primair.px(),     // x component of momentum
                              primair.py(),     // y component of momentum
                              primair.pz());    // z component of momentum
        eplus_particle->SetMass(eplus->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( eplus_particle );
        
        // neutron
        G4ParticleDefinition *f16F = G4IonTable::GetIonTable()->GetIon(9, 16, 0.0);
        
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(f16F,                  // particle code
                              secondaire.px(),         // x component of momentum
                              secondaire.py(),         // y component of momentum
                              secondaire.pz());        // z component of momentum
        n_particle->SetMass(f16F->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        
        
        //Adding neutrino to see what happens M.B.
        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(nu,                  // particle code
                              e_nu*ev_nu_dir.x(),         // x component of momentum
                              e_nu*ev_nu_dir.y(),         // y component of momentum
                              e_nu*ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
        
        
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void VertexGen_SN::GenerateICCVertex( G4Event* argEvent,G4ThreeVector& dx,G4double dt,G4double e_nu){
        
        
        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        Eval2BodyKinematicICC(e_nu, ev_nu_dir);
        //G4cout << "Breakpoint 4 ICC" << e_nu << G4endl;
        
#ifdef DEBUG
        G4cout << "Neutrino " << ev_nu_dir << "" << G4endl;
        G4cout << "Positron " << primair << "" << G4endl;
        G4cout << "Neutron "  << secondaire << "" << G4endl;
#endif
        
        // -- Create particles
        // FIXME: Should I also add the neutrino and make these daughters of it?
        // positron
        G4PrimaryParticle* eplus_particle =
        new G4PrimaryParticle(eplus,              // particle code
                              primair.px(),     // x component of momentum
                              primair.py(),     // y component of momentum
                              primair.pz());    // z component of momentum
        eplus_particle->SetMass(eplus->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( eplus_particle );
        //G4cout << "Breakpoint 5 ICC" << e_nu << G4endl;
        
        
        // Due to issues with Nitrogen 16, Nitrogen 15 (stable) is used instead
        G4ParticleDefinition *f16N = G4IonTable::GetIonTable()->GetIon(7, 16, 0.0);
        //G4cout << "Breakpoint 6 ICC" << e_nu << G4endl;
        
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(f16N,                  // particle code
                              secondaire.px(),         // x component of momentum
                              secondaire.py(),         // y component of momentum
                              secondaire.pz());        // z component of momentum
        n_particle->SetMass(f16N->GetPDGMass()); // Geant4 is silly.
        
        vertex->SetPrimary( n_particle );
        
        //Adding neutrino to see what happens M.B.
        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(-12,                  // particle code
                              e_nu*ev_nu_dir.x(),         // x component of momentum
                              e_nu*ev_nu_dir.y(),         // y component of momentum
                              e_nu*ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
        
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void VertexGen_SN::GenerateNCVertex( G4Event* argEvent,G4ThreeVector& dx,G4double dt,G4double e_g){
        
        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        //Gammas comming off of NC should be isotropic in nature
        G4ThreeVector eg_nu_dir(nu_dir); // By default use specified direction
        G4double theta = acos(2.0 * G4UniformRand() - 1.0);
        G4double phi = 2.0 * G4UniformRand() * CLHEP::pi;
        eg_nu_dir.setRThetaPhi(1.0, theta, phi);
        
#ifdef DEBUG
        G4cout << "Neutrino " << ev_nu_dir << "" << G4endl;
        G4cout << "Neutron "  << secondaire << "" << G4endl;
#endif
        
        // Need to find a way to split the recoil and excitation energy
        // Excited nuclei might work
        //        G4ParticleDefinition *fNC = G4IonTable::GetIonTable()->GetIon(8, 16, e_g/2.0);
        
        // neutron
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(n,                  // particle code
                               e_g*eg_nu_dir.x(),         // x component of momentum
                               e_g*eg_nu_dir.y(),         // y component of momentum
                               e_g*eg_nu_dir.z());        // z component of momentum
        n_particle->SetMass(n->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        
        
        // Make an assumption that we will not see the 1.73 MeV beta decay from O15
        G4ParticleDefinition *f15O = G4IonTable::GetIonTable()->GetIon(8, 15, 0.0);
        
        G4PrimaryParticle* r_particle =
        new G4PrimaryParticle(f15O,                  // particle code
                              e_g*eg_nu_dir.x(),         // x component of momentum
                              e_g*eg_nu_dir.y(),         // y component of momentum
                              e_g*eg_nu_dir.z());        // z component of momentum
        r_particle->SetMass(f15O->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( r_particle );
        
        if (G4UniformRand() < 0.21) { // Value taken Langanke 0.37/1.76
            
            G4PrimaryParticle* s_particle =
            new G4PrimaryParticle(22,                  // particle code
                                  -e_g*eg_nu_dir.x(),         // x component of momentum
                                  -e_g*eg_nu_dir.y(),         // y component of momentum
                                  -e_g*eg_nu_dir.z());        // z component of momentum
            s_particle->SetMass(0.0); // Geant4 is silly.
            vertex->SetPrimary( s_particle );
        }
        
        //Adding neutrino to see what happens M.B. Chosing random neutrino energy, currently decoupled 
        
        Double_t neutrino_energy = sngen.GetNCRandomNuEnergy();
        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(14,                  // particle code
                              neutrino_energy*ev_nu_dir.x(),         // x component of momentum
                              neutrino_energy*ev_nu_dir.y(),         // y component of momentum
                              neutrino_energy*ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
    }
    
    void VertexGen_SN::GenerateINCVertex( G4Event* argEvent,G4ThreeVector& dx,G4double dt,G4double e_g){
        
        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        //Gammas comming off of NC should be isotropic in nature
        G4ThreeVector eg_nu_dir(nu_dir); // By default use specified direction
        G4double theta = acos(2.0 * G4UniformRand() - 1.0);
        G4double phi = 2.0 * G4UniformRand() * CLHEP::pi;
        eg_nu_dir.setRThetaPhi(1.0, theta, phi);
        
#ifdef DEBUG
        G4cout << "Neutrino " << ev_nu_dir << "" << G4endl;
        G4cout << "Neutron "  << secondaire << "" << G4endl;
#endif
        
        // Need to find a way to split the recoil and excitation energy
        // Excited nuclei might work
        //        G4ParticleDefinition *fNC = G4IonTable::GetIonTable()->GetIon(8, 16, e_g/2.0);
        
        // neutron
        G4PrimaryParticle* p_particle =
        new G4PrimaryParticle(p ,                  // particle code
                              e_g*eg_nu_dir.x(),         // x component of momentum
                              e_g*eg_nu_dir.y(),         // y component of momentum
                              e_g*eg_nu_dir.z());        // z component of momentum
        p_particle->SetMass(p->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( p_particle );
        
        
        // recoil
        G4ParticleDefinition *f15N = G4IonTable::GetIonTable()->GetIon(7, 15, 0.0);
        
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(f15N,                  // particle code
                              e_g*eg_nu_dir.x(),         // x component of momentum
                              e_g*eg_nu_dir.y(),         // y component of momentum
                              e_g*eg_nu_dir.z());        // z component of momentum
        n_particle->SetMass(f15N->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        
        if (G4UniformRand() < 0.376) { // Value taken Langanke 1.41/3.75
            
            G4PrimaryParticle* s_particle =
            new G4PrimaryParticle(22,                  // particle code
                                  -e_g*eg_nu_dir.x(),         // x component of momentum
                                  -e_g*eg_nu_dir.y(),         // y component of momentum
                                  -e_g*eg_nu_dir.z());        // z component of momentum
            s_particle->SetMass(0.0); // Geant4 is silly.
            vertex->SetPrimary( s_particle );
        }
        
        //Adding neutrino to see what happens M.B.
        Double_t neutrino_energy = sngen.GetNCRandomNuEnergy();

        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(-14,                  // particle code
                              neutrino_energy*ev_nu_dir.x(),         // x component of momentum
                              neutrino_energy*ev_nu_dir.y(),         // y component of momentum
                              neutrino_energy*ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
    }
    
    
    

    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    G4double VertexGen_SN::GetElectronEnergy(G4double enu){
        // Adapted from code written by Dan Hellfeld
        
        G4double m_e     = 0.51099891;
        G4double sigma_0 = 88.06*(pow(10,-46));
        G4double g_1     = 0.23;
        G4double g_2     = 0.73;
        
#ifdef DEBUG
        std::cout << "eee enu is " << enu << "\n";
#endif
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
        
        delete sigma_Te;
        
#ifdef DEBUG
        std::cout << "eee E_electron is " << E_electron << "\n";
        if (E_electron>14.0) std::cout << "eee99 E_electron is " << E_electron << "\n";
#endif
        return E_electron;
    }
    
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    CLHEP::HepLorentzVector VertexGen_SN::GetEmomentum(G4double enu,
                                                       G4double eelectron,
                                                       G4ThreeVector neutrino_dir) {
        G4double m_e     = 0.51099891;
        
        // Calculate scattering angle from neutrino and scattered electron energy
        // Taken from C. Giunti, C. Kim, Fundamentals of Neutrino Physics and Astrophysics,
        //Oxford University Press, Oxford, 2007.
        G4double theta = acos((sqrt(((eelectron*(pow((m_e+enu),2)))/((2*m_e*(pow(enu,2)))+((pow(enu,2))*eelectron))))));
        
#ifdef DEBUG
        G4cout << "Neutrino vector = {" << neutrino_dir.x() << ", " << neutrino_dir.y() << ", " << neutrino_dir.z() << "}\n";
        G4cout << "Cosine scattering angle (cos(theta)) = " << cos(theta) << "\n";
#endif
        
        // Randomly sample phi from 0 to 2pi
        G4double phi = G4UniformRand()*(2.*CLHEP::pi);
        
        // Construct electron vector by rotating the neutrino vector
        G4ThreeVector rotation_axis = neutrino_dir.orthogonal();
        rotation_axis.rotate(phi, neutrino_dir);
        G4ThreeVector e_direction = neutrino_dir.rotate(theta, rotation_axis);
        
        // Set up electron 4 vector to generate event
        G4double p_mag = sqrt(pow(eelectron,2)+(2*m_e*eelectron));
        CLHEP::HepLorentzVector e_momentum;
        e_momentum.setPx(p_mag * e_direction.x());
        e_momentum.setPy(p_mag * e_direction.y());
        e_momentum.setPz(p_mag * e_direction.z());
        e_momentum.setE(eelectron + m_e);
        
#ifdef DEBUG
        G4cout << "Electron vector = {" << e_direction.x() << ", " << e_direction.y() << ", " << e_direction.z() << "}\n";
        G4cout << "-----------------------------------------\n";
#endif
        
        return(e_momentum);
    }
    
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    void VertexGen_SN::Eval2BodyKinematicIBD(G4double enu, G4ThreeVector ev_nu_dir) {
        
        G4double targetMass =  CLHEP::proton_mass_c2;
        G4double recoilMass =  CLHEP::neutron_mass_c2;
        //Taken from IBDGen.cc
        G4double DELTA = recoilMass - targetMass;
        //        G4cout << "Delta value " << DELTA << G4endl;
        //        double GFERMI = 1.16639e-11 / CLHEP::MeV / CLHEP::MeV;
        
//        G4double CosThetaLab = -1.0+2.0*CLHEP::HepUniformRand();
        G4double CosThetaLab = FindCosTheta(enu,targetMass,recoilMass);
        
        // Pick energy of neutrino and relative direction of positron
        //            GenInteraction(enu, CosThetaLab);
        
        // Zero'th order approximation of positron quantities (infinite nucleon mass)
        G4double E0 = enu - DELTA;
        G4double p0 = sqrt(E0*E0-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
        G4double v0 = p0/E0;
        // First order correction for finite nucleon mass
        G4double Ysquared = (DELTA*DELTA-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2)/2;
        G4double E1 = E0*(1-enu/targetMass*(1-v0*CosThetaLab))
        - Ysquared/targetMass;
        G4double p1 = sqrt(E1*E1-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
        
        // Compute nu 4-momentum
        CLHEP::HepLorentzVector neutrino;
        neutrino.setVect(ev_nu_dir * enu); // MeV (divide by c if need real units)
        neutrino.setE(enu);
        
        // Compute positron 4-momentum
        CLHEP::Hep3Vector pos_momentum(p1*ev_nu_dir);
        
        // Rotation from nu direction to pos direction.
        G4double theta = acos(CosThetaLab);
        G4double phi = 2*CLHEP::pi*CLHEP::HepUniformRand();  // Random phi
        CLHEP::Hep3Vector rotation_axis = ev_nu_dir.orthogonal();
        rotation_axis.rotate(phi, ev_nu_dir);
        pos_momentum.rotate(theta, rotation_axis);
        
        primair.setVect(pos_momentum);
        primair.setE(E1);
        
        // Compute neutron 4-momentum
        secondaire.setVect(neutrino.vect() - primair.vect());
        secondaire.setE(sqrt(secondaire.vect().mag2() + recoilMass*recoilMass));
        
    }
    
    
    void VertexGen_SN::Eval2BodyKinematicCC(G4double enu, G4ThreeVector ev_nu_dir) {
        
        G4ParticleDefinition *fTarget = G4IonTable::GetIonTable()->GetIon(8, 16, 0.0);
        G4ParticleDefinition *fRecoil = G4IonTable::GetIonTable()->GetIon(9, 16, 0.0);
        
        G4double targetMass =  fTarget->GetPDGMass();  // Mass of oxygen16
        G4double recoilMass =  fRecoil->GetPDGMass(); // Mass of nitrogen16
        //Taken from IBDGen.cc
        G4double DELTA = recoilMass - targetMass;
        
        //        G4cout << "Delta value CC " << DELTA << G4endl;
        
        //        double GFERMI = 1.16639e-11 / CLHEP::MeV / CLHEP::MeV;
        
//        G4double CosThetaLab = -1.0+2.0*CLHEP::HepUniformRand();
        G4double CosThetaLab = FindCosTheta(enu,targetMass,recoilMass);

        // Pick energy of neutrino and relative direction of positron
        //            GenInteraction(enu, CosThetaLab);
        
        // Zero'th order approximation of positron quantities (infinite nucleon mass)
        G4double E0 = enu - DELTA;
        G4double p0 = sqrt(E0*E0-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
        G4double v0 = p0/E0;
        // First order correction for finite nucleon mass
        G4double Ysquared = (DELTA*DELTA-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2)/2;
        G4double E1 = E0*(1-enu/targetMass*(1-v0*CosThetaLab))
        - Ysquared/targetMass;
        G4double p1 = sqrt(E1*E1-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
        
        // Compute nu 4-momentum
        CLHEP::HepLorentzVector neutrino;
        neutrino.setVect(ev_nu_dir * enu); // MeV (divide by c if need real units)
        neutrino.setE(enu);
        
        // Compute positron 4-momentum
        CLHEP::Hep3Vector pos_momentum(p1*ev_nu_dir);
        
        // Rotation from nu direction to pos direction.
        G4double theta = acos(CosThetaLab);
        G4double phi = 2*CLHEP::pi*CLHEP::HepUniformRand();  // Random phi
        CLHEP::Hep3Vector rotation_axis = ev_nu_dir.orthogonal();
        rotation_axis.rotate(phi, ev_nu_dir);
        pos_momentum.rotate(theta, rotation_axis);
        
        primair.setVect(pos_momentum);
        primair.setE(E1);
        
        // Compute neutron 4-momentum
        secondaire.setVect(neutrino.vect() - primair.vect());
        secondaire.setE(sqrt(secondaire.vect().mag2() + recoilMass*recoilMass));
        
    }
    
    void VertexGen_SN::Eval2BodyKinematicICC(G4double enu, G4ThreeVector ev_nu_dir) {
        G4ParticleDefinition *fTarget = G4IonTable::GetIonTable()->GetIon(8, 16, 0.0);
        G4ParticleDefinition *fRecoil = G4IonTable::GetIonTable()->GetIon(7, 16, 0.0);
        
        G4double targetMass =  fTarget->GetPDGMass();  // Mass of oxygen16
        G4double recoilMass =  fRecoil->GetPDGMass(); // Mass of nitrogen16
        //Taken from IBDGen.cc
        G4double DELTA = recoilMass - targetMass;
        
        //        G4cout << "Delta value ICC " << DELTA << G4endl;
        
        //        double GFERMI = 1.16639e-11 / CLHEP::MeV / CLHEP::MeV;
        
//        G4double CosThetaLab = -1.0+2.0*CLHEP::HepUniformRand();
        G4double CosThetaLab = FindCosTheta(enu,targetMass,recoilMass);
        
        // Pick energy of neutrino and relative direction of positron
        //            GenInteraction(enu, CosThetaLab);
        
        // Zero'th order approximation of positron quantities (infinite nucleon mass)
        G4double E0 = enu - DELTA;
        G4double p0 = sqrt(E0*E0-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
        G4double v0 = p0/E0;
        // First order correction for finite nucleon mass
        G4double Ysquared = (DELTA*DELTA-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2)/2;
        G4double E1 = E0*(1-enu/targetMass*(1-v0*CosThetaLab))- Ysquared/targetMass;
        G4double p1 = sqrt(E1*E1-CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
        //G4cout << "Breakpoint 1 ICC" << DELTA << G4endl;
        
        // Compute nu 4-momentum
        CLHEP::HepLorentzVector neutrino;
        neutrino.setVect(ev_nu_dir * enu); // MeV (divide by c if need real units)
        neutrino.setE(enu);
        
        // Compute positron 4-momentum
        CLHEP::Hep3Vector pos_momentum(p1*ev_nu_dir);
        
        // Rotation from nu direction to pos direction.
        G4double theta = acos(CosThetaLab);
        G4double phi = 2*CLHEP::pi*CLHEP::HepUniformRand();  // Random phi
        CLHEP::Hep3Vector rotation_axis = ev_nu_dir.orthogonal();
        rotation_axis.rotate(phi, ev_nu_dir);
        pos_momentum.rotate(theta, rotation_axis);
        
        //G4cout << "Breakpoint 2 ICC" << DELTA << G4endl;
        
        
        primair.setVect(pos_momentum);
        primair.setE(E1);
        
        // Compute neutron 4-momentum
        secondaire.setVect(neutrino.vect() - primair.vect());
        secondaire.setE(sqrt(secondaire.vect().mag2() + recoilMass*recoilMass));
        
        //G4cout << "Breakpoint 3 ICC" << sqrt(secondaire.vect().mag2() + recoilMass*recoilMass) << G4endl;
        
        
    }
    
    
    
    
    double  VertexGen_SN::FindCosTheta(G4double Enu, G4double target_mass_c2,G4double recoil_mass_c2)
    {
    
    
        double electron_mass_c2 = CLHEP::electron_mass_c2;
        
        double DELTA = recoil_mass_c2 - target_mass_c2;
        double GFERMI = 1.16639e-11 ;
        
        double CosThetaC = (0.9741+9756)/2.;
        double RadCor = 0.024;
        
        const double Sigma0 = GFERMI*GFERMI*CosThetaC*CosThetaC/CLHEP::pi*(1+RadCor);
        const double f = 1.;
        const double f2 = 3.706;
        const double g = 1.26;
        
        double E0 = Enu - DELTA;
        if(E0<electron_mass_c2) {E0=electron_mass_c2;}
        
        double p0 = sqrt(E0*E0-electron_mass_c2*electron_mass_c2);
        double v0 = p0/E0;
        
        double q1 = 2.*(f+f2)*g*(2.*E0+DELTA);
        double q2 = v0;
        double q3 = 2.*(f+f2)*g*electron_mass_c2*electron_mass_c2/E0;
        double q4 = (f*f+g*g)*(DELTA);
        double q5 = (f*f+g*g)*electron_mass_c2*electron_mass_c2/E0;
        double q6 = (f*f+3.*g*g)*(E0+DELTA);
        double q7 = (f*f+3.*g*g)*DELTA;
        double q8 = (f*f-g*g)*(E0+DELTA);
        double q9 = (f*f-g*g)*DELTA;
        double q10 = (f*f+3.*g*g);
        double q11 = (f*f-g*g);
        double q12 = electron_mass_c2*electron_mass_c2;
        double q13 = 1./target_mass_c2*E0*p0;
        double q14 = Sigma0/2.; // For completness sake
        double q15 = (DELTA*DELTA-electron_mass_c2*electron_mass_c2)/2./target_mass_c2;
        double q16 = E0;
        double q17 = Enu/target_mass_c2;

        TF1 *E1 = new TF1("E1","([16]  * (1.0 -[17]*(1 - [2]*x) )  -[15])",-1,1);
        E1->SetParameter(2, q2);
        E1->SetParameter(15, q15);
        E1->SetParameter(16, q16);
        E1->SetParameter(17, q17);
        
        
        TF1 *P1 = new TF1("P1","sqrt(E1*E1 - [12])",-1,1);
        P1->SetParameter(12, q12);
        
        //v1 = P1/E1
        
        TF1 *crossSection =  new TF1("crossSection"," [0]*[14]*(([10] + [11] * P1/E1 * x)*P1*E1 - (([1]*(1.-[2]*x)-[3]) + ([4]*(1.+[2]*x)+[5])  +   ([6]*(1.-x/[2])-[7])  +   ([8]*(1.-x/[2])-[9]) * [2]*x) *[13])",-1,1);
        crossSection->SetNpx(10000);
        
        crossSection->SetParameter(0, 1e15);
        
        crossSection->SetParameter(1, q1);
        crossSection->SetParameter(2, q2);
        crossSection->SetParameter(3, q3);
        crossSection->SetParameter(4, q4);
        crossSection->SetParameter(5, q5);
        crossSection->SetParameter(6, q6);
        crossSection->SetParameter(7, q7);
        crossSection->SetParameter(8, q8);
        crossSection->SetParameter(9, q9);
        crossSection->SetParameter(10, q10);
        crossSection->SetParameter(11, q11);
        crossSection->SetParameter(12, q12);
        crossSection->SetParameter(13, q13);
        crossSection->SetParameter(14, q14);
        crossSection->SetParameter(15, q15);
        crossSection->SetParameter(16, q16);
        crossSection->SetParameter(17, q17);
        
        double CosThetaLab = crossSection->GetRandom();
        
        
        
        delete crossSection;
        delete E1;
        delete P1;

        return CosThetaLab;
        
        
        
    
    }
    
    
    
    
    
} // namespace RAT
