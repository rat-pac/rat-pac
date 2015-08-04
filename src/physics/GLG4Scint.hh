#ifndef GLG4Scint_h
#define GLG4Scint_h 1
/** @file GLG4Scint.hh
    Declares GLG4Scint class and helpers.
    
    This file is part of the GenericLAND software library.
    $Id: GLG4Scint.hh,v 1.2 2006/03/08 03:52:41 volsung Exp $

    @author Glenn Horton-Smith (Tohoku) 28-Jan-1999
*/

// [see detailed class documentation below]

/////////////
// Includes
/////////////

#include "globals.hh"
#include "local_g4compat.hh"
#include "templates.hh"
#include "vector"
#include "G4ThreeVector.hh"
#include "G4ParticleMomentum.hh"
#include "G4Step.hh"
#include "G4OpticalPhoton.hh"
#include "G4DynamicParticle.hh"
#include "G4Material.hh" 
#include "G4PhysicsTable.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4ParticleChange.hh"
#include "G4UImessenger.hh"

#include "G4VProcess.hh"

#include "RAT/DummyProcess.hh"

#include "G4hParametrisedLossModel.hh"

class G4UIcommand;
class G4UIdirectory;

/////////////////////
// Class Definition
/////////////////////

/**
  GLG4Scint is an extremely modified version of the G4Scintillation
  process, so much so that it's not even a G4Process anymore!
  Features include arbitrary scintillation light time profile and
  spectra, Birks' law, particle-dependent specification of all
  parameters, and reemission of optical photons killed by other processes.

    - Has a GenericPostPostStepDoIt() function (note two "Post"s)
      instead of a PostStepDoIt() function.  GenericPostPostStepDoIt()
      should be called by user in UserSteppingAction.  This guarantees
      that GLG4Scint will absolutely be the last process considered, and
      will definitely see the energy loss by charged particles accurately.
      
    - Modified to allow specification of absolute yield spectra,
      resolution scale, Birk's-law coefficient, and digitized waveform,
      customized for medium and (optionally) particle type.
      
    - No longer calls G4MaterialPropertiesTable::GetProperty() in
      [Post]PostStepDoit() -- all needed data can be found quickly in
      the internal physics table.
      
    - Uses poisson random distribution for number of photons if
      mean number of photons <= 12.
      
    - The total scintillation yield is now found implicitly from
      the integral of the scintillation spectrum, which must now be
      in units of photons per photon energy.
    
    - The above feature has been modified by Dario Motta: a scintillation yield
      CAN be defined and -if found- used instead of the implicit integral of the
      scintillation spectrum. This allows having scintillators with the same
      spectrum, but different light yields.
          
    - The materials property tables used are
        SCINTILLATION  ==  scintillation spectrum
        SCINTWAVEFORM  ==  scintillation waveform or time constant
        SCINTMOD       ==  resolution scale, Birk's constant, reference dE/dx
  
    - SCINTILLATION is required in each scintillating medium.
      (Okay to omit if you don't want the medium to scintillate.)
      
    - If SCINTWAVEFORM is missing, uses exponential waveform with default
      ScintillationTime.  If SCINTWAVEFORM contains negative "Momentum"'s
      then each "Momentum" is the decay time and its corresponding value
      is the relative strength of that exponential decay.
      Otherwise, the "PhotonEnergy" of each element is a time, and the
      Value of each element is the relative strength.
      
    - Default values of resolution scale (=1.0), Birk's constant (=0.0)
      and reference dE/dx (=0.0) are used if all or part of SCINTMOD is
      is missing.  SCINTMOD "PhotonEnergy" values should be set to the
      index number (0.0, 1.0, 2.0, with no units).
      
    - Birk's law (see 1998 Particle Data Booklet eq. 25.1) is implemented
      as
   yield(dE/dx) = yield_ref * dE/dx * (1 + kb*(dE/dx)_ref) / (1 + kb*(dE/dx)).
      I.e., the scintillation spectrum given in SCINTILLATION is
      measured for particles with dE/dx = (dE/dx)_ref.  The usual
      formula is recovered if (dE/dx)_ref = 0.0 (the default).
      This is useful if you have an empirically-measured spectrum for
      some densely-ionizing particle (like an alpha).
      
    - The constructor now accepts an additional string argument, tablename,
      which allows selection of alternate property tables.  E.g,
      tablename = "neutron" might be used to allow specification of a
      different waveform for scintillation due to neutron energy deposition.
      The code then searches for tables with names of the form
         "SCINTILLATIONneutron"
      If it finds such a table, that table is used in preference to
      the default (un-suffixed) table when stepping particles of that type.
      
    - The process generates at most maxTracksPerStep secondaries per step.
      If more "real" photons are needed, it increases the weight of the
      tracked opticalphotons.  Opticalphotons are thus macro-particles in
      the high-scintillation case.  The code preserves an integer number
      of real photons per macro-particle.
*/ 

class GLG4Scint : public G4UImessenger {
public:
  class MyPhysicsTable {
  public:
    class Entry {
      public:
        Entry();
        ~Entry();
        void Build(const G4String& name, int material_index,
                   G4MaterialPropertiesTable *matprops);

        G4PhysicsOrderedFreeVector* spectrumIntegral;
        G4PhysicsOrderedFreeVector* reemissionIntegral;
        G4PhysicsOrderedFreeVector* timeIntegral;
        bool I_own_spectrumIntegral, I_own_timeIntegral;
        bool HaveTimeConsts;
        G4double resolutionScale;
        G4double birksConstant;
        G4double DMsConstant; //for Dongming et al. QF
        G4double ref_dE_dx;
        G4double light_yield;
        G4MaterialPropertyVector* QuenchingArray;
    };

    static MyPhysicsTable* FindOrBuild(const G4String& name);
    static const MyPhysicsTable* GetDefault(void) { return head; }
    void IncUsedBy(void) { ++used_by_count; }
    void DecUsedBy(void) { if (--used_by_count <= 0) delete this; }
    const Entry* GetEntry(int i) const { return data + i; }
    void Dump(void) const;

    G4String* name;

  private:
    friend class GLG4Scint;
    MyPhysicsTable();
    ~MyPhysicsTable();
    void Build(const G4String& newname);

    MyPhysicsTable* next;
    G4int used_by_count;
    Entry* data;
    G4int length;

    static MyPhysicsTable* head;
  };
  
  ////////////////////////////////
  // Constructors and Destructor
  ////////////////////////////////

  GLG4Scint(const G4String& tableName= "", G4double lowerMassLimit= 0.0);
  ~GLG4Scint();  

  ////////////
  // Methods
  ////////////

  G4VParticleChange* PostPostStepDoIt(const G4Track& aTrack,
                                      const G4Step&  aStep);
  G4double GetLowerMassLimit(void) const;
  void DumpInfo() const;
  MyPhysicsTable* GetMyPhysicsTable(void) const;
  G4int GetVerboseLevel(void) const;
  void  SetVerboseLevel(G4int level);    

  // Methods are for G4UImessenger
  void SetNewValue(G4UIcommand * command,G4String newValues);
  G4String GetCurrentValue(G4UIcommand * command);

  ////////////////
  // static methods
  ////////////////

  static G4VParticleChange* GenericPostPostStepDoIt(const G4Step *pStep);

  // For energy deposition diagnosis
  static void ResetTotEdep() {
    totEdep = totEdep_quenched = totEdep_time = 0.0;
    scintCentroidSum *= 0.0;
  }
  static G4double GetTotEdep() { return totEdep; }
  static G4double GetTotEdepQuenched() { return totEdep_quenched; }
  static G4double GetTotEdepTime() { return totEdep_time; }
  static G4bool GetDoScintillation() { return doScintillation; }
  static G4ThreeVector GetScintCentroid() {
    return scintCentroidSum * (1.0 / totEdep_quenched);
  }
  
protected:
  int verboseLevel;

  // Below is the pointer to the physics table which this instance
  // of GLG4Scint will use.  You may create a separate instance
  // of GLG4Scint for each particle, if you like.
  MyPhysicsTable* myPhysicsTable;

  // below is the lower mass limit for this instance of GLG4Scint
  G4double myLowerMassLimit;

  // return value of PostPostStepDoIt
  G4ParticleChange aParticleChange;
  
  ////////////////
  // static variables
  ////////////////  

  // vector of all existing GLG4Scint objects.
  // They register themselves when created,
  // remove themselves when deleted.
  // Used by GenericPostPostStepDoIt
  static G4std::vector<GLG4Scint*> masterVectorOfGLG4Scint;

  // top level of scintillation command
  static G4UIdirectory* GLG4ScintDir;

  // universal maximum number of secondary tracks per step for GLG4Scint
  static G4int maxTracksPerStep;

  // universal mean number of true photons per secondary track in GLG4Scint
  static G4double meanPhotonsPerSecondary;

  // universal on/off flag
  static G4bool doScintillation;

  // on/off flag for absorbed opticalphoton reemission
  static G4bool doReemission;

  // total real energy deposited and total quenched energy deposited
  static G4double totEdep;
  static G4double totEdep_quenched;
  static G4double totEdep_time;
  static G4ThreeVector scintCentroidSum;

  // Bogus processes used to tag photons created in GLG4Scint
  static DummyProcess scintProcess;
  static DummyProcess reemissionProcess;

  // Quenching Factor
  static G4double QuenchingFactor;

  // User-given (constant) quenching factor flag
  static G4bool UserQF;

  static G4String fPrimaryName;
  static G4double fPrimaryEnergy;
  static G4std::vector<DummyProcess*> reemissionProcessVector;
  static G4int fPhotonCount;
 
  //precision goal for the iterative time delay
  static double TimePrecGoal;

  // maximum number of iterations before giving up time delay refinement
  static int maxiter;

public:
  //methods to access the Quenching Factor
  static G4double GetQuenchingFactor() { return QuenchingFactor; }
  static void SetQuenchingFactor(G4double qf);

  static G4double GetTimePrecGoal() { return TimePrecGoal; }
  static void SetTimePrecGoal(double prec);
  static int GetMaxIter() { return maxiter; }
  static void SetMaxIter(int maxit) { maxiter = maxit; }
};

////////////////////
// Inline methods
////////////////////

inline GLG4Scint::MyPhysicsTable * GLG4Scint::GetMyPhysicsTable() const {
  return myPhysicsTable;
}

inline void GLG4Scint::DumpInfo() const {
  G4cout << "GLG4Scint[" << *(myPhysicsTable->name) << "] {\n"
   << "  myLowerMassLimit=" << myLowerMassLimit << G4endl;
  if (myPhysicsTable) {
    if (verboseLevel >= 2)
      myPhysicsTable->Dump();
  }
  G4cout << "}" << G4endl;
}

inline G4double GLG4Scint::GetLowerMassLimit() const {
  return myLowerMassLimit;
}

inline void GLG4Scint::SetVerboseLevel(int level) { verboseLevel = level; }

inline G4int GLG4Scint::GetVerboseLevel() const { return verboseLevel; }

#endif  // GLG4Scint_h

