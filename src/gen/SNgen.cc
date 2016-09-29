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

#include <RAT/SNgen.hh>
#include <RAT/SNgenMessenger.hh>
#include <RAT/DB.hh>

#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <cmath>

TGraph *graphIBD;
TGraph *graphES;
TGraph *graphCC;
TGraph *graphICC;
TGraph *graphNC;
TGraph *graphINC;
TGraph *graphNCRate;

namespace RAT {
    
    // WGS: Constants copied from various places to make the code work.
    const double XMaxDefault  = 1e-45; // Reasonable minimum for x-section (cm^2).
    const double GFERMI = 1.16639e-11 / CLHEP::MeV / CLHEP::MeV;
    const double XcMeVtoCmsqrd = 0.389379292e-21;
    
    // WGS: We have to start from some value of sin2theta; use the stanard-model value:
    const double SNgen::WEAKANGLE = 0.2227;
    const int    SNgen::NTRIAL    = 10000;
    
    const double SNgen::IBDDEFAULT = 0.926;
    const double SNgen::ESDEFAULT = 0.0295;
    const double SNgen::CCDEFAULT = 0.003;
    const double SNgen::ICCDEFAULT = 0.024;
    const double SNgen::NCDEFAULT = 0.0175;
    const int SNgen::MODELDEFAULT = 1;//1: livermore, 2: gkvm
    
    SNgen::SNgen()
    {
        // Initialize everything.
        Reset();
        
        // Create a messenger to allow the user to change some ES parameters.
        messenger = new SNgenMessenger(this);
        
        // Get parameters from database. Note that we get the flux from the
        // IBD values; we assume that the flux for ES is the same as the
        // flux for IBD.
        //        DBLinkPtr libd = DB::Get()->GetLink("SN_SPECTRUM");
        //
        //        Emin = libd->GetD("emin");
        //        Emax = libd->GetD("emax");
        // Flux function
        //        rmpflux.Set(libd->GetDArray("spec_e"), libd->GetDArray("spec_flux"));
        
        // Other useful numbers
        //        FluxMax = rmpflux(Emin);
        
        // Get the electron mass.
        //        G4ParticleDefinition* electron = G4ParticleTable::GetParticleTable()->FindParticle("e-");
        //        massElectron = electron->GetPDGMass();
    }
    
    
    SNgen::~SNgen()
    {
        // I compulsively delete unused pointers.
        if ( messenger != 0 )
        {
            delete messenger;
            messenger = 0;
        }
    }
    
    
    CLHEP::HepLorentzVector SNgen::GenerateEvent(const G4ThreeVector& theNeutrino)
    {
        //
        //  Check if the maximum throwing number has been set.
        //
        if (!GetNormFlag()) SetXSecMax(NTRIAL);
        double XSecNorm = GetXSecMax();
        
        // Throw values against a cross-section.
        bool passed=false;
        double E, Nu;
        
        while(!passed){
            // Pick a random E and Nu.
            E = GetRandomNumber(Emin, Emax);
            Nu = GetRandomNumber(0., E);
            
            // Decided whether to draw again based on relative cross-section.
            float XCtest = XSecNorm * FluxMax * GetRandomNumber(0.,1.);
            double XCWeight = GetXSec(E, Nu);
            double FluxWeight = rmpflux(E);
            passed = XCWeight * FluxWeight > XCtest;
        }
        
        //
        //  Calculate the neutrino pT and incoming angle
        //
        G4double pT = E * theNeutrino.perp()/theNeutrino.mag();
        G4double pTq = asin(pT/E);
        
        //
        //  Set kinematic variables from kinetic energy
        //
        G4double X = 1.;
        G4double Y = Nu / E;
        G4double Q2  = 2. * E * X * Y * massElectron;
        G4double sin2q = Q2 / (4. * E * (E - Nu));
        G4double theta_lab = 2. * asin(sqrt(sin2q)) + pTq;
        G4double phi = GetRandomNumber(0., 2.*M_PI);
        
        //
        //  Now that energy/Q2 is selected, write a momentum transfer 4-vector q.
        //
        G4double qp = sqrt(pow(Nu,2) + Q2);
        CLHEP::HepLorentzVector qVector;
        qVector.setPx(qp * sin(theta_lab) * cos(phi));
        qVector.setPy(qp * sin(theta_lab) * sin(phi));
        qVector.setPz(qp * cos(theta_lab));
        qVector.setE(Nu);
        
        //
        // Let pe_new = pe - (pv - pv_new) == pe - q;
        //
        CLHEP::HepLorentzVector theElectron;
        theElectron.setE(massElectron);
        theElectron += qVector;
        
        return theElectron;
    }
    
    void SNgen::Reset()
    {
        XSecMax = 0.0;
        SetNormFlag(false);
        SetMixingAngle(WEAKANGLE);
        SetNeutrinoMoment(0.0);
        
        SetIBDAmplitude (IBDDEFAULT);
        SetESAmplitude (ESDEFAULT);
        SetCCAmplitude (CCDEFAULT);
        SetICCAmplitude (ICCDEFAULT);
        SetNCAmplitude (NCDEFAULT);
        SetModel (MODELDEFAULT);
    }
    
    void SNgen::SetIBDAmplitude (double IBDAm)
    {
        if ((IBDAm < 0.) || (IBDAm > 1.))
        {
            G4cerr << "Set your IBD Amplitude between 0 and 1." << G4endl;
            return;
        }
        IBDAmp = IBDAm;
    }
    
    
    void SNgen::SetESAmplitude (double ESAm)
    {
        if ((ESAm < 0.) || (ESAm > 1.))
        {
            G4cerr << "Set your ES Amplitude between 0 and 1." << G4endl;
            return;
        }
        ESAmp = ESAm;
    }
    
    void SNgen::SetCCAmplitude (double CCAm)
    {
        if ((CCAm < 0.) || (CCAm > 1.))
        {
            G4cerr << "Set your CC Amplitude between 0 and 1." << G4endl;
            return;
        }
        CCAmp = CCAm;
    }
    
    void SNgen::SetICCAmplitude (double ICCAm)
    {
        if ((ICCAm < 0.) || (ICCAm > 1.))
        {
            G4cerr << "Set your ICC Amplitude between 0 and 1." << G4endl;
            return;
        }
        ICCAmp = ICCAm;
    }
    
    void SNgen::SetNCAmplitude (double NCAm)
    {
        if ((NCAm < 0.) || (NCAm > 1.))
        {
            G4cerr << "Set your NC Amplitude between 0 and 1." << G4endl;
            return;
        }
        NCAmp = NCAm;
    }
    
    void SNgen::SetModel (double ModelTm)
    {
        if ((ModelTm < 1.) || (ModelTm > 2.))
        {
            G4cerr << "Set your model between 1 (livermore) and 2. (gvkm)" << G4endl;
            return;
        }
        ModelTmp = ModelTm;
    }
    
    
    
    void SNgen::Show()
    {
        G4cout << "Elastic Scatteing Settings:" << G4endl;
        G4cout << "Weak Mixing Angle (sinsq(ThetaW)):" << GetMixingAngle() << G4endl;
        G4cout << "Neutrino Magnetic Moment: " << GetMagneticMoment() << G4endl;
    }
    
    void SNgen::SetMixingAngle(double sin2thw)
    {
        if ((sin2thw < 0.) || (sin2thw > 1.))
        {
            G4cerr << "Error in value setting." << G4endl;
            return;
        }
        SinSqThetaW = sin2thw;
    }
    
    void SNgen::SetNeutrinoMoment(double vMu)
    {
        if (vMu < 0.)
        {
            G4cerr << "Error in value setting." << G4endl;
            return;
        }
        MagneticMoment = vMu;
    }
    
    double SNgen::GetXSec(double Enu, double T){
        
        double XC = 0.;
        
        //  Set up constants for cross-section scale.
        double XCunits = XcMeVtoCmsqrd;
        double Sigma0 = GFERMI * GFERMI * massElectron / (2. * M_PI);
        
        //  Set up weak mixing parameters & neutrino magnetic moment.
        double sin2thw = GetMixingAngle();
        double vMu = GetMagneticMoment();
        double g_v = 2.*sin2thw + 0.5;
        double g_a = -0.5;
        
        //  Reject events in prohibited regions.
        if (T > Enu) return 0.;
        
        //  Compute differential cross-section
        XC = pow((g_v + g_a),2)
        +pow((g_v - g_a),2)*pow((1.-T/Enu),2)
        +(pow(g_a,2) - pow(g_v,2))*(massElectron * T / pow(Enu,2));
        
        XC *= Sigma0 ;
        
        //  Add term due to neutrino magnetic moment.
        static const double alphainv = 1./CLHEP::fine_structure_const;
        if (T > 0.) XC += (M_PI / pow(alphainv,2) * pow(vMu,2) / pow(massElectron,2)) * (1. - T/Enu)/T;
        
        //  Convert to detector units and return.
        XC *= XCunits;
        
        return XC;
    }
    
    void SNgen::SetXSecMax(int ntry)
    {
        double xMax = XMaxDefault;
        
        for(int i = 0; i < ntry+1; i++)
        {
            double Enu = GetRandomNumber(0.,10.);
            double T = GetRandomNumber(0.,Enu);
            double xsec = GetXSec(Enu,T);
            if (xsec > xMax) xMax = xsec;
        }
        
        XSecMax = xMax;
        SetNormFlag(true);
    }
    
    double SNgen::GetRandomNumber(double rmin, double rmax)
    {
        double rnd = G4UniformRand(); // random number from 0 to 1.
        double value = rmin + (rmax - rmin) * rnd;
        return value;
    }
    
    double IBDTGraph2TF1(Double_t *x, Double_t *){ return graphIBD->Eval(x[0]);}
    double ESTGraph2TF1(Double_t *x, Double_t *){ return graphES->Eval(x[0]);}
    double CCTGraph2TF1(Double_t *x, Double_t *){ return graphCC->Eval(x[0]);}
    double ICCTGraph2TF1(Double_t *x, Double_t *){ return graphICC->Eval(x[0]);}
    double NCTGraph2TF1(Double_t *x, Double_t *){ return graphNC->Eval(x[0]);}
    double INCTGraph2TF1(Double_t *x, Double_t *){ return graphINC->Eval(x[0]);}
    
    double SNgen::GetIBDRandomEnergy(){
        return funcIBD->GetRandom();
    }
    double SNgen::GetESRandomEnergy(){
        return funcES->GetRandom();
    }
    double SNgen::GetCCRandomEnergy(){
        return funcCC->GetRandom();
    }
    double SNgen::GetICCRandomEnergy(){
        return funcICC->GetRandom();
    }
    double SNgen::GetNCRandomEnergy(){
        return funcNC->GetRandom();
    }
    double SNgen::GetINCRandomEnergy(){
        return funcINC->GetRandom();
    }
    
    void SNgen::LoadSpectra(){
        int model = GetModel();
        Double_t magsumTot,totIBD,totES,totCC,totICC,totNC,x,y;
        
        G4cout << "\n===================== Supernova information ======================" << G4endl;
        
        //Load in the Livermore model
        if (model == 1){
            G4cout << "\nUsing the livermore model. Within the this model the following rates are present.\nThese rates are not used in the processing of these events, but may be set by the user\nmanualy " << G4endl;
            
            
            //////////////////////////// IBD //////////////////////////////////////////////////////
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_ibd");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphIBD = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphIBD->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            G4cout << "Total IBD integrate spectra is  " << magsumTot<<G4endl;
            totIBD = magsumTot;
            
            
            //////////////////////////// CC //////////////////////////////////////////////////////
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nue_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphCC = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphCC->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            G4cout << "Total CC integrate spectra is  " << magsumTot<<G4endl;
            totCC = magsumTot;
            
            //////////////////////////// ICC //////////////////////////////////////////////////////
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nuebar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphICC = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphICC->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            G4cout << "Total ICC integrate spectra is  " << magsumTot<<G4endl;
            totICC = magsumTot;
            
            
            //////////////////////////// NC //////////////////////////////////////////////////////
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nc_nue_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            graphNCRate = new TGraph();
            
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            //            G4cout << "Total NC integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nc_numu_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nc_nutau_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nc_nuebar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nc_numubar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nc_nutaubar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            //////////////////////////// ES //////////////////////////////////////////////////////
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nue_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphES = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]);
                G4cout << "ES nue " << spec_E[istep]  << " " <<spec_mag[istep] << G4endl;
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nuebar_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
                G4cout << "ES nuebar x:(" << spec_E[istep]  << ", " <<x << "), E:("<< spec_mag[istep] << " + " << y <<" = " << spec_mag[istep]+y << ")" <<G4endl;
                
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_numu_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
                
                G4cout << "ES numu x:(" << spec_E[istep]  << ", " <<x << "), E:("<< spec_mag[istep] << " + " << y <<" = " << spec_mag[istep]+y << ")" <<G4endl;
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_numubar_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
                G4cout << "ES numubar x:(" << spec_E[istep]  << ", " <<x << "), E:("<< spec_mag[istep] << " + " << y <<" = " << spec_mag[istep]+y << ")" <<G4endl;
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nutau_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
                G4cout << "ES nutau x:(" << spec_E[istep]  << ", " <<x << "), E:("<< spec_mag[istep] << " + " << y <<" = " << spec_mag[istep]+y << ")" <<G4endl;
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "livermore_nutaubar_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
                G4cout << "ES nutaubar x:(" << spec_E[istep]  << ", " <<x << "), E:("<< spec_mag[istep] << " + " << y <<" = " << spec_mag[istep]+y << ")" <<G4endl;
            }
            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            //////////////////////////// TALLY //////////////////////////////////////////////////////
            
            
            
            Double_t tot = totIBD+totES+totCC+totICC+totNC;
            G4cout << "(ibd,es,cc,icc,nc): " << "("<< totIBD/tot <<", "<< totES/tot <<", "<< totCC/tot <<", "<< totICC/tot <<", "<< totNC/tot <<")\n" <<G4endl;
        }// GVKM MODELgvkm
        else if (model ==2){
            G4cout << "\nUsing the gvkm model. Within the this model the following rates are present.\nThese rates are not used in the processing of these events, but may be set by the user\nmanualy  " << G4endl;
            
            
            //////////////////////////// IBD //////////////////////////////////////////////////////
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_ibd");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphIBD = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphIBD->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            G4cout << "Total IBD integrate spectra is  " << magsumTot<<G4endl;
            totIBD = magsumTot;
            
            
            //////////////////////////// CC //////////////////////////////////////////////////////
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nue_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphCC = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphCC->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            G4cout << "Total CC integrate spectra is  " << magsumTot<<G4endl;
            totCC = magsumTot;
            
            //////////////////////////// ICC //////////////////////////////////////////////////////
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nuebar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphICC = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphICC->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            G4cout << "Total ICC integrate spectra is  " << magsumTot<<G4endl;
            totICC = magsumTot;
            
            
            //////////////////////////// NC //////////////////////////////////////////////////////
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nc_nue_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            graphNCRate = new TGraph();
            
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            //            G4cout << "Total NC integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nc_numu_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nc_nutau_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nc_nuebar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nc_numubar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nc_nutaubar_O16");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphNCRate->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphNCRate->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totNC = magsumTot;
            
            //////////////////////////// ES //////////////////////////////////////////////////////
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nue_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            graphES = new TGraph();
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                magsumTot+=(spec_mag[istep]);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nuebar_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_numu_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_numubar_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nutau_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            //            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            
            
            _lspec = DB::Get()->GetLink("SN_SPECTRUM", "gvkm_nutaubar_e");
            // Flux function
            spec_E.clear();
            spec_mag.clear();
            spec_E   = _lspec->GetDArray("spec_e");
            spec_mag = _lspec->GetDArray("spec_mag");
            magsumTot = 0.0;
            for(unsigned int istep = 0; istep<spec_E.size(); ++istep){
                graphES->GetPoint(istep,x,y);
                magsumTot+=(spec_mag[istep]+y);
                graphES->SetPoint(istep,spec_E[istep],spec_mag[istep]+y);
            }
            G4cout << "Total ES integrate spectra is  " << magsumTot<<G4endl;
            totES = magsumTot;
            
            //////////////////////////// TALLY //////////////////////////////////////////////////////
            
            
            
            Double_t tot = totIBD+totES+totCC+totICC+totNC;
            G4cout << "(ibd,es,cc,icc,nc): " << "("<< totIBD/tot <<", "<< totES/tot <<", "<< totCC/tot <<", "<< totICC/tot <<", "<< totNC/tot <<")\n" <<G4endl;
            
        }
        
        //Neutral current event get a special treatment.
        
        graphNC= new TGraph();
        graphNC->SetPoint(0,0,0.0);
        graphNC->SetPoint(1,5.25,0.0);
        graphNC->SetPoint(2,5.30,0.73);
        graphNC->SetPoint(3,5.35,0.0);
        graphNC->SetPoint(4,6.28,0);
        graphNC->SetPoint(5,6.33,0.84);
        graphNC->SetPoint(6,6.38,0.0);
        graphNC->SetPoint(7,7.18,0);
        graphNC->SetPoint(8,7.23,0.29);
        graphNC->SetPoint(9,7.28,0.0);
        graphNC->SetPoint(10,7.51,0);
        graphNC->SetPoint(11,7.56,0.05);
        graphNC->SetPoint(12,7.61,0.0);
        graphNC->SetPoint(13,8.27,0);
        graphNC->SetPoint(14,8.32,0.07);
        graphNC->SetPoint(15,8.37,0.0);
        graphNC->SetPoint(16,8.52,0);
        graphNC->SetPoint(17,8.57,0.05);
        graphNC->SetPoint(18,8.62,0.0);
        graphNC->SetPoint(19,9.11,0);
        graphNC->SetPoint(20,9.16,0.31);
        graphNC->SetPoint(21,9.21,0.0);
        graphNC->SetPoint(22,9.78,0);
        graphNC->SetPoint(23,9.83,0.14);
        graphNC->SetPoint(24,9.88,0.0);
        graphNC->SetPoint(25,10.0,0.0);
        
        
        graphINC= new TGraph();
        graphINC->SetPoint(0,0,0.0);
        graphINC->SetPoint(1,5.20,0.0);
        graphINC->SetPoint(2,5.25,  0.28);
        graphINC->SetPoint(3,5.30,0.0);
        graphINC->SetPoint(4,6.13,0);
        graphINC->SetPoint(5,6.18,  0.21);
        graphINC->SetPoint(6,6.23,0.0);
        graphINC->SetPoint(7,6.71,0);
        graphINC->SetPoint(8,6.76,  0.14);
        graphINC->SetPoint(9,6.81,0.0);
        graphINC->SetPoint(10,7.23,0);
        graphINC->SetPoint(11,7.28,  0.02);
        graphINC->SetPoint(12,7.31,0.0);
        graphINC->SetPoint(13,10.0,0.0);
        
        
        
        funcIBD = new TF1("funcIBD",IBDTGraph2TF1,2,100,0);
        funcES= new TF1("funcES",ESTGraph2TF1,2,100,0);
        funcCC= new TF1("funcCC",CCTGraph2TF1,2,100,0);
        funcICC= new TF1("funcICC",ICCTGraph2TF1,2,100,0);
        funcNC= new TF1("funcNC",NCTGraph2TF1,0,10,0);
        funcINC= new TF1("funcINC",INCTGraph2TF1,0,10,0);
        
        
        
    }
    
    
} // namespace RAT
