// This code is adapted from the Geant4 v10 optical wavelength shifting model.
//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: G4OpWLS.cc 71487 2013-06-17 08:19:40Z gcosmo $
//
////////////////////////////////////////////////////////////////////////
// Optical Photon WaveLength Shifting (WLS) Class Implementation
////////////////////////////////////////////////////////////////////////
//
// File:        G4OpWLS.cc
// Description: Discrete Process -- Wavelength Shifting of Optical Photons
// Version:     1.0
// Created:     2003-05-13
// Author:      John Paul Archambault
//              (Adaptation of G4Scintillation and G4OpAbsorption)
// Updated:     2005-07-28 - add G4ProcessType to constructor
//              2006-05-07 - add G4VWLSTimeGeneratorProfile
// mail:        gum@triumf.ca
//              jparcham@phys.ualberta.ca
//
////////////////////////////////////////////////////////////////////////

#include "AltOpWLS.hh"

#include "G4ios.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4OpProcessSubType.hh"

#include "G4WLSTimeGeneratorProfileDelta.hh"
#include "G4WLSTimeGeneratorProfileExponential.hh"

//Include ROOT.
#include "TROOT.h"
#include "TLeaf.h"
#include "TFile.h"
#include "TTree.h"
#include <vector>
#include <cmath>

using namespace std;

/////////////////////////
// Class Implementation
/////////////////////////

/////////////////
// Constructors
/////////////////

AltOpWLS::AltOpWLS(const G4String& processName, G4ProcessType type)
  : G4VDiscreteProcess(processName, type)
{
  SetProcessSubType(fOpWLS);

  theIntegralTable = NULL;
  theQYTable = NULL;
 
  if (verboseLevel>0) {
    G4cout << GetProcessName() << " is created " << G4endl;
  }

  WLSTimeGeneratorProfile = 
       new G4WLSTimeGeneratorProfileDelta("WLSTimeGeneratorProfileDelta");

}

////////////////
// Destructors
////////////////

AltOpWLS::~AltOpWLS()
{
  if (theIntegralTable != 0) {
    theIntegralTable->clearAndDestroy();
    delete theIntegralTable;
  }
  delete WLSTimeGeneratorProfile;
}

////////////
// Methods
////////////

void AltOpWLS::BuildPhysicsTable(const G4ParticleDefinition&)
{
  //This method is called during initialization.
    if (!theIntegralTable) BuildThePhysicsTable();
    if(!theQYTable) BuildTheQYTable();
}

// PostStepDoIt
// -------------
//
G4VParticleChange*
AltOpWLS::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep)
{
  aParticleChange.Initialize(aTrack);
  
  aParticleChange.ProposeTrackStatus(fStopAndKill);

  if (verboseLevel>0) {
    G4cout << "\n** Photon absorbed! **" << G4endl;
  }
  
  const G4Material* aMaterial = aTrack.GetMaterial();

  G4StepPoint* pPostStepPoint = aStep.GetPostStepPoint();
    
  G4MaterialPropertiesTable* aMaterialPropertiesTable =
    aMaterial->GetMaterialPropertiesTable();
  if (!aMaterialPropertiesTable)
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);

  const G4MaterialPropertyVector* WLS_Intensity = 
    aMaterialPropertiesTable->GetProperty("WLSCOMPONENT"); 

  if (!WLS_Intensity)
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);

  G4double primaryEnergy = aTrack.GetDynamicParticle()->GetKineticEnergy();


  //Implement the QY sampling
  G4int NumPhotons = 0;

  G4PhysicsOrderedFreeVector* QYValues =
    (G4PhysicsOrderedFreeVector*)((*theQYTable)(aMaterial->GetIndex()));

  double theQY = 0;

  if(QYValues){
    //Case where energy is lower than the min energy; set to min value.
    if(primaryEnergy<QYValues->GetMinLowEdgeEnergy())
      {
	theQY = QYValues->GetMinValue();
      }
    //Case where energy is higher than the max energy; set to max value.
    else if(QYValues->GetMaxLowEdgeEnergy()<primaryEnergy)
      {
	theQY = QYValues->GetMaxValue();
      }
    //Set to the nearest energy bin.
    else{
    theQY = QYValues->Value(primaryEnergy);
    }
  }

  //Do a monte carlo trial. Later on I could implement multiple photon emission
  //if that needs to be accounted for.
  if(G4UniformRand()<theQY){
    NumPhotons = 1;
  }
  else{
    //return unchanged primary and no secondaries.
    aParticleChange.SetNumberOfSecondaries(0);
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  aParticleChange.SetNumberOfSecondaries(NumPhotons);


  G4int materialIndex = aMaterial->GetIndex();

  // Retrieve the WLS Integral for this material
  // new G4PhysicsOrderedFreeVector allocated to hold CII's

  G4double WLSTime = 0.*ns;
  G4PhysicsOrderedFreeVector* WLSIntegral = 0;

  WLSTime   = aMaterialPropertiesTable->
    GetConstProperty("WLSTIMECONSTANT");
  //WLSIntegral =
  //(G4PhysicsOrderedFreeVector*)((*theIntegralTable)(materialIndex));
   
  // Max WLS Integral
  
  //G4double CIImax = WLSIntegral->GetMaxValue();
 
  G4int NumberOfPhotons = NumPhotons;
 
  for (G4int i = 0; i < NumPhotons; i++) {

    //Get the energy of the WLS photon
    G4double sampledEnergy;
    sampledEnergy = AltOpWLS::GetEmEnergy(primaryEnergy);

    if (verboseLevel>1) {
      G4cout << "sampledEnergy = " << sampledEnergy << G4endl;
    }

    // Generate random photon direction
    
    G4double cost = 1. - 2.*G4UniformRand();
    G4double sint = std::sqrt((1.-cost)*(1.+cost));

    G4double phi = twopi*G4UniformRand();
    G4double sinp = std::sin(phi);
    G4double cosp = std::cos(phi);
    
    G4double px = sint*cosp;
    G4double py = sint*sinp;
    G4double pz = cost;
    
    // Create photon momentum direction vector
    
    G4ParticleMomentum photonMomentum(px, py, pz);
    
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
    
    // Generate a new photon:
    
    G4DynamicParticle* aWLSPhoton =
      new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(),
			    photonMomentum);
    aWLSPhoton->SetPolarization
      (photonPolarization.x(),
       photonPolarization.y(),
       photonPolarization.z());
    
    aWLSPhoton->SetKineticEnergy(sampledEnergy);
    
    // Generate new G4Track object:
    
    // Must give position of WLS optical photon

    G4double TimeDelay = WLSTimeGeneratorProfile->GenerateTime(WLSTime);
    G4double aSecondaryTime = (pPostStepPoint->GetGlobalTime()) + TimeDelay;

    G4ThreeVector aSecondaryPosition = pPostStepPoint->GetPosition();

    G4Track* aSecondaryTrack = 
      new G4Track(aWLSPhoton,aSecondaryTime,aSecondaryPosition);
   
    aSecondaryTrack->SetTouchableHandle(aTrack.GetTouchableHandle()); 
    // aSecondaryTrack->SetTouchableHandle((G4VTouchable*)0);
    
    aSecondaryTrack->SetParentID(aTrack.GetTrackID());
    
    aParticleChange.AddSecondary(aSecondaryTrack);
  }


  if (verboseLevel>0) {
    G4cout << "\n Exiting from AltOpWLS::DoIt -- NumberOfSecondaries = " 
	   << aParticleChange.GetNumberOfSecondaries() << G4endl;  
  }

  return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
}

// BuildThePhysicsTable for the wavelength shifting process
// --------------------------------------------------
//

void AltOpWLS::BuildThePhysicsTable()
{
  if (theIntegralTable) return;
  
  const G4MaterialTable* theMaterialTable = 
    G4Material::GetMaterialTable();
  G4int numOfMaterials = G4Material::GetNumberOfMaterials();
  
  // create new physics table
  
  if(!theIntegralTable)theIntegralTable = new G4PhysicsTable(numOfMaterials);
  
  // loop for materials
  
  for (G4int i=0 ; i < numOfMaterials; i++)
    {
      G4PhysicsOrderedFreeVector* aPhysicsOrderedFreeVector =
	new G4PhysicsOrderedFreeVector();
      
      // Retrieve vector of WLS wavelength intensity for
      // the material from the material's optical properties table.
      
      G4Material* aMaterial = (*theMaterialTable)[i];

      G4MaterialPropertiesTable* aMaterialPropertiesTable =
	aMaterial->GetMaterialPropertiesTable();

      if (aMaterialPropertiesTable) {

	G4MaterialPropertyVector* theWLSVector = 
	  aMaterialPropertiesTable->GetProperty("WLSCOMPONENT");

	if (theWLSVector) {
	  
	  // Retrieve the first intensity point in vector
	  // of (photon energy, intensity) pairs
	  
	  G4double currentIN = (*theWLSVector)[0];
	  
	  if (currentIN >= 0.0) {

	    // Create first (photon energy) 
	   
	    G4double currentPM = theWLSVector->Energy(0);
	    
	    G4double currentCII = 0.0;
	    
	    aPhysicsOrderedFreeVector->
	      InsertValues(currentPM , currentCII);
	    
	    // Set previous values to current ones prior to loop
	    
	    G4double prevPM  = currentPM;
	    G4double prevCII = currentCII;
	    G4double prevIN  = currentIN;
	    
	    // loop over all (photon energy, intensity)
	    // pairs stored for this material

            for (size_t j = 1;
                 j < theWLSVector->GetVectorLength();
                 j++)	    
	      {
		currentPM = theWLSVector->Energy(j);
		currentIN = (*theWLSVector)[j];
		
		currentCII = 0.5 * (prevIN + currentIN);
		
		currentCII = prevCII +
		  (currentPM - prevPM) * currentCII;
		
		aPhysicsOrderedFreeVector->
		  InsertValues(currentPM, currentCII);
		
		prevPM  = currentPM;
		prevCII = currentCII;
		prevIN  = currentIN;
	      }
	  }
	}
      }
	// The WLS integral for a given material
	// will be inserted in the table according to the
	// position of the material in the material table.

	theIntegralTable->insertAt(i,aPhysicsOrderedFreeVector);
    }
}


//This is essentially a duplicate of BuildThePhysicsTable, but rather than get
//the CDF for the "WLSCOMPONENT" variable, it gets it for "QUANTUMYIELD".
void AltOpWLS::BuildTheQYTable()
{
  if (theQYTable) return;
  //G4cout << "Building the QY Table" << G4endl;
  
  const G4MaterialTable* theMaterialTable = 
    G4Material::GetMaterialTable();
  G4int numOfMaterials = G4Material::GetNumberOfMaterials();
  
  // create new physics table
  theQYTable = new G4PhysicsTable(numOfMaterials);
  
  // loop for materials
  for (G4int i=0 ; i < numOfMaterials; i++)
    {      
      // Retrieve vector of WLS wavelength intensity for
      // the material from the material's optical properties table.
      G4Material* aMaterial = (*theMaterialTable)[i];

      G4MaterialPropertiesTable* aMaterialPropertiesTable =
	aMaterial->GetMaterialPropertiesTable();

      G4MaterialPropertyVector* theQYVector = new G4MaterialPropertyVector();

      if (aMaterialPropertiesTable) {
	theQYVector = aMaterialPropertiesTable->GetProperty("QUANTUMYIELD");
      }

	// The WLS integral for a given material
	// will be inserted in the table according to the
	// position of the material in the material table.
	theQYTable->insertAt(i,theQYVector);
    }
}


// GetMeanFreePath
// ---------------
//
G4double AltOpWLS::GetMeanFreePath(const G4Track& aTrack,
 				         G4double ,
				         G4ForceCondition* )
{
  const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
  const G4Material* aMaterial = aTrack.GetMaterial();

  G4double thePhotonEnergy = aParticle->GetTotalEnergy();

  G4MaterialPropertiesTable* aMaterialPropertyTable;
  G4MaterialPropertyVector* AttenuationLengthVector;
	
  G4double AttenuationLength = DBL_MAX;

  aMaterialPropertyTable = aMaterial->GetMaterialPropertiesTable();

  if ( aMaterialPropertyTable ) {
    AttenuationLengthVector = aMaterialPropertyTable->
      GetProperty("WLSABSLENGTH");
    if ( AttenuationLengthVector ){
      AttenuationLength = AttenuationLengthVector->
	Value(thePhotonEnergy);
    }
    else {
      //G4cout << "No WLS absorption length specified" << G4endl;
      //G4cout << "The Photon Energy = " << (1240./(thePhotonEnergy/eV)) << "nm"
      //     << G4endl;
    }
  }
  else {
    //G4cout << "No WLS absortion length specified" << G4endl;
  }
  
  return AttenuationLength;
}

void AltOpWLS::UseTimeProfile(const G4String name)
{
  if (name == "delta")
    {
      delete WLSTimeGeneratorProfile;
      WLSTimeGeneratorProfile = 
             new G4WLSTimeGeneratorProfileDelta("delta");
    }
  else if (name == "exponential")
    {
      delete WLSTimeGeneratorProfile;
      WLSTimeGeneratorProfile =
             new G4WLSTimeGeneratorProfileExponential("exponential");
    }
  else
    {
      G4Exception("AltOpWLS::UseTimeProfile", "em0202",
                  FatalException,
                  "generator does not exist");
    }
}

G4double AltOpWLS::GetEmEnergy(G4double ExEn){
  //Convert ExEn to LambEx in nm
  G4double LambEx = 1239.84187/(ExEn/eV);  

  //Loop the ExEmData events until a longer wavelength event is found.
  int theEvent = 0;
  int UpperEvt = 0;
  bool interp = true;
  bool FoundEvt = false;

  while(theEvent<ExEmData.size()){
    if(ExEmData.at(theEvent).at(0).at(0)>LambEx){
      UpperEvt = theEvent;
      FoundEvt = true;
      break;
    }
    theEvent++;
  }

  int LowerEvt = 0;
  if(!FoundEvt){return ExEn;}//Wavelength too long, just return same.
  else if(UpperEvt!=0){LowerEvt = UpperEvt-1;}//interpolate
  else{interp = false;}//Don't interpolate beyond bounds.
  
  double UpperEx = ExEmData.at(UpperEvt).at(0).at(0);
  vector<double> UpperInten = ExEmData.at(UpperEvt).at(2);
  double LowerEx = 0;
  vector<double> LowerInten;
  if(interp){
    LowerEx = ExEmData.at(LowerEvt).at(0).at(0);
    LowerInten = ExEmData.at(LowerEvt).at(2);
  }
  vector<double> InterpDist;
  double RunningSum = 0;
  vector<double> InterpCDF;

  double theUpperInten = 0;

  for(int i = 0; i<UpperInten.size(); i++){
    if(interp){
      //This check is because my input data is a bit stupid.
      if(isnan(UpperInten.at(i))){theUpperInten = 0;}
      else{theUpperInten = UpperInten.at(i);}
      
      InterpDist.push_back(LowerInten.at(i) + (LambEx-LowerEx)*
			   (theUpperInten-LowerInten.at(i))
			   /(UpperEx-LowerEx));
    }
    else{InterpDist.push_back(UpperInten.at(i));}

    RunningSum += InterpDist.back();
    InterpCDF.push_back(RunningSum);
  }

  //Perform a monte carlo sample
  G4double sample = G4UniformRand()*RunningSum;

  G4int theIndex = 0;
  while(theIndex<InterpCDF.size()){
    if(InterpCDF.at(theIndex)>sample){
      return 1240./(ExEmData.at(LowerEvt).at(1).at(theIndex)/eV);
    }
    theIndex++;
  }

  G4cout << "ERROR: Ex/Em matrix sampling finished with no result" << G4endl;
  G4cout << "LambdaEx = " << LambEx << " nm." << G4endl;
  return 0;

}


void* AltOpWLS::GetPointerToValue(TBranch* theBranch, int entry,
				const char* name){
  theBranch->GetEntry(entry);
  TLeaf* theLeaf = theBranch->GetLeaf(name);
  return theLeaf->GetValuePointer();
}

#include <sys/stat.h>
//Arguments: File name, Tree name containing Ex/Em data, Branch names for
//excitation wavelength and emission intensity. Bins are assumed to be nm.
void AltOpWLS::SetExEmData(string fname){
  //////////////////////////////////////////////////////////////////////////////
  //Currently, the data are stored in the ExEmData vector, with the exciting
  //wavelength for event i stored in ExEmData.at(i).at(0).at(0), the emitting
  //wavelengths stored in ExEmData.at(i).at(1).at(:), and the normalized 
  //stored at ExEmData.at(i).at(2).at(:).
  //These data are read into memory to speed things up.
  //////////////////////////////////////////////////////////////////////////////

  //Check file is there
  struct stat buffer;
  if(stat(fname.c_str(), &buffer)!=0){
    G4cout << "Warning: Could not find Ex/Em data file for AltOPWLS model"
	   << G4endl;
    return;
  }

  //I assume that the events are stored in such a way that they are
  //monotonically increasing with wavelength
  //G4cout << "Opening the Ex/Em root file and getting branches... ";
  TFile* f = new TFile(fname.c_str());
  TTree* theTree = (TTree*)f->Get("FluorSpec");
  TBranch* ExBranch = (TBranch*)theTree->GetBranch("Lambda_ex");
  TBranch* EmBranch = (TBranch*)theTree->GetBranch("Wavelength");
  TBranch* IntenBranch = (TBranch*)theTree->GetBranch("Intensity");

  int nEntries = theTree->GetEntries();

  ExEmData.clear();

  vector<double> ExWavelength;
  vector<double> EmWavelengths;
  vector<double> EmIntensities;
  vector< vector<double> > theData;
  double theIntegral;

  for(int i = 0; i<nEntries; i++){
    ExWavelength.clear();
    theData.clear();
    theIntegral = 0;

    //I should get these in a different way...
    ExWavelength.
      push_back(*(double*)(AltOpWLS::GetPointerToValue(ExBranch, i,
						       ExBranch->GetName())));

    EmWavelengths = *(vector<double>*)
      (AltOpWLS::GetPointerToValue(EmBranch, i, EmBranch->GetName()));

    EmIntensities = *(vector<double>*)
      (AltOpWLS::GetPointerToValue(IntenBranch, i, IntenBranch->GetName()));

    //Load the data into slots 1, 2, and 3.
    theData.push_back(ExWavelength);
    theData.push_back(EmWavelengths);

    //Normalize the emission intensities.
    for(size_t j = 0; j<EmIntensities.size(); j++){
      theIntegral += EmIntensities.at(j);
    }
    for(size_t j = 0; j<EmIntensities.size(); j++){
      EmIntensities.at(j) = EmIntensities.at(j)/theIntegral;
    }
    theData.push_back(EmIntensities);

    ExEmData.push_back(theData);

  }

  return;

}
