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
        G4double neutrinoEnergy;
        
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
            double rand = G4UniformRand();
            //Fifty fifty chance of going 15N or 15O. There are different potential energy gamma
            //associate with the different choice.
            if (rand<0.5) {
                Double_t gammaEnergy = sngen.GetNCRandomEnergy();
                GenerateNCVertex(argEvent,dx,dt,gammaEnergy);
            }else {
                Double_t gammaEnergy = sngen.GetINCRandomEnergy();
                GenerateINCVertex(argEvent,dx,dt,gammaEnergy);
            }
        }else{
            G4cout << "No interactions was chosen, something is wrong with the code" << G4endl;
            G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
            argEvent->AddPrimaryVertex(vertex);
        }
    }
    
    
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
    
    
    
//    ////////////////////////////////////////////////////////////////////////////////////////////////
//    double  VertexGen_SN::pickEnergyFromSpectrum(){
//        
//        _lspec = DB::Get()->GetLink("SN_SPECTRUM", specname);
//        
//        // Flux function
//        spec_E.clear();
//        spec_mag.clear();
//        spec_E   = _lspec->GetDArray("spec_e");
//        spec_mag = _lspec->GetDArray("spec_mag");
//        
//        TGraph *tempGraph = new TGraph();
//        
//        float magsumTot = 0.0;
//        
//        for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
//            magsumTot+=(spec_mag[istep]);
//        }
//        float tot = 0.0;
//        for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
//            tot+=spec_mag[istep]/magsumTot;
//            tempGraph->SetPoint(istep,spec_E[istep],tot);
//        }
//        
//        float pickEnergy = G4UniformRand();
//        
//        //        float magsum = 0;
//        for (float foundEnergy=0.0; foundEnergy<100.0; foundEnergy+=0.01) {
//            if(tempGraph->Eval(foundEnergy)>pickEnergy){
//                tempGraph->Delete();
//                return foundEnergy;
//            }
//        }
//        G4cout << "Something is wrong with the energy picker" << G4endl;
//        
//        tempGraph->Delete();
//        return 0;
//        
//    }
    
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
        
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(22,                  // particle code
                              e_g*eg_nu_dir.x(),         // x component of momentum
                              e_g*eg_nu_dir.y(),         // y component of momentum
                              e_g*eg_nu_dir.z());        // z component of momentum
        n_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        
        
        //Adding neutrino to see what happens M.B.
        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(16,                  // particle code
                              ev_nu_dir.x(),         // x component of momentum
                              ev_nu_dir.y(),         // y component of momentum
                              ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
    }
    
    void VertexGen_SN::GenerateINCVertex( G4Event* argEvent,G4ThreeVector& dx,G4double dt,G4double e_g){
        
        G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
        G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
        
        G4ThreeVector eg_nu_dir(nu_dir); // By default use specified direction
        
        G4double theta = acos(2.0 * G4UniformRand() - 1.0);
        G4double phi = 2.0 * G4UniformRand() * CLHEP::pi;
        eg_nu_dir.setRThetaPhi(1.0, theta, phi);
        
        
#ifdef DEBUG
        G4cout << "Neutrino " << ev_nu_dir << "" << G4endl;
        G4cout << "Neutron "  << secondaire << "" << G4endl;
#endif
        
        // Need to find a way to split the recoil and excitation energy
        //
        // Excited nuclei
        //        G4ParticleDefinition *fNC = G4IonTable::GetIonTable()->GetIon(8, 16, e_g/2.0);
        
        
        
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(22,                  // particle code
                              e_g*eg_nu_dir.x(),         // x component of momentum
                              e_g*eg_nu_dir.y(),         // y component of momentum
                              e_g*eg_nu_dir.z());        // z component of momentum
        n_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        
        
        //Adding neutrino to see what happens M.B.
        G4PrimaryParticle* nu_particle =
        new G4PrimaryParticle(-16,                  // particle code
                              ev_nu_dir.x(),         // x component of momentum
                              ev_nu_dir.y(),         // y component of momentum
                              ev_nu_dir.z());        // z component of momentum
        nu_particle->SetMass(0.0); // Geant4 is silly.
        vertex->SetPrimary( nu_particle );
        
        argEvent->AddPrimaryVertex(vertex);
    }
    
    
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
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
        
        G4double CosThetaLab = -1.0+2.0*CLHEP::HepUniformRand();
        
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
        
        G4double CosThetaLab = -1.0+2.0*CLHEP::HepUniformRand();
        
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
        
        G4double CosThetaLab = -1.0+2.0*CLHEP::HepUniformRand();
        
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
    
    
} // namespace RAT
