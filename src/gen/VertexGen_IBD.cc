#include <globals.hh>
#include <G4ParticleDefinition.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <G4LorentzVector.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4Event.hh>

#include <RAT/GLG4PosGen.hh>
#include <sstream>
#include <RAT/VertexGen_IBD.hh>
#include <RAT/IBDgen.hh>
#include <RAT/GLG4StringUtil.hh>

namespace RAT {

  
VertexGen_IBD::VertexGen_IBD(const char *arg_dbname)
  : GLG4VertexGen(arg_dbname), nu_dir(0.,0.,0.)
{
  nu = G4ParticleTable::GetParticleTable()->FindParticle("anti_nu_e");  
  eplus = G4ParticleTable::GetParticleTable()->FindParticle("e+");  
  n = G4ParticleTable::GetParticleTable()->FindParticle("neutron");  
}

VertexGen_IBD::~VertexGen_IBD()
{
  
}

void VertexGen_IBD::
GeneratePrimaryVertex(G4Event *argEvent,
		      G4ThreeVector &dx,
		      G4double dt)
{
  G4PrimaryVertex* vertex= new G4PrimaryVertex(dx, dt);
  G4ThreeVector ev_nu_dir(nu_dir); // By default use specified direction
  
  if (ev_nu_dir.mag2() == 0.0) {
    // Pick isotropic direction
    double theta = acos(2.0 * G4UniformRand() - 1.0);
    double phi = 2.0 * G4UniformRand() * CLHEP::pi;
    ev_nu_dir.setRThetaPhi(1.0, theta, phi);
  }

  // -- Generate inverse beta decay interaction
  G4LorentzVector mom_nu, mom_eplus, mom_n;

  ibd.GenEvent(ev_nu_dir, mom_nu, mom_eplus, mom_n);

  // -- Create particles
  // FIXME: Should I also add the neutrino and make these daughters of it?
  // positron
  G4PrimaryParticle* eplus_particle =
    new G4PrimaryParticle(eplus,              // particle code
			  mom_eplus.px(),     // x component of momentum
			  mom_eplus.py(),     // y component of momentum
			  mom_eplus.pz());    // z component of momentum
  eplus_particle->SetMass(eplus->GetPDGMass()); // Geant4 is silly.
  vertex->SetPrimary( eplus_particle );  

  // neutron
  G4PrimaryParticle* n_particle =
    new G4PrimaryParticle(n,                  // particle code
			  mom_n.px(),         // x component of momentum
			  mom_n.py(),         // y component of momentum
			  mom_n.pz());        // z component of momentum
  n_particle->SetMass(n->GetPDGMass()); // Geant4 is silly.
  vertex->SetPrimary( n_particle );

  argEvent->AddPrimaryVertex(vertex);
}


void VertexGen_IBD::
SetState(G4String newValues)
{
  newValues = util_strip_default(newValues); // from GLG4StringUtil
  if (newValues.length() == 0) {
    // print help and current state
    G4cout << "Current state of this VertexGen_IBD:\n"
	   << " \"" << GetState() << "\"\n" << G4endl;
    G4cout << "Format of argument to VertexGen_IBD::SetState: \n"
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


G4String VertexGen_IBD::
GetState()
{
  std::ostringstream os;

  os << nu_dir.x() << "\t" << nu_dir.y() << "\t" << nu_dir.z() << std::ends;

  G4String rv(os.str());
  return rv;
}


} // namespace RAT
