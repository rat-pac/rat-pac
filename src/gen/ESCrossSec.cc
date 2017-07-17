// ESCrossSec.cc
// Contact person: Nuno Barros <nfbarros@hep.upenn.edu>
// See ESCrossSec.hh for more details
//———————————————————————//

// -- Geant4 includes
#include <G4PhysicalConstants.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>

// -- RAT includes
#include <RAT/DB.hh>
#include <RAT/ESCrossSec.hh>
#include <RAT/ESCrossSecMessenger.hh>
#include <RAT/Log.hh>

// -- ROOT includes
#include <CLHEP/Units/SystemOfUnits.h>
#include <CLHEP/Units/PhysicalConstants.h>
#include <TH1F.h>

using namespace CLHEP;

namespace RAT {

/**
// Couple of constants
// For consistency the values are taken from Geant4 directly
// except for Fermi's constant (fGf) which for some reason
// I couldn't find the reference
// Some constants have a constant multiplicative term
// That's to get the values into the correct units expected
// in the code, which are different from the ones
// provided from Geant4/CLHEP
 */

const double  RAT::ESCrossSec::fGf    = 1.166371e-5;        	// Fermi constant (GeV^-2)
const double  RAT::ESCrossSec::fhbarc = hbarc*1e12; 			// hbar*c (MeV*fm)
const double  RAT::ESCrossSec::fhbarc2= fhbarc*fhbarc*1e-5; 	// hbar*c^2(GeV^2 mb)
const double  RAT::ESCrossSec::falpha = fine_structure_const;   //


/**
 * Some other static constants that are valid for all instances of the class.
 * Limits for which the table should not be trusted (the interpolation is not as efficient).
 */
const double RAT::ESCrossSec::fTableTeMin = 0.05;
const double RAT::ESCrossSec::fTableTeMax = 0.95;

/**
 * The weak mixing angle (\f$ \sin^{2}\theta_{W}\f$) is declared only as static
 * (no const modifier) as it can be changed by macro command.
 * This change should reflect all instances of the class.
 */
double RAT::ESCrossSec::fsinthetaW2 = 0.23116;

ESCrossSec::ESCrossSec(const char* flavor)
{
	fReactionStr = flavor;
	Defaults();

	// Messenger to override some parameters
	fMessenger = new ESCrossSecMessenger(this);

}

ESCrossSec::~ESCrossSec()
{

	if (fMessenger != NULL) {
		delete fMessenger;
		fMessenger = NULL;
	}

}

void ESCrossSec::Defaults()
{
	// load default parameters

	//fMe = (G4ParticleTable::GetParticleTable()->FindParticle("e-"))->GetPDGMass(); // MeV

	// defaults to PDG value.
	// Can be tuned in the macro file or in the command
	// file through the ESxsectionMessenger
	// /generator/es/xsection/wma
	//fsinthetaW2 = 0.23116; // effective angle PDG 2010

	fMe = (G4ParticleTable::GetParticleTable()->FindParticle("e-"))->GetPDGMass(); // MeV
	sigmaoverme = 2*fGf*fGf*fMe*1.0e9*fhbarc2/pi; // 10^-42 cm^2/MeV

	// Default calculation strategy
	// Can be set by messenger
	// /generator/es/xsection/strategy
	SetReaction(fReactionStr);

	/** Options for strategy:
	 *
	 *  * 1:       previous routine
	 *  * 2:       new routine w/o  rad. cor.
	 *  * 3:       new routine with rad. cor. w/o table
	 *  * 4:       new routine with rad. cor. with table
	 *  * others:  new routine with rad. cor. with or
	 *          w/o table (fTableTeMin/fTableTeMax)
	 */

  if (fReaction == nue || fReaction == numu)
    SetRadiativeCorrection(4);
  else
    SetRadiativeCorrection(1);



//	// Limits for which the table should not be trusted (the interpolation is not as efficient).
//	fTableTeMin = 0.05;       // don't use table from 0 upto 0.05*Temax
//	fTableTeMax = 0.90;       // don't use table from 0.90*Temax upto Temax

//	sigmaoverme = 2*fGf*fGf*fMe*1.0e9*fhbarc2/pi; // 10^-42 cm^2/MeV

}

//-------------------------------------------------------------------


/// Calculates total cross section for a given neutrino energy
double ESCrossSec::Sigma(const double Enu) const
{
	// return total cross section in units cm^-42
	// for laboratory neutrino energy Enu
	// The calculation varies according to the chosen strategy

	// First do the most basic check
	if (Enu == 0) return 0.0;
	if (Enu < 0) {
		std::stringstream ss;
		ss << "[ESCrossSec]::Sigma : Invalid neutrino Energy ( Enu = "
				<< Enu << " ).";
		RAT::Log::Die(ss.str(),1);
		throw std::invalid_argument("Invalid neutrino energy (" + ss.str() + ").");
	}

	double   sigma = 0.0;
	double   Temax = Enu - 1.0/(2.0/fMe + 1.0/Enu);

	if (fRadiativeCorrection == 1) {

		/////////////////////////////////////////////////////////////
		// previous routine
		/////////////////////////////////////////////////////////////

		const double   gL2 = fgL*fgL;
		const double   gR2 = fgR*fgR;
		const double   gLR = fgL*fgR;

		sigma =    (gL2 + gR2)*Temax
				- (gR2/Enu + gLR*fMe/(2.0*Enu*Enu))*Temax*Temax
				+ (gR2/(3.0*Enu*Enu))*Temax*Temax*Temax;

		sigma = sigma*sigmaoverme;

		return sigma;

	} else {

		/////////////////////////////////////////////////////////////
		// radiative correction
		/////////////////////////////////////////////////////////////
		const int k = ((int)(Enu/fEnuStepTot) - 1);
		// Different than 1,2 or 3 --> Table
		if (fRadiativeCorrection != 3 && fRadiativeCorrection != 2 &&
				k >= 0 && k < fNDataTot - 1) {

			if ( fReaction == nue || fReaction == numu ) {
				// interpolate by a straight line
				sigma = fTableTot_e(Enu);

			} else {
				// no support for nuebar and numubar
				// in tables
				std::stringstream ss;
				ss << "[ESCrossSec]::Sigma : " << fReactionStr
					<< " not supported with Radiative correction strategy "
					<< fRadiativeCorrection << " !!";
				RAT::Log::Die(ss.str(),1);
				throw;
			}

			return sigma;

		} else {
			// Apply Radiative corrections without the table
			// fRadiativeCorrection = 2,3 or 4 in the outer range

			// To avoid log(-1.0e-15) in dSigmadT()
			Temax = Temax *(1.0 - 1.0e-13);

			const int     istep = 1000;     // should not be hard corded??
			const double  dstep = Temax / (double) istep;
			double  Te;

			for (int i = 1; i <= istep; i++) {
				Te = dstep * (double) i;
				sigma = sigma + dSigmadT(Enu, Te) * dstep;
			}
			return sigma;
		}

	}


	// Should never reach this point
	// Throw an exception if that happens
	std::stringstream ss;
	ss << "[ESCrossSec]::Sigma : Reached end of function while calculating Sigma. Something is wrong with the calculation.\n";

	ss << "[ESCrossSec]::Sigma : Current parameters : Calculation Strategy : [ "
			<< fRadiativeCorrection << " ], Reaction : [ " << fReactionStr << " ].\n";
	ss << "[ESCrossSec]::Sigma : Enu : [ " << Enu << " ], TeMax  : [ "
			<<  Temax << " ].";
	RAT::Log::Die(ss.str(),1);
	throw;
}

//--------------------------------------
double ESCrossSec::SigmaLab(const double elab) const
{
	return Sigma(elab);
}

//--------------------------------------
// Differential cross-section calculation

double ESCrossSec::dSigmadT(const double Enu,const double Te) const
{
	// Differential cross section
	//  d Sigma
	//  -------
	//  d Te
	//
	//  Enu = laboratory neutrino energy        -- units: MeV
	//  Te  = laboratory recoil electron energy -- units: MeV
	//  dSigmadT units:  10^-42 cm^2/MeV

	// check kinematical limit
	double Temax = Enu - 1.0/(2.0/fMe + 1.0/Enu);

	if(Te <= 0.0 || Te > Temax) return 0.;


	int k1;
	int k2;

	if (fRadiativeCorrection == 1) {
		/////////////////////////////////////////////////////////////
		// previous routine
		/////////////////////////////////////////////////////////////

		// Taken from Bahcall "Neutrino Astrophysics" eqn 8.29 and 8.31
		//
		// radiative corrections are NOT included in this option
		//

		// hacked together from eqn A1 of Bahcall, Kamionkowski and Sirlin
		// astro-ph/9502003

		const double z = Te/Enu;
		const double mq = fMe/Enu;

		const double   gL2 = fgL*fgL;
		const double   gR2 = fgR*fgR;
		const double   gLR = fgL*fgR;
		const double   oneMz = 1.0 - z;

		double dsigdT = sigmaoverme * (gL2 + (gR2*oneMz*oneMz) - gLR*mq*z);


		return dsigdT;
	} //if (fRadiativeCorrection == 1)
	else {

		/////////////////////////////////////////////////////////////
		// radiative correction
		/////////////////////////////////////////////////////////////

		/// \todo Optimize the interpolation.

		// check table
		//     k1 = (int)(Enu/fEnuStepDif) - 1;
		//     k2 = (int)(Te/fEnuStepDif) - 1;
		k1 = (int)((Enu/fEnuStepDif +0.5) - 1);
		k2 = (int)((Te/fEnuStepDif +0.5) - 1);

		if ( k1 < 0 || k2 < 0 ) {
			std::stringstream ss;
			warn << "[ESCrossSec]::dSigmadT : Got invalid values for variables k_i: k1 " << k1
					<< " k2 " << k2 << newline;

			warn << "[ESCrossSec]::dSigmadT : Enu : [ " << Enu << " ], Te  : [ "
					<<  Te << " ] " << " fEnuStepDif : " << fEnuStepDif << "\n\n" << newline;


            Log::Die("[ESCrossSec]::dSigmadT : Got invalid values for variables k_i: k1 " + util_to_string(k1) + " k2 " + util_to_string(k2));
		}

		double e1 = fEnuStepDif * (double) (k1+1);
		double e2 = fEnuStepDif * (double) (k1+2);
		double t1 = fEnuStepDif * (double) (k2+1);
		double t2 = fEnuStepDif * (double) (k2+2);

		double sig_e1_t1 = 0.0;
		double sig_e1_t2 = 0.0;
		double sig_e2_t1 = 0.0;
		double sig_e2_t2 = 0.0;

		double Te1 = Temax * fTableTeMin;
		double Te2 = Temax * fTableTeMax;

		/**
		 * List of relevant variables at this point:
		 * - k1 : Index of Enu part of the table.
		 * - k2 : Index of the Te part of the table.
		 * - e1,e1 : Closest points in Enu in the table.
		 * - t1,t2 : Closest points in Te in the table.
		 * - sig_e1_t1,sig_e2_t2,sig_e2_t1,sig_e1_t2 : \f$ \frac{d\sigma}{dT}\left(e_{i},Te_{j}\right)\f$
		 * - Te1,Te2 : Limits on the recoil energy in the table.
		 */

		// Check indexes against table entries
		if (k1 >= 0 && k1 < fNDataDif-1 && k2 >= 0 && k2 < fNDataDif-1) {

			// Get the closest points of \f$\frac{d\sigma}{dT}\f$
			if ( fReaction == nue || fReaction == numu) {
				sig_e1_t1 = fTableDif[k1 * fNDataDif + k2];
				sig_e1_t2 = fTableDif[k1 * fNDataDif + k2+1];
				sig_e2_t1 = fTableDif[(k1+1) * fNDataDif + k2];
				sig_e2_t2 = fTableDif[(k1+1) * fNDataDif + k2+1];
			} else {
				warn << "[ESCrossSec]::dSigmadT : " << fReactionStr
						<< " not supported with Radiative correction strategy "
						<< fRadiativeCorrection << " !!" << newline;
                Log::Die("[ESCrossSec]::dSigmadT : " + fReactionStr
						+ " not supported with Radiative correction strategy "
						+ util_to_string(fRadiativeCorrection) + " !!");
			}

			// If strategy is 4 (1 has already been put out)
			// If diff. cross-sections are valid
			// If recoil energy is inside table limits.
			if (fRadiativeCorrection != 3 && fRadiativeCorrection != 2 &&
					sig_e1_t1 > 0.0 && sig_e1_t2 > 0.0 &&
					sig_e2_t1 > 0.0 && sig_e2_t2 > 0.0 &&
					(fRadiativeCorrection == 4 || (Te > Te1 && Te < Te2))) {

				// Interpolate from the table.
				double r1 = (sig_e1_t2 - sig_e1_t1) / (t2 -t1)
					* (Te - t1) +  sig_e1_t1;
				double r2 = (sig_e2_t2 - sig_e2_t1) / (t2 -t1)
					* (Te - t1) +  sig_e2_t1;
				return ((r2 - r1) / (e2 - e1) * (Enu - e1) + r1);

			} else {
				// If strategy is not 2 or 3
				// If strategy is 4 but Te is outside interpolation range

				double E = fMe + Te;

				double x = sqrt(1.0 + 2.0 * fMe / Te);

				double z = Te / Enu;
				double el = sqrt(E * E - fMe * fMe);
				double IT = 1.0 / 6.0 * (1.0 / 3.0 + (3.0 - x * x)
						* (0.5 * x * log((x + 1.0) / (x - 1.0)) - 1.0));

				// FIXME : Correct this otherwise the maximum won't be for Te = Temax
				// To avoid log(-1.0e15) in fm,fpz,fpm when Te = Temax
				if (1.0-z-fMe/(E+el) <= 0.0) {
#ifdef RATDEBUG
					debug << "[ESCrossSec]::dSigmadT :  warning: 1.0-z-fMe/(E+el) = "
							<< 1.0-z-fMe/(E+el) << newline;
					debug << "[ESCrossSec]::dSigmadT : Enu = " << Enu << " Te = " <<Te << newline;
#endif
					return 0.0;
				}

				// just use central values for pnc and kappa (correct ?)
				double gl = 0, gr = 0, kappa = 0, pnc = 1.0126;

				if ( fReaction == nue) {
					kappa = 0.9791 + 0.0097 * IT;
					if (fRadiativeCorrection == 2) {
						pnc = kappa = 1.0;
					}
					gl =  pnc * (0.5 - kappa * fsinthetaW2) - 1.0;
					gr = -pnc * kappa * fsinthetaW2;
				} else if (fReaction == numu)    {
					kappa = 0.9970 - 0.00037 * IT;
					if (fRadiativeCorrection == 2) {
						pnc = kappa = 1.0;
					}
					gl =  pnc * (0.5 - kappa * fsinthetaW2);
					gr = -pnc * kappa * fsinthetaW2;

				} else {
					warn << "[ESCrossSec]::dSigmadT : " << fReactionStr
							<< " not supported with Radiative correction strategy "
							<< fRadiativeCorrection << " !!" << newline;
                    Log::Die("[ESCrossSec]::dSigmadT : " + fReactionStr
							+ " not supported with Radiative correction strategy "
							+ util_to_string(fRadiativeCorrection) + " !!");
					// not supported yet
				}

				// turn off radiative correction,
				// if fRadiativeCorrection == 2
				double fm  = 0.0;
				double fpz = 0.0;
				double fpm = 0.0;

				if (fRadiativeCorrection != 2) {
					double  fL(double x);
					fm = (E/el * log((E+el)/fMe)-1.0) *
							(2.0*log(1.0-z-fMe/(E+el))
					- log(1.0-z) - 0.5*log(z) - 5.0/12.0) + 0.5 *
					(fL(z) - fL(el/E)) - 0.5 * log(1.0-z)*log(1.0-z) -
					(11.0/12.0 + 0.5 * z) * log(1.0-z)
					+ z * (log(z) + 0.5 * log(2.0*Enu/fMe))
					- (31.0/18.0 + 1.0/12.0 * log(z))*el/E
					- 11.0/12.0 * z + z*z/24.0;

					fpz = (E/el * log((E+el)/fMe)-1.0) *
							((1.0-z)*(1.0-z) * (2.0*log(1.0-z-fMe/(E+el))
					- log(1.0-z) - 0.5*log(z) - 2.0/3.0) - 0.5 *
					(z*z * log(z) + 1.0 - z))
					- 0.5 * (1.0-z)*(1.0-z) * (log(1.0-z)*log(1.0-z) + el/E *
							(fL(1.0-z) - log(z) * log(1.0-z)))
							+ log(1.0-z) * (z*z/2.0*log(z) + (1.0-z)/3.0*
									(2.0*z-1.0/2.0))
									- 0.5 * z*z * fL(1.0-z) - z*(1.0-2.0*z)/3.0 *
									log(z) - z*(1.0-z)/6.0 - el/E/12.0*(log(z) +
											(1.0-z)*(115.0-109.0*z)/6.0);

					fpm = (E/el * log((E+el)/fMe)-1.0) * 2.0 *
							log(1.0-z-fMe/(E+el));
				}

				double dsigma_dT = 2.0 * fGf*fGf * fMe / pi * (
						gl * gl * (1.0 + falpha / pi * fm)
						+ gr * gr * ((1.0 - z) * (1.0 - z) + falpha / pi * fpz)
						- gr * gl * fMe / Enu * z * (1.0 + falpha / pi * fpm));

				return   dsigma_dT * fhbarc2 * 1.0e9;
			}
		} else {
			warn << "[ESCrossSec]::dSigmadT : Got invalid values for variables k_i: k1 " << k1
					<< " k2 " << k2 << newline;
            Log::Die("[ESCrossSec]::dSigmadT : Got invalid values for variables k_i: k1 " + util_to_string(k1) + " k2 " + util_to_string(k2));
		}

	}

	warn << "[ESCrossSec]::dSigmadT > Reached end of function while calculating dSigmadT. Something is wrong with the calculation." << newline;
	warn << "[ESCrossSec]::dSigmadT > Current parameters : Calculation Strategy : [ "
			<< fRadiativeCorrection << " ], Reaction : [ " << fReactionStr << " ] " << newline;
	warn << "[ESCrossSec]::dSigmadT > Enu : [ " << Enu << " ], Te  : [ "
			<<  Te << " ], TeMax  : [ "  <<  Temax << " ] " << newline;
	warn << "[ESCrossSec]::dSigmadT > k1 : " << k1 << " k2 : " << k2 << " fEnuStepDif : " << fEnuStepDif << newline;
    Log::Die("Failed calculation of dSigmadT.");
	return 0.0;

}

double ESCrossSec::IntegraldSigmadT(const double Enu,const double T1,const double T2) const
{
	// Integrate dSigma/dT of Enu from T1 to T2


	int i;
	const int  nbins = 100;
	double integral;
	double x1 = T1;
	double x2 = T2;
	double T,dsig;
	double Tstep = (x2-x1)/(double)nbins;


	TH1F *h = new TH1F("h1","h1",nbins,x1,x2);
	for(i=0, T=x1;i<nbins;i++,T+= Tstep){
		dsig = dSigmadT(Enu,T);
		h->Fill(T + (Tstep/2.0),dsig * Tstep);
	}
	integral = h->Integral();
	delete h;

	return(integral);
}


//--------------------------------------


double ESCrossSec::dSigmadCosTh(const double Enu,const double CosTh) const
{
	// Differential cross section
	//  d Sigma
	//  -------
	//  d CosTh
	//
	//  Enu = laboratory neutrino energy        -- units: MeV
	//  CosTh = laboratory recoil electron direction cosine
	//  dSigmadT units:  10^-42 cm^2/MeV
	//
	// Taken from Bahcall "Neutrino Astrophysics" eqn 8.40
	//
	// modified: M. Chen 14 March 2001
	// correcting John Bahcall's bad Te in units of me formulae




	const double mu = CosTh;
	const double mu2 = mu*mu;
	const double Enu2 = Enu*Enu;
	const double meEnu = fMe + Enu;

	// first see if scattering angle is greater than 90 degrees
	// (not allowed)
	if(mu < 0.0){
		return 0.0;
	}



	double Denom = meEnu*meEnu - Enu2*mu2;

	double T  = (2.0 * Enu2 * mu2 * fMe) / Denom;

	double dsigdT = dSigmadT(Enu,T);

	return dsigdT *fMe*4.0*meEnu*meEnu*Enu2*mu/(Denom*Denom);
}



//---------------- private routine ----------------------------

// fL(): add by y.t. 14-JAN-2003
double fL(const double x)
{
	int istep = 1000;    // should not be hard corded??

	double dstep = x / (double) istep;
	double t;
	double sum = 0.0;

	for (int i = 1; i <= istep; i++) {
		t = dstep * (double) i;
		sum = sum + log(fabs(1.0 - t))/t * dstep;
	}

	return sum;
}


//--------------------------------------


void ESCrossSec::SetRadiativeCorrection(const int ii) {
	fRadiativeCorrection = ii;

	// if any of the table options are set we need to load the table
	if (fRadiativeCorrection != 1) LoadTablesDB();

}

//--------------------------------------

void ESCrossSec::LoadTablesDB()
{
	// Get two different links (total and differential)
	// depending on the reaction
	// load the total cross section table
	std::string tblname = "pnue_tot_";
	tblname += fReactionStr;

	DBLinkPtr linkdb = DB::Get()->GetLink("ESXS",tblname.c_str());
	fNDataTot = 0;
	fEnuStepTot = 0.0;

	fNDataTot   = linkdb->GetI("NData");
	fEnuStepTot = linkdb->GetD("EStep");
	fTableTot_e.Set(linkdb->GetDArray("data_enu"),linkdb->GetDArray("data_xsec"));

	// now load the differential cross section table
	// This is a "square" table. It contains as many entries in Enu as Te
	// It increases first in Enu and then in T
	tblname = "pnue_dif_";
	tblname += fReactionStr;
	linkdb = DB::Get()->GetLink("ESXS",tblname.c_str());
	fNDataDif = linkdb->GetI("NData");
	fEnuStepDif = linkdb->GetD("EStep");

	fTableDif = linkdb->GetDArray("data_xsec");

}

//_________________________________________________________________________
void ESCrossSec::SetReaction(const std::string &rstr)
{

	// Set the reaction type
	// valid types are:  nue     --  nu_e + e       -> nu_e + e
	//                   nuebar  --  anti nu_e + e  -> anti nu_e + e
	//                   numu    --  nu_mu + e      -> nu_mu + e
	//                   numubar --  anti nu_mu + e -> anti nu_mu + e

	if (rstr == std::string("nue")) {
		fReactionStr = rstr;
		fReaction = nue;
	} else if (rstr == std::string("numu")) {
		fReactionStr = rstr;
		fReaction = numu;
	} else if (rstr == std::string("numubar")) {
		fReactionStr = rstr;
		fReaction = numubar;
	} else if (rstr == std::string("nuebar")) {
		fReactionStr = rstr;
		fReaction = nuebar;
	} else {
		warn << "ESCrossSec::SetReaction > Unknown reaction [ "
				<< rstr << " ]." << newline;
		throw std::invalid_argument("Unknown reaction " + rstr);
	}

	// Now that the reaction is set perform
	// the calculation of the gL and gR constants
	// These only depend on the weak mixing angle
	// and the specific reaction type
	CalcG();
	//LoadTablesDB();

}

//________________________________________________________________________________
void ESCrossSec::SetSinThetaW(const double &sintw) {
	fsinthetaW2 = sintw;
	// New calculation.
	// Force recalculation of gL and GR
	CalcG();
}

TGraph *ESCrossSec::DrawdSigmadT(const double Enu) const {
#ifdef RATDEBUG
	debug << "[ESCrossSec]::DrawdSigmadT : Sampling Enu=" << Enu << " MeV." << newline;
#endif

	TGraph *g = new TGraph();
	const int npoints = 1024;
	const double emin = 0.0, emax = Enu;
	const double xwid = emax - emin;
	const double xstep = xwid/(double)npoints;
#ifdef RATDEBUG
	debug << "[ESCrossSec]::DrawdSigmadT : Sampling data: emin=" << emin << " MeV."
			<< " emax=" << emax << " MeV xwid=" << xwid << " MeV  xstep=" << xstep
			<< newline;
#endif

	double Te,dsigma;
	for (int ip = 0; ip < npoints; ++ip) {
		Te = emin + (double)ip*xstep;
		dsigma = dSigmadT(Enu,Te);
		g->SetPoint(ip,Te,dsigma);
	}
	//g->Print();
	return g;
}
} // -- namespace RAT
