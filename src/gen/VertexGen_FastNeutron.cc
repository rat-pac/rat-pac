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

#define G4std  std
namespace RAT {
    static const double DDEFAULT = 400;
    static const double EDEFAULT = 10;

    VertexGen_FastNeutron::VertexGen_FastNeutron(const char *arg_dbname)
    : GLG4VertexGen(arg_dbname)
    {
        n = G4ParticleTable::GetParticleTable()->FindParticle("neutron");
//        n = G4ParticleTable::GetParticleTable()->FindParticle("e-");

        messenger = new FastNeutronMessenger(this);

        // constructor
//        DDEFAULT = 400;
//        EDEFAULT = 1;
        
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
        
        G4PrimaryVertex* vertex= new G4PrimaryVertex( dx, dt );

        Double_t D = GetDepth();
        Double_t En_min = GetEnThreshold();


        
//        G4ParticleDefinition *fFastNeutron = G4IonTable::GetIonTable()->GetIon(Z , A, E);
//        
        G4PrimaryParticle* n_particle =
        new G4PrimaryParticle(n,                  // particle code
                              D*CLHEP::MeV ,         // x component of momentum
                              0.0,         // y component of momentum
                              0.0);        // z component of momentum
        n_particle->SetMass(n->GetPDGMass()); // Geant4 is silly.
        vertex->SetPrimary( n_particle );
        
        event->AddPrimaryVertex(vertex);
    }
    
    ///-------------------------------------------------------------------------
    void VertexGen_FastNeutron::SetState(G4String newValues)
    {
        if (newValues.length() == 0) {
            // print help and current state
            G4cout 	<< "Current state of this VertexGen_FastNeutron:\n"
            << " \"" << GetState() << "\"\n" << G4endl;
            G4cout 	<< "Format of argument to VertexGen_FastNeutron::SetState: \n"
            << " \"pname  specname  (Elo Ehi)\"\n"
            << " pname = particle name \n"
            << " specname = FastNeutron name as given in ratdb \n"
            << " Elo Ehi = optional limits on energy range of generated particles "
            << G4endl;
            return;
        }
        
        std::istringstream is(newValues.c_str());
        G4std::string pname, specname;
        // Read in the particle type
        is >> pname;
        if (is.fail() || pname.length()==0){
            Log::Die("VertexGen_FastNeutron: Incorrect vertex setting " + newValues);
        }
        G4ParticleDefinition * newTestGunG4Code =
        G4ParticleTable::GetParticleTable()->FindParticle(pname);
        if (newTestGunG4Code == NULL) {
            // not a particle name
            // see if we can parse it as an ion, e.g., U238 or Bi214
            G4std::string elementName;
            int A,Z;
            if (pname[1] >= '0' && pname[1] <='9') {
                A= atoi(pname.substr(1).c_str());
                elementName= pname.substr(0,1);
            } else {
                A= atoi(pname.substr(2).c_str());
                elementName= pname.substr(0,2);
            }
            if (A > 0) {
                for (Z=1; Z<=GLG4VertexGen_Gun::numberOfElements; Z++){
                    if (elementName == GLG4VertexGen_Gun::theElementNames[Z-1]) break;
                    if (Z <= GLG4VertexGen_Gun::numberOfElements){
                        newTestGunG4Code=G4IonTable::GetIonTable()->GetIon(Z, A, 0.0);
                        G4cout << " FastNeutron Vertex: Setting ion with A = " << A << " Z = " << Z << G4endl;
                    }
                }
            }
            if (newTestGunG4Code == NULL) {
                G4cerr  << "FastNeutron Vertex: Could not find particle type " << pname
                << " defaulting to electron " << G4endl;
                _particle = "e-";
                return;
            }
        }else{
            G4cout << "FastNeutron Vertex: Setting particle = " << pname << G4endl;
        }
        // so store the name and the particle definition
        _particle = pname;
        _pDef= newTestGunG4Code;
        
        // Read in the FastNeutron name
        is >> specname;
        // check that FastNeutron is in database - not sure what happens if this fails - tidy!
        //        DBLinkPtr lspec = DB::Get()->GetLink("FastNeutron", specname);
        //        if(lspec){
        //            _FastNeutron = specname;
        //            G4cout << "FastNeutron Vertex: Setting FastNeutron " << specname << G4endl;
        //        }else{
        //            G4cerr << "Could not find FastNeutron " << specname << " using default, flat FastNeutron " << G4endl;
        //        }
        
        // finally ready to initialise the FastNeutron!
//        this->InitialiseFastNeutron();
        return;
    }
    
    ///-------------------------------------------------------------------------
    G4String VertexGen_FastNeutron::GetState()
    {
        // State setting is the particle name and FastNeutron name	- return it
        G4std::ostringstream os;
        os << _particle << " " << _FastNeutron << G4std::ends;
        G4String rv(os.str());
        return rv;
    }
    
    
    ///-------------------------------------------------------------------------
    
    
    
    void VertexGen_FastNeutron::SetDepth (double _tmpVal)
    {
        if ((_tmpVal < 0.) || (_tmpVal > 8000.))
        {
            G4cerr << "Set your IBD Amplitude between 0 and 8000." << G4endl;
            return;
        }
        valueD = _tmpVal;
    }
    
    void VertexGen_FastNeutron::SetEnThreshold (double _tmpVal)
    {
        if ((_tmpVal > 1.) || (_tmpVal > 1000.))
        {
            G4cerr << "Set your IBD Amplitude between 1 and 1000." << G4endl;
            return;
        }
        valueE = _tmpVal;
    }
    
}
