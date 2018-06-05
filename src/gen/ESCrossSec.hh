////////////////////////////////////////////////////////////////////
///
///         Code contributed by the SNO+ collaboration
///
/// \class RAT::ESCrossSec
///
/// \brief Calculates neutrino-electron elastic scattering.
/// (based on original QSNO code by F. Duncan, M. Chen and Y. Takeuchi).
///
/// \author Nuno Barros <nfbarros@hep.upenn.edu> -- contact person
///
/// REVISION HISTORY:\n
/// 23-NOV-2010 - Nuno Barros 	- Imported and adapted into SNO+ RAT
///             				- Shamelessly taken from SNO QPhysics class PNuE
///
/// 22-JUN-2012 - Nuno Barros	- Cleaned up code and solved a small problem with unit conversion.
///
/// 07-JUL-2012 - Nuno Barros 	- Removed all Geant4 streamers replacing them by RAT log objects.
///								- Changed geant4 data types by C++ data types (suggestion by CIC).
///								- Revised constness of members.
///
///
/// \details Calculates the neutrino-electron ES cross section.
///			The calculation is performed either for the ES_e (W+Z) channel, or the ES_mu,tau (Z) channel.
/// 		Some remarks concerning the calculations.
///			\param E,Enu 	: Neutrino energy (MeV).
///			\param T,Te		: Recoil electron energy (MeV).
///			\return Cross section in units of $10^{-42}cm^{2}$
///
///			Available strategies for the ES cross section calculation (set by messenger):
///			- 1 : Original routine from QSNO::PNuE (Bahcall).
///			- 2 : Improved routine from QSNO::PNuE (without rad. corrections).
///			- 3 : Improved routine from QSNO::PNuE (with rad. corrections - analytical).
///			- 4 (default) : Improved routine from QSNO::PNuE (with rad. corrections - table).
///
///
////////////////////////////////////////////////////////////////////

#ifndef __RAT_ESCrossSec__
#define __RAT_ESCrossSec__

// G4 headers
#include <globals.hh>
// RAT headers
#include <RAT/LinearInterp.hh>
#include <RAT/GLG4StringUtil.hh>
// ROOT headers
#include <TGraph.h>

namespace RAT {

  // Forward declarations within the namespace
  class ESCrossSecMessenger;


    class ESCrossSec {

    public:
	enum NuEType {nue,nuebar,numu,numubar};

	ESCrossSec(const char* flavor = "nue");

	~ESCrossSec();

	// Set's the defaults for the calculation
	void Defaults();


	/**
	 * @brief Calculate the total cross section for the neutrino energy Enu.
	 *
	 * @param Enu
	 * @return total cross section in units of \f$ 10^{-42} cm^{2} \f$ .
	 */
	double Sigma(const double Enu) const;


	/**
	 * @brief Calculate the differential cross section for the neutrino energy Enu.
	 *
	 * @param Enu Incoming neutrino energy (MeV).
	 * @param Te  Recoil electron energy (MeV).
	 * @return Differencial cross section \f$ \frac{d\sigma}{dT} \f$ in units of \f$ 10^{-42} cm^{2} \f$ .
	 */
	double dSigmadT(const double Enu,const double Te) const;

	/**
	 * Integrate the differential cross section between recoil energies T1 and T2.
	 *
	 * @param Enu Incoming neutrino energy (MeV).
	 * @param T1  Lower limit of recoil energy interval (MeV).
	 * @param T2  Upper limit of recoil energy interval (MeV).
	 * @return \f$ \left.\frac{d\sigma}{dT}\right|_{\left[T1;T2\right]} \f$ in units of \f$ 10^{-42} cm^{2} \f$ .
	 */
	double IntegraldSigmadT(const double Enu,const double T1,const double T2) const ;

	/**
	 * @brief Calculate the differential cross section as a function of the recoil angle.
	 *
	 * @param Enu Incoming neutrino energy.
	 * @param CosTh Cosine of laboratory recoil angle of the electron.
	 * @return  \f$ \frac{d\sigma}{d\cos \theta} \f$ in units of \f$ 10^{-42} cm^{2} \f$ .
	 */
	double dSigmadCosTh(const double Enu,const double CosTh) const;

	/**
	 * @brief 3D equivalent of ESCrossSec::dSigmadCosTh
	 *
	 * @param Enu Incoming neutrino energy.
	 * @param theta Cosine (FIXME) of laboratory recoil angle of the electron.
	 * @param phi 	Azimuthal recoil angle.
	 * @return  \f$ \frac{d\sigma}{d\Omega} \f$ in units of \f$ 10^{-42} cm^{2} \f$ .
	 *
	 * @note Usually not used. Needs further verification.
	 *
	 */
	double dSigmadOmega(const double Enu,const double theta, const double phi) const;


	//-----------------------------------------------------------------------
	/**
	 * @brief Setter for which calculation to use.
	 *
	 * The available calculations are:
	 * 	-# 1 : No radiative corrections (Bahcall first calculation).
	 * 	-# 2 : No radiative corrections from table.
	 * 	-# 3 : Full analytical calculation with radiative corrections.
	 * 	-# 4 : Full calculations with radiative corrections from table (default).
	 *
	 * @param ii Option for calculation to use.
	 *
	 */
	void SetRadiativeCorrection(const int ii);

	/**
	 * @brief Getter of the cross section calculation type being used.
	 *
	 * @return index of calculation type.
	 */
	inline int GetRadiativeCorrection( ) const {return fRadiativeCorrection;};

	/**
	 * @brief Setter for the Weak mixing angle.
	 *
	 * @param sintw Weak mixing angle : \f$ \sin \theta_{W} \f$
	 *
	 */
	void SetSinThetaW(const double &sintw ) ; //{fsinthetaW2 = sintw;};

	/**
	 * @brief Getter for the Weak mixing angle.
	 *
	 * @return Weak mixing angle : \f$ \sin \theta_{W} \f$
	 *
	 */
	double GetSinThetaW() const {return fsinthetaW2;};

	/**
	 * @brief Setter for the neutrino type to be used.
	 *
	 * @param reaction neutrino type (nue,numu,nuebar,numubar)
	 */
	void SetReaction(const std::string &reaction);

	/**
	 * @brief Getter for the neutrino type to be used.
	 *
	 * @return reaction neutrino type (nue,numu,nuebar,numubar)
	 */
    const std::string& GetReaction() const {return fReactionStr;};

	/**
	 * @brief Return a TGraph with the differential cross section for an incoming neutrino with energy Enu.
	 * @param Enu Incoming neutrino energy (MeV)
	 * @return TGraph with the shape of \f$ \frac{d\sigma}{dT} \f$ in units of \f$ 10^{-42} cm^{2} \f$ .
	 */
	TGraph *DrawdSigmadT(const double Enu) const;

	/**
	 * Returns the global normalization of the cross section calculation.
	 * For precision reasons, the cross-section is performed on a different scale, and therefore
	 * any result returned by the calculation is missing this scale, which has to be applied separately.
	 *
	 * @return cross section scaling factor (1e-42)
	 */
	double CrossSecNorm() const {return 1e-42;};

    protected:

	/**
	 * @brief Internal function to load the data from the DB.
	 */
	void LoadTablesDB();

	void CalcG();


    private:

	/**
	 * Rename of ESCrossSec::Sigma. To be discontinued.
	 *
	 * @see ESCrossSec::Sigma
	 * @param Enu Incoming neutrino energy.
	 * @return total cross section in units of \f$ 10^{-42} cm^{2} \f$ .
	 */
    double SigmaLab(double Enu) const;


	NuEType fReaction;   	/// Reaction type
	std::string fReactionStr;	/// String characterizing the reaction type

	double fEmin,fEmax;	/// Auxiliary variables to deal with the energies

	// Some constants
	static const double  fGf; 		/// Fermi constant (GeV^-2)
	static const double  fhbarc; 	/// hbar*c (MeV*fm)
	static const double  fhbarc2;	/// hbar*c^2(GeV^2 mb)
	static const double  falpha;	/// radiative correction term


	/**
	 * This variable is defined as static (and not const) because it can be changed
	 * in the macro file. However, the change should propagate to all instances of the class
	 */
	static double  fsinthetaW2;

	double  fMe;			/// electron mass
	double  sigmaoverme; 	/// \f$ \frac{\sigma}{m_{e^{-}}}\f$

	double  fgL;
	double  fgR;

	//-----------------------------------------------------------------------
	// add by y.t. 16-JAN-2003
	int     fRadiativeCorrection;  // flag to use radiative correction or not
	//double  fL(double x);        // internal routine


	/// Vars to manipulate the tables. Since the tables are the same for all instances, these can be static
	static const double  fTableTeMin;
	static const double  fTableTeMax;

	// Total cross section table
	int    fNDataTot;
	float  fEnuStepTot;
	LinearInterp<double> fTableTot_e;

	// Differential cross section table
	int                  fNDataDif;
	float                fEnuStepDif;
	LinearInterp<double> fTableDif_e;
	std::vector<double>  fTableDif;

	ESCrossSecMessenger *fMessenger;

    };

//
// Some inline definitions to make the code a bit faster
//

    inline void ESCrossSec::CalcG()
    {
	// calculate the gL and gR for
	// the reaction type

	if ( fReaction == nue)           {

	    fgL =  0.5 + fsinthetaW2;
	    fgR =  fsinthetaW2;

	} else if (fReaction == nuebar)  {

	    fgL =   fsinthetaW2;
	    fgR =   0.5 + fsinthetaW2;

	} else if (fReaction == numu)    {

	    fgL =   -0.5 + fsinthetaW2;
	    fgR =    fsinthetaW2;

	} else if (fReaction == numubar) {

	    fgL =   fsinthetaW2;
	    fgR =   -0.5 + fsinthetaW2;

	}
	else throw std::invalid_argument("Unknown reaction " + fReactionStr);
    }
}



#endif






