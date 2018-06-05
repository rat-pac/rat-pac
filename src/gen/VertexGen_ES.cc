// Calculates the cross-section for neutrino-elastic scattering  
// as function of neutrino energy and the electron's recoil energy.
// Allow for variations in the weak mixing angle and the possibility 
// of a neutrino magnetic moment
//
// J. Formaggio (UW) -02/09/2005

#include <RAT/VertexGen_ES.hh>
#include <RAT/ESgen.hh>
#include <RAT/PrimaryVertexInformation.hh>

#include <RAT/GLG4PosGen.hh>
#include <RAT/GLG4StringUtil.hh>
#include <RAT/StringUtil.hh>

#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4Event.hh>
#include <Randomize.hh>
#include <globals.hh>

#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Units/PhysicalConstants.h>

#include <sstream>
#include <cmath>

namespace RAT {

  VertexGen_ES::VertexGen_ES(const char *arg_dbname)
    : GLG4VertexGen(arg_dbname), fNuDir(0.,0.,0.), fDBName("SOLAR"), fRandomDir(false)
  {
    fElectron = G4ParticleTable::GetParticleTable()->FindParticle("e-");  
    fNue = G4ParticleTable::GetParticleTable()->FindParticle("nu_e");
    fNumu = G4ParticleTable::GetParticleTable()->FindParticle("nu_mu");
    fElectronMass = fElectron->GetPDGMass();
    fESgen = new ESgen();
  }

  VertexGen_ES::~VertexGen_ES() {
    if (fESgen) delete fESgen;
  }


  void VertexGen_ES::
    GeneratePrimaryVertex(G4Event* argEvent,
			G4ThreeVector& dx,
			G4double dt)
  {
    //
    // Build the primary vertex with the position and time
    //
    G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);

    //
    // Build the incoming neutrino direction.
    // If this generator was not called from a higher level generator it is possible that the neutrino direction is not set
    //
    if (fNuDir.mag2() == 0.0 || fRandomDir) {
      fRandomDir = true;
      // Pick isotropic direction
      double theta = acos(2.0 * G4UniformRand() - 1.0);
      double phi = 2.0 * G4UniformRand() * CLHEP::pi;
      fNuDir.setRThetaPhi(1.0, theta, phi);
    }

    // Generate elastic-scattering interaction using ESgen.
    // NB: Updated to keep track of the neutrino as well as the electron
    // For the moment the ESgen does not full mom_nu so it is empty
    //

    CLHEP::HepLorentzVector mom_electron, mom_nu;
    fESgen->GenerateEvent( fNuDir, mom_nu, mom_electron );


    // -- Create particle at vertex
    // FIXME: Should we keep track of the outgoing neutrino as well?
    //       If so ESgen needs to be updated to pass the new neutrino direction.
    

    G4PrimaryParticle* electron_particle =
      new G4PrimaryParticle(fElectron,           // particle code
			    mom_electron.px(),     // x component of momentum
			    mom_electron.py(),     // y component of momentum
			    mom_electron.pz());    // z component of momentum
    electron_particle->SetMass(fElectronMass); // This seems to help in VertexGen_IBD
    vertex->SetPrimary( electron_particle );  

    // Add the incoming neutrino as the primary
    G4PrimaryParticle *neutrinoparent;
    if(this->GetNuFlavor()=="nue"){
      neutrinoparent = new G4PrimaryParticle(fNue,
          mom_nu.px(),
          mom_nu.py(),
          mom_nu.pz());
    }else if(this->GetNuFlavor()=="numu"){
      neutrinoparent = new G4PrimaryParticle(fNumu,
          mom_nu.px(),
          mom_nu.py(),
          mom_nu.pz());
    }else{
      return;
    }


    // We DON'T Add this one to the vertex as we don't want to propagate it but alongside
    // so that the information is present for extraction in Gsim
    PrimaryVertexInformation *vertinfo = new PrimaryVertexInformation();
    vertinfo->AddNewParentParticle(neutrinoparent);

    vertex->SetUserInformation(vertinfo);
    argEvent->AddPrimaryVertex(vertex);

  }


  void VertexGen_ES::SetState(G4String newValues)
  {
    newValues = util_strip_default(newValues); // from GLG4StringUtil
    if (newValues.length() == 0) {
      // print help and current state
      G4cout << "Current state of this VertexGen_ES:\n"
	     << " \"" << GetState() << "\"\n" << G4endl;
      G4cout << "Format of argument to VertexGen_ES::SetState: \n"
        " \"nu_dir_x nu_dir_y nu_dir_z [db_name:][db_flux:nu_flavor]\"\n"
        " where fNuDir is the initial direction of the incoming neutrino.\n"
        " Does not have to be normalized.  Set to \"0. 0. 0.\" for isotropic\n"
        " neutrino direction."
	     << G4endl;
      return;
    }

    std::istringstream is(newValues.c_str());
    double x, y, z;
    std::string rest;
    is >> x >> y >> z >> rest;
    if (is.fail()){
      G4cout << "VertexGen_ES : Failed to extract state from input string.\n";
      return;
    }

    // We take care of normalising the input direction here
    if ( x == 0. && y == 0. && z == 0. )
      fNuDir.set(0., 0., 0.);
    else
      fNuDir = G4ThreeVector(x, y, z).unit();

    // Now check that everything else is in "rest"
    if (rest.length() == 0)
      return;

    std::string parseparams = ":";
    rest = strip(rest,parseparams);
    std::vector<std::string> params = util_split(rest,parseparams);
    switch (params.size()) {
      case 3:
        // First entry is the database name
        this->SetDBName(params[0]);
        this->SetFlux(params[1]);
        this->SetNuFlavor(params[2]);
        break;
      case 2:
        this->SetFlux(params[0]);
        this->SetNuFlavor(params[1]);
        break;
      case 1:
        this->SetDBName(params[0]);
      default:
        G4cout << "VertexGen_ES : Detected only " << params.size() << " neutrino state terms (1,2, or 3 expected).\n";
        return;
    }
  }


  G4String VertexGen_ES::GetState()
  {
    std::ostringstream os;

    os << fNuDir.x() << "\t" << fNuDir.y() << "\t" << fNuDir.z() << std::ends;

    G4String rv(os.str());
    return rv;
  }

  void VertexGen_ES::SetFlux(const G4String flux) {
    if (fFlux == flux) return;
    fFlux = flux;
    fESgen->SetNuType(fFlux);
  }

  void VertexGen_ES::SetNuFlavor(const G4String flavor) {
    if (fNuFlavor == flavor) return;
    fNuFlavor = flavor;
    fESgen->SetNuFlavor(fNuFlavor);
  }

  void VertexGen_ES::SetDBName(const G4String name) {
    if (fDBName == name) return;
    fDBName = name;
    fESgen->SetDBName(fDBName);
  }



} // namespace RAT
