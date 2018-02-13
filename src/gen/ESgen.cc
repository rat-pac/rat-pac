// Generates an neutrino-elastic scattering event, based on the
// cross-section as function of neutrino energy and the electron's
// recoil energy.  Allow for variations in the weak mixing angle and
// the possibility of a neutrino magnetic moment
//
// J. Formaggio (UW) -02/09/2005

// Converted to Geant4+GLG4Sim+RAT by Bill Seligman (07-Feb-2006).
// I'm following the code structure of the IBD classes:
// RATVertexGen_ES handles the G4-related tasks of constructing an
// event, while this class deals with the physics of the
// cross-section.  Some of the code (the flux in particular) is copied
// from IBDgen.

#include <RAT/ESgen.hh>
#include <RAT/ESCrossSec.hh>
#include <RAT/DB.hh>

#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <TGraph.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <TMath.h>

#include <cmath>

using namespace CLHEP;

namespace RAT {

  // This class is a helper to take care of the type of spectrum that is going to be used and
  // reads the RATDB entries accordingly.

  ESgen::ESgen() : fNuType("pep"), fNuFlavor("nue"),fXS(NULL), fNuSpectrum(NULL), fFluxMax(0.),
  fGenLoaded(false),fSpectrumRndm(0),fDBName("SOLAR")
  {
    // Initialize pointers
    fMassElectron = electron_mass_c2;

    // Load the default generator
    // Later, depending on the options passed this can be reloaded
    // Data needed to Load the stuff: type of spectrum (solar or ibd, flux and flavor)
    LoadGenerator();
  }

  void ESgen::LoadGenerator() {
    // Check if the generator is already loaded.
    // If it is, do nothing
    if (fGenLoaded)
      return;

    // The parameters are taken from the database, depending of the option passed.
    // The original test with IBD data should still work
    DBLinkPtr linkdb;

    if (fDBName != "SOLAR") {
      // should be IBD data
      linkdb = DB::Get()->GetLink(fDBName);
      fNuFlavor = "nuebar";
    }else{
      // Solar generator
      // The nu type is obtained from the job options (it defaults to pep)
      linkdb = DB::Get()->GetLink(fDBName,fNuType);
      fTotalFlux = linkdb->GetD("flux");
    }
    fEnuMin = linkdb->GetD("emin");
    fEnuMax = linkdb->GetD("emax");
    fEnuTbl = linkdb->GetDArray("spec_e");
    fFluxTbl = linkdb->GetDArray("spec_flux");

    // Check what type of ES generator we are dealing with
    // Depending on type the parameters and cross section pointers
    // are initialized differently
    
    fNuSpectrum = new TGraph(fEnuTbl.size(),&fEnuTbl[0],&fFluxTbl[0]);

    // initialize the cross-section
    if (fXS != 0) {
      delete fXS;
    }
    fXS = new ESCrossSec(fNuFlavor);

    // To sample neutrino energy need to scale flux by total
    // cross section at that neutrino energy
    std::vector<double> csScaledFluxTbl(fFluxTbl.size(),0);
    for (size_t i=0;i<csScaledFluxTbl.size();i++){
      csScaledFluxTbl[i] = fFluxTbl[i] * fXS->Sigma(fEnuTbl[i]);
    }


    // If random sampler hasn't been initialized yet, lets do it now
    if (!fSpectrumRndm) {
      // Be7 is always a particular case due to its discrete nature
      // The last parameter is set to 1 to disallow interpolations
      if (fNuType == "be7") {
        fSpectrumRndm = new CLHEP::RandGeneral(&csScaledFluxTbl[0],fFluxTbl.size(),1);
      } else {
        // set interpolation bit to 0 to allow for interpolations in continuous
        // spectra
        fSpectrumRndm = new CLHEP::RandGeneral(&csScaledFluxTbl[0],fFluxTbl.size(),0);
      }
    }

    // If it reaches this point without failing then everything should be fine
    fGenLoaded = true;
  }


  ESgen::~ESgen()
  {
    if ( fXS != 0 )
    {
      delete fXS;
      fXS = 0;
    }

    if ( fNuSpectrum  != 0) {
      delete fNuSpectrum;
      fNuSpectrum = 0;
    }

    if (fSpectrumRndm) {
      delete fSpectrumRndm;
      fSpectrumRndm = 0;
    }

  }


  void ESgen::GenerateEvent(const G4ThreeVector& theNeutrino,
      G4LorentzVector& nu_incoming,
      G4LorentzVector& electron)
  {

    // Check if the generator has been loaded successfully
    // For now just throw something that can be caught at an upper level.
    // Need to define a set of specific exceptions
    if (!fGenLoaded) {
      G4Exception("[ESgen]::GenerateEvent","ArgError",FatalErrorInArgument,"Vertex generation called but it seems that it is not ready yet.");
    }

    ///!
    ///! Have to be careful with the line neutrino types (pep)
    ///! and even more careful with the double line (be7)
    ///!

    // Throw values against a cross-section.
    //bool passed=false;
    double Enu, Te;

    // Updated sampler (orders of magnitude faster)
    // Given the neutrino energy, use the differential cross section
    // shape and sample from it.
    Enu = SampleNuEnergy()*MeV;
    Te = SampleRecoilEnergy(Enu)*MeV;


    // from the incoming neutrino we have already the initial direction.
    // The final electron direction will follow that.

    // Now we have :
    // - the initial direction of the neutrino (unnormalized)
    // - the energy of the neutrino
    // - the recoil energy of the electron

    // build the 4-momentum vector of the neutrino
    // We will only use the neutrino initial momentum as a baseline to add up to the electron direction

    G4double theta_e = acos(sqrt((Te*(fMassElectron+Enu)*(fMassElectron+Enu))/(2*fMassElectron*Enu*Enu + Enu*Enu*Te)));

    G4double tot_Ee = Te + fMassElectron;
    G4double p_e = sqrt(tot_Ee*tot_Ee - fMassElectron*fMassElectron);

    // We have theta. Now randomly generate phi
    G4double phi_e = twopi*G4UniformRand();

    // This is the incoming neutrino information
    nu_incoming.setVect(theNeutrino*Enu);
    nu_incoming.setE(Enu);

    // compute electron 4-momentum
    G4ThreeVector e_mom(p_e*theNeutrino);

    // Rotation from nu direction into electron direction
    G4ThreeVector rotation_axis = theNeutrino.orthogonal();
    rotation_axis.rotate(phi_e,theNeutrino);
    e_mom.rotate(theta_e,rotation_axis);
    electron.setVect(e_mom);
    electron.setE(tot_Ee);


    // TODO: If we want to keep track of the outgoing neutrino have to add it
    // here as well and store the information in a new argument G4LorentzVector variable
    // For the moment we only pass the incoming neutrino information back.
  }

  void ESgen::Reset()
  {
    // Reset the falg dependent objects.
    // After this method a call to LoadGenerator should always follow
    if (fNuSpectrum) {
      delete fNuSpectrum;
      fNuSpectrum = 0;
    }
    if (fSpectrumRndm) {
      delete fSpectrumRndm;
      fSpectrumRndm = 0;
    }

    LoadGenerator();
  }

  void ESgen::Show()
  {
    G4cout << "Elastic Scattering Settings:\n";
    G4cout << "NuType : " << fNuType.c_str() << "\n";

  }

  //
  // If we change the neutrino type we should reload the generator
  // to force it to reload the spectra from the database
  void ESgen::SetNuType(const G4String &nutype) {

    if (fGenType != nutype ) {
      fNuType = nutype;
      fGenLoaded = false;
      LoadGenerator();
    }
  }

  //
  // If we change the neutrino flavor we should reload the generator
  // to force it to reload the spectra from the database
  void ESgen::SetNuFlavor(const G4String &nuflavor) {

    if (fNuFlavor != nuflavor ) {
      fNuFlavor = nuflavor;
      fGenLoaded = false;
      LoadGenerator();
    }
  }

  void ESgen::SetDBName(const G4String name) {
    if (fDBName != name) {
      fDBName = name;
      fGenLoaded = false;
      LoadGenerator();
    }
  }

  // This function samples the energy spectrum of the chosen neutrino and
  // decides from it the proper energy.
  // Keep in mind that pep is always the same, but be7 is a *very* special case
  G4double ESgen::SampleRecoilEnergy(G4double Enu) {

    G4double Te = 0.0;

    // Get the shape of the differential cross section.
    TGraph *dsigmadt = fXS->DrawdSigmadT(Enu);

    // Interpolate between the discrete points
    CLHEP::RandGeneral *rndm = new CLHEP::RandGeneral(dsigmadt->GetY(),dsigmadt->GetN(),0);

    Te = rndm->shoot()*(dsigmadt->GetX())[dsigmadt->GetN()-1];
    delete rndm;
    delete dsigmadt;

    return Te;
  }

  // This function samples the energy spectrum of the chosen neutrino and
  // decides from it the proper energy.
  // Keep in mind that pep is always the same, but be7 is a *very* special case
  G4double ESgen::SampleNuEnergy() {
    G4double Enu = 0.0;
    G4double tmp = 0.0;

    if (fNuType == G4String("pep")) {
      // This is a line flux. Only 1 energy possible.
      Enu = fEnuMin;
    } else if ( fNuType == G4String("be7") ) {
      // The neutrino energy of be7 must follow the branching ratio.
      // Otherwise the flux is not properly sampled.
      // Use CLHEP RandGeneral generator, with interpolation disabled to build
      // a discrete distribution of two states.
      // The random generator will return either 0 or 0.5
      tmp = fSpectrumRndm->shoot();
      Enu = (tmp<0.5)?fEnuMin:fEnuMax;
    } else {
      // Continuous distributions will return a random number between 0 and 1
      // Following the spectrum shape.
      // Scale it to the energy of the spectrum
      double scale = fEnuMax - fEnuMin;
      Enu = fEnuMin + fSpectrumRndm->shoot() * scale;
    }

    return Enu;
  }

} // namespace RAT
