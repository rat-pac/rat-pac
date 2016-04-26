/**
 * @class BNLOpWLS
 *
 * A custom wavelength-shifting process developed by L. Bignell at BNL.
 *
 * @author L. Bignell, BNL
 *
 * This code is adapted from the Geant4 v10 optical wavelength shifting model,
 * G4OpWLS, by John Paul Archambault.
 *
 * The original header block appears below.
 */
#ifndef __BNLOpWLS__
#define __BNLOpWLS__

#include <globals.hh>
#include <templates.hh>
#include <G4Step.hh>
#include <G4VDiscreteProcess.hh>
#include <G4OpticalPhoton.hh>
#include <G4PhysicsOrderedFreeVector.hh>
#include <vector>

class G4VWLSTimeGeneratorProfile;
class G4PhysicsTable;
class BNLOpWLSData;
// FIXME
//class TBranch;

class BNLOpWLS : public G4VDiscreteProcess {
public:
  BNLOpWLS(const G4String& processName="OpWLS", G4ProcessType type=fOptical);
  virtual ~BNLOpWLS();

private:
  BNLOpWLS(const BNLOpWLS& right);
  BNLOpWLS& operator=(const BNLOpWLS& right);

public:
  // Argument is the energy of the exciting photon
  G4double GetEmEnergy(G4double);

  // Arguments: File name, Tree name containing Ex/Em data, Branch names for
  // excitation wavelength and emission intensity. Bins are assumed to be nm.
  //
  // Currently, the data are stored in the ExEmData vector, with the exciting
  // wavelength for event i stored in ExEmData.at(i).at(0).at(0), the emitting
  // wavelengths stored in ExEmData.at(i).at(1).at(:), and the normalized 
  // stored at ExEmData.at(i).at(2).at(:).
  // These data are read into memory to speed things up.
  void SetExEmData(std::string fname);

  // This is where the QY will be sampled. The argument
  // is the exciting wavelength energy.
  G4int SampleQY(G4double);

  // Returns true -> 'is applicable' only for an optical photon.
  G4bool IsApplicable(const G4ParticleDefinition& aParticleType);

  // Build table at a right time
  void BuildPhysicsTable(const G4ParticleDefinition& aParticleType);

  // Returns the absorption length for bulk absorption of optical
  // photons in media with a specified attenuation length.
  G4double GetMeanFreePath(const G4Track& aTrack,
                           G4double,
                           G4ForceCondition*);

  // This is the method implementing bulk absorption of optical
  // photons.
  G4VParticleChange* PostStepDoIt(const G4Track& aTrack,
                                  const G4Step& aStep);

  // Returns the address of the WLS integral table.
  G4PhysicsTable* GetIntegralTable() const;

  // Selects the time profile generator
  void UseTimeProfile(const G4String name);

private:
  // Is the WLS integral table
  void BuildThePhysicsTable();

  // This is essentially a duplicate of BuildThePhysicsTable, but rather than
  // get the CDF for the "WLSCOMPONENT" variable, it gets it for "QUANTUMYIELD"
  void BuildTheQYTable();

  BNLOpWLSData* wlsData;

protected:
  G4VWLSTimeGeneratorProfile* WLSTimeGeneratorProfile;
  G4PhysicsTable* theIntegralTable;
  G4PhysicsTable* theQYTable;
};

// Inline methods

inline
G4bool BNLOpWLS::IsApplicable(const G4ParticleDefinition& aParticleType) {
   return (&aParticleType == G4OpticalPhoton::OpticalPhoton());
}

inline G4PhysicsTable* BNLOpWLS::GetIntegralTable() const {
  return theIntegralTable;
}

#endif  // __BNLOpWLS__

