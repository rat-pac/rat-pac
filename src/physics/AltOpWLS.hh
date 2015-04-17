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
// $Id: G4OpWLS.hh 71487 2013-06-17 08:19:40Z gcosmo $
//
////////////////////////////////////////////////////////////////////////
// Optical Photon WaveLength Shifting (WLS) Class Definition
////////////////////////////////////////////////////////////////////////
//
// File:        G4OpWLS.hh
// Description: Discrete Process -- Wavelength Shifting of Optical Photons 
// Version:     1.0
// Created:     2003-05-13
// Author:      John Paul Archambault
//              (Adaptation of G4Scintillation and G4OpAbsorption)
// Updated:     2005-07-28 add G4ProcessType to constructor
//              2006-05-07 - add G4VWLSTimeGeneratorProfile
// mail:        gum@triumf.ca
//              jparcham@phys.ualberta.ca
//
////////////////////////////////////////////////////////////////////////

#ifndef AltOpWLS_h
#define AltOpWLS_h 1

/////////////
// Includes
/////////////

#include "globals.hh"
#include "templates.hh"
#include "Randomize.hh"
#include "G4Poisson.hh"
#include "G4ThreeVector.hh"
#include "G4ParticleMomentum.hh"
#include "G4Step.hh"
#include "G4VDiscreteProcess.hh"
#include "G4DynamicParticle.hh"
#include "G4Material.hh"
#include "G4OpticalPhoton.hh"
#include "G4PhysicsTable.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4VWLSTimeGeneratorProfile.hh"
#include "TBranch.h"
#include <vector>

using namespace std;

// Class Description:
// Discrete Process -- Bulk absorption of Optical Photons.
// Class inherits publicly from G4VDiscreteProcess
// Class Description - End:

/////////////////////
// Class Definition
/////////////////////

class G4VWLSTimeGeneratorProfile;

class AltOpWLS : public G4VDiscreteProcess 
{

public:

        ////////////////////////////////
        // Constructors and Destructor
        ////////////////////////////////

        AltOpWLS(const G4String& processName = "OpWLS",
                         G4ProcessType type = fOptical);
        ~AltOpWLS();

private:

        AltOpWLS(const AltOpWLS &right);

        //////////////
        // Operators
        //////////////

        AltOpWLS& operator=(const AltOpWLS &right);

public:

        ////////////
        // Methods
        ////////////

  G4double GetEmEnergy(G4double);//Argument is the energy of the exciting photon

  void SetExEmData(string);

  //Method to get a value from a tree. Arguments; pointer to branch, event #,
  //name of branch.
  void* GetPointerToValue(TBranch* , int ,const char* );

  G4int SampleQY(G4double);//This is where the QY will be sampled. The argument
  //is the exciting wavelength energy.

        G4bool IsApplicable(const G4ParticleDefinition& aParticleType);
        // Returns true -> 'is applicable' only for an optical photon.

        void BuildPhysicsTable(const G4ParticleDefinition& aParticleType);
        // Build table at a right time

        G4double GetMeanFreePath(const G4Track& aTrack,
                                 G4double ,
                                 G4ForceCondition* );
        // Returns the absorption length for bulk absorption of optical
        // photons in media with a specified attenuation length.

        G4VParticleChange* PostStepDoIt(const G4Track& aTrack,
                                        const G4Step&  aStep);
        // This is the method implementing bulk absorption of optical
        // photons.

        G4PhysicsTable* GetIntegralTable() const;
        // Returns the address of the WLS integral table.

        void DumpPhysicsTable() const;
        // Prints the WLS integral table.

        void UseTimeProfile(const G4String name);
        // Selects the time profile generator

private:

        void BuildThePhysicsTable();
        // Is the WLS integral table;

  void BuildTheQYTable();

  vector< vector< vector<double> > > ExEmData;


protected:

        G4VWLSTimeGeneratorProfile* WLSTimeGeneratorProfile;
        G4PhysicsTable* theIntegralTable;
  G4PhysicsTable* theQYTable;

};

////////////////////
// Inline methods
////////////////////

inline
G4bool AltOpWLS::IsApplicable(const G4ParticleDefinition& aParticleType)
{
   return ( &aParticleType == G4OpticalPhoton::OpticalPhoton() );
}

inline
G4PhysicsTable* AltOpWLS::GetIntegralTable() const
{
  return theIntegralTable;
}

inline
void AltOpWLS::DumpPhysicsTable() const
{
  G4int PhysicsTableSize = theIntegralTable->entries();
  G4PhysicsOrderedFreeVector *v;
 
  for (G4int i = 0 ; i < PhysicsTableSize ; i++ )
    {
      v = (G4PhysicsOrderedFreeVector*)(*theIntegralTable)[i];
      v->DumpValues();
    }
}

#endif
