#include <RAT/VertexGen_PhotonBomb.hh>
#include <RAT/LinearInterp.hh>
#include <RAT/Log.hh>
#include <RAT/DB.hh>
#include <Randomize.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>
#include <G4Event.hh>
#include <G4ParticleTable.hh>
#include <G4PrimaryParticle.hh>
#include <G4PrimaryVertex.hh>
#include <G4ThreeVector.hh>

namespace RAT {
  VertexGen_PhotonBomb::VertexGen_PhotonBomb(const char *arg_dbname)
    : GLG4VertexGen(arg_dbname)
  {
    fOpticalPhoton = G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");
    SetState("1 400"); // one photon per event, 400 nm
    fRndmEnergy = 0;
    fMinEnergy = 0.0;
    fMaxEnergy = 0.0;
    fMaterial = "";
  }
  
  VertexGen_PhotonBomb::~VertexGen_PhotonBomb()
  {
    delete fRndmEnergy;
  }
  
  void VertexGen_PhotonBomb::
  GeneratePrimaryVertex(G4Event *event,
                        G4ThreeVector &dx,
                        G4double dt)
  {
    for (int i = 0; i < fNumPhotons; i++) {
      // Pick direction isotropically
      G4ThreeVector mom;
      double theta = acos(2.0 * G4UniformRand() - 1.0);
      double phi = 2.0 * G4UniformRand() * CLHEP::pi;

      // Use fixed energy unless spectrum was provided
      double energy;
      if(fRndmEnergy)
	energy = fMinEnergy + (fMaxEnergy - fMinEnergy) * fRndmEnergy->shoot();
      else
	energy = fEnergy;
      mom.setRThetaPhi(energy, theta, phi); // Momentum == energy units in GEANT4
      // Distribute times expoenentially, but don't bother picking a 
      // random number if there is no time constant
      double expt = 0.0;
      if(fExpTime > 0.0)
	expt = - fExpTime * log(G4UniformRand());
      
      G4PrimaryVertex* vertex= new G4PrimaryVertex(dx, dt + expt);
      G4PrimaryParticle* photon = new G4PrimaryParticle(fOpticalPhoton, 
                                                        mom.x(),mom.y(),mom.z());
      // Generate random polarization
      phi = (G4UniformRand()*2.0-1.0)*CLHEP::pi;
      G4ThreeVector e1 = mom.orthogonal().unit();
      G4ThreeVector e2 = mom.unit().cross(e1);
      G4ThreeVector pol = e1*cos(phi)+e2*sin(phi);
      photon->SetPolarization(pol.x(), pol.y(), pol.z());
      photon->SetMass(0.0); // Seems odd, but used in GLG4VertexGen_Gun
      
      vertex->SetPrimary(photon);
      event->AddPrimaryVertex(vertex);
    } // loop over photons
  }
  
  void VertexGen_PhotonBomb::SetState(G4String newValues)
  {
    if (newValues.length() == 0) {
      // print help and current state
      G4cout << "Current state of this VertexGen_PhotonBomb:\n"
             << " \"" << GetState() << "\"\n" << G4endl;
      G4cout << "Format of argument to VertexGen_PhotonBomb::SetState: \n"
        " \"num_photons wavelength_nm\"\n" << G4endl;
      return;
    }

    std::istringstream is(newValues.c_str());
    int num, wavelength;
    is >> num >> wavelength;
    if (is.fail()){
      // check for scintillation wavelength spectrum
      is.str(newValues.c_str());
      is.clear();
      std::string material;
      is >> num >> material;
      if (is.fail())
	Log::Die("VertexGen_PhotonBomb: Incorrect vertex setting " + newValues);
      fMaterial = material;

      // get the scintillation wavelength spectrum
      DBLinkPtr loptics = DB::Get()->GetLink("OPTICS", material);
      std::vector<double> wlarr = loptics->GetDArray("SCINTILLATION_value1");
      std::vector<double> wlamp = loptics->GetDArray("SCINTILLATION_value2");
      for (unsigned i=0; i<wlarr.size(); i++)
	wlarr[i] = CLHEP::hbarc * CLHEP::twopi / (wlarr[i] * CLHEP::nm);
      if(wlarr.front() > wlarr.back()){
	reverse(wlarr.begin(), wlarr.end());
	reverse(wlamp.begin(), wlamp.end());
      }
      for (unsigned i=1; i<wlarr.size(); i++)
	if(wlarr[i-1] >= wlarr[i])
	  Log::Die("VertexGen_PhotonBomb: wavelengths out of order");
      
      // use a linear interpolator to get a uniform sampling with bin
      // size smaller than the smallest bin in order to use RandGeneral
      LinearInterp<double> energyInterp(wlarr, wlamp);
      double step = 1.0e9;
      for(int i=0; i<(int)wlarr.size()-1; i++)
	step = fmin(step, wlarr[i+1]-wlarr[i]);
      step /= 2;
      int nbins = (int) ((energyInterp.Max()-energyInterp.Min()) / step) + 1;
      step = (energyInterp.Max() - energyInterp.Min()) / (nbins - 1);
      
      // get the oversampled array, small padding at ends to avoid range error
      double* energyResample = new double[nbins];
      energyResample[0] = energyInterp(energyInterp.Min() + step*1e-6);
      energyResample[nbins-1] = energyInterp(energyInterp.Max() - step*1e-6);
      for (int i=1; i<nbins-1; i++)
	energyResample[i] = energyInterp(energyInterp.Min() + i * step);
      fMinEnergy = energyInterp.Min();
      fMaxEnergy = energyInterp.Max();

      if(fRndmEnergy)
	delete fRndmEnergy;
      fRndmEnergy = new CLHEP::RandGeneral(energyResample, nbins);
    }
    else
      fEnergy = CLHEP::hbarc * CLHEP::twopi / (wavelength * CLHEP::nm);

    double exp = 0.0;
    is >> exp;
    if (exp < 0.0)
      Log::Die("VertexGen_PhotonBomb: Exponential time constant must be positive");
    
    fNumPhotons = num;
    fExpTime = exp;
  }
  
  G4String VertexGen_PhotonBomb::GetState()
  {
    if(fRndmEnergy)
      return dformat("%d\t%s\t%f", fNumPhotons, fMaterial.c_str(), fExpTime);
    else
      return dformat("%d\t%f\t%f", fNumPhotons, fEnergy, fExpTime);
  }
  
} // namespace RAT
