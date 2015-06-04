
// This file is part of the GenericLAND software library.
// $Id: GLG4PhysicsList.cc,v 1.4 2006/01/20 03:56:47 volsung Exp $
//
// GLG4PhysicsList.cc by Glenn Horton-Smith, Feb 1999
////////////////////////////////////////////////////////////////
// GLG4PhysicsList
////////////////////////////////////////////////////////////////
//
//      This is a version for maximum particle set
//       (copied from ExN06, with improved scintillation added)
// ------------------------------------------------------------
// Modification History:
//  G. Horton-Smith 2001.01.12:  Add hadronic processes
//

#include "GLG4PhysicsList.hh"

#include "G4ParticleDefinition.hh"
#include "G4ParticleWithCuts.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4ParticleTypes.hh"
#include "G4ParticleTable.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4ios.hh"
#include "local_g4compat.hh"
#include "iomanip"

#include "G4IonConstructor.hh"
#include "G4IonTable.hh"
#include "G4Ions.hh"
#include "G4RadioactiveDecay.hh"
#include "G4NeutronInelasticProcess.hh" // added by V.V. Golovko @ 5 January, 2009

#include "G4NeutronBuilder.hh"
#include "GLG4NeutronDiffusionAndCapture.hh"
#include "GLG4DeferTrackProc.hh"

#include "G4LossTableManager.hh"
#include "G4EmProcessOptions.hh"
#include "G4Version.hh"

#include <RAT/DB.hh>

#define MYNAME_PhysicsList  GLG4PhysicsList
#define MYNAME_string       "GLG4"


MYNAME_PhysicsList::MYNAME_PhysicsList():  G4VUserPhysicsList()
{
  G4LossTableManager::Instance();
  currentDefaultCut   = 0.010 * mm;
  cutForGamma         = currentDefaultCut;
  cutForElectron      = currentDefaultCut;
  cutForPositron      = currentDefaultCut;

  SetVerboseLevel(1);
}

MYNAME_PhysicsList::~MYNAME_PhysicsList()
{
}


// bosons
#include "G4ChargedGeantino.hh"
#include "G4Geantino.hh"
#include "G4Gamma.hh"
#include "G4OpticalPhoton.hh"

// leptons
#include "G4MuonPlus.hh"
#include "G4MuonMinus.hh"
#include "G4NeutrinoMu.hh"
#include "G4AntiNeutrinoMu.hh"

#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4NeutrinoE.hh"
#include "G4AntiNeutrinoE.hh"

// NEST
#include <RAT/G4ThermalElectron.hh>

// Mesons
#include "G4PionPlus.hh"
#include "G4PionMinus.hh"
#include "G4PionZero.hh"
#include "G4Eta.hh"
#include "G4EtaPrime.hh"

#include "G4KaonPlus.hh"
#include "G4KaonMinus.hh"
#include "G4KaonZero.hh"
#include "G4AntiKaonZero.hh"
#include "G4KaonZeroLong.hh"
#include "G4KaonZeroShort.hh"

// Baryons
#include "G4Proton.hh"
#include "G4AntiProton.hh"
#include "G4Neutron.hh"
#include "G4AntiNeutron.hh"

// Nuclei
#include "G4Deuteron.hh"
#include "G4Triton.hh"
#include "G4Alpha.hh"
#include "G4GenericIon.hh"
//added KJP 42611
#include "G4He3.hh"


void MYNAME_PhysicsList::ConstructParticle()
{
  // In this method, static member functions should be called
  // for all particles which you want to use.
  // This ensures that objects of these particle types will be
  // created in the program. 

  // pseudo-particles
  G4Geantino::GeantinoDefinition();
  G4ChargedGeantino::ChargedGeantinoDefinition();

  // gamma
  G4Gamma::GammaDefinition();

  // optical photon
  G4OpticalPhoton::OpticalPhotonDefinition();

  // leptons
  G4Electron::ElectronDefinition();
  G4Positron::PositronDefinition();
  G4MuonPlus::MuonPlusDefinition();
  G4MuonMinus::MuonMinusDefinition();

  G4NeutrinoE::NeutrinoEDefinition();
  G4AntiNeutrinoE::AntiNeutrinoEDefinition();
  G4NeutrinoMu::NeutrinoMuDefinition();
  G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();

  // NEST
  RAT::G4ThermalElectron::ThermalElectronDefinition();

  // mesons
  G4PionPlus::PionPlusDefinition();
  G4PionMinus::PionMinusDefinition();
  G4PionZero::PionZeroDefinition();
  G4Eta::EtaDefinition();
  G4EtaPrime::EtaPrimeDefinition();
  G4KaonPlus::KaonPlusDefinition();
  G4KaonMinus::KaonMinusDefinition();
  G4KaonZero::KaonZeroDefinition();
  G4AntiKaonZero::AntiKaonZeroDefinition();
  G4KaonZeroLong::KaonZeroLongDefinition();
  G4KaonZeroShort::KaonZeroShortDefinition();

  // baryons
  G4Proton::ProtonDefinition();
  G4AntiProton::AntiProtonDefinition();
  G4Neutron::NeutronDefinition();
  G4AntiNeutron::AntiNeutronDefinition();

  // ions
  G4Deuteron::DeuteronDefinition();
  G4Triton::TritonDefinition();
  G4Alpha::AlphaDefinition();
  G4GenericIon::GenericIonDefinition();
  G4He3::He3Definition();
  //  G4IonConstructor MyIonConstructor;
  //  MyIonConstructor.ConstructParticle();
}

void MYNAME_PhysicsList::ConstructProcess()
{
  AddTransportation();
  AddParameterisation();
  ConstructEM();
  ConstructHadronic();
  ConstructGeneral();
  ConstructOp();
}

#include "G4FastSimulationManagerProcess.hh"

void MYNAME_PhysicsList::AddParameterisation()
{
  G4FastSimulationManagerProcess* 
    theFastSimulationManagerProcess = 
      new G4FastSimulationManagerProcess();
  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    // both postStep and alongStep action are required if the detector
    // makes use of ghost volumes. If no ghost, the postStep
    // is sufficient (and faster?).
#define GLG4_USES_GHOST_VOLUMES 0
#if GLG4_USES_GHOST_VOLUMES
    pmanager->AddProcess(theFastSimulationManagerProcess, -1, 1, 1);
#else
    pmanager->AddProcess(theFastSimulationManagerProcess, -1, -1, 1);
#endif
  }
}

#include "G4Decay.hh"

void MYNAME_PhysicsList::ConstructGeneral()
{
  //
  // declare particle decay process for each applicable particle
  // also GLG4DeferTrackProc
  //
  G4Decay* theDecayProcess = new G4Decay();
  //GLG4DeferTrackProc* theDeferProcess= new GLG4DeferTrackProc();
  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    if (theDecayProcess->IsApplicable(*particle)) {
      pmanager->AddDiscreteProcess(theDecayProcess);
      // set ordering for PostStepDoIt and AtRestDoIt
      pmanager->SetProcessOrdering(theDecayProcess, idxPostStep);
      pmanager->SetProcessOrdering(theDecayProcess, idxAtRest);
    }
    // Remove defer track process until such time as we can reassess how to
    // use it best.
    //pmanager->AddDiscreteProcess(theDeferProcess);
  }
  
  //
  //
  // Declare radioactive decay to the GenericIon in the IonTable.
  //
  const G4IonTable *theIonTable =
    G4ParticleTable::GetParticleTable()->GetIonTable();
  G4RadioactiveDecay *theRadioactiveDecay = new G4RadioactiveDecay();
  theRadioactiveDecay->SetVerboseLevel(0);
  theRadioactiveDecay->SetAnalogueMonteCarlo (true);
  theRadioactiveDecay->SetFBeta (true);
  for (G4int i=0; i<theIonTable->Entries(); i++)
  {
    G4String particleName = theIonTable->GetParticle(i)->GetParticleName();
    if (particleName == "GenericIon")
    {
      G4ProcessManager* pmanager =
        theIonTable->GetParticle(i)->GetProcessManager();
      pmanager->SetVerboseLevel(0);
      pmanager ->AddProcess(theRadioactiveDecay);
      pmanager ->SetProcessOrdering(theRadioactiveDecay, idxPostStep);
      pmanager ->SetProcessOrdering(theRadioactiveDecay, idxAtRest);
    }
  }
}

#include "G4AntiNeutronAnnihilationAtRest.hh"
#include "G4AntiProtonAnnihilationAtRest.hh"
#include "G4KaonMinusAbsorptionAtRest.hh"
#include "G4PionMinusAbsorptionAtRest.hh"
#include "G4MuonMinusCaptureAtRest.hh"
// #include "G4NeutronCaptureAtRest.hh" // wrong physics, don't use! [GAHS]

#include "G4HadronElasticProcess.hh"   
#include "G4HadronInelasticProcess.hh" 
#include "G4HadronFissionProcess.hh"   
#include "G4HadronCaptureProcess.hh"         

#include "G4NeutronHPElastic.hh"
#include "G4NeutronHPInelastic.hh"
#include "G4NeutronHPFission.hh"
#include "G4NeutronHPCapture.hh"

// high precision < 20 MeV
#include "G4NeutronHPCaptureData.hh"
#include "G4NeutronHPFissionData.hh"
#include "G4NeutronHPElasticData.hh"
#include "G4NeutronHPInelasticData.hh"

// generic models for 0 to infinite energy (used for E > 20 MeV)
// (these may actually be "translations" of the GEANT3.21/GHEISHA models....)
#include "G4LElastic.hh"
#include "G4LFission.hh"
#include "G4LCapture.hh"

// Low energy (used for 20 MeV < E < 50 GeV)
#include "G4LENeutronInelastic.hh"  // 0 to 55 GeV

// High energy (used for > 50 GeV)
#include "G4HENeutronInelastic.hh"  // 45 GeV to 10 TeV

#include "HadronPhysicsQGSP_BERT.hh"
// Muon Physics
#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"
#include "G4MuonNuclearProcess.hh"



#if (G4VERSION_NUMBER >= 600)
#define G4std  std
inline void AddDataSet(class G4HadronicProcess*p, class G4VCrossSectionDataSet*d) { p->AddDataSet(d); }
#else
inline void AddDataSet(class G4HadronElasticProcess*p, class G4VCrossSectionDataSet*d) { p->GetCrossSectionDataStore()->AddDataSet(d); }
inline void AddDataSet(class G4HadronInelasticProcess*p, class G4VCrossSectionDataSet*d) { p->GetCrossSectionDataStore()->AddDataSet(d); }
inline void AddDataSet(class G4HadronFissionProcess*p, class G4VCrossSectionDataSet*d) { p->GetCrossSectionDataStore()->AddDataSet(d); }
inline void AddDataSet(class G4HadronCaptureProcess*p, class G4VCrossSectionDataSet*d) { p->GetCrossSectionDataStore()->AddDataSet(d); }
#endif




void MYNAME_PhysicsList::ConstructHadronic()
{
  const bool omit_neutron_hp = false;
  

  RAT::DB *db = RAT::DB::Get();
  RAT::DBLinkPtr lmc = db->GetLink("MC");

  if ( lmc->GetI("hadronic_processes") == 0 ) {
    G4cerr << "Warning, Hadronic processes omitted.\n";
    return;
  }

  G4cerr << "Note: +++ INCLUDING neutron_hp model. +++" << G4endl;

    

  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();

    if ( particle == G4AntiNeutron::AntiNeutron() ) {
      pmanager->AddRestProcess(new G4AntiNeutronAnnihilationAtRest());
    }
    
    if ( particle == G4AntiProton::AntiProton() ) {
      pmanager->AddRestProcess(new G4AntiProtonAnnihilationAtRest());
    }
    
    if ( particle == G4KaonMinus::KaonMinus() ) {
      pmanager->AddRestProcess(new G4KaonMinusAbsorptionAtRest());
    }
    
    if ( particle == G4PionMinus::PionMinus() ) {
      pmanager->AddRestProcess(new G4PionMinusAbsorptionAtRest());
    }
    
    if ( particle == G4MuonMinus::MuonMinus() ) {
      pmanager->AddRestProcess(new G4MuonMinusCaptureAtRest(),ordLast);
    }
    
    if ( particle == G4Neutron::Neutron() ) {
      
      G4HadronElasticProcess*   theHadronElasticProcess
	= new   G4HadronElasticProcess();
      G4LElastic* theNeutronLElastic
	= new   G4LElastic();
      if (omit_neutron_hp ) {
	theHadronElasticProcess->RegisterMe( theNeutronLElastic );
      }
      else {
	G4NeutronHPElastic* theNeutronHPElastic
	  = new   G4NeutronHPElastic();
	theNeutronHPElastic->SetMaxEnergy( 20.*MeV );
	theNeutronLElastic->SetMinEnergy( 20.*MeV );
	theHadronElasticProcess->RegisterMe( theNeutronHPElastic );
	theHadronElasticProcess->RegisterMe( theNeutronLElastic );
	AddDataSet(theHadronElasticProcess, new G4NeutronHPElasticData() );
      }
      pmanager->AddDiscreteProcess( theHadronElasticProcess );
      
      G4NeutronInelasticProcess*   theNeutronInelasticProcess
	= new   G4NeutronInelasticProcess();
      G4LENeutronInelastic* theNeutronLENeutronInelastic
	= new   G4LENeutronInelastic();
      G4HENeutronInelastic* theNeutronHENeutronInelastic
	= new   G4HENeutronInelastic();
      if (omit_neutron_hp) {
	theNeutronInelasticProcess->RegisterMe( theNeutronLENeutronInelastic );
	theNeutronInelasticProcess->RegisterMe( theNeutronHENeutronInelastic );
      }
      else {
	G4NeutronHPInelastic* theNeutronHPInelastic
	  = new   G4NeutronHPInelastic();
	theNeutronHPInelastic->SetMaxEnergy( 20.*MeV );
	theNeutronLENeutronInelastic->SetMinEnergy( 20.*MeV );
	theNeutronInelasticProcess->RegisterMe( theNeutronHPInelastic );
	theNeutronInelasticProcess->RegisterMe( theNeutronLENeutronInelastic );
	theNeutronInelasticProcess->RegisterMe( theNeutronHENeutronInelastic );
	AddDataSet(theNeutronInelasticProcess, new G4NeutronHPInelasticData() );
      }
      pmanager->AddDiscreteProcess( theNeutronInelasticProcess );
      
      G4HadronFissionProcess*   theHadronFissionProcess
	= new   G4HadronFissionProcess();
      G4LFission* theNeutronLFission
	= new   G4LFission();
      if (omit_neutron_hp) {
	theHadronFissionProcess->RegisterMe( theNeutronLFission );
      }
      else {
	G4NeutronHPFission* theNeutronHPFission
	  = new   G4NeutronHPFission();
	theNeutronHPFission->SetMaxEnergy( 20.*MeV );
	theNeutronLFission->SetMinEnergy( 20.*MeV );
	theHadronFissionProcess->RegisterMe( theNeutronHPFission );
	theHadronFissionProcess->RegisterMe( theNeutronLFission );
	AddDataSet(theHadronFissionProcess, new G4NeutronHPFissionData() );
      }
      pmanager->AddDiscreteProcess( theHadronFissionProcess );
      
      G4HadronCaptureProcess*   theCaptureProcess
	= new   G4HadronCaptureProcess();
      G4LCapture* theNeutronLCapture
	= new   G4LCapture();
      if (omit_neutron_hp) {
	theCaptureProcess->RegisterMe( theNeutronLCapture );
      }
      else {
	G4NeutronHPCapture* theNeutronHPCapture
	  = new   G4NeutronHPCapture();
	theNeutronHPCapture->SetMaxEnergy( 20.*MeV );
	theNeutronLCapture->SetMinEnergy( 20.*MeV );
	theCaptureProcess->RegisterMe( theNeutronHPCapture );
	theCaptureProcess->RegisterMe( theNeutronLCapture );
	AddDataSet(theCaptureProcess, new G4NeutronHPCaptureData() );
      }
      pmanager->AddDiscreteProcess( theCaptureProcess );

      // special Genericland process that works with GLG4PrimaryGeneratorAction
      // to stack particles from neutron capture after diffusion
      // deactivated by default because it is unexpected by new users
      //GLG4NeutronDiffusionAndCapture*   theNeutronDiffusion
      //= new   GLG4NeutronDiffusionAndCapture();
      //pmanager->AddDiscreteProcess( theNeutronDiffusion );
      //pmanager->SetProcessActivation( theNeutronDiffusion, false );
      
      // ( end neutron )
    }

  }
}

#include "G4RayleighScattering.hh"
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"

#include "G4MuIonisation.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4MuPairProduction.hh"

#include "G4ionIonisation.hh"
#include "G4hBremsstrahlung.hh"
#include "G4hIonisation.hh"
#include "G4hPairProduction.hh"
#include "G4hMultipleScattering.hh"
#include "G4MuMultipleScattering.hh"

void MYNAME_PhysicsList::ConstructEM()
{
  RAT::DB *db = RAT::DB::Get();
  RAT::DBLinkPtr lmc = db->GetLink("MC");

  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();

    if (particleName == "gamma") {
      pmanager->AddDiscreteProcess(new G4RayleighScattering);
      pmanager->AddDiscreteProcess(new G4PhotoElectricEffect);
      pmanager->AddDiscreteProcess(new G4ComptonScattering);
      pmanager->AddDiscreteProcess(new G4GammaConversion);
    } else if (particleName == "e-") {

      G4eMultipleScattering* msc = new G4eMultipleScattering();
      msc->SetStepLimitType(fUseDistanceToBoundary);
      pmanager->AddProcess(msc,                   -1, 1, 1);
      G4eIonisation* eIoni = new G4eIonisation();
      eIoni->SetStepFunction(0.2, 10*um);
      pmanager->AddProcess(eIoni,                 -1, 2, 2);
      // bremsstrahlung has to read: -1, 3, 3 for std em physics
      // or: -1,-1, 3 for low eng em (G4LowEnergyBremsstrahlung)
      // -1, -1,  3 is std compatible     
      pmanager->AddProcess(new G4eBremsstrahlung, -1, 3, 3);

    } else if (particleName == "e+") {

      G4eMultipleScattering* msc = new G4eMultipleScattering();
      msc->SetStepLimitType(fUseDistanceToBoundary);
      pmanager->AddProcess(msc,                   -1, 1, 1);
      G4eIonisation* eIoni = new G4eIonisation();
      eIoni->SetStepFunction(0.2, 10*um);
      pmanager->AddProcess(eIoni,                 -1, 2, 2);
      pmanager->AddProcess(new G4eBremsstrahlung, -1, 3, 3);
      pmanager->AddProcess(new G4eplusAnnihilation,0,-1, 4);

    } else if( particleName == "mu+" ||
               particleName == "mu-"    ) {

      if ( lmc->GetI("muon_processes") == 0 ) {
	G4cerr << "Warning, muon processes omitted.\n";
      }
      else {
	pmanager->AddProcess(new G4MuMultipleScattering, -1, 1, 1);
	G4MuIonisation* muIoni = new G4MuIonisation();
	muIoni->SetStepFunction(0.2, 50*um);
	pmanager->AddProcess(muIoni,                    -1, 2, 2);
	pmanager->AddProcess(new G4MuBremsstrahlung,    -1,-3, 3);
	pmanager->AddProcess(new G4MuPairProduction,    -1,-4, 4);

	// move to hadronic?	
	  pmanager->AddProcess(new G4MuonNuclearProcess(),-1,-1,5);
      }

    } else if (particleName == "alpha" ||
             particleName == "He3" ||
	       particleName == "GenericIon") {

      pmanager->AddProcess(new G4hMultipleScattering, -1, 1, 1);
      G4ionIonisation* ionIoni = new G4ionIonisation();
      ionIoni->SetStepFunction(0.1, 2*um);
      pmanager->AddProcess(ionIoni,                   -1, 2, 2);

    } else if (particleName == "pi+" ||
             particleName == "pi-" ||
	       particleName == "proton" ) {

      pmanager->AddProcess(new G4hMultipleScattering, -1, 1, 1);
      G4hIonisation* hIoni = new G4hIonisation();
      hIoni->SetStepFunction(0.2, 5*um);
      pmanager->AddProcess(hIoni,                     -1, 2, 2);
      pmanager->AddProcess(new G4hBremsstrahlung,     -1,-3, 3);
      pmanager->AddProcess(new G4hPairProduction,     -1,-4, 4);

    } else if (particleName == "B+" ||
               particleName == "B-" ||
               particleName == "D+" ||
               particleName == "D-" ||
               particleName == "Ds+" ||
               particleName == "Ds-" ||
             particleName == "anti_lambda_c+" ||
             particleName == "anti_omega-" ||
             particleName == "anti_proton" ||
             particleName == "anti_sigma_c+" ||
             particleName == "anti_sigma_c++" ||
             particleName == "anti_sigma+" ||
             particleName == "anti_sigma-" ||
             particleName == "anti_xi_c+" ||
             particleName == "anti_xi-" ||
             particleName == "deuteron" ||
               particleName == "kaon+" ||
             particleName == "kaon-" ||
               particleName == "lambda_c+" ||
             particleName == "omega-" ||
             particleName == "sigma_c+" ||
             particleName == "sigma_c++" ||
             particleName == "sigma+" ||
             particleName == "sigma-" ||
             particleName == "tau+" ||
             particleName == "tau-" ||
             particleName == "triton" ||
             particleName == "xi_c+" ||
	       particleName == "xi-" ) {

      pmanager->AddProcess(new G4hMultipleScattering, -1, 1, 1);
      pmanager->AddProcess(new G4hIonisation,         -1, 2, 2);

    } else if ((!particle->IsShortLived()) &&
               (particle->GetPDGCharge() != 0.0) &&
               (particle->GetParticleName() != "chargedgeantino")) {
      //all others charged particles except geantino
      /*
    pmanager->AddProcess(new G4MultipleScattering,-1, 1, 1);
    pmanager->AddProcess(new G4hIonisation,       -1, 2, 2);
      */
    }
  }// iterator


  // Em options
  //
  G4EmProcessOptions opt;
  opt.SetVerbose(1);

  // Multiple Coulomb scattering
  //
  //opt.SetMscStepLimitation(fUseDistanceToBoundary);
  //opt.SetMscRangeFactor(0.02);

  // Physics tables
  //
  opt.SetMinEnergy(100*eV);
  //opt.SetMaxEnergy(100*GeV);
  opt.SetMaxEnergy(100*TeV);
  opt.SetDEDXBinning(220);
  opt.SetLambdaBinning(220);
  //opt.SetSplineFlag(true);

  // Ionization
  //
  //opt.SetSubCutoff(true);
}

#include "RAT/Cerenkov.hh"
#include "RAT/WlsScintillation.hh"
#include "G4Scintillation.hh"
#include "G4OpBoundaryProcess.hh"
#include "GLG4OpAttenuation.hh"
#include "RAT/OpWLS.hh"
#include "GLG4Scint.hh"
#include "RAT/GLG4SteppingAction.hh"
#include "RAT/G4S1Light.hh" // NEST

void MYNAME_PhysicsList::ConstructOp()
{
  RAT::Cerenkov* theCerenkovProcess = new RAT::Cerenkov("Cerenkov");

  GLG4OpAttenuation* theAttenuationProcess = new GLG4OpAttenuation();
  G4OpBoundaryProcess* theBoundaryProcess  = new G4OpBoundaryProcess();

  //  GLG4OpAttenuation implements Rayleigh scattering.
  //  G4OpRayleigh is not used for the following two reasons:
  //    1) It doesn't even try to work for anything other than water.
  //    2) It doesn't actually work for water, either.

  RAT::OpWLS* theWLSProcess = new RAT::OpWLS();

  RAT::DBLinkPtr lmc = RAT::DB::Get()->GetLink("MC");

  // Check whether using the standard or NEST scintillation module
  G4String scintPhysList;
  try { scintPhysList = lmc->GetS("scintillation"); }
  catch (RAT::DBNotFoundError &e) { scintPhysList = "glg4"; }

  RAT::G4S1Light* theNestScintProcess=0;

  if ( scintPhysList == "nest" ) {
    theNestScintProcess = new RAT::G4S1Light("Scintillation");
    theNestScintProcess->SetScintillationYieldFactor(1.);
    theNestScintProcess->SetTrackSecondariesFirst(false);

    GLG4SteppingAction::fUseGLG4 = false;
  }
  else if ( scintPhysList == "glg4" ) {

    G4double protonMass= G4Proton::Proton()->GetPDGMass();
  
    // e-like scintillation
    GLG4Scint* theDefaultScintProcess = new GLG4Scint();

    // Nuclear recoil-like scintillation (from A=10 nuclei)
    GLG4Scint* theHeavyScintProcess = new GLG4Scint("heavy", 0.9*protonMass);

    if (verboseLevel > 0) {
      theDefaultScintProcess->DumpInfo();
      theHeavyScintProcess->DumpInfo();
    }

    theDefaultScintProcess->SetVerboseLevel(1);
    theHeavyScintProcess->SetVerboseLevel(1);
  }
  else
    printf("Unknown scintillation physics list");

  G4String tpbScintPhysList;
  try{ tpbScintPhysList = lmc->GetS("tpb_scintillation"); }
  catch (RAT::DBNotFoundError &e) { tpbScintPhysList = "g4scint"; }

  G4Scintillation* theG4scintAlphaInTpbScintProcess = 0;
  WlsScintillation* theAlphaInTpbScintProcess = 0;
  WlsScintillation* theElectronInTpbScintProcess = 0;
  WlsScintillation* theProtonInTpbScintProcess = 0;
  WlsScintillation* theIonInTpbScintProcess = 0;
  if(tpbScintPhysList == "g4scint"){
    theG4scintAlphaInTpbScintProcess = new G4Scintillation("AlphaInTpbScintillation");  
    theG4scintAlphaInTpbScintProcess->SetTrackSecondariesFirst(true);
    theG4scintAlphaInTpbScintProcess->SetScintillationYieldFactor(1.0);
    theG4scintAlphaInTpbScintProcess->SetScintillationExcitationRatio(0.67);  
  }
  else if(tpbScintPhysList == "wlsscint"){
    theAlphaInTpbScintProcess = new WlsScintillation("AlphaInTpbScintillation");  
    theAlphaInTpbScintProcess->SetTrackSecondariesFirst(false);
    theAlphaInTpbScintProcess->SetScintillationYieldFactor(1.0);
    theAlphaInTpbScintProcess->SetScintillationExcitationRatio(0.33);

    theElectronInTpbScintProcess = new WlsScintillation("ElectronInTpbScintillation");
    theElectronInTpbScintProcess->SetTrackSecondariesFirst(false);
    theElectronInTpbScintProcess->SetScintillationYieldFactor(4.44); // IEEE TRANSACTIONS ON NUCLEAR SCIENCE, VOL. 56, NO. 3, JUNE 2009, normalized to alphas from Tina'a paper
    theElectronInTpbScintProcess->SetScintillationExcitationRatio(0.926);

    theProtonInTpbScintProcess = new WlsScintillation("ProtonInTpbScintillation");
    theProtonInTpbScintProcess->SetTrackSecondariesFirst(false);
    theProtonInTpbScintProcess->SetScintillationYieldFactor(1.); // assumed same as for alphas
    theProtonInTpbScintProcess->SetScintillationExcitationRatio(0.33); // assumed same as for alphas

    theIonInTpbScintProcess = new WlsScintillation("IonInTpbScintillation");
    theIonInTpbScintProcess->SetTrackSecondariesFirst(false);
    theIonInTpbScintProcess->SetScintillationYieldFactor(0.25/0.81); // a guess, FIXME?
    theIonInTpbScintProcess->SetScintillationExcitationRatio(0.2); // IEEE TRANSACTIONS ON NUCLEAR SCIENCE, VOL. 56, NO. 3, JUNE 2009
  }
  else
    printf("Unkown TPB scintillation physics list");

  // request that cerenkov and scintillation photons be tracked first,
  // before continuing originating particle step.  Otherwise, we
  // get too many secondaries!
  theCerenkovProcess->SetTrackSecondariesFirst(true);

  if (verboseLevel > 0) {
    theCerenkovProcess->DumpInfo();
    theAttenuationProcess->DumpInfo();
    theWLSProcess->DumpInfo();
  }

  theCerenkovProcess->SetVerboseLevel(0);
  theAttenuationProcess->SetVerboseLevel(0);
  theWLSProcess->SetVerboseLevel(0);

  // For more accurate multiple scattering
  G4int MaxNumPhotons = 4;
  theCerenkovProcess->SetTrackSecondariesFirst(true);
  theCerenkovProcess->SetMaxNumPhotonsPerStep(MaxNumPhotons);

  theParticleIterator->reset();
  while( (*theParticleIterator)() ){
    G4ParticleDefinition* particle = theParticleIterator->value();
    G4ProcessManager* pmanager = particle->GetProcessManager();
    G4String particleName = particle->GetParticleName();
    if (theCerenkovProcess->IsApplicable(*particle)) {
      pmanager->AddProcess(theCerenkovProcess);
      pmanager->SetProcessOrdering(theCerenkovProcess,idxPostStep);
    }
    if (particleName == "opticalphoton") {
      pmanager->AddDiscreteProcess(theAttenuationProcess);
      pmanager->AddDiscreteProcess(theBoundaryProcess);

      theWLSProcess->UseTimeProfile("exponential");
      pmanager->AddDiscreteProcess(theWLSProcess);

    }
    if (theNestScintProcess)
      if (theNestScintProcess->IsApplicable(*particle)){
	printf("Defined Ar scintillation using NEST\n");
	pmanager->AddProcess(theNestScintProcess,ordDefault,ordInActive,ordDefault);
      }
    if (theG4scintAlphaInTpbScintProcess && particleName == "alpha"){
	printf("Defined alpha scintillation in TPB using G4Scintillation\n");
	pmanager->AddProcess(theG4scintAlphaInTpbScintProcess);
	pmanager->SetProcessOrderingToLast(theG4scintAlphaInTpbScintProcess, idxAtRest);
	pmanager->SetProcessOrderingToLast(theG4scintAlphaInTpbScintProcess, idxPostStep);
    }
    if (theAlphaInTpbScintProcess && particleName == "alpha")
      if (theAlphaInTpbScintProcess->IsApplicable(*particle)) {
	printf("Defined alpha scintillation in TPB using WlsScintillation\n");
	pmanager->AddProcess(theAlphaInTpbScintProcess);
	pmanager->SetProcessOrderingToLast(theAlphaInTpbScintProcess, idxAtRest);
	pmanager->SetProcessOrderingToLast(theAlphaInTpbScintProcess, idxPostStep);
      }
    if (theElectronInTpbScintProcess && particleName == "e-")
      if(theElectronInTpbScintProcess->IsApplicable(*particle)) {
	printf("Defined e- scintillation in TPB using WlsScintillation\n");
	pmanager->AddProcess(theElectronInTpbScintProcess);
	pmanager->SetProcessOrderingToLast(theElectronInTpbScintProcess, idxAtRest);
	pmanager->SetProcessOrderingToLast(theElectronInTpbScintProcess, idxPostStep);
      }
    if(theProtonInTpbScintProcess && particleName == "proton")
      if(theProtonInTpbScintProcess->IsApplicable(*particle)) {
	printf("Defined proton scintillation in TPB using WlsScintillation\n");
	pmanager->AddProcess(theProtonInTpbScintProcess);
	pmanager->SetProcessOrderingToLast(theProtonInTpbScintProcess, idxAtRest);
	pmanager->SetProcessOrderingToLast(theProtonInTpbScintProcess, idxPostStep);
      }
    if(theIonInTpbScintProcess && particleName == "GenericIon")
      if(theIonInTpbScintProcess->IsApplicable(*particle)) {
	printf("Defined ion scintillation in TPB using WlsScintillation\n");
	pmanager->AddProcess(theIonInTpbScintProcess);
	pmanager->SetProcessOrderingToLast(theIonInTpbScintProcess, idxAtRest);
	pmanager->SetProcessOrderingToLast(theIonInTpbScintProcess, idxPostStep);
      }

    //
    // if using GLG4scint, argon scintillation process is no longer a 
    // Geant4 "Process", so it is not added to the process manager's list
    //
  }
}



void MYNAME_PhysicsList::SetCuts()
{
  G4LossTableManager::Instance();
  currentDefaultCut   = 0.010 * mm;
  cutForGamma         = currentDefaultCut;
  cutForElectron      = currentDefaultCut;
  cutForPositron      = currentDefaultCut;


  double cutForHadron = currentDefaultCut;
  double cutForIon    = currentDefaultCut;

  // set cut values for gamma at first and for e- second and next for e+,
  // because some processes for e+/e- need cut values for gamma
  SetCutValue(cutForGamma, "gamma");
  SetCutValue(cutForElectron, "e-");
  SetCutValue(cutForPositron, "e+");

  // set cut values for proton and anti_proton before all other hadrons
  // because some processes for hadrons need cut values for proton/anti_proton
  SetCutValue(cutForHadron, "proton");
  SetCutValue(cutForHadron, "anti_proton");
  SetCutValue(cutForHadron, "neutron");
  SetCutValue(cutForIon, "alpha");
  SetCutValue(cutForIon, "GenericIon");

  if (verboseLevel>0) {
      DumpCutValuesTable();
      G4cout << (MYNAME_string "PhysicsList::SetCuts: done") << G4endl;
  }
}
