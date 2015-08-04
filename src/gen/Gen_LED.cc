#include <RAT/Gen_LED.hh>
#include <RAT/EventInfo.hh>
#include <RAT/Log.hh>
#include <RAT/GLG4TimeGen.hh>
#include <Randomize.hh>
#include <G4Event.hh>
#include <G4Track.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTable.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4ThreeVector.hh>
#include <RAT/LinearInterp.hh>

#include <RAT/DB.hh>
#include <RAT/TimeUtil.hh>

using namespace std;

namespace RAT {


Gen_LED::Gen_LED() : stateStr("default"), timeGen(0), next_led(0)
{
  // Default time generator
  timeGen = new GLG4TimeGen_Poisson();

  // Photon definition (assume /run/initialize already done)
  photonDef = G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");
  SetState(stateStr);
}

Gen_LED::~Gen_LED()
{
  delete timeGen;
  delete rand_wl;
  delete rand_angle;  
  delete rand_time;
}

void Gen_LED::GenerateEvent(G4Event *event)
{
  // Get information on next LED to fire
  G4ThreeVector pos(led_x[next_led]*CLHEP::mm, led_y[next_led]*CLHEP::mm, led_z[next_led]*CLHEP::mm);
  G4ThreeVector normal = -pos.unit();
  G4ThreeVector perp = normal.orthogonal().unit();

  float wavelength;

  G4double t0 = NextTime();

  // Record in extra G4Event information
  EventInfo *exinfo = dynamic_cast<EventInfo*>(event->GetUserInformation());
  DS::Calib *calib = new DS::Calib;
  calib->SetSourceName("LED");
  calib->SetID(next_led); // LED number is ID number (first source)
  calib->SetMode((int) led_wavelength[next_led]); //store average LED wavelength
  calib->SetIntensity(photons_per_event);
  calib->SetPosition( TVector3(pos.x(), pos.y(), pos.z()) );
  calib->SetUTC(AddNanoseconds(exinfo->utc, (long) t0));
  exinfo->SetCalib(calib);


  // Go to next LED, but wrap around
  next_led = (next_led + 1) % led_x.size();
	if(!oneLED){//fire all LEDs in same event
		for(int iLED=0;iLED<int(led_x.size());iLED++){
			pos.set(led_x[iLED]*CLHEP::mm,led_y[iLED]*CLHEP::mm,led_z[iLED]*CLHEP::mm);
  		normal=-pos.unit();
  		perp=normal.orthogonal().unit();

			for(int i=0;i<photons_per_LED[iLED];i++){
				double t1=0;
				if(unif_mode)
					t1=0;
				else
					t1=rand_time->shoot()*(time_max-time_min)+time_min;
				if(mono_wl_mode)
  	    	wavelength=led_wavelength[iLED]; // nm
	    	else
      		wavelength=rand_wl->shoot()*(wl_max-wl_min)+wl_min;
				double energy=CLHEP::hbarc*CLHEP::twopi/(wavelength*CLHEP::nm);
				double theta;
	    	if(iso_mode)
      		theta=acos(0.9999*(2.0*G4UniformRand()-1.0));
    		else if(!multi_ang_dist_mode)
  	    	theta=rand_angle->shoot()*(angle_max-angle_min)+angle_min;
				else if(iLED<int(rand_angles.size()))
					theta=rand_angles[iLED]->shoot()*(angle_maxs[iLED]-angle_mins[iLED])+angle_mins[iLED];
				else{
					warn<<"Warning: missing "<<next_led<<"-th angular distr., only "<<rand_angles.size()<<" exist! Reusing last distrib.\n";
					theta=rand_angles[rand_angles.size()-1]->shoot()*(angle_maxs[rand_angles.size()-1]-angle_mins[rand_angles.size()-1])+angle_mins[rand_angles.size()-1];
				}
				double phi=CLHEP::twopi*G4UniformRand();

				G4ThreeVector mom(normal);
 	   		mom.rotate(theta,perp);
 	   		mom.rotate(phi,normal);
 	   		mom.setMag(energy);
				G4PrimaryVertex *vertex=new G4PrimaryVertex(pos,t0+t1);
	    	G4PrimaryParticle* particle=new G4PrimaryParticle(photonDef,
					mom.x(),mom.y(),mom.z());
    	//set polarization
    	phi=(G4UniformRand()*2.0-1.0)*CLHEP::pi;
    	G4ThreeVector e1=mom.orthogonal().unit();
    	G4ThreeVector e2=mom.unit().cross(e1);
    	G4ThreeVector pol=e1*cos(phi)+e2*sin(phi);
    	particle->SetPolarization(pol.x(),pol.y(),pol.z());
    	particle->SetMass(0.0);

    	vertex->SetPrimary(particle);
    	event->AddPrimaryVertex(vertex);
			}
		}
	}
	else{//fire single LED per event
  // Add verticies for each photon
  	for (int i=0; i < photons_per_event; i++) {

	    double t1 = 0;

    	if (unif_mode)
  	    t1 = 0; // sec
	    else
      	t1 = rand_time->shoot() * (time_max - time_min) + time_min;

    	if (mono_wl_mode)
  	    wavelength = led_wavelength[next_led]; // nm
	    else
      	wavelength = rand_wl->shoot() * (wl_max - wl_min) + wl_min;

    	float energy = CLHEP::hbarc * CLHEP::twopi / (wavelength * CLHEP::nm);
    	float momentum = energy; // GEANT uses momentum in same units as energy

  	  double theta;
	    if (iso_mode)
      	theta = acos(0.9999 * (2.0*G4UniformRand()-1.0));
    	else if(!multi_ang_dist_mode)
  	    theta = rand_angle->shoot() * (angle_max - angle_min) + angle_min;
			else if(next_led<int(rand_angles.size()))
				theta=rand_angles[next_led]->shoot()*(angle_maxs[next_led]-angle_mins[next_led])+angle_mins[next_led];
			else{
				warn<<"Warning: missing "<<next_led<<"-th angular distr., only "<<rand_angles.size()<<" exist! Reusing last distrib.\n";
				theta=rand_angles[rand_angles.size()-1]->shoot()*(angle_maxs[rand_angles.size()-1]-angle_mins[rand_angles.size()-1])+angle_mins[rand_angles.size()-1];
			}

	    double phi = CLHEP::twopi * G4UniformRand();

    	// Calc momentum
  	  G4ThreeVector mom(normal);
 	   	mom.rotate(theta, perp); // Rotate away from LED direction (polar angle)
	    mom.rotate(phi, normal); // Rotate around LED direction (phi)
    	mom.setMag(momentum);    // Scale to right magnitude

  	  G4PrimaryVertex *vertex = new G4PrimaryVertex(pos, t0+t1);
	    G4PrimaryParticle *particle = new G4PrimaryParticle(photonDef,
								mom.x(),
								mom.y(),
								mom.z());
    	// Generate random polarization
    	phi = (G4UniformRand()*2.0-1.0)*CLHEP::pi;
    	G4ThreeVector e1 = mom.orthogonal().unit();
    	G4ThreeVector e2 = mom.unit().cross(e1);
    	G4ThreeVector pol = e1*cos(phi)+e2*sin(phi);
    	particle->SetPolarization(pol.x(), pol.y(), pol.z());
    	particle->SetMass(0.0); // Seems odd, but used in GLG4VertexGen_Gun

    	vertex->SetPrimary(particle);
    	event->AddPrimaryVertex(vertex);
  	}
	}
}

void Gen_LED::ResetTime(double offset)
{
  nextTime = timeGen->GenerateEventTime() + offset;
}
 
void Gen_LED::SetState(G4String state)
{
//if this LED type exists in  either plane
  if(DB::Get()->GetDefaultTable("LED",state)||DB::Get()->GetUserTable("LED",state))
    stateStr=state;
  else
    stateStr="default";
  if(DB::Get()->GetLink("LED",stateStr)->GetIndex()==stateStr)
    SetLEDParameters(stateStr);
}

void Gen_LED::SetLEDParameters(G4String state)
{
  // Load database values
  DBLinkPtr lled = DB::Get()->GetLink("LED",state);
  photons_per_event = lled->GetI("intensity");
  try{photons_per_LED=lled->GetIArray("intensities");}
  catch(DBNotFoundError &e){};
  led_x = lled->GetDArray("x");
  led_y = lled->GetDArray("y");
  led_z = lled->GetDArray("z");
  led_wavelength = lled->GetDArray("wavelength");
	//add some code robustness
	Log::Assert((led_x.size()==led_y.size() && led_y.size()==led_z.size()),"Some LEDs miss some coordinate(s)\n");
	Log::Assert((led_x.size()<=led_wavelength.size()),"Some LEDs miss a wavelength\n");

  string intensity_mode="single";
  try{intensity_mode=lled->GetS("intensity_mode");}
  catch(DBNotFoundError &e){};
	info<<intensity_mode.data()<<" LED mode\n";
	if(intensity_mode=="single")
		oneLED=true;
	else if(intensity_mode=="chain")
		oneLED=false;
	if(oneLED==false)
		Log::Assert((led_x.size()<=photons_per_LED.size()),"Some LEDs miss their intensity\n");
					
  string time_mode="unif";
  try{time_mode = lled->GetS("time_mode");}
  catch(DBNotFoundError &e){};
  if (time_mode == "unif")
    unif_mode = true;
  else if (time_mode == "dist")
    unif_mode = false;
  else
    Log::Die(dformat("Gen_LED: LED.time_mode = \"%s\" is invalid.", 
			time_mode.c_str()));

  string angle_mode="iso";
 	try{angle_mode=lled->GetS("angle_mode");}
  catch(DBNotFoundError &e){};
	multi_ang_dist_mode=false;
	ang_dist_mode=false;
	iso_mode=false;
  if (angle_mode == "iso")
    iso_mode = true;
  else if (angle_mode == "dist")
    ang_dist_mode = true;
  else if(angle_mode=="multidist")
		multi_ang_dist_mode=true;
  else
    Log::Die(dformat("Gen_LED: LED.angle_mode = \"%s\" is invalid.", 
			angle_mode.c_str()));
	info<<"LED angle mode: "<<angle_mode.data()<<"\n";

  string wl_mode = lled->GetS("wl_mode");
  if (wl_mode == "mono")
    mono_wl_mode = true;
  else if (wl_mode == "dist")
    mono_wl_mode = false;
  else
    Log::Die(dformat("Gen_LED: LED.wl_mode = \"%s\" is invalid.", 
			wl_mode.c_str()));

  // Time, angle and wavelength distributions
  rand_time = 0;
  if (!unif_mode) {

    // RandGeneral requires a uniformly sampled distribution, so we use
    // a linear interpolator to resample this arbitrary distribution
    LinearInterp<double> time(lled->GetDArray("dist_time"),
			      lled->GetDArray("dist_time_intensity"));

    time_min = time.Min();
    time_max = time.Max();
    int nbins = time.Points();
    float step = (time_max - time_min) / (nbins - 1) *0.9999;
    double *dist_time_intensity = new double[nbins];    
    for (int i=0; i < nbins; i++)
      dist_time_intensity[i] = time(time_min + i * step);
      
    if(rand_time)
      delete rand_time;
    rand_time = new CLHEP::RandGeneral(dist_time_intensity, nbins);
  }

  rand_angle = 0;
  if (!iso_mode) {
		LinearInterp<double> angle;
		double* dist_angle_intensity=NULL;
		int nbins=-1;
		if(!multi_ang_dist_mode){

    // RandGeneral requires a uniformly sampled distribution, so we use
    // a linear interpolator to resample this arbitrary distribution
    angle.Set(lled->GetDArray("dist_angle"),
			      lled->GetDArray("dist_angle_intensity"));

    angle_min = angle.Min();
    angle_max = angle.Max();
    nbins = angle.Points();
    float step = (angle_max - angle_min) / (nbins - 1) *0.9999;
		if(dist_angle_intensity)
			delete dist_angle_intensity;
    dist_angle_intensity = new double[nbins];    
    for (int i=0; i < nbins; i++)
      dist_angle_intensity[i] = angle(angle_min + i * step);
      
    if(rand_angle)
      delete rand_angle;
    rand_angle = new CLHEP::RandGeneral(dist_angle_intensity, nbins);
		}
		else{
			if(int(led_x.size())!=lled->GetI("n_ang_dists"))
				warn<<lled->GetI("n_ang_dists")<<" angular distributions, but "<<led_x.size()<<" LEDs...!\n";
			double step=-.1;
			angle_maxs.clear();
			angle_mins.clear();
			vector<double> angles,angleints;
			for(int i=0;i<lled->GetI("n_ang_dists");i++){
				try{angles=lled->GetDArray(dformat("dist_angle%d",i));}
			  catch(DBNotFoundError &e){break;};//stop at 1st failure to read the database
				try{angleints=lled->GetDArray(dformat("dist_angle_intensity%d",i));}
			  catch(DBNotFoundError &e){break;};
				angle.Set(angles,angleints);
				angle_mins.push_back(angle.Min());
				angle_maxs.push_back(angle.Max());
				nbins=angle.Points();
				step=(angle_maxs[i]-angle_mins[i])/nbins;
				delete dist_angle_intensity;
				dist_angle_intensity=new double[nbins];
				for(int j=0;j<nbins;j++)
					dist_angle_intensity[j]=angle(angle_mins[i]+j*step);
				rand_angle=new CLHEP::RandGeneral(dist_angle_intensity,nbins);
				rand_angles.push_back(rand_angle);
				angles.clear();angleints.clear();
			}
			if(int(rand_angles.size())!=lled->GetI("n_ang_dists"))
				warn<<"Loaded "<<rand_angles.size()<<" angular distributions. Should be "<<lled->GetI("n_ang_dists")<<"... Data file needs to be rechecked!\n";
			for(int iangs=0;iangs<int(rand_angles.size());iangs++){
				Log::Assert(rand_angles[iangs]!=NULL,dformat("Failed loading an angular distribution for %d-th LED",iangs));
			}
		}
  }

  rand_wl = 0;
  if (!mono_wl_mode) {

    // RandGeneral requires a uniformly sampled distribution, so we use
    // a linear interpolator to resample this arbitrary distribution
    LinearInterp<double> wl(lled->GetDArray("dist_wl"),
			   lled->GetDArray("dist_wl_intensity"));

    wl_min = wl.Min();
    wl_max = wl.Max();
    int nbins = wl.Points();
    float step = (wl_max - wl_min) / (nbins - 1) * 0.9999;
    double *dist_wl_intensity = new double[nbins];
    for (int i=0; i < nbins; i++)
      dist_wl_intensity[i] = wl(wl_min + i * step);
    
      
    if(rand_wl)
      delete rand_wl;
    rand_wl = new CLHEP::RandGeneral(dist_wl_intensity, nbins);
  }  
}


G4String Gen_LED::GetState() const
{
  return stateStr;
}

void Gen_LED::SetTimeState(G4String state)
{
  if (timeGen)
    timeGen->SetState(state);
  else
    G4cerr << "Gen_LED error: Cannot set time state, no time generator selected" << G4endl;
}

G4String Gen_LED::GetTimeState() const
{
  if (timeGen)
    return timeGen->GetState();
  else
    return G4String("Gen_LED error: no time generator selected");
}

void Gen_LED::SetVertexState(G4String /*state*/)
{
    G4cerr << "Gen_LED error: Cannot set vertex state." << G4endl;
}

G4String Gen_LED::GetVertexState() const
{
  return G4String("Gen_LED error: no vertex generator");
}

void Gen_LED::SetPosState(G4String /*state*/)
{
  G4cerr << "Gen_LED error: Cannot set position state, no position generator" << G4endl;
}


G4String Gen_LED::GetPosState() const
{
  return G4String("Gen_LED error: no pos generator");
}


} // namespace RAT
