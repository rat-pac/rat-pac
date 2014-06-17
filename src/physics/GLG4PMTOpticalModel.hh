/** @file GLG4PMTOpticalModel.hh
    Defines a FastSimulationModel class for handling optical photon
    interactions with PMT: partial reflection, transmission, absorption,
    and hit generation.
    
    This file is part of the GenericLAND software library.
    $Id: GLG4PMTOpticalModel.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
    
    @author Glenn Horton-Smith, March 20, 2001.
    @author Dario Motta, Feb. 23 2005: Formalism light interaction with photocathode.
*/

#ifndef __GLG4PMTOpticalModel_hh__
#define __GLG4PMTOpticalModel_hh__

#include "G4VFastSimulationModel.hh"
#include "G4MaterialPropertyVector.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4UImessenger.hh"
#include <utility>
#include <vector>

class G4UIcommand;
class G4UIdirectory;
class G4Region;

G4complex carcsin(G4complex theta); //complex sin^-1
G4complex gfunc(G4complex ni, G4complex nj, G4complex ti, G4complex tj);
G4complex rfunc(G4complex ni, G4complex nj, G4complex ti, G4complex tj);
G4complex trfunc(G4complex ni, G4complex nj, G4complex ti, G4complex tj,
		 G4complex tk);

class GLG4PMTOpticalModel : public G4VFastSimulationModel,
			    public G4UImessenger
{
public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
  // 28-Jul-2006 WGS: Must define a G4Region for Fast Simulations
  // (change from Geant 4.7 to Geant 4.8).
  GLG4PMTOpticalModel (G4String, G4Region*, G4LogicalVolume* body, 
                       G4LogicalBorderSurface *pc_log_surface, 
		       double efficiency_correction=1.0,
		       double dynodeTop = 0.0, double dynodeRadius = 0.0,
		       double prepulseProb = 0.0);
  // Note: There is no GLG4PMTOpticalModel(G4String) constructor.
  ~GLG4PMTOpticalModel ();

  //------------------------------
  // Virtual methods of the base
  // class to be coded by the user
  //------------------------------

  G4bool IsApplicable(const G4ParticleDefinition&);
  G4bool ModelTrigger(const G4FastTrack &);
  void DoIt(const G4FastTrack&, G4FastStep&);

  // following two methods are for G4UImessenger
  void SetNewValue(G4UIcommand * command,G4String newValues);
  G4String GetCurrentValue(G4UIcommand * command);
  
  void SetBEfficiencyCorrection(std::vector<std::pair<int,double> > _BEffiCorr){BEfficiencyCorrection=_BEffiCorr;G4cout<<GetName()<<": B correction table set\n";}
  void DumpBEfficiencyCorrectionTable()
  {
    G4cout<<"Magnetic correction table for the PMT efficiencies of "<<GetName()<<":\nPMT ID  corr. factor\n";
    for(int i=0;i<int(BEfficiencyCorrection.size());i++)
      G4cout<<BEfficiencyCorrection[i].first<<","<<BEfficiencyCorrection[i].second<<"\n";
  }
  
private:
  // material property vector pointers, initialized in constructor, 
  // so we don't have to look them up every time DoIt is called.
  G4MaterialPropertyVector * _rindex_glass;        // function of photon energy
  G4MaterialPropertyVector * _rindex_photocathode; // function of photon energy
  G4MaterialPropertyVector * _kindex_photocathode; // n~ = n+ik, as in M.D.Lay
  G4MaterialPropertyVector * _efficiency_photocathode; // necessary?
  G4MaterialPropertyVector * _thickness_photocathode; // function of Z (mm)
  // interior solid (vacuum), also initialized in constructor,
  // so we don't have to look it up each time DoIt is called.
  // Note it is implicitly assumed everywhere in the code that this solid
  // is vacuum-filled and placed in the body with no offset or rotation.
  G4VSolid * _inner1_solid;
  G4VPhysicalVolume * _inner1_phys,* _inner2_phys, * _central_gap_phys;

  // "luxury level" -- how fancy should the optical model be?
  G4int _luxlevel;
  G4double _efficiency_correction; // global efficiency correction, default to 1.0
  G4double _dynodeTop;
  G4double _dynodeRadius;
  G4double _prepulseProb;

  // verbose level -- how verbose to be (diagnostics and such)
  G4int _verbosity;

  // directory for commands
  static G4UIdirectory* fgCmdDir;

  // "current values" of many parameters, for efficiency
  // [I claim it is quicker to access these than to
  // push them on the stack when calling CalculateCoefficients, Reflect, etc.]
  // The following are set by DoIt() prior to any CalculateCoefficients() call.
  G4double _photon_energy; // energy of current photon
  G4double _wavelength;    // wavelength of current photon
  G4double _n1;            // index of refraction of curr. medium at wavelength
  G4double _n2, _k2;       // index of refraction of photocathode at wavelength
  G4double _n3;            // index of refraction of far side at wavelength
  G4double _efficiency;    // efficiency of photocathode at wavelength (?)
  G4double _thickness;     // thickness of photocathode at current position
  G4double _cos_theta1;    // cosine of angle of incidence
  // The following are set by CalculateCoefficients()
  // and used by DoIt(), Refract(), and Reflect():
  G4double _sin_theta1;    // sine of angle of incidence
  G4double _sin_theta3;    // sine of angle of refraction
  G4double _cos_theta3;    // cosine of angle of refraction
  G4double fR_s;           // reflection coefficient for s-polarized light
  G4double fT_s;           // transmission coefficient for s-polarized light
  G4double fR_p;           // reflection coefficient for p-polarized light
  G4double fT_p;           // transmission coefficient for p-polarized light
  G4double fR_n;           // reference for fR_s/p at normal incidence
  G4double fT_n;           // reference for fT_s/p at normal incidence

  // private methods
  void CalculateCoefficients(); // calculate and set fR_s, etc.
  void Reflect(G4ThreeVector &dir, G4ThreeVector &pol, G4ThreeVector &norm);
  void Refract(G4ThreeVector &dir, G4ThreeVector &pol, G4ThreeVector &norm);

  static double surfaceTolerance;
  
  std::vector<std::pair<int,double> > BEfficiencyCorrection;
};

#endif
