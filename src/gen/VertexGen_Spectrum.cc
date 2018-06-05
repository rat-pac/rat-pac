#include <RAT/VertexGen_Spectrum.hh>
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
	VertexGen_Spectrum::VertexGen_Spectrum(const char *arg_dbname)
		: GLG4VertexGen(arg_dbname)
	{	
		// constructor	
		_particle = "e-";
		_spectrum = "flat";
		_lspec    = DB::Get()->GetLink("SPECTRUM", _spectrum);	// default
		_emin 	  = 0;	// no valid range for spectrum until properly initialised
		_emax	  = 0;
		Elim_Ulo  = 0;
		Elim_Uhi  = 0;
		Elim_Tlo  = 0;
		Elim_Thi  = 0;
	}
  
///-------------------------------------------------------------------------
	VertexGen_Spectrum::~VertexGen_Spectrum()
	{
		// destructor
	}

///-------------------------------------------------------------------------
	void VertexGen_Spectrum::GeneratePrimaryVertex(G4Event *event,
                    G4ThreeVector &dx,
                    G4double dt)
	{
		// Where the main work is done - the astute may notice a strong similarity to the Gun generator!
		
		G4PrimaryVertex* vertex= new G4PrimaryVertex( dx, dt );
		G4PrimaryParticle* particle;
    
    	        // Get particle mass for use when the particle is actually created, later in this method
		// get particle information
		_pDef= G4ParticleTable::GetParticleTable()->FindParticle(_particle);
		// and then get the actual mass
		G4double mass = _pDef->GetPDGMass();
    
		
    
    	        G4double _ke = this->SampleEnergy();
		// after generating the event we must reset any temporary limits on the energy range
		Elim_Tlo = Elim_Ulo;
		Elim_Thi = Elim_Uhi;

		// isotropic direction
		G4double phi= 2.*CLHEP::pi* G4UniformRand();
		G4double cosTheta = -1. + 2. * G4UniformRand();
		G4double sinTheta = sqrt(1. - cosTheta * cosTheta);
		G4double ux = sinTheta * cos(phi);
		G4double uy = sinTheta * sin(phi);
		G4double uz = cosTheta;

		G4ThreeVector dir = G4ThreeVector(ux,uy,uz);

		G4ThreeVector rmom( dir * sqrt(_ke*(_ke+2.*mass)) );
		particle= new G4PrimaryParticle(_pDef,   	// particle code
			      		rmom.x(),           			// x component of momentum
			      		rmom.y(),           			// y component of momentum
			      		rmom.z());		    			// z component of momentum
	        particle->SetMass(mass); // NOW set the mass
		// random polarisation
		phi= (G4UniformRand()*2.0-1.0)*M_PI;
		G4ThreeVector e1= dir.orthogonal().unit();
		G4ThreeVector e2= dir.cross(e1);
		G4ThreeVector rpol= e1*cos(phi)+e2*sin(phi);
		particle->SetPolarization(rpol.x(), rpol.y(), rpol.z());


		vertex->SetPrimary( particle );

		event->AddPrimaryVertex(vertex);
	}
	
///-------------------------------------------------------------------------
	void VertexGen_Spectrum::SetState(G4String newValues)
	{
		if (newValues.length() == 0) {
			// print help and current state
			G4cout 	<< "Current state of this VertexGen_Spectrum:\n"
     			  	<< " \"" << GetState() << "\"\n" << G4endl;
			G4cout 	<< "Format of argument to VertexGen_Spectrum::SetState: \n"
				   	<< " \"pname  specname  (Elo Ehi)\"\n" 
					<< " pname = particle name \n"
					<< " specname = spectrum name as given in ratdb \n"
					<< " Elo Ehi = optional limits on energy range of generated particles "
					<< G4endl;
			return;
		}

		std::istringstream is(newValues.c_str());
		G4std::string pname, specname;
		// Read in the particle type
		is >> pname;
		if (is.fail() || pname.length()==0){
			Log::Die("VertexGen_Spectrum: Incorrect vertex setting " + newValues);
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
						G4cout << " Spectrum Vertex: Setting ion with A = " << A << " Z = " << Z << G4endl;
					}
				}
			}	
			if (newTestGunG4Code == NULL) {
				G4cerr  << "Spectrum Vertex: Could not find particle type " << pname 
						<< " defaulting to electron " << G4endl;
				_particle = "e-";
				return;
			}
		}else{
			G4cout << "Spectrum Vertex: Setting particle = " << pname << G4endl;
		}
		// so store the name and the particle definition
		_particle = pname;
  		_pDef= newTestGunG4Code;
		
		// Read in the spectrum name
		is >> specname;
		// check that spectrum is in database - not sure what happens if this fails - tidy!
		DBLinkPtr lspec = DB::Get()->GetLink("SPECTRUM", specname);
		if(lspec){
			 _lspec	  = lspec;
			_spectrum = specname;	
			G4cout << "Spectrum Vertex: Setting spectrum " << specname << G4endl;
		}else{
			G4cerr << "Could not find spectrum " << specname << " using default, flat spectrum " << G4endl;
		}
		
		// Has the user supplied energy limits too? 
		float Elo, Ehi;
		is >> Elo >> Ehi;
		if(is.fail()){
			// obviously not, so make the universal limits huge
			Elim_Ulo = 0;
			Elim_Uhi = 9999999;
		}else{
			// user has selected universal limits
			Elim_Ulo = Elo;
			Elim_Uhi = Ehi;
			G4cout << "Limiting spectrum to range " 
				   << Elim_Ulo << " - " << Elim_Uhi
				   << G4endl; 
		}

		// finally ready to initialise the spectrum!
		this->InitialiseSpectrum();
		return;
	}

///-------------------------------------------------------------------------
	G4String VertexGen_Spectrum::GetState()
	{
		// State setting is the particle name and spectrum name	- return it
  		G4std::ostringstream os;
		os << _particle << " " << _spectrum << G4std::ends;
		G4String rv(os.str());
		return rv;
	}

///-------------------------------------------------------------------------
	void VertexGen_Spectrum::InitialiseSpectrum()
	{
		// Initialise the spectrum	
		// clear any stored values
		spec_E.clear();
		spec_cummag.clear();
		// firstly get the energies	and store the endpoints
		spec_E= _lspec->GetDArray("spec_e");
		_emin = spec_E.front();
		_emax = spec_E.back();
		if(_emax-_emin<=0 || _emin<0 || _emax<0){
			// We have a problem with the energy range
			Log::Die("VertexGen_Spectrum: Nonsensical energy range for spectrum " + _spectrum);			
		}		
			
		// now get the magnitudes
		spec_mag = _lspec->GetDArray("spec_mag");
		// now create a cumulative magnitude vector
		float magsum = 0;
		spec_cummag.push_back(0.0);
		for(unsigned int istep = 1; istep<spec_E.size(); ++istep){
		        // use trapazoid rule to produce linear interpolation
		        // of spectrum
		  magsum+=(spec_mag[istep] + spec_mag[istep-1])/2.0 * (spec_E[istep] - spec_E[istep-1]);
			spec_cummag.push_back(magsum);
		}
		
		// check that this works with the universal energy limits
		if((_emax < Elim_Ulo)||(_emin > Elim_Uhi)){
			Log::Die("VertexGen_Spectrum: selected spectrum does not lie within chosen energy range: " + _spectrum);						
		}
		
		// and finally make sure temporary limits are set to universal ones
		Elim_Tlo = Elim_Ulo;
		Elim_Thi = Elim_Uhi;
		G4cout << "Spectrum " << _spectrum << " initialised " << G4endl;
	}  
	

///-------------------------------------------------------------------------
	float VertexGen_Spectrum::SampleEnergy()
	{
		// Return a value for KE sampled from the spectrum between the limits Elo and Ehi
		float Elo, Ehi;
		// decide whether, Elo and Ehi should be universal or temporary limits (which is more stringent?)
		if(Elim_Tlo>Elim_Ulo){
			Elo = Elim_Tlo;
		}else{
			Elo = Elim_Ulo;
		}
		if(Elim_Thi<Elim_Uhi){
			Ehi = Elim_Thi;
		}else{
			Ehi = Elim_Uhi;
		}
		// (assume linear interpolation between specified points in spectrum)
		float start = spec_cummag.front();
		float stop = spec_cummag.back();
		if(Elo>_emin){
			// We have to apply a more stringent lower energy limit
			int istep = 0;
			while(spec_E[istep]<Elo){
				start = spec_cummag[istep];
				istep++;
			}
			// and interpolate last bit
			start += ((Elo-spec_E[istep-1])/(spec_E[istep]-spec_E[istep-1]))*(spec_cummag[istep]-spec_cummag[istep-1]);
		}
		if(Ehi<_emax){
			int istep = spec_E.size()-1;
			while(spec_E[istep]>Ehi){
				stop = spec_cummag[istep];
				G4cout << "* " <<  istep << " " << stop << " " << spec_E[istep] << " " << Ehi << G4endl;
				istep--;
			}
			// and interpolate the last bit
			stop -= ((spec_E[istep+1]-Ehi)/(spec_E[istep+1]-spec_E[istep]))*(spec_cummag[istep+1]-spec_cummag[istep]);
		}		 
		
		// now throw a random number between these limits 
		float random = start+G4UniformRand()*(stop-start);
		// now loop through cummulative distribution to choose energy
		int istep = 0;
		while(spec_cummag[istep+1]<random){
			istep++;
		}

		// The desired energy is bracketed by istep and istep+1 points.
		// Since the cumulative function is an integral of a linearly
		// interpolated spectrum, it is quadratic between data points.
		// Find energy assuming quadratic shape.

		float spec_slope = (spec_mag[istep+1] - spec_mag[istep]) / (spec_E[istep+1] - spec_E[istep]);
		
		float energy;
		float delta_y = random - spec_cummag[istep];
		if (fabs(spec_slope) < 1e-6) {
		  energy = spec_E[istep] + delta_y/spec_mag[istep];
		} else {
		  float a = 0.5 * spec_slope;
		  float b = spec_mag[istep] - spec_slope * spec_E[istep];
		  float c = -a * spec_E[istep] * spec_E[istep] - b * spec_E[istep] - delta_y;
		  energy = (-b + sqrt(b*b - 4 * a * c))/(2*a);
		}

		return energy;				
	}

///-------------------------------------------------------------------------
	void VertexGen_Spectrum::LimitEnergies(float Elo, float Ehi){
		// Set the limits for the generated energy range
		// first check this makes sense
		if((Elo>_emax)||(Elo>Elim_Uhi)||(Ehi<_emin)||(Ehi<Elim_Ulo)||((Ehi-Elo)<=0)){
			G4cout 	<< "Spectrum Vertex: temporary energy limits " << Elo << " - " << Ehi 
				    << " don't make sense, not applied \n" 
					<< "spectrum in range " << _emin << " - " << _emax << " MeV with universal limits " 
					<< Elim_Ulo << " - " << Elim_Uhi 	
					<< G4endl;
			return;
		}
		Elim_Tlo = Elo;
		Elim_Thi = Ehi;
		return;
	}

///-------------------------------------------------------------------------
	float VertexGen_Spectrum::EMaximum(){
		// return the maximum possible energy - accounting for spectrum and universal limits on it
		if(_emax<Elim_Uhi){
			return _emax;
		}else{
			return Elim_Uhi;
		}	
	}

///-------------------------------------------------------------------------
	float VertexGen_Spectrum::EMinimum(){
		// return the minimum possible energy - accounting for spectrum and universal limits on it
		if(_emin>Elim_Ulo){
			return _emin;
		}else{
			return Elim_Ulo;
		}	
	}

///-------------------------------------------------------------------------
}
