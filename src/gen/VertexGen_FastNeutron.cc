#include <RAT/VertexGen_FastNeutron.hh>
#include <RAT/FastNeutronMessenger.hh>
#include <RAT/Log.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <G4Event.hh>
#include <G4ParticleTable.hh>
#include <G4IonTable.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4ThreeVector.hh>

#include "TF1.h"
#include "TF2.h"
#include "TGraph.h"

#include <globals.hh>
#include <G4ParticleDefinition.hh>

#include <G4LorentzVector.hh>
#include <RAT/GLG4PosGen.hh>
#include <sstream>
#include <RAT/GLG4StringUtil.hh>

//Allows the transformation of a TGraph to a TF1
TGraph *graphTMP;
double TGraph2TF1(Double_t *x, Double_t *){ return graphTMP->Eval(x[0]);}


#define G4std  std
namespace RAT {
    
    
    
    VertexGen_FastNeutron::VertexGen_FastNeutron(const char *arg_dbname)
    : GLG4VertexGen(arg_dbname), nu_dir(0.,0.,0.)
    {
        n = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
        
        SetDepth(400.);
        SetEnThreshold(10.);
        SetSideBool(1);
        
        messenger = new FastNeutronMessenger(this);
        Double_t Depth = GetDepth();
        LoadTangHortonSmithCosTheta(Depth);

        
    }
    
    ///-------------------------------------------------------------------------
    VertexGen_FastNeutron::~VertexGen_FastNeutron()
    {
        // destructor
    }
    
    ///-------------------------------------------------------------------------
    void VertexGen_FastNeutron::GeneratePrimaryVertex(G4Event *event,
                                                      G4ThreeVector &dx,
                                                      G4double dt)
    {
        // Where the main work is done - the astute may notice a strong similarity to the Gun generator!
        

        //Find the cosmic muon direction
        
        Double_t muCosTheta = GetRandomMuonCosTheta();
        G4double phi = 2.0 * G4UniformRand() * CLHEP::pi;
        
        G4ThreeVector ev_mu_dir(nu_dir); // By default use specified direction
        ev_mu_dir.setRThetaPhi(1.0, acos(-muCosTheta), phi);//Changed such that nu_dir is changed, not e_nu_dir
        
        //Find the scattering angle and neutron energy
        Double_t D = GetDepth();
        Double_t En_min = GetEnThreshold();
        Double_t cosTheta,neutronEnergy;
        GetMeiHimeParameters(D,En_min,cosTheta,neutronEnergy); // Load the cosTheta and neutronEnergy
        G4double phi2 = 2.0 * G4UniformRand() * CLHEP::pi;
        G4ThreeVector ev_n_dir(nu_dir); // By default use specified direction
        ev_n_dir.setRThetaPhi(1.0, acos(-cosTheta), phi2);//Changed such that nu_dir is changed, not e_nu_dir
        
        
        //Rotate the neutron vector
//        G4ThreeVector neutronDirection = -dx.unit();
        // Construct electron vector by rotating the neutrino vector
        G4ThreeVector rotation_axis = ev_mu_dir.orthogonal();
        rotation_axis.rotate(phi2, ev_mu_dir);
        G4ThreeVector neutronDirection = ev_mu_dir.rotate(acos(cosTheta), rotation_axis);
        
        
        //Find the kinetic information of the neutron
        double massN = n->GetPDGMass();
        double totNeutronMomentum = sqrt(pow(neutronEnergy+massN,2)-pow(massN,2));
        G4ThreeVector momentum = neutronDirection*totNeutronMomentum;
//        G4cout << "( " << muCosTheta << ", "<< phi << ") (" << cosTheta << ", " << phi2 <<") " << ev_mu_dir << " " << ev_n_dir << " " << neutronDirection<< "\n";

        // Generate the geant4 neutron_particle vertex
        G4PrimaryVertex* vertex= new G4PrimaryVertex( dx, dt );
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(n,                  // particle code
                              momentum.x() ,         // x component of momentum
                              momentum.y(),         // y component of momentum
                              momentum.z());        // z component of momentum
        n_particle->SetMass(n->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        event->AddPrimaryVertex(vertex);
    }
    
    ///-------------------------------------------------------------------------
    void VertexGen_FastNeutron::SetState(G4String newValues)
    {
        newValues = util_strip_default(newValues); // from GLG4StringUtil
        if (newValues.length() == 0) {
            // print help and current state
            G4cout << "Current state of this VertexGen_ES:\n"
            << " \"" << GetState() << "\"\n" << G4endl;
            G4cout << "Format of argument to VertexGen_ES::SetState: \n"
            " \"nu_dir_x nu_dir_y nu_dir_z\"\n"
            " where nu_dir is the initial direction of the reactor antineutrino.\n"
            " Does not have to be normalized.  Set to \"0. 0. 0.\" for isotropic\n"
            " neutrino direction."
            << G4endl;
            return;
        }
        
        std::istringstream is(newValues.c_str());
        double x, y, z;
        is >> x >> y >> z;
        if (is.fail())
            return;
        
        if ( x == 0. && y == 0. && z == 0. )
            nu_dir.set(0., 0., 0.);
        else
            nu_dir = G4ThreeVector(x, y, z).unit();
    }
    
    ///-------------------------------------------------------------------------
    G4String VertexGen_FastNeutron::GetState()
    {
        std::ostringstream os;
        
        os << nu_dir.x() << "\t" << nu_dir.y() << "\t" << nu_dir.z() << std::ends;
        
        G4String rv(os.str());
        return rv;
    }
    
    
    ///-------------------------------------------------------------------------
    
    
    
    void VertexGen_FastNeutron::SetDepth (double _tmpVal)
    {
        if ((_tmpVal < 0.) || (_tmpVal > 8000.))
        {
            G4cerr << "Set your depth between 0 and 8000." << G4endl;
            return;
        }
        valueD = _tmpVal;
    }
    
    void VertexGen_FastNeutron::SetEnThreshold (double _tmpVal)
    {
        if ((_tmpVal < 1.) || (_tmpVal > 1000.))
        {
            G4cerr << "Set your energy threshold between 1 and 1000." << G4endl;
            return;
        }
        valueE = _tmpVal;
    }
    
    
    void VertexGen_FastNeutron::SetSideBool (double _tmpVal)
    {
        if ((_tmpVal <0) || (_tmpVal > 1))
        {
            G4cerr << "Set your wall to either at 0 or 1" << G4endl;
            return;
        }
        valueSB = _tmpVal;
    }
    
    
    void VertexGen_FastNeutron::GetMeiHimeParameters(double depth, double emin, double &cosTheta,double &energy){
        //Muon selection adapted from Caleb Roeker's code
        
        double b = 0.4;// parameter in km.w.e.^-1
        double h = depth/1000.;// depth in km.w.e.
        double gamma = 3.77;// dimensionless parameter
        double epsilon = 693.;// parameter in GeV
        // The average muon energy at the specified depth for flat overburden
        double muonEnergy = (epsilon*(1-exp(-b*h)))/(gamma - 2.);
        // The B(E_u) parameter from Wang
        double B = 0.52 - 0.58*exp(-0.0099*muonEnergy);
        
        // The Mei and Hime or Wang distribution converted to MeV
        TF1 *MeiHimeDistribution = new TF1("MeiHimeDistribution",
                                           "[1]*(exp(-7.*x/(1000.))/(x/(1000.)) + [0]*exp(-2.*x/(1000.)))",
                                           emin,1000.0);
        
        MeiHimeDistribution->SetNpx(10000);
        MeiHimeDistribution->SetParameter(0,B);
        MeiHimeDistribution->SetParameter(1,1.);
        
        energy = MeiHimeDistribution->GetRandom();
        
        delete MeiHimeDistribution;
        
        TF1 *MeiHimeCosDistribution = new TF1("MeiHimeCosDistribution",
                                              "[0]/(pow((1-x),[1])+[2])",
                                              -1,1);
        
        double B0 = 0.482*pow(muonEnergy,0.045);
        double C0 = 0.832*pow(muonEnergy,-0.152);
        
        MeiHimeCosDistribution->SetNpx(10000);
        MeiHimeCosDistribution->SetParameter(0,1);
        MeiHimeCosDistribution->SetParameter(1,B0);
        MeiHimeCosDistribution->SetParameter(2,C0);
        
        
        cosTheta = MeiHimeCosDistribution->GetRandom();
        
        MeiHimeCosDistribution->Delete();
        
        //        G4cout << energy << " " << cosTheta << " " << B0 << " " << C0 << G4endl;
        
        //        return energy;
        
    }
    void VertexGen_FastNeutron::LoadTangHortonSmithCosTheta(double depth){
        
        // Equation (10) of Tang, Horton-Smith PhysRevD.74.053007
        TF1 *cosThetaPrime = new TF1("cosThetaPrime","sqrt((x**2 +[0]**2 + [1]*x**[2] + [3]*x**[4])/(1.0+[0]**2+[1]+[3]))",0,1);
        cosThetaPrime->SetParameter(0,0.102573);
        cosThetaPrime->SetParameter(1,-0.068287);
        cosThetaPrime->SetParameter(2,0.958633);
        cosThetaPrime->SetParameter(3,0.0407253);
        cosThetaPrime->SetParameter(4,0.817285);
        
        // Equation (3) from Tang, Horton-Smith PhysRevD.74.053007
        TF2 *func1  = new TF2("func1", "0.14 * 10**y* (10**y*(1.0+3.64/(10**y*cosThetaPrime**1.29)))**(-2.7) * (1./(1. + 1.1*10**y*cosThetaPrime/115.0) + 0.054/(1.0+1.1*10**y*cosThetaPrime/850.0))", 0.0001, 1.0,-3 ,6);
        
        //Definition of the function that evaluates the distance to earth's crust as function of cosTheta angles
        TF1 *fL = new TF1("fL","sqrt(([0]+[1])**2-([0]-[2])**2 * sin(acos(x))**2)- ([0]-[2])*x",-1.,1.);
        
        Double_t density = 2.65; //Density of rock material used by range equation below
        depth = depth/density/1000.;
        fL->SetParameter(0,6378.0);//#Radius of earth
        fL->SetParameter(1,0.0);//#Height with respect to ocean of muon
        fL->SetParameter(2,depth);// # WATCHBOY
        
        // Energy bins
        Double_t energyCSDA[43] = {10.0e-3, 14.0e-3, 20.0e-3, 30.0e-3, 40.0e-3, 80.0e-3,100.e-3, 140.e-3, 200.e-3, 300.e-3, 400.e-3, 800.e-3,1.00, 1.40, 2.00, 3.00, 4.00, 8.00,10.0, 14.0, 20.0, 30.0, 40.0, 80.0,100., 1, 200., 300., 400., 800.,1.00e3, 1.40e3, 2.00e3, 3.00e3, 4.00e3, 8.00e3,10.0e3, 14.0e3, 20.0e3, 30.0e3, 40.0e3, 80.0e3,100.e3};
        
        //Range in rock of density 2.65
        Double_t rangeCSDA[43] = {8.516e-1,1.542e0,2.866e0,5.698e0,9.145e0,2.676e1,3.696e1,5.879e1,9.332e1,1.524e2,2.115e2,4.418e2,5.534e2,7.712e2,1.088e3,1.599e3,2.095e3,3.998e3,4.920e3,6.724e3,9.360e3,1.362e4,1.776e4,3.343e4,4.084e4,5.495e4,7.459e4,1.040e5,1.302e5,2.129e5,2.453e5,2.990e5,3.616e5,4.384e5,4.957e5,6.400e5,6.877e5,7.603e5,8.379e5,9.264e5,9.894e5,1.141e6,1.189e6};
        
        //Definition of the TGraph object containing the range of the particle
        TGraph *range = new TGraph(43,energyCSDA,rangeCSDA);
        

        // Cos theta bins
        Double_t cosThetaValues[101];
        
        // attenuated muons
        Double_t attenuationValues[101];
        
        int cnt = 0;
        double tmp_l, e_tmp, integral;
        for (double x = 0.01; x <=1.0; x+=0.01) {
            tmp_l = fL->Eval(x);
            e_tmp = 0.001;
            while( range->Eval(e_tmp)< tmp_l){
                e_tmp+=1;
            }
            integral = evalIntegral(func1,x,e_tmp);
            cosThetaValues[cnt]    = x;
            attenuationValues[cnt] = integral;
            cnt+=1;
        }
        
        graphTMP = new TGraph(100,cosThetaValues,attenuationValues);
        
        funcMuonCosTheta = new TF1("funcMuonCosTheta",TGraph2TF1,0.01,1.0);
        
        cosThetaPrime->Delete();
        func1->Delete();
        fL->Delete();
        range->Delete();
        
    }
    
    double VertexGen_FastNeutron::evalIntegral(TF2 *func1,double x,double e_tmp){
        Double_t xmin = 1e-3;
//        Double_t xmax = 1e6;
        const static Int_t nbins = 100;
        Double_t logxmin = log10(e_tmp);
        Double_t logxmax = 8.0;
        Double_t binwidth= (logxmax-logxmin)/double(nbins);
        Double_t xbins[nbins];
        xbins[0] = e_tmp;
        Double_t norm = log(10)*binwidth;
        Double_t tmp = 0.0, a = 0.0;
        for(int i=0.0;i<nbins;i++){
            a  = xmin + pow(10,logxmin+i*binwidth);
            tmp+=func1->Eval(x,log10(a));
        }
        return norm*tmp;
    }
    
    
    
    double VertexGen_FastNeutron::GetRandomMuonCosTheta(){
        return funcMuonCosTheta->GetRandom();
    }

}
