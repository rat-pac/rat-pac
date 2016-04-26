#include <vector>
#include <cmath>
#include <sys/stat.h>
#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>
#include <G4Material.hh>
#include <G4MaterialPropertiesTable.hh>
#include <G4StepPoint.hh>
#include <G4WLSTimeGeneratorProfileDelta.hh>
#include <G4WLSTimeGeneratorProfileExponential.hh>
#include <G4OpProcessSubType.hh>
#include <Randomize.hh>
#include <RAT/BNLOpWLS.hh>
#include <RAT/BNLOpWLSData.hh>

using namespace std;

BNLOpWLS::BNLOpWLS(const G4String& processName, G4ProcessType type)
    : G4VDiscreteProcess(processName, type) {
  SetProcessSubType(fOpWLS);

  theIntegralTable = NULL;
  theQYTable = NULL;
  wlsData = NULL;
 
  WLSTimeGeneratorProfile =
    new G4WLSTimeGeneratorProfileDelta("WLSTimeGeneratorProfileDelta");
}

BNLOpWLS::~BNLOpWLS() {
  if (theIntegralTable != NULL) {
    theIntegralTable->clearAndDestroy();
    delete theIntegralTable;
  }
  delete WLSTimeGeneratorProfile;
  delete wlsData;
}

void BNLOpWLS::BuildPhysicsTable(const G4ParticleDefinition&) {
  if (!theIntegralTable) {
    BuildThePhysicsTable();
  }

  if (!theQYTable) {
    BuildTheQYTable();
  }
}

G4VParticleChange*
BNLOpWLS::PostStepDoIt(const G4Track& aTrack, const G4Step& aStep) {
  aParticleChange.Initialize(aTrack);
  aParticleChange.ProposeTrackStatus(fStopAndKill);

  const G4Material* aMaterial = aTrack.GetMaterial();

  G4StepPoint* pPostStepPoint = aStep.GetPostStepPoint();
    
  G4MaterialPropertiesTable* aMaterialPropertiesTable =
    aMaterial->GetMaterialPropertiesTable();

  if (!aMaterialPropertiesTable) {
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  const G4MaterialPropertyVector* wlsIntensity = 
    aMaterialPropertiesTable->GetProperty("WLSCOMPONENT"); 

  if (!wlsIntensity) {
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  G4double primaryEnergy = aTrack.GetDynamicParticle()->GetKineticEnergy();

  // Implement the QY sampling
  G4int NumPhotons = 0;

  G4PhysicsOrderedFreeVector* QYValues =
    (G4PhysicsOrderedFreeVector*)((*theQYTable)(aMaterial->GetIndex()));

  double theQY = 0;

  if (QYValues) {
    // Case where energy is lower than the min energy; set to min value.
    if (primaryEnergy<QYValues->GetMinLowEdgeEnergy()) {
      theQY = QYValues->GetMinValue();
    }
    // Case where energy is higher than the max energy; set to max value.
    else if (QYValues->GetMaxLowEdgeEnergy() < primaryEnergy) {
      theQY = QYValues->GetMaxValue();
    }
    // Set to the nearest energy bin.
    else {
      theQY = QYValues->Value(primaryEnergy);
    }
  }

  // Do a monte carlo trial. Later on I could implement multiple photon
  // emission if that needs to be accounted for.
  if (G4UniformRand() < theQY) {
    NumPhotons = 1;
  }
  else {
    // Return unchanged primary and no secondaries.
    aParticleChange.SetNumberOfSecondaries(0);
    return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  aParticleChange.SetNumberOfSecondaries(NumPhotons);

  // Retrieve the WLS Integral for this material
  // new G4PhysicsOrderedFreeVector allocated to hold CII's
  G4double WLSTime = 0. * CLHEP::ns;

  WLSTime = aMaterialPropertiesTable->GetConstProperty("WLSTIMECONSTANT");
   
  for (G4int i=0; i<NumPhotons; i++) {
    //Get the energy of the WLS photon
    G4double sampledEnergy = BNLOpWLS::GetEmEnergy(primaryEnergy);

    // Generate random photon direction
    G4double cost = 1. - 2. * G4UniformRand();
    G4double sint = std::sqrt((1.-cost)*(1.+cost));

    G4double phi = twopi * G4UniformRand();
    G4double sinp = std::sin(phi);
    G4double cosp = std::cos(phi);
    
    G4double px = sint * cosp;
    G4double py = sint * sinp;
    G4double pz = cost;
    
    // Create photon momentum direction vector
    G4ParticleMomentum photonMomentum(px, py, pz);
    
    // Determine polarization of new photon
    G4double sx = cost * cosp;
    G4double sy = cost * sinp;
    G4double sz = -sint;

    G4ThreeVector photonPolarization(sx, sy, sz);

    G4ThreeVector perp = photonMomentum.cross(photonPolarization);

    phi = twopi * G4UniformRand();
    sinp = std::sin(phi);
    cosp = std::cos(phi);

    photonPolarization = cosp * photonPolarization + sinp * perp;

    photonPolarization = photonPolarization.unit();

    // Generate a new photon
    G4DynamicParticle* aWLSPhoton =
      new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(), photonMomentum);
    aWLSPhoton->SetPolarization(photonPolarization.x(),
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
    aSecondaryTrack->SetParentID(aTrack.GetTrackID());
    aParticleChange.AddSecondary(aSecondaryTrack);
  }

  return G4VDiscreteProcess::PostStepDoIt(aTrack, aStep);
}

void BNLOpWLS::BuildThePhysicsTable()
{
  if (theIntegralTable) {
    return;
  }
  
  const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
  G4int numOfMaterials = G4Material::GetNumberOfMaterials();
  
  // create new physics table
  if (!theIntegralTable) {
    theIntegralTable = new G4PhysicsTable(numOfMaterials);
  }
  
  // loop for materials
  for (G4int i=0; i<numOfMaterials; i++) {
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
          aPhysicsOrderedFreeVector->InsertValues(currentPM, currentCII);

          // Set previous values to current ones prior to loop
          G4double prevPM  = currentPM;
          G4double prevCII = currentCII;
          G4double prevIN  = currentIN;

          // loop over all (photon energy, intensity)
          // pairs stored for this material
          for (size_t j=1; j<theWLSVector->GetVectorLength(); j++) {
            currentPM = theWLSVector->Energy(j);
            currentIN = (*theWLSVector)[j];
            currentCII = 0.5 * (prevIN + currentIN);
            currentCII = prevCII + (currentPM - prevPM) * currentCII;
            aPhysicsOrderedFreeVector->InsertValues(currentPM, currentCII);
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
    theIntegralTable->insertAt(i, aPhysicsOrderedFreeVector);
  }
}

void BNLOpWLS::BuildTheQYTable() {
  if (theQYTable) {
    return;
  }
  
  const G4MaterialTable* theMaterialTable = G4Material::GetMaterialTable();
  G4int numOfMaterials = G4Material::GetNumberOfMaterials();
  
  // create new physics table
  theQYTable = new G4PhysicsTable(numOfMaterials);
  
  // loop for materials
  for (G4int i=0; i<numOfMaterials; i++) {
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
    theQYTable->insertAt(i, theQYVector);
  }
}

G4double BNLOpWLS::GetMeanFreePath(const G4Track& aTrack,
                                   G4double,
                                   G4ForceCondition*) {
  const G4DynamicParticle* aParticle = aTrack.GetDynamicParticle();
  const G4Material* aMaterial = aTrack.GetMaterial();

  G4double thePhotonEnergy = aParticle->GetTotalEnergy();

  G4MaterialPropertiesTable* aMaterialPropertyTable;
  G4MaterialPropertyVector* AttenuationLengthVector;
  
  G4double AttenuationLength = DBL_MAX;

  aMaterialPropertyTable = aMaterial->GetMaterialPropertiesTable();

  if (aMaterialPropertyTable) {
    AttenuationLengthVector =
      aMaterialPropertyTable->GetProperty("WLSABSLENGTH");
    if (AttenuationLengthVector) {
      AttenuationLength = AttenuationLengthVector->Value(thePhotonEnergy);
    }
  }

  return AttenuationLength;
}

void BNLOpWLS::UseTimeProfile(const G4String name) {
  if (name == "delta") {
    delete WLSTimeGeneratorProfile;
    WLSTimeGeneratorProfile = new G4WLSTimeGeneratorProfileDelta("delta");
  }
  else if (name == "exponential") {
    delete WLSTimeGeneratorProfile;
    WLSTimeGeneratorProfile =
      new G4WLSTimeGeneratorProfileExponential("exponential");
  }
  else {
    G4Exception("BNLOpWLS::UseTimeProfile", "em0202", FatalException,
                "generator does not exist");
  }
}

G4double BNLOpWLS::GetEmEnergy(G4double ExEn) {
  // Convert ExEn to LambEx in nm
  G4double LambEx = 1239.84187 / (ExEn/eV);  

  // Loop the ExEmData events until a longer wavelength event is found.
  size_t theEvent = 0;
  size_t UpperEvt = 0;
  bool interp = true;
  bool FoundEvt = false;

  while (theEvent<wlsData->ExEmData.size()) {
    if (wlsData->ExEmData.at(theEvent).at(0).at(0) > LambEx) {
      UpperEvt = theEvent;
      FoundEvt = true;
      break;
    }
    theEvent++;
  }

  int LowerEvt = 0;
  if (!FoundEvt) {
    // Wavelength too long, just return same.
    return ExEn;
  }
  else if (UpperEvt != 0) {
    // Interpolate
    LowerEvt = UpperEvt-1;
  }
  else {
    // Don't interpolate beyond bounds.
    interp = false;
  }
  
  double UpperEx = wlsData->ExEmData.at(UpperEvt).at(0).at(0);
  vector<double> UpperInten = wlsData->ExEmData.at(UpperEvt).at(2);
  double LowerEx = 0;
  vector<double> LowerInten;

  if (interp) {
    LowerEx = wlsData->ExEmData.at(LowerEvt).at(0).at(0);
    LowerInten = wlsData->ExEmData.at(LowerEvt).at(2);
  }

  vector<double> InterpDist;
  double RunningSum = 0;
  vector<double> InterpCDF;

  double theUpperInten = 0;

  for (size_t i=0; i<UpperInten.size(); i++) {
    if (interp) {
      // This check is because my input data is a bit stupid.
      if (isnan(UpperInten.at(i))) {
        theUpperInten = 0;
      }
      else {
        theUpperInten = UpperInten.at(i);
      }
      
      InterpDist.push_back(
        LowerInten.at(i) +
          (LambEx-LowerEx) *
            (theUpperInten-LowerInten.at(i)) / 
              (UpperEx-LowerEx));
    }
    else {
      InterpDist.push_back(UpperInten.at(i));
    }

    RunningSum += InterpDist.back();
    InterpCDF.push_back(RunningSum);
  }

  // Perform a monte carlo sample
  G4double sample = G4UniformRand() * RunningSum;

  size_t theIndex = 0;
  while (theIndex < InterpCDF.size()) {
    if (InterpCDF.at(theIndex) > sample) {
      return 1240. / (wlsData->ExEmData.at(LowerEvt).at(1).at(theIndex)/CLHEP::eV);
    }
    theIndex++;
  }

  cout << "BNLOpWLS::GetEmEnergy: Error: Ex/Em matrix sampling finished "
       << "with no result" << endl
       << "LambEx = " << LambEx << " nm." << endl;

  return 0;
}

void BNLOpWLS::SetExEmData(string fname) {
  // File loading handled in a separate class to avoid G4/ROOT clashes
  wlsData = new BNLOpWLSData(fname);
}

