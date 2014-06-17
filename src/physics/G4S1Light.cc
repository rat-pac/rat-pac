//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The NEST program is intended for use with the Geant4 software,   *
// * which is copyright of the Copyright Holders of the Geant4        *
// * Collaboration. This additional software is copyright of the NEST *
// * development team. As such, it is subject to the terms and        *
// * conditions of both the Geant4 License, included with your copy   *
// * of Geant4 and available at http://cern.ch/geant4/license, as     *
// * well as the NEST License included with the download of NEST and  *
// * available at http://nest.physics.ucdavis.edu/                    *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutions, nor the agencies providing financial support for   *
// * this work make any representation or warranty, express or        *
// * implied, regarding this software system, or assume any liability *
// * for its use. Please read the pdf license or view it online       *
// * before download for the full disclaimer and lack of liability.   *
// *                                                                  *
// * This code implementation is based on work by Peter Gumplinger    *
// * and his fellow collaborators on Geant4 and is distributed with   *
// * the express written consent of the Geant4 collaboration. By      *
// * using, copying, modifying, or sharing the software (or any work  *
// * based on the software) you agree to acknowledge use of both NEST *
// * and Geant4 in resulting scientific publications, and you         *
// * indicate your acceptance of all the terms and conditions of the  *
// * licenses, which must always be included with this code.          *
// ********************************************************************
//
//
// Noble Element Simulation Technique "G4S1Light" physics process
//
////////////////////////////////////////////////////////////////////////
// S1 Scintillation Light Class Implementation
////////////////////////////////////////////////////////////////////////
//
// File:        G4S1Light.cc (lives in physicslist/src)
// Description: (Rest)Discrete Process - Generation of S1 Photons
// Version:     0.98 Final
// Created:     Thursday, January 17, 2013 12:35 PM
// Author:      Matthew Szydagis, UC Davis
//
// mail:        mmszydagis@ucdavis.edu, matthew.szydagis@gmail.com
//
////////////////////////////////////////////////////////////////////////

#include "G4ParticleTypes.hh" //lets you refer to G4OpticalPhoton, etc.
#include "G4EmProcessSubType.hh" //lets you call this process Scintillation
#include "G4Version.hh" //tells you what Geant4 version you are running

#include "G4S1Light.hh"

namespace RAT {

#define MIN_ENE -1*eV //lets you turn NEST off BELOW a certain energy
#define MAX_ENE 1.*TeV //lets you turn NEST off ABOVE a certain energy
#define HIENLIM 5*MeV //energy at which Doke model used exclusively

#define R_TOL 0.2*mm //tolerance (for edge events)
#define R_MAX 1*km //for corraling diffusing electrons
G4bool diffusion = true;

G4bool SinglePhase=true, ThomasImelTail=true, OutElectrons=true;
G4double GetGasElectronDriftSpeed(G4double efieldinput,G4double density);
G4double GetLiquidElectronDriftSpeed(double T, double F, G4bool M, G4int Z);
G4double CalculateElectronLET ( G4double E, G4int Z );

G4double biExc = 0.77; //for alpha particles (bi-excitonic collisions)
G4double UnivScreenFunc ( G4double E, G4double Z, G4double A );

G4int BinomFluct(G4int N0, G4double prob); //function for doing fluctuations

void InitMatPropValues ( G4MaterialPropertiesTable* nobleElementMat );

#define Density_LXe 2.9 //reference density for density-dep. effects
#define Density_LAr 1.393
#define Density_LNe 1.207
#define Density_LKr 2.413

G4S1Light::G4S1Light(const G4String& processName,
		           G4ProcessType type)
      : G4VRestDiscreteProcess(processName, type)
{
        //luxManager = LUXSimManager::GetManager();
        SetProcessSubType(fScintillation);
	
        fTrackSecondariesFirst = false;
	//particles die first, then scintillation is generated
	
        if (verboseLevel>0) {
	  G4cout << GetProcessName() << " is created " << G4endl;
        }
}

G4S1Light::~G4S1Light(){} //destructor needed to avoid linker error

G4VParticleChange*
G4S1Light::AtRestDoIt(const G4Track& aTrack, const G4Step& aStep)

// This routine simply calls the equivalent PostStepDoIt since all the
// necessary information resides in aStep.GetTotalEnergyDeposit()
{
  return G4S1Light::PostStepDoIt(aTrack, aStep);
}

G4VParticleChange*
G4S1Light::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep)
// this is the most important function, where all light & charge yields happen!
{
        aParticleChange.Initialize(aTrack);
	
	if ( !YieldFactor ) //set YF=0 when you want S1Light off in your sim
          return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
	
	if( aTrack.GetParentID() == 0 && aTrack.GetCurrentStepNumber() == 1 ) {
	  fExcitedNucleus = false; //an initialization or reset
	  fVeryHighEnergy = false; //initializes or (later) resets this
	  fAlpha = false; //ditto
	  fMultipleScattering = false;
	}

        const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
	G4ParticleDefinition *pDef = aParticle->GetDefinition();
	G4String particleName = pDef->GetParticleName();
        const G4Material* aMaterial = aStep.GetPreStepPoint()->GetMaterial();
	const G4Material* bMaterial = aStep.GetPostStepPoint()->GetMaterial();
	
	if((particleName == "neutron" || particleName == "antineutron") &&
	   aStep.GetTotalEnergyDeposit() <= 0)
	  return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
	
	// code for determining whether the present/next material is noble
	// element, or, in other words, for checking if either is a valid NEST
	// scintillating material, and save Z for later L calculation, or
	// return if no valid scintillators are found on this step, which is
	// protection against G4Exception or seg. fault/violation
	G4Element *ElementA = NULL, *ElementB = NULL;
	if (aMaterial) {
	  const G4ElementVector* theElementVector1 =
	    aMaterial->GetElementVector();
	  ElementA = (*theElementVector1)[0];
	}
	if (bMaterial) {
	  const G4ElementVector* theElementVector2 =
	    bMaterial->GetElementVector();
	  ElementB = (*theElementVector2)[0];
	}
	G4int z1,z2,j=1; G4bool NobleNow=false,NobleLater=false;
	if (ElementA) z1 = (G4int)(ElementA->GetZ()); else z1 = -1;
	if (ElementB) z2 = (G4int)(ElementB->GetZ()); else z2 = -1;
	if ( z1==2 || z1==10 || z1==18 || z1==36 || z1==54 ) {
	  NobleNow = true;
	  j = (G4int)aMaterial->GetMaterialPropertiesTable()->
	    GetConstProperty("TOTALNUM_INT_SITES"); //get current number
	  if ( j < 0 ) {
	    InitMatPropValues(aMaterial->GetMaterialPropertiesTable());
	    j = 0; //no sites yet
	  } //material properties initialized
	} //end of atomic number check
	if ( z2==2 || z2==10 || z2==18 || z2==36 || z2==54 ) {
	  NobleLater = true;
	  j = (G4int)bMaterial->GetMaterialPropertiesTable()->
	    GetConstProperty("TOTALNUM_INT_SITES");
	  if ( j < 0 ) {
	    InitMatPropValues(bMaterial->GetMaterialPropertiesTable());
	    j = 0; //no sites yet
	  } //material properties initialized
	} //end of atomic number check
	
	if ( !NobleNow && !NobleLater )
	  return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
	
	// retrieval of the particle's position, time, attributes at both the 
	// beginning and the end of the current step along its track
        G4StepPoint* pPreStepPoint  = aStep.GetPreStepPoint();
	G4StepPoint* pPostStepPoint = aStep.GetPostStepPoint();
	G4ThreeVector x1 = pPostStepPoint->GetPosition();
        G4ThreeVector x0 = pPreStepPoint->GetPosition();
	G4double evtStrt = pPreStepPoint->GetGlobalTime();
        G4double      t0 = pPreStepPoint->GetLocalTime();
	G4double      t1 = pPostStepPoint->GetLocalTime();
	
	// now check if we're entering a scintillating material (inside) or
        // leaving one (outside), in order to determine (later on in the code,
        // based on the booleans inside & outside) whether to add/subtract
        // energy that can potentially be deposited from the system
	G4bool outside = false, inside = false, InsAndOuts = false;
        G4MaterialPropertiesTable* aMaterialPropertiesTable =
	  aMaterial->GetMaterialPropertiesTable();
	if ( NobleNow && !NobleLater ) outside = true;
	if ( !NobleNow && NobleLater ) {
	  aMaterial = bMaterial; inside = true; z1 = z2;
	  aMaterialPropertiesTable = bMaterial->GetMaterialPropertiesTable();
	}
	if ( NobleNow && NobleLater && 
	     aMaterial->GetDensity() != bMaterial->GetDensity() )
	  InsAndOuts = true;
	
	//      Get the LUXSimMaterials pointer
        //LUXSimMaterials *luxMaterials = LUXSimMaterials::GetMaterials();
	
	// retrieve scintillation-related material properties
	G4double Density = aMaterial->GetDensity()/(g/cm3);
	G4double nDensity = Density*AVO; //molar mass factor applied below
	G4int Phase = aMaterial->GetState(); //solid, liquid, or gas?
	G4double ElectricField, FieldSign; //for field quenching of S1
	G4bool GlobalFields = false;
	if ( !WIN && !TOP && !ANE && !SRF && !GAT && !CTH && !BOT && !PMT ) {
	  ElectricField = aMaterialPropertiesTable->
            GetConstProperty("ELECTRICFIELD"); GlobalFields = true; }
	else {
	  if ( x1[2] < WIN && x1[2] > TOP && Phase == kStateGas )
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELDWINDOW");
	  else if ( x1[2] < TOP && x1[2] > ANE && Phase == kStateGas )
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELDTOP");
	  else if ( x1[2] < ANE && x1[2] > SRF && Phase == kStateGas )
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELDANODE");
	  else if ( x1[2] < SRF && x1[2] > GAT && Phase == kStateLiquid )
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELDSURFACE");
	  else if ( x1[2] < GAT && x1[2] > CTH && Phase == kStateLiquid )
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELDGATE");
	  else if ( x1[2] < CTH && x1[2] > BOT && Phase == kStateLiquid )
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELDCATHODE");
	  else if ( x1[2] < BOT && x1[2] > PMT && Phase == kStateLiquid )
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELDBOTTOM");
	  else
	    ElectricField = aMaterialPropertiesTable->
	      GetConstProperty("ELECTRICFIELD");
	}
        if ( ElectricField >= 0 ) FieldSign = 1; else FieldSign = -1;
	ElectricField = fabs((1e3*ElectricField)/(kilovolt/cm));
	G4double Temperature = aMaterial->GetTemperature();
	G4double ScintillationYield, ResolutionScale, R0 = 1.0*um,
	  DokeBirks[3], ThomasImel = 0.00, delta = 1*mm;
	DokeBirks[0] = 0.00; DokeBirks[2] = 1.00;
	G4double PhotMean = 7*eV, PhotWidth = 1.0*eV; //photon properties
	G4double SingTripRatioR, SingTripRatioX, tau1, tau3, tauR = 0*ns;
	switch ( z1 ) { //sort prop. by noble element atomic#
	case 2: //helium
	  ScintillationYield = 1 / (41.3*eV); //all W's from noble gas book
	  ExcitationRatio = 0.00; //nominal (true value unknown)
	  ResolutionScale = 0.2; //Aprile, Bolotnikov, Bolozdynya, Doke
	  PhotMean = 15.9*eV;
	  tau1 = G4RandGauss::shoot(10.0*ns,0.0*ns);
	  tau3 = 1.6e3*ns;
          tauR = G4RandGauss::shoot(13.00*s,2.00*s); //McKinsey et al. 2003
	  break;
	case 10: //neon
	  ScintillationYield = 1 / (29.2*eV);
	  ExcitationRatio = 0.00; //nominal (true value unknown)
	  ResolutionScale = 0.13; //Aprile et. al book
	  PhotMean = 15.5*eV; PhotWidth = 0.26*eV;
	  tau1 = G4RandGauss::shoot(10.0*ns,10.*ns);
          tau3 = G4RandGauss::shoot(15.4e3*ns,200*ns); //Nikkel et al. 2008
	  break;
	case 18: //argon
	  if ( Phase == kStateGas ) {
		ScintillationYield = 1 / (67.9*eV); //Chandrasekharan et al. NIM A 546 (2005) 426
		ResolutionScale = 0.2; //Aprile et. al book
	  }
	  else {
		ScintillationYield = 1 / (19.5*eV);
		ResolutionScale = 0.107; //Doke 1976
	  }
	  ExcitationRatio = 0.21; //Aprile et. al book
	  R0 = 1.568*um; //Mozumder 1995
	  if(ElectricField) {
	    ThomasImel = 0.156977*pow(ElectricField,-0.1);
	    DokeBirks[0] = 0.07*pow((ElectricField/1.0e3),-0.85);
	    DokeBirks[2] = 0.00;
	  }
	  else {
	    ThomasImel = 0.099;
	    DokeBirks[0] = 0.0003;
	    DokeBirks[2] = 0.75;
	  } nDensity /= 39.948; //molar mass in grams per mole
	  PhotMean = 9.69*eV; PhotWidth = 0.22*eV;

	  tau1 = G4RandGauss::shoot(6.5*ns,0.8*ns); //err from wgted avg.
	  if ( Phase == kStateGas ) {
		tau3 = G4RandGauss::shoot(3200*ns,50*ns); //Amsler et al. 2008 JINST 3 P02001
	  }
	  else {
		tau3 = G4RandGauss::shoot(1453*ns,10*ns); //Acciarri et al. 2010 JINST 5 P06003
	  }
	  tauR = G4RandGauss::shoot(0.8*ns,0.2*ns); //Kubota 1979
	  biExc = 0.6; break;
	case 36: //krypton
	  if ( Phase == kStateGas ) ScintillationYield = 1 / (30.0*eV);
	  else ScintillationYield = 1 / (15.0*eV);
	  ExcitationRatio = 0.08; //Aprile et. al book
	  ResolutionScale = 0.05; //Doke 1976
	  PhotMean = 8.43*eV;
	  tau1 = G4RandGauss::shoot(2.8*ns,.04*ns);
	  tau3 = G4RandGauss::shoot(93.*ns,1.1*ns);
	  tauR = G4RandGauss::shoot(12.*ns,.76*ns);
	  break;
	case 54: //xenon
	default: nDensity /= 131.293;
	  ScintillationYield = 48.814+0.80877*Density+2.6721*pow(Density,2.);
	  ScintillationYield /= keV; //Units: [#quanta(ph/e-) per keV]
	  //W = 13.7 eV for all recoils, Dahl thesis (that's @2.84 g/cm^3)
	  //the exciton-to-ion ratio (~0.06 for LXe at 3 g/cm^3)
	  ExcitationRatio = 0.4-0.11131*Density-0.0026651*pow(Density,2.);
	  ResolutionScale = 1.00 * //Fano factor <<1
	    (0.12724-0.032152*Density-0.0013492*pow(Density,2.));
	  //~0.1 for GXe w/ formula from Bolotnikov et al. 1995
	  if ( Phase == kStateLiquid ) {
	    ResolutionScale *= 1.5; //to get it to be ~0.03 for LXe
	    R0 = 16.6*um; //for zero electric field
	    //length scale above which Doke model used instead of Thomas-Imel
	    if(ElectricField) //change it with field (see NEST paper)
	      R0 = 69.492*pow(ElectricField,-0.50422)*um;
	    if(ElectricField) { //formulae & values all from NEST paper
	      DokeBirks[0]= 19.171*pow(ElectricField+25.552,-0.83057)+0.026772;
	      DokeBirks[2] = 0.00; //only volume recombination (above)
	    }
	    else { //zero electric field magnitude
	      DokeBirks[0] = 0.18; //volume/columnar recombination factor (A)
	      DokeBirks[2] = 0.58; //geminate/Onsager recombination (C)
	    }
	    //"ThomasImel" is alpha/(a^2*v), the recomb. coeff.
	    ThomasImel = 0.05; //aka xi/(4*N_i) from the NEST paper
	    //distance used to determine when one is at a new interaction site
	    delta = 0.4*mm; //distance ~30 keV x-ray travels in LXe
	    PhotMean = 6.97*eV; PhotWidth = 0.23*eV;
	    // 178+/-14nmFWHM, taken from Jortner JchPh 42 '65.
	    //these singlet and triplet times may not be the ones you're
	    //used to, but are the world average: Kubota 79, Hitachi 83 (2
	    //data sets), Teymourian 11, Morikawa 89, and Akimov '02
	    tau1 = G4RandGauss::shoot(3.1*ns,.7*ns); //err from wgted avg.
	    tau3 = G4RandGauss::shoot(24.*ns,1.*ns); //ibid.
	  } //end liquid
	  else if ( Phase == kStateGas ) {
	    if(!fAlpha) ExcitationRatio=0.07; //Nygren NIM A 603 (2009) p. 340
	    else { biExc = 1.00;
	      ScintillationYield = 1 / (12.98*eV); } //Saito 2003
	    R0 = 0.0*um; //all Doke/Birks interactions (except for alphas)
	    G4double Townsend = (ElectricField/nDensity)*1e17;
	    DokeBirks[0] = 0.0000; //all geminate (except at zero, low fields)
	    DokeBirks[2] = 0.1933*pow(Density,2.6199)+0.29754 - 
	      (0.045439*pow(Density,2.4689)+0.066034)*log10(ElectricField);
	    if ( ElectricField>6990 ) DokeBirks[2]=0.0;
	    if ( ElectricField<1000 ) DokeBirks[2]=0.2;
	    if ( ElectricField<100. ) { DokeBirks[0]=0.18; DokeBirks[2]=0.58; }
	    if( Density < 0.061 ) ThomasImel = 0.041973*pow(Density,1.8105);
	    else if( Density >= 0.061 && Density <= 0.167 )
	      ThomasImel=5.9583e-5+0.0048523*Density-0.023109*pow(Density,2.);
	    else ThomasImel = 6.2552e-6*pow(Density,-1.9963);
	  if(ElectricField)ThomasImel=1.2733e-5*pow(Townsend/Density,-0.68426);
	    // field\density dependence from Kobayashi 2004 and Saito 2003
	    PhotMean = 7.1*eV; PhotWidth = 0.2*eV;
	    tau1 = G4RandGauss::shoot(5.18*ns,1.55*ns);
	    tau3 = G4RandGauss::shoot(100.1*ns,7.9*ns);
	  } //end gas information (preliminary guesses)
	  else {
	    tau1 = 3.5*ns; tau3 = 20.*ns; tauR = 40.*ns;
	  } //solid Xe
	}
	
	// log present and running tally of energy deposition in this section
	G4double anExcitationEnergy = ((const G4Ions*)(pDef))->
	  GetExcitationEnergy(); //grab nuclear energy level
        G4double TotalEnergyDeposit = //total energy deposited so far
          aMaterialPropertiesTable->GetConstProperty( "ENERGY_DEPOSIT_TOT" );
	G4bool convert = false, annihil = false;
	//set up special cases for pair production and positron annihilation
	if(pPreStepPoint->GetKineticEnergy()>=(2*electron_mass_c2) && 
	   !pPostStepPoint->GetKineticEnergy() && 
	   !aStep.GetTotalEnergyDeposit() && aParticle->GetPDGcode()==22) {
	  convert = true; TotalEnergyDeposit = electron_mass_c2;
	}
	if(pPreStepPoint->GetKineticEnergy() && 
	   !pPostStepPoint->GetKineticEnergy() && 
	   aParticle->GetPDGcode()==-11) {
	  annihil = true; TotalEnergyDeposit += aStep.GetTotalEnergyDeposit();
	}
	G4bool either = false;
        if(inside || outside || convert || annihil || InsAndOuts) either=true;
        //conditions for returning when energy deposits too low
        if( anExcitationEnergy<100*eV && aStep.GetTotalEnergyDeposit()<1*eV &&
	   !either && !fExcitedNucleus )
          return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
	//add current deposit to total energy budget
        if ( !annihil ) TotalEnergyDeposit += aStep.GetTotalEnergyDeposit();
        if ( !convert ) aMaterialPropertiesTable->
	  AddConstProperty( "ENERGY_DEPOSIT_TOT", TotalEnergyDeposit );
	//save current deposit for determining number of quanta produced now
        TotalEnergyDeposit = aStep.GetTotalEnergyDeposit();
	
	// check what the current "goal" E is for dumping scintillation,
	// often the initial kinetic energy of the parent particle, and deal
	// with all other energy-related matters in this block of code
	G4double InitialKinetEnergy = aMaterialPropertiesTable->
	  GetConstProperty( "ENERGY_DEPOSIT_GOL" );
	//if zero, add up initial potential and kinetic energies now
        if ( InitialKinetEnergy == 0 ) {
	  G4double tE = pPreStepPoint->GetKineticEnergy()+anExcitationEnergy;
	  if ( (fabs(tE-1.8*keV) < eV || fabs(tE-9.4*keV) < eV) &&
	       Phase == kStateLiquid && z1 == 54 ) tE = 9.4*keV;
	  if ( fKr83m && ElectricField != 0 )
	    DokeBirks[2] = 0.10;
          aMaterialPropertiesTable->
	    AddConstProperty ( "ENERGY_DEPOSIT_GOL", tE );
	  //excited nucleus is special case where accuracy reduced for total
          //energy deposition because of G4 inaccuracies and scintillation is
	  //forced-dumped when that nucleus is fully de-excited
          if ( anExcitationEnergy ) fExcitedNucleus = true;
	}
	//if a particle is leaving, remove its kinetic energy from the goal
	//energy, as this will never get deposited (if depositable)
	if(outside){ aMaterialPropertiesTable->
	    AddConstProperty("ENERGY_DEPOSIT_GOL",
	    InitialKinetEnergy-pPostStepPoint->GetKineticEnergy());
	  if(aMaterialPropertiesTable->
	     GetConstProperty("ENERGY_DEPOSIT_GOL")<0)
            aMaterialPropertiesTable->AddConstProperty("ENERGY_DEPOSIT_GOL",0);
	}
	//if a particle is coming back into your scintillator, then add its
	//energy to the goal energy
	if(inside) { aMaterialPropertiesTable->
	    AddConstProperty("ENERGY_DEPOSIT_GOL",
	    InitialKinetEnergy+pPreStepPoint->GetKineticEnergy());
	  if ( TotalEnergyDeposit > 0 && InitialKinetEnergy == 0 ) {
            aMaterialPropertiesTable->AddConstProperty("ENERGY_DEPOSIT_GOL",0);
            TotalEnergyDeposit = .000000;
          }
        }
	if ( InsAndOuts ) {
	  //G4double dribble = pPostStepPoint->GetKineticEnergy() - 
	  //pPreStepPoint->GetKineticEnergy();
	  aMaterialPropertiesTable->
	    AddConstProperty("ENERGY_DEPOSIT_GOL",(-0.1*keV)+
	    InitialKinetEnergy-pPostStepPoint->GetKineticEnergy());
	  InitialKinetEnergy = bMaterial->GetMaterialPropertiesTable()->
	    GetConstProperty("ENERGY_DEPOSIT_GOL");
	  bMaterial->GetMaterialPropertiesTable()->
	    AddConstProperty("ENERGY_DEPOSIT_GOL",(-0.1*keV)+
	    InitialKinetEnergy+pPreStepPoint->GetKineticEnergy());
	  if(aMaterialPropertiesTable->
	     GetConstProperty("ENERGY_DEPOSIT_GOL")<0)
	    aMaterialPropertiesTable->AddConstProperty("ENERGY_DEPOSIT_GOL",0);
	  if ( bMaterial->GetMaterialPropertiesTable()->
	     GetConstProperty("ENERGY_DEPOSIT_GOL") < 0 )
	    bMaterial->GetMaterialPropertiesTable()->
	     AddConstProperty ( "ENERGY_DEPOSIT_GOL", 0 );
        }
	InitialKinetEnergy = aMaterialPropertiesTable->
	  GetConstProperty("ENERGY_DEPOSIT_GOL"); //grab current goal E
	if ( annihil ) //if an annihilation occurred, add energy of two gammas
	  InitialKinetEnergy += 2*electron_mass_c2;
	//if pair production occurs, then subtract energy to cancel with the
	//energy that will be added in the line above when the e+ dies
	if ( convert )
	  InitialKinetEnergy -= 2*electron_mass_c2;
	//update the relevant material property (goal energy)
	aMaterialPropertiesTable->
	  AddConstProperty("ENERGY_DEPOSIT_GOL",InitialKinetEnergy);
	if (anExcitationEnergy < 1e-100 && aStep.GetTotalEnergyDeposit()==0 &&
	aMaterialPropertiesTable->GetConstProperty("ENERGY_DEPOSIT_GOL")==0 &&
	aMaterialPropertiesTable->GetConstProperty("ENERGY_DEPOSIT_TOT")==0)
	return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
	
	G4String procName;
        if ( aTrack.GetCreatorProcess() )
          procName = aTrack.GetCreatorProcess()->GetProcessName();
        else
          procName = "NULL";
        if ( procName == "eBrem" && outside && !OutElectrons )
          fMultipleScattering = true;
	
	// next 2 codeblocks deal with position-related things
	if ( fAlpha ) delta = 1000.*km;
	G4int i, k, counter = 0; G4double pos[3];
	if ( outside ) { //leaving
	  if ( aParticle->GetPDGcode() == 11 && !OutElectrons )
	    fMultipleScattering = true;
	  x1 = x0; //prevents generation of quanta outside active volume
	} //no scint. for e-'s that leave
	
	char xCoord[80]; char yCoord[80]; char zCoord[80];
	G4bool exists = false; //for querying whether set-up of new site needed
	for(i=0;i<j;i++) { //loop over all saved interaction sites
	  counter = i; //save site# for later use in storing properties
	  sprintf(xCoord,"POS_X_%d",i); sprintf(yCoord,"POS_Y_%d",i);
	  sprintf(zCoord,"POS_Z_%d",i);
	  pos[0] = aMaterialPropertiesTable->GetConstProperty(xCoord);
	  pos[1] = aMaterialPropertiesTable->GetConstProperty(yCoord);
	  pos[2] = aMaterialPropertiesTable->GetConstProperty(zCoord);
	  if ( sqrt(pow(x1[0]-pos[0],2.)+pow(x1[1]-pos[1],2.)+
		    pow(x1[2]-pos[2],2.)) < delta ) {
	    exists = true; break; //we find interaction is close to an old one
	  }
	}
	if(!exists && TotalEnergyDeposit) { //current interaction too far away
	  counter = j;
	  sprintf(xCoord,"POS_X_%i",j); sprintf(yCoord,"POS_Y_%i",j); 
	  sprintf(zCoord,"POS_Z_%i",j);
	  //save 3-space coordinates of the new interaction site
	  aMaterialPropertiesTable->AddConstProperty( xCoord, x1[0] );
	  aMaterialPropertiesTable->AddConstProperty( yCoord, x1[1] );
	  aMaterialPropertiesTable->AddConstProperty( zCoord, x1[2] );
	  j++; //increment number of sites
          aMaterialPropertiesTable-> //save
            AddConstProperty( "TOTALNUM_INT_SITES", j );
	}
	
	// this is where nuclear recoil "L" factor is handled: total yield is
	// reduced for nuclear recoil as per Lindhard theory
	
	//we assume you have a mono-elemental scintillator only
	//now, grab A's and Z's of current particle and of material (avg)
	G4double a1 = ElementA->GetA();
	z2 = pDef->GetAtomicNumber(); 
	G4double a2 = (G4double)(pDef->GetAtomicMass());
	if ( particleName == "alpha" || (z2 == 2 && a2 == 4) )
          fAlpha = true; //used later to get S1 pulse shape correct for alpha
	if ( fAlpha || abs(aParticle->GetPDGcode()) == 2112 )
          a2 = a1; //get average A for element at hand
	G4double epsilon = 11.5*(TotalEnergyDeposit/keV)*pow(z1,(-7./3.));
	G4double gamma = 3.*pow(epsilon,0.15)+0.7*pow(epsilon,0.6)+epsilon;
	G4double kappa = 0.133*pow(z1,(2./3.))*pow(a2,(-1./2.))*(2./3.);
	//check if we are dealing with nuclear recoil (Z same as material)
	if ( (z1 == z2 && pDef->GetParticleType() == "nucleus") || 
	     particleName == "neutron" || particleName == "antineutron" ) {
	  YieldFactor=(kappa*gamma)/(1+kappa*gamma); //Lindhard factor
	  if ( z1 == 18 && Phase == kStateLiquid )
	    YieldFactor=0.23*(1+exp(-5*epsilon)); //liquid argon L_eff
	  //just a few safety checks, like for recombProb below
	  if ( YieldFactor > 1 ) YieldFactor = 1;
	  if ( YieldFactor < 0 ) YieldFactor = 0;
	  if ( ElectricField == 0 && Phase == kStateLiquid ) {
	    if ( z1 == 54 ) ThomasImel = 0.19;
	    if ( z1 == 18 ) ThomasImel = 0.25;
	  } //special TIB parameters for nuclear recoil only, in LXe and LAr
	  ExcitationRatio = 0.69337 + 
	    0.3065*exp(-0.008806*pow(ElectricField,0.76313));
	}
	else YieldFactor = 1.000; //default
	
	// determine ultimate #quanta from current E-deposition (ph+e-)
	G4double MeanNumberOfQuanta = //total mean number of exc/ions
	  ScintillationYield*TotalEnergyDeposit;
	//the total number of either quanta produced is equal to product of the
	//work function, the energy deposited, and yield reduction, for NR
	G4double sigma = sqrt(ResolutionScale*MeanNumberOfQuanta); //Fano
	G4int NumQuanta = //stochastic variation in NumQuanta
	  G4int(floor(G4RandGauss::shoot(MeanNumberOfQuanta,sigma)+0.5));
	G4double LeffVar = G4RandGauss::shoot(YieldFactor,0.25*YieldFactor);
	if (LeffVar > 1) LeffVar = 1.00000; if (LeffVar < 0) LeffVar = 0;
	if ( YieldFactor < 1 ) NumQuanta = BinomFluct(NumQuanta,LeffVar);
	//if E below work function, can't make any quanta, and if NumQuanta
	//less than zero because Gaussian fluctuated low, update to zero
	if(TotalEnergyDeposit < 1/ScintillationYield || NumQuanta < 0)
	  NumQuanta = 0;
	
	// next section binomially assigns quanta to excitons and ions
	G4int NumExcitons = 
	  BinomFluct(NumQuanta,ExcitationRatio/(1+ExcitationRatio));
	G4int NumIons = NumQuanta - NumExcitons;
	
	// this section calculates recombination following the modified Birks'
	// Law of Doke, deposition by deposition, and may be overridden later
	// in code if a low enough energy necessitates switching to the 
	// Thomas-Imel box model for recombination instead (determined by site)
	G4double dE, dx=0, LET=0, recombProb;
	dE = TotalEnergyDeposit/MeV;
	if ( particleName != "e-" && particleName != "e+" && z1 != z2 &&
	     particleName != "mu-" && particleName != "mu+" ) {
	  //in other words, if it's a gamma,ion,proton,alpha,pion,et al. do not
	  //use the step length provided by Geant4 because it's not relevant,
	  //instead calculate an estimated LET and range of the electrons that
	  //would have been produced if Geant4 could track them
	  LET = CalculateElectronLET( 1000*dE, z1 );
	  if(LET) dx = dE/(Density*LET); //find the range based on the LET
	  if(abs(aParticle->GetPDGcode())==2112) dx=0;
        }
        else { //normal case of an e-/+ energy deposition recorded by Geant
	  dx = aStep.GetStepLength()/cm;
	  if(dx) LET = (dE/dx)*(1/Density); //lin. energy xfer (prop. to dE/dx)
	  if ( LET > 0 && dE > 0 && dx > 0 ) {
	    G4double ratio = CalculateElectronLET(dE*1e3,z1)/LET;
	    if ( j == 1 && ratio < 0.7 && !ThomasImelTail && 
		 particleName == "e-" ) {
	      dx /= ratio; LET *= ratio; }}
	}
	DokeBirks[1] = DokeBirks[0]/(1-DokeBirks[2]); //B=A/(1-C) (see paper)
	//Doke/Birks' Law as spelled out in the NEST paper
	recombProb = (DokeBirks[0]*LET)/(1+DokeBirks[1]*LET)+DokeBirks[2];
	if ( Phase == kStateLiquid ) {
	  if ( z1 == 54 ) recombProb *= (Density/Density_LXe);
	  if ( z1 == 18 ) recombProb *= (Density/Density_LAr);
	}
	//check against unphysicality resulting from rounding errors
	if(recombProb<0) recombProb=0;
	if(recombProb>1) recombProb=1;
	//use binomial distribution to assign photons, electrons, where photons
	//are excitons plus recombined ionization electrons, while final
	//collected electrons are the "escape" (non-recombined) electrons
	G4int NumPhotons = NumExcitons + BinomFluct(NumIons,recombProb);
	G4int NumElectrons = NumQuanta - NumPhotons;
	
	// next section increments the numbers of excitons, ions, photons, and
	// electrons for the appropriate interaction site; it only appears to
	// be redundant by saving seemingly no longer needed exciton and ion
	// counts, these having been already used to calculate the number of ph
	// and e- above, whereas it does need this later for Thomas-Imel model
	char numExc[80]; char numIon[80]; char numPho[80]; char numEle[80];
	sprintf(numExc,"N_EXC_%i",counter); sprintf(numIon,"N_ION_%i",counter);
	NumExcitons += (G4int)aMaterialPropertiesTable->
	  GetConstProperty( numExc );
        NumIons     += (G4int)aMaterialPropertiesTable->
	  GetConstProperty( numIon );
        aMaterialPropertiesTable->AddConstProperty( numExc, NumExcitons );
        aMaterialPropertiesTable->AddConstProperty( numIon, NumIons     );
        sprintf(numPho,"N_PHO_%i",counter); sprintf(numEle,"N_ELE_%i",counter);
	NumPhotons   +=(G4int)aMaterialPropertiesTable->
	  GetConstProperty( numPho );
	NumElectrons +=(G4int)aMaterialPropertiesTable->
	  GetConstProperty( numEle );
	aMaterialPropertiesTable->AddConstProperty( numPho, NumPhotons   );
	aMaterialPropertiesTable->AddConstProperty( numEle, NumElectrons );
	
	// increment and save the total track length, and save interaction
	// times for later, when generating the scintillation quanta
	char trackL[80]; char time00[80]; char time01[80]; char energy[80];
	sprintf(trackL,"TRACK_%i",counter); sprintf(energy,"ENRGY_%i",counter);
	sprintf(time00,"TIME0_%i",counter); sprintf(time01,"TIME1_%i",counter);
	delta = aMaterialPropertiesTable->GetConstProperty( trackL );
	G4double energ = aMaterialPropertiesTable->GetConstProperty( energy );
	delta += dx*cm; energ += dE*MeV;
	aMaterialPropertiesTable->AddConstProperty( trackL, delta );
	aMaterialPropertiesTable->AddConstProperty( energy, energ );
	if ( TotalEnergyDeposit > 0 ) {
	  G4double deltaTime = aMaterialPropertiesTable->
	    GetConstProperty( time00 );
	  //for charged particles, which continuously lose energy, use initial
	  //interaction time as the minimum time, otherwise use only the final
	  if (aParticle->GetCharge() != 0) {
	    if (t0 < deltaTime)
	      aMaterialPropertiesTable->AddConstProperty( time00, t0 );
	  }
	  else {
	    if (t1 < deltaTime)
	      aMaterialPropertiesTable->AddConstProperty( time00, t1 );
	  }
	  deltaTime = aMaterialPropertiesTable->GetConstProperty( time01 );
	  //find the maximum possible scintillation "birth" time
	  if (t1 > deltaTime)
	    aMaterialPropertiesTable->AddConstProperty( time01, t1 );
	}
	
	// begin the process of setting up creation of scint./ionization
	TotalEnergyDeposit=aMaterialPropertiesTable->
	  GetConstProperty("ENERGY_DEPOSIT_TOT"); //get the total E deposited
	InitialKinetEnergy=aMaterialPropertiesTable->
	  GetConstProperty("ENERGY_DEPOSIT_GOL"); //E that should have been
	if(InitialKinetEnergy > HIENLIM && 
	   abs(aParticle->GetPDGcode()) != 2112) fVeryHighEnergy=true;
	G4double safety; //margin of error for TotalE.. - InitialKinetEnergy
	if (fVeryHighEnergy && !fExcitedNucleus) safety = 0.2*keV;
	else safety = 2.*eV;
	
	//force a scintillation dump for NR and for full nuclear de-excitation
	if( !anExcitationEnergy && pDef->GetParticleType() == "nucleus" && 
	    aTrack.GetTrackStatus() != fAlive && !fAlpha )
	  InitialKinetEnergy = TotalEnergyDeposit;
	if ( particleName == "neutron" || particleName == "antineutron" )
	  InitialKinetEnergy = TotalEnergyDeposit;
	
	//force a dump of all saved scintillation under the following
	//conditions: energy goal reached, and current particle dead, or an 
	//error has occurred and total has exceeded goal (shouldn't happen)
	if( fabs(TotalEnergyDeposit-InitialKinetEnergy)<safety || 
	    TotalEnergyDeposit>=InitialKinetEnergy ){
	  dx = 0; dE = 0;
	  //calculate the total number of quanta from all sites and all
	  //interactions so that the number of secondaries gets set correctly
	  NumPhotons = 0; NumElectrons = 0;
	  for(i=0;i<j;i++) {
	    sprintf(numPho,"N_PHO_%d",i); sprintf(numEle,"N_ELE_%d",i);
	    NumPhotons  +=(G4int)aMaterialPropertiesTable->
	      GetConstProperty( numPho );
            NumElectrons+=(G4int)aMaterialPropertiesTable->
	      GetConstProperty( numEle );
	    sprintf(trackL,"TRACK_%d",i); sprintf(energy,"ENRGY_%d",i);
	    //add up track lengths of all sites, for a total LET calc (later)
            dx += aMaterialPropertiesTable->GetConstProperty(trackL);
	    dE += aMaterialPropertiesTable->GetConstProperty(energy);
	  }
	  G4int buffer = 100; if ( fVeryHighEnergy ) buffer = 1;
	  aParticleChange.SetNumberOfSecondaries(
		 buffer*(NumPhotons+NumElectrons));
	  if (fTrackSecondariesFirst) {
	    if (aTrack.GetTrackStatus() == fAlive )
	      aParticleChange.ProposeTrackStatus(fSuspend);
	  }
	  
	  // begin the loop over all sites which generates all the quanta
	  for(i=0;i<j;i++) {
	    // get the position X,Y,Z, exciton and ion numbers, total track 
	    // length of the site, and interaction times
	    sprintf(xCoord,"POS_X_%d",i); sprintf(yCoord,"POS_Y_%d",i);
	    sprintf(zCoord,"POS_Z_%d",i);
	    sprintf(numExc,"N_EXC_%d",i); sprintf(numIon,"N_ION_%d",i);
	    sprintf(numPho,"N_PHO_%d",i); sprintf(numEle,"N_ELE_%d",i);
	    NumExcitons = (G4int)aMaterialPropertiesTable->
	      GetConstProperty( numExc );
	    NumIons     = (G4int)aMaterialPropertiesTable->
	      GetConstProperty( numIon );
	    sprintf(trackL,"TRACK_%d",i); sprintf(energy,"ENRGY_%d",i);
	    sprintf(time00,"TIME0_%d",i); sprintf(time01,"TIME1_%d",i);
	    delta = aMaterialPropertiesTable->GetConstProperty( trackL );
	    energ = aMaterialPropertiesTable->GetConstProperty( energy );
	    t0 = aMaterialPropertiesTable->GetConstProperty( time00 );
	    t1 = aMaterialPropertiesTable->GetConstProperty( time01 );
	    
	    //if site is small enough, override the Doke/Birks' model with
	    //Thomas-Imel, but not if we're dealing with super-high energy 
	    //particles, and if it's NR force Thomas-Imel (though NR should be
	    //already short enough in track even up to O(100) keV)
	    if ( (delta < R0 && !fVeryHighEnergy) || z2 == z1 || fAlpha ) {
	      if( z1 == 54 && ElectricField && //see NEST paper for ER formula
		  Phase == kStateLiquid ) {
		if ( abs ( z1 - z2 ) && //electron recoil
		     abs ( aParticle->GetPDGcode() ) != 2112 ) {
		  ThomasImel = 0.056776*pow(ElectricField,-0.11844);
		  if ( fAlpha ) //technically ER, but special
		    ThomasImel=0.057675*pow(ElectricField,-0.49362);
		} //end electron recoil (ER)
		else { //nuclear recoil
		  // spline of NR data of C.E. Dahl PhD Thesis Princeton '09
		  // functions found using zunzun.com
		  ThomasImel =
		    -0.15169*pow((ElectricField+215.25),0.01811)+0.20952;
		} //end NR information
		// Never let LY exceed 0-field yield!
		if (ThomasImel > 0.19) ThomasImel = 0.19;
		if (ThomasImel < 0.00) ThomasImel = 0.00;
	      } //end non-zero E-field segment
	      if ( Phase == kStateLiquid ) {
		if ( z1 == 54 ) ThomasImel *= pow((Density/2.84),0.3);
		if ( z1 == 18 ) ThomasImel *= pow((Density/Density_LAr),0.3);
	      }
	      //calculate the Thomas-Imel recombination probability, which
	      //depends on energy via NumIons, but not on dE/dx, and protect
	      //against seg fault by ensuring a positive number of ions
	      if (NumIons > 0) {
		G4double xi;
		xi = (G4double(NumIons)/4.)*ThomasImel;
		if ( InitialKinetEnergy == 9.4*keV ) {
		  G4double NumIonsEff = 1.066e7*pow(t0/ns,-1.303)*
		    (0.17163+162.32/(ElectricField+191.39));
		  if ( NumIonsEff > 1e6 ) NumIonsEff = 1e6;
		  xi = (G4double(NumIonsEff)/4.)*ThomasImel;
		}
		if ( fKr83m && ElectricField==0 )
		  xi = (G4double(1.3*NumIons)/4.)*ThomasImel;
		recombProb = 1-log(1+xi)/xi;
		if(recombProb<0) recombProb=0;
		if(recombProb>1) recombProb=1;
	      }
	      //just like Doke: simple binomial distribution
	      NumPhotons = NumExcitons + BinomFluct(NumIons,recombProb);
	      NumElectrons = (NumExcitons + NumIons) - NumPhotons;
	      //override Doke NumPhotons and NumElectrons
	      aMaterialPropertiesTable->
		AddConstProperty( numPho, NumPhotons   );
	      aMaterialPropertiesTable->
		AddConstProperty( numEle, NumElectrons );
	    }
	    
	    // grab NumPhotons/NumElectrons, which come from Birks if
	    // the Thomas-Imel block of code above was not executed
            NumPhotons  = (G4int)aMaterialPropertiesTable->
	      GetConstProperty( numPho );
            NumElectrons =(G4int)aMaterialPropertiesTable->
	      GetConstProperty( numEle );
	    
	    // extra Fano factor caused by recomb. fluct.
	    G4double FanoFactor =0; //ionization channel
	    if(Phase == kStateLiquid && YieldFactor == 1) {
	      FanoFactor = 
	      2575.9*pow((ElectricField+15.154),-0.64064)-1.4707;
	      if ( fKr83m ) TotalEnergyDeposit = 4*keV;
	      if ( (dE/keV) <= 100 && ElectricField >= 0 ) {
		G4double keVee = (TotalEnergyDeposit/(100.*keV));
		if ( keVee <= 0.06 )
		  FanoFactor *= -0.00075+0.4625*keVee+34.375*pow(keVee,2.);
		else
		  FanoFactor *= 0.069554+1.7322*keVee-.80215*pow(keVee,2.);
	      }
	    }
	    if ( Phase == kStateGas && Density>0.5 ) FanoFactor =
	      0.42857-4.7857*Density+7.8571*pow(Density,2.);
	    if( FanoFactor <= 0 || fVeryHighEnergy ) FanoFactor = 0;
	    NumQuanta = NumPhotons + NumElectrons; 
	    if(z1==54 && FanoFactor) NumElectrons = G4int(
	    floor(G4RandGauss::shoot(NumElectrons,
		     sqrt(FanoFactor*NumElectrons))+0.5));
	    NumPhotons = NumQuanta - NumElectrons;
	    if ( NumElectrons <= 0 ) NumElectrons = 0;
	    if (   NumPhotons <= 0 )   NumPhotons = 0;
	    else { //other effects
	      if ( fAlpha ) //bi-excitonic quenching due to high dE/dx
		NumPhotons = BinomFluct(NumPhotons,biExc);
	      NumPhotons = BinomFluct(NumPhotons,QE_EFF);
	    } NumElectrons = G4int(floor(NumElectrons*phe_per_e+0.5));
	    
	    // new stuff to make Kr-83m work properly
	    if(fKr83m || InitialKinetEnergy==9.4*keV) fKr83m += dE/keV;
	    if(fKr83m > 41) fKr83m = 0;
	    if ( SinglePhase ) //for a 1-phase det. don't propagate e-'s
	      NumElectrons = 0; //saves simulation time
	    
	    // reset material properties numExc, numIon, numPho, numEle, as
	    // their values have been used or stored elsewhere already
	    aMaterialPropertiesTable->AddConstProperty( numExc, 0 );
	    aMaterialPropertiesTable->AddConstProperty( numIon, 0 );
	    aMaterialPropertiesTable->AddConstProperty( numPho, 0 );
	    aMaterialPropertiesTable->AddConstProperty( numEle, 0 );
	    
	    // start particle creation loop
	    if( InitialKinetEnergy < MAX_ENE && InitialKinetEnergy > MIN_ENE &&
	       !fMultipleScattering )
	      NumQuanta = NumPhotons + NumElectrons;
	    else NumQuanta = 0;
	    for(k = 0; k < NumQuanta; k++) {
	      G4double sampledEnergy;
	      G4DynamicParticle* aQuantum;
	      
	      // Generate random direction
	      G4double cost = 1. - 2.*G4UniformRand();
	      G4double sint = std::sqrt((1.-cost)*(1.+cost));
	      G4double phi = twopi*G4UniformRand();
	      G4double sinp = std::sin(phi); G4double cosp = std::cos(phi);
	      G4double px = sint*cosp; G4double py = sint*sinp;
	      G4double pz = cost;
	      
	      // Create momentum direction vector
	      G4ParticleMomentum photonMomentum(px, py, pz);
	      
	      // case of photon-specific stuff
	      if (k < NumPhotons) {
		// Determine polarization of new photon 
		G4double sx = cost*cosp;
		G4double sy = cost*sinp; 
		G4double sz = -sint;
		G4ThreeVector photonPolarization(sx, sy, sz);
		G4ThreeVector perp = photonMomentum.cross(photonPolarization);
		phi = twopi*G4UniformRand();
		sinp = std::sin(phi);
		cosp = std::cos(phi);
		photonPolarization = cosp * photonPolarization + sinp * perp;
		photonPolarization = photonPolarization.unit();
		
		// Generate a new photon or electron:
		sampledEnergy = G4RandGauss::shoot(PhotMean,PhotWidth);
		aQuantum = 
		  new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(),
					photonMomentum);
		aQuantum->SetPolarization(photonPolarization.x(),
					  photonPolarization.y(),
					  photonPolarization.z());
	      }
	      
	      else { // this else statement is for ionization electrons
		if(ElectricField) {
		  // point all electrons straight up, for drifting
		  G4ParticleMomentum electronMomentum(0, 0, -FieldSign);
		  aQuantum = 
		    new G4DynamicParticle(G4ThermalElectron::ThermalElectron(),
					  electronMomentum);
		  if ( Phase == kStateGas ) {
		    sampledEnergy =
		      GetGasElectronDriftSpeed(ElectricField,nDensity);
		  }
		  else
		    sampledEnergy = GetLiquidElectronDriftSpeed(
		    Temperature,ElectricField,MillerDriftSpeed,z1);
		}
		else {
		  // use "photonMomentum" for the electrons in the case of zero
		  // electric field, which is just randomized vector we made
		  aQuantum = 
		    new G4DynamicParticle(G4ThermalElectron::ThermalElectron(),
					  photonMomentum);
		  sampledEnergy = 1.38e-23*(joule/kelvin)*Temperature;
		}
	      }

	      //assign energy to make particle real
	      aQuantum->SetKineticEnergy(sampledEnergy);
	      if (verboseLevel>1) //verbosity stuff
		G4cout << "sampledEnergy = " << sampledEnergy << G4endl;
	      
	      // Generate new G4Track object:
	      // emission time distribution
	      
	      // first an initial birth time is provided that is typically
	      // <<1 ns after the initial interaction in the simulation, then
	      // singlet, triplet lifetimes, and recombination time, are
	      // handled here, to create a realistic S1 pulse shape/timing
	      G4double aSecondaryTime = t0+G4UniformRand()*(t1-t0)+evtStrt;
	      if (tau1<0) tau1=0; if (tau3<0) tau3=0; if (tauR<0) tauR=0;
	      if ( aQuantum->GetDefinition()->
		   GetParticleName()=="opticalphoton" ) {
		if ( abs(z2-z1) && !fAlpha && //electron recoil
		     abs(aParticle->GetPDGcode()) != 2112 ) {
		  LET = (energ/MeV)/(delta/cm)*(1/Density); //avg LET over all
		  //in future, this will be done interaction by interaction
		  // Next, find the recombination time, which is LET-dependent
		  // via ionization density (Kubota et al. Phys. Rev. B 20
		  // (1979) 3486). We find the LET-dependence by fitting to the
		  // E-dependence (Akimov et al. Phys. Lett. B 524 (2002) 245).
		  if ( Phase == kStateLiquid && z1 == 54 )
		    tauR = 3.5*((1+0.41*LET)/(0.18*LET))*ns
		              *exp(-0.00900*ElectricField);
		  //field dependence based on fitting Fig. 9 of Dawson et al.
		  //NIM A 545 (2005) 690
		  //singlet-triplet ratios adapted from Kubota 1979, converted
		  //into correct units to work here, and separately done for
		  //excitation and recombination processes for electron recoils
		  //and assumed same for all LET (may vary)
		  SingTripRatioX = G4RandGauss::shoot(0.17,0.05);
		  SingTripRatioR = G4RandGauss::shoot(0.8,0.2);
		  if ( z1 == 18 ) {
		    SingTripRatioR = 0.2701+0.003379*LET-4.7338e-5*pow(LET,2.)
		      +8.1449e-6*pow(LET,3.); SingTripRatioX = SingTripRatioR;
		    if( LET < 3 ) {
		      SingTripRatioX = G4RandGauss::shoot(0.36,0.06);
		      SingTripRatioR = G4RandGauss::shoot(0.5,0.2); }
		  }
		}
		else if ( fAlpha ) { //alpha particles
                  SingTripRatioR = G4RandGauss::shoot(2.3,0.51);
		  //currently based on Dawson 05 and Tey. 11 (arXiv:1103.3689)
		  //real ratio is likely a gentle function of LET
		  if (z1==18) SingTripRatioR = (-0.065492+1.9996
		    *exp(-dE/MeV))/(1+0.082154/pow(dE/MeV,2.)) + 2.1811;
                  SingTripRatioX = SingTripRatioR;
		}
		else { //nuclear recoil
		  //based loosely on Hitachi et al. Phys. Rev. B 27 (1983) 5279
		  //with an eye to reproducing Akimov 2002 Fig. 9
		  SingTripRatioR = G4RandGauss::shoot(7.8,1.5);
		  if (z1==18) SingTripRatioR = 0.22218*pow(energ/keV,0.48211);
		  SingTripRatioX = SingTripRatioR;
		}
		// now, use binomial distributions to determine singlet and
		// triplet states (and do separately for initially excited guys
		// and recombining)
		if ( k > NumExcitons ) {
		  //the recombination time is non-exponential, but approximates
		  //to exp at long timescales (see Kubota '79)
		  aSecondaryTime += tauR*(1./G4UniformRand()-1);
		  if(G4UniformRand()<SingTripRatioR/(1+SingTripRatioR))
		    aSecondaryTime -= tau1*log(G4UniformRand());
		  else aSecondaryTime -= tau3*log(G4UniformRand());
		}
		else {
		  if(G4UniformRand()<SingTripRatioX/(1+SingTripRatioX))
		    aSecondaryTime -= tau1*log(G4UniformRand());
		  else aSecondaryTime -= tau3*log(G4UniformRand());
		}
	      }
	      else { //electron trapping at the liquid/gas interface
		G4double gainField = 12;
		G4double tauTrap = 884.83-62.069*gainField;
		if ( Phase == kStateLiquid )
		  aSecondaryTime -= tauTrap*ns*log(G4UniformRand());
	      }
	      
	      // emission position distribution -- 
	      // Generate the position of a new photon or electron, with NO
	      // stochastic variation because that could lead to particles
	      // being mistakenly generated outside of your active region by
	      // Geant4, but real-life finite detector position resolution
	      // wipes out any effects from here anyway...
	      x0[0] = aMaterialPropertiesTable->GetConstProperty( xCoord );
	      x0[1] = aMaterialPropertiesTable->GetConstProperty( yCoord );
	      x0[2] = aMaterialPropertiesTable->GetConstProperty( zCoord );
	      G4double radius = sqrt(pow(x0[0],2.)+pow(x0[1],2.));
	      //re-scale radius to ensure no generation of quanta outside
              //the active volume of your simulation due to Geant4 rounding
	      if ( radius >= R_TOL ) {
		if (x0[0] == 0) x0[0] = 1*nm; if (x0[1] == 0) x0[1] = 1*nm;
		radius -= R_TOL; phi = atan ( x0[1] / x0[0] );
		x0[0] = fabs(radius*cos(phi))*((fabs(x0[0]))/(x0[0]));
		x0[1] = fabs(radius*sin(phi))*((fabs(x0[1]))/(x0[1]));
	      }
	      //position of the new secondary particle is ready for use
	      G4ThreeVector aSecondaryPosition = x0;
	      if ( k >= NumPhotons && diffusion && ElectricField > 0 ) {
		G4double D_T = 64*pow(1e-3*ElectricField,-.17);
		//fit to Aprile and Doke 2009, arXiv:0910.4956 (Fig. 12)
		G4double D_L = 13.859*pow(1e-3*ElectricField,-0.58559);
		//fit to Aprile and Doke and Sorensen 2011, arXiv:1102.2865
		if ( Phase == kStateLiquid && z1 == 18 ) {
		  D_T = 93.342*pow(ElectricField/nDensity,0.041322);
		  D_L = 0.15 * D_T; }
		if ( Phase == kStateGas && z1 == 54 ) {
		  D_L=4.265+19097/ElectricField-1.7397e6/pow(ElectricField,2.)+
		    1.2477e8/pow(ElectricField,3.); D_T *= 0.01;
		} D_T *= cm2/s; D_L *= cm2/s;
		if (ElectricField<100 && Phase == kStateLiquid) D_L = 8*cm2/s;
		G4double vDrift = sqrt((2*sampledEnergy)/(EMASS));
		if ( BORDER == 0 ) x0[2] = 0;
		G4double sigmaDT = sqrt(2*D_T*fabs(BORDER-x0[2])/vDrift);
		G4double sigmaDL = sqrt(2*D_L*fabs(BORDER-x0[2])/vDrift);
		G4double dr = fabs(G4RandGauss::shoot(0.,sigmaDT));
		phi = twopi * G4UniformRand();
		aSecondaryPosition[0] += cos(phi) * dr;
		aSecondaryPosition[1] += sin(phi) * dr;
		aSecondaryPosition[2] += G4RandGauss::shoot(0.,sigmaDL);
		radius = sqrt(pow(aSecondaryPosition[0],2.)+
			      pow(aSecondaryPosition[1],2.));
		if(aSecondaryPosition[2] >= BORDER && Phase == kStateLiquid) {
		  if ( BORDER != 0 ) aSecondaryPosition[2] = BORDER - R_TOL; }
		if(aSecondaryPosition[2] <= PMT && !GlobalFields)
		  aSecondaryPosition[2] = PMT + R_TOL;
	      } //end of electron diffusion code
	      
	      // GEANT4 business: stuff you need to make a new track
	      if ( aSecondaryTime < 0 ) aSecondaryTime = 0; //no neg. time
	      G4Track * aSecondaryTrack = 
		new G4Track(aQuantum,aSecondaryTime,aSecondaryPosition);
	      if ( k < NumPhotons || radius < R_MAX )
		aParticleChange.AddSecondary(aSecondaryTrack);
	    }
	    
	    //reset bunch of things when done with an interaction site
	    aMaterialPropertiesTable->AddConstProperty( xCoord, 999*km );
	    aMaterialPropertiesTable->AddConstProperty( yCoord, 999*km );
	    aMaterialPropertiesTable->AddConstProperty( zCoord, 999*km );
	    aMaterialPropertiesTable->AddConstProperty( trackL, 0*um );
	    aMaterialPropertiesTable->AddConstProperty( energy, 0*eV );
	    aMaterialPropertiesTable->AddConstProperty( time00, DBL_MAX );
	    aMaterialPropertiesTable->AddConstProperty( time01, -1*ns );
	    
	    if (verboseLevel>0) { //more verbose stuff
	      G4cout << "\n Exiting from G4S1Light::DoIt -- "
		     << "NumberOfSecondaries = "
		     << aParticleChange.GetNumberOfSecondaries() << G4endl;
	    }
	  } //end of interaction site loop

	  //more things to reset...
	  aMaterialPropertiesTable->
	    AddConstProperty( "TOTALNUM_INT_SITES", 0 );
	  aMaterialPropertiesTable->
	    AddConstProperty( "ENERGY_DEPOSIT_TOT", 0*keV );
	  aMaterialPropertiesTable->
	    AddConstProperty( "ENERGY_DEPOSIT_GOL", 0*MeV );
	  fExcitedNucleus = false;
	  fAlpha = false;
	}
	
	//don't do anything when you're not ready to scintillate
        else {
          aParticleChange.SetNumberOfSecondaries(0);
          return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
        }
	
	//the end (exiting)
	return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
}

// GetMeanFreePath
// ---------------
G4double G4S1Light::GetMeanFreePath(const G4Track&,
                                          G4double ,
                                          G4ForceCondition* condition)
{
        *condition = StronglyForced;
	// what this does is enforce the G4S1Light physics process as always
	// happening, so in effect scintillation is a meta-process on top of
	// any and all other energy depositions which may occur, just like the
	// original G4Scintillation (disregard DBL_MAX, this function makes the
	// mean free path zero really, not infinite)

        return DBL_MAX; //a C-defined constant
}

// GetMeanLifeTime
// ---------------
G4double G4S1Light::GetMeanLifeTime(const G4Track&,
                                          G4ForceCondition* condition)
{
        *condition = Forced;
	// this function and this condition has the same effect as the above
        return DBL_MAX;
}

G4double GetLiquidElectronDriftSpeed(G4double tempinput, G4double efieldinput, 
				     G4bool Miller, G4int Z) {
  if(efieldinput<0) efieldinput *= (-1);
  //Liquid equation one (165K) coefficients
  G4double onea=144623.235704015,
    oneb=850.812714257629,
    onec=1192.87056676815,
    oned=-395969.575204061,
    onef=-355.484170008875,
    oneg=-227.266219627672,
    oneh=223831.601257495,
    onei=6.1778950907965,
    onej=18.7831533426398,
    onek=-76132.6018884368;
  //Liquid equation two (200K) coefficients
  G4double twoa=17486639.7118995,
    twob=-113.174284723134,
    twoc=28.005913193763,
    twod=167994210.094027,
    twof=-6766.42962575088,
    twog=901.474643115395,
    twoh=-185240292.471665,
    twoi=-633.297790813084,
    twoj=87.1756135457949;
  //Liquid equation three (230K) coefficients
  G4double thra=10626463726.9833,
    thrb=224025158.134792,
    thrc=123254826.300172,
    thrd=-4563.5678061122,
    thrf=-1715.269592063,
    thrg=-694181.921834368,
    thrh=-50.9753281079838,
    thri=58.3785811395493,
    thrj=201512.080026704;
  G4double y1=0,y2=0,f1=0,f2=0,f3=0,edrift=0,
    t1=0,t2=0,frac=0,slope=0,intercept=0;

  //Equations defined
  f1=onea/(1+exp(-(efieldinput-oneb)/onec))+oned/
    (1+exp(-(efieldinput-onef)/oneg))+
    oneh/(1+exp(-(efieldinput-onei)/onej))+onek;
  f2=twoa/(1+exp(-(efieldinput-twob)/twoc))+twod/
    (1+exp(-(efieldinput-twof)/twog))+
    twoh/(1+exp(-(efieldinput-twoi)/twoj));
  f3=thra*exp(-thrb*efieldinput)+thrc*exp(-(pow(efieldinput-thrd,2))/
                                          (thrf*thrf))+
    thrg*exp(-(pow(efieldinput-thrh,2)/(thri*thri)))+thrj;

  if(efieldinput<20 && efieldinput>=0) {
    f1=2951*efieldinput;
    f2=5312*efieldinput;
    f3=7101*efieldinput;
  }
  //Cases for tempinput decides which 2 equations to use lin. interpolation
  if(tempinput<200.0 && tempinput>165.0) {
    y1=f1;
    y2=f2;
    t1=165.0;
    t2=200.0;
  }
  if(tempinput<230.0 && tempinput>200.0) {
    y1=f2;
    y2=f3;
    t1=200.0;
    t2=230.0;
  }
  if((tempinput>230.0 || tempinput<165.0) && !Miller) {
    G4cout << "\nWARNING: TEMPERATURE OUT OF RANGE (165-230 K)\n";
    return 0;
  }
  if (tempinput == 165.0) edrift = f1;
  else if (tempinput == 200.0) edrift = f2;
  else if (tempinput == 230.0) edrift = f3;
  else { //Linear interpolation
    frac=(tempinput-t1)/(t2-t1);
    slope = (y1-y2)/(t1-t2);
    intercept=y1-slope*t1;
    edrift=slope*tempinput+intercept;
  }
  
  if ( Miller ) {
    if ( efieldinput <= 40. )
      edrift = -0.13274+0.041082*efieldinput-0.0006886*pow(efieldinput,2.)+
	5.5503e-6*pow(efieldinput,3.);
    else
      edrift = 0.060774*efieldinput/pow(1+0.11336*pow(efieldinput,0.5218),2.);
    if ( efieldinput >= 1e5 ) edrift = 2.7;
    if ( efieldinput >= 100 )
      edrift -= 0.017 * ( tempinput - 163 );
    else
      edrift += 0.017 * ( tempinput - 163 );
    edrift *= 1e5; //put into units of cm/sec. from mm/usec.
  }
  if ( Z == 18 ) edrift = 1e5 * (
    .097384*pow(log10(efieldinput),3.0622)-.018614*sqrt(efieldinput) );
  if ( edrift < 0 ) edrift = 0.;
  edrift = 0.5*EMASS*pow(edrift*cm/s,2.);
  return edrift;
}

G4double CalculateElectronLET ( G4double E, G4int Z ) {
  G4double LET;
  switch ( Z ) {
  case 54:
  //use a spline fit to online ESTAR data
  if ( E >= 1 ) LET = 58.482-61.183*log10(E)+19.749*pow(log10(E),2)+
    2.3101*pow(log10(E),3)-3.3469*pow(log10(E),4)+
    0.96788*pow(log10(E),5)-0.12619*pow(log10(E),6)+0.0065108*pow(log10(E),7);
  //at energies <1 keV, use a different spline, determined manually by
  //generating sub-keV electrons in Geant4 and looking at their ranges, since
  //ESTAR does not go this low
  else if ( E>0 && E<1 ) LET = 6.9463+815.98*E-4828*pow(E,2)+17079*pow(E,3)-
    36394*pow(E,4)+44553*pow(E,5)-28659*pow(E,6)+7483.8*pow(E,7);
  else
    LET = 0;
  break;
  case 18: default:
  if ( E >= 1 ) LET = 116.70-162.97*log10(E)+99.361*pow(log10(E),2)-
    33.405*pow(log10(E),3)+6.5069*pow(log10(E),4)-
    0.69334*pow(log10(E),5)+.031563*pow(log10(E),6);
  else if ( E>0 && E<1 ) LET = 100;
  else
    LET = 0;
  }
  return LET;
}

G4int BinomFluct ( G4int N0, G4double prob ) {
  G4double mean = N0*prob;
  G4double sigma = sqrt(N0*prob*(1-prob));
  G4int N1 = 0;
  if ( prob == 0.00 ) return N1;
  if ( prob == 1.00 ) return N0;
  
  if ( N0 < 10 ) {
    for(G4int i = 0; i < N0; i++) {
      if(G4UniformRand() < prob) N1++;
    }
  }
  else {
    N1 = G4int(floor(G4RandGauss::shoot(mean,sigma)+0.5));
  }
  if ( N1 > N0 ) N1 = N0;
  if ( N1 < 0 ) N1 = 0;
  return N1;
}

void InitMatPropValues ( G4MaterialPropertiesTable *nobleElementMat ) {
  char xCoord[80]; char yCoord[80]; char zCoord[80];
  char numExc[80]; char numIon[80]; char numPho[80]; char numEle[80];
  char trackL[80]; char time00[80]; char time01[80]; char energy[80];
  
  // for loop to initialize the interaction site mat'l properties
  for( G4int i=0; i<10000; i++ ) {
    sprintf(xCoord,"POS_X_%d",i); sprintf(yCoord,"POS_Y_%d",i);
    sprintf(zCoord,"POS_Z_%d",i);
    nobleElementMat->AddConstProperty( xCoord, 999*km );
    nobleElementMat->AddConstProperty( yCoord, 999*km );
    nobleElementMat->AddConstProperty( zCoord, 999*km );
    sprintf(numExc,"N_EXC_%d",i); sprintf(numIon,"N_ION_%d",i);
    sprintf(numPho,"N_PHO_%d",i); sprintf(numEle,"N_ELE_%d",i);
    nobleElementMat->AddConstProperty( numExc, 0 );
    nobleElementMat->AddConstProperty( numIon, 0 );
    nobleElementMat->AddConstProperty( numPho, 0 );
    nobleElementMat->AddConstProperty( numEle, 0 );
    sprintf(trackL,"TRACK_%d",i); sprintf(energy,"ENRGY_%d",i);
    sprintf(time00,"TIME0_%d",i); sprintf(time01,"TIME1_%d",i);
    nobleElementMat->AddConstProperty( trackL, 0*um );
    nobleElementMat->AddConstProperty( energy, 0*eV );
    nobleElementMat->AddConstProperty( time00, DBL_MAX );
    nobleElementMat->AddConstProperty( time01,-1*ns );
  }
  
  // we initialize the total number of interaction sites, a variable for
  // updating the amount of energy deposited thus far in the medium, and a
  // variable for storing the amount of energy expected to be deposited
  nobleElementMat->AddConstProperty( "TOTALNUM_INT_SITES", 0 );
  nobleElementMat->AddConstProperty( "ENERGY_DEPOSIT_TOT", 0*keV );
  nobleElementMat->AddConstProperty( "ENERGY_DEPOSIT_GOL", 0*MeV );
  return;
}

G4double UnivScreenFunc ( G4double E, G4double Z, G4double A ) {
  G4double a_0 = 5.29e-11*m; G4double a = 0.626*a_0*pow(Z,(-1./3.));
  G4double epsilon_0 = 8.854e-12*(farad/m);
  G4double epsilon = (a*E*2.*twopi*epsilon_0)/(2*pow(eplus,2.)*pow(Z,2.));
  G4double zeta_0 = pow(Z,(1./6.)); G4double m_N = A*1.66e-27*kg;
  G4double hbar = 6.582e-16*eV*s;
  if ( Z == 54 ) {
    epsilon *= 1.068; //zeta_0 = 1.63;
  } //special case for LXe from Bezrukov et al. 2011
  G4double s_n = log(1+1.1383*epsilon)/(2.*(epsilon +
		       0.01321*pow(epsilon,0.21226) +
		       0.19593*sqrt(epsilon)));
  G4double s_e = (a_0*zeta_0/a)*hbar*sqrt(8*epsilon*2.*twopi*epsilon_0/
					  (a*m_N*pow(eplus,2.)));
  return 1.38e5*0.5*(1+tanh(50*epsilon-0.25))*epsilon*(s_e/s_n);
}

} // namespace RAT
