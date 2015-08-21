// Created by Daniel Hellfeld (07/2015)
// Follows directly from VertexGen_ES.cc

#include <RAT/VertexGen_ReactorES.hh>
#include <RAT/ReactorESgen.hh>

#include <RAT/GLG4PosGen.hh>
#include <RAT/GLG4StringUtil.hh>

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

  VertexGen_ReactorES::VertexGen_ReactorES(const char *arg_dbname): GLG4VertexGen(arg_dbname), nu_dir(0.,0.,0.){
      
    electron = G4ParticleTable::GetParticleTable()->FindParticle("e-");  
    m_electron = electron->GetPDGMass();
  }

  VertexGen_ReactorES::~VertexGen_ReactorES() {}


  void VertexGen_ReactorES:: GeneratePrimaryVertex(G4Event* argEvent, G4ThreeVector& dx, G4double dt){
      
    G4PrimaryVertex* vertex = new G4PrimaryVertex(dx, dt);
    G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
  
    if (ev_nu_dir.mag2() == 0.0) {
      // Pick isotropic direction
      double theta = acos(2.0 * G4UniformRand() - 1.0);
		double phi = 2.0 * G4UniformRand() * CLHEP::pi;
      ev_nu_dir.setRThetaPhi(1.0, theta, phi);
    }

    // Generate elastic-scattering interaction using ESgen.
    CLHEP::HepLorentzVector mom_electron = reactoresgen.GenerateEvent( ev_nu_dir );

    // -- Create particle at vertex
    G4PrimaryParticle* electron_particle =
      new G4PrimaryParticle(electron,           // particle code
			    mom_electron.px(),              // x component of momentum
			    mom_electron.py(),              // y component of momentum
				mom_electron.pz());             // z component of momentum
    electron_particle->SetMass(m_electron);     // This seems to help in VertexGen_IBD
    vertex->SetPrimary( electron_particle );  

    argEvent->AddPrimaryVertex(vertex);
  }


  void VertexGen_ReactorES::SetState(G4String newValues) {
      
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


  G4String VertexGen_ReactorES::GetState(){
      
    std::ostringstream os;

    os << nu_dir.x() << "\t" << nu_dir.y() << "\t" << nu_dir.z() << std::ends;

    G4String rv(os.str());
    return rv;
  }

} // namespace RAT
