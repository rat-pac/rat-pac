///////////////////////////////////////////////////////////////////
// RAT::VertexGen_Decay0
// See notes in Decay0.hh, COPYING.decay0
///////////////////////////////////////////////////////////////////

#ifndef __RAT_VertexGen_Decay0__
#define __RAT_VertexGen_Decay0__

#include <RAT/GLG4VertexGen.hh>
#include "RAT/DB.hh"
#include <RAT/Decay0.hh>
#include <fstream>
#include <G4Event.hh>
#include <G4ThreeVector.hh>
#include <globals.hh>
#include <vector>
namespace RAT {

class VertexGen_Decay0 : public GLG4VertexGen {
  public:
    VertexGen_Decay0(const char *arg_dbname="decay0");
    virtual ~VertexGen_Decay0();

    virtual void  GeneratePrimaryVertex(G4Event *argEvent,
                                        G4ThreeVector &dx,
                                        G4double dt);
    virtual void   SetState(G4String newValues );
    virtual        G4String GetState();
	virtual void   GetParentAZ(G4int &A1, G4int &Z1, G4int &A2, G4int &Z2);

  private:
    G4String  fCodeToName(G4int code);
    void StripIsotopeSuffix();

    G4String fType;                 // defines or double beta decay "2beta" or
                                    // background and source study "backg"
    G4String   fIsotope;            // parent isotope after stripping suffixes
    G4String   fIsotopeRawIn;         // parent isotope as passed in the macro
    bool      fHasTimeCutoff;
    bool      fHasAlphaCut;

    G4int fLevel;                   // daughter energy level
    G4int fMode;                    // decay mode
    G4float fLoE, fHiE;             // limit for energy spectrum

    DBLinkPtr fLdecay;

  protected:
    Decay0 *fDecay0;
};

} // namespace RAT

#endif

