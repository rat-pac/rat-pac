/** \file CfSource.hh
 *  CfSource header file.
 *
 *  Author: Matthew Worcester
 */
#ifndef __RAT_CfSource__
#define __RAT_CfSource__

#include <CLHEP/Vector/LorentzVector.h>

#include <cmath>
#include <iostream>
#include <vector>

/** CfSource generates prompt photons and delayed neutrons from
 *  radioactive Californium decays.  The supported isotope is 252,
 *  which is also the default isotope given ReactorConstants.  The
 *  neutrons are generated with the correct multiplicity and each 
 *  neutron is given an energy by the Cf252NeutronSpectrum function,
 *  which the probability density of the produced neutrons as a
 *  function of neutron energy.  The prompt photons are generated 
 *  by the Cf252GammaMultiplicityFit and Cf252GammaSpectrum 
 *  functions.
 *
 *  The number and locations of the Cf sources are read in from 
 *  cf252_position.txt and parsed by CfSource to yield the 
 *  source vertex R, cos(theta), and phi.
 *
 *  23-Mar-2006 WGS: The previous paragraph is now superceded by
 *  generator structure of RAT.  The cff252_position.txt file is no
 *  longe read.  Multiple "sources" are now implemented via multiple
 *  RAT generators.
 */

namespace RAT {

  class CfSource{

  public:

    /** CfSource constructor.  Use a default isotope of Cf252, which
     * currently is the only isotope implement in this class. */
    CfSource(int newIsotope = 252);

    /** CfSource destructor. */
    ~CfSource();

    /** CfSource copy constructor. */
    CfSource(const CfSource& CfSource);

    /** CfSource overloaded = operator */
    CfSource& operator=(const CfSource& rhs);

    /** Returns the neutron multiplicity */
    int GetNumNeutron() const {return Nneutron;}

    /** Returns the prompt photon multiplicity */
    int GetNumGamma() const {return Ngamma;}

    /** Returns the energy of the neutrons produced by each Cf decay.
     *  Called with the integer index for each neutron in the neutronE
     *  array, from 0 to the total number of neutrons. */
    CLHEP::HepLorentzVector GetCfNeutronMomentum(int n) const {return neutronE[n];}
    double GetCfNeutronTime(int n) const {return Tneutron[n];}

    /** Returns the energy of the gammas produced by each Cf decay.
     *  Called with the integer index for each gamma in the gammaE
     *  array, from 0 to the total number of gammas/source. */
    CLHEP::HepLorentzVector GetCfGammaMomentum(int n) const {return gammaE[n];}
    double GetCfGammaTime(int n) const {return Tgamma[n];}

  private:

    int Isotope;

    static const int maxNeutron = 8;
    static const int maxGamma = 25;
    int Nneutron, Ngamma;

    /** \var neutronE 
     * Array containing the momentum of the neutrons from
     *  each Cf decay, indexed from 0 to the total number of
     *  neutrons. */
    std::vector<CLHEP::HepLorentzVector> neutronE;
    std::vector<float> Tneutron;

    /** \var gammaE 
     *  Array containing the momentum of the gammas from
     *  each Cf decay, indexed from 0 to the total number of
     *  gammas. */
    std::vector<CLHEP::HepLorentzVector> gammaE;
    std::vector<float> Tgamma;

    // G4 particle definitions.
    static double massNeutron;

    /** The probability density of the prompt neutrons from the Cf 
     *  decay as a function of neutron energy. */
    static float Cf252NeutronSpectrum(const float& x);

    /** The probability density of the prompt gammas from the Cf 
     *  decay as a function of integer gamma multiplicity (exact). */
    static float Cf252GammaMultiplicity(const int& x);

    /** The probability density of the prompt gammas from the Cf 
     *  decay as a function of float gamma multiplicity (fit). */
    static float Cf252GammaMultiplicityFit(const float& x);

    /** The probability density of the prompt gammas from the Cf 
     *  decay as a function of gamma energy. */
    static float Cf252GammaSpectrum(const float& x);
  };

} // namespace RAT

#endif
