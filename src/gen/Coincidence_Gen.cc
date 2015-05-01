// RAT::Coincidence_Gen
// 16-July-2008 JR Wilson

// See comments in Coincidence_Gen.hh

#include <RAT/Coincidence_Gen.hh>
#include <RAT/CoincidenceMessenger.hh>

#include <RAT/Log.hh>

#include "GLG4VertexGen.hh"
#include <RAT/GLG4PosGen.hh>
#include <RAT/GLG4TimeGen.hh>
#include <RAT/Factory.hh>
#include <RAT/GLG4StringUtil.hh>

#include <G4Event.hh>
#include <G4PrimaryVertex.hh>
#include <G4ThreeVector.hh>

#include <Randomize.hh> 
#include <CLHEP/Units/SystemOfUnits.h>

#include <vector>
#include <cstring>
using std::vector;
using std::string;

#define G4std  std

//#undef DEBUG

namespace RAT {
	Coincidence_Gen::Coincidence_Gen() : stateStr(""), vertexGen(0), posGen(0){

		// Create a messenger to allow the user to change some ES parameters.
		messenger = new CoincidenceMessenger(this);
    	
		// As in the generator, use a default time generator if the
    	// user does not supply one.
    	timeGen 	= new GLG4TimeGen_Poisson();
		// also use default time window
		fTimeWindow = 400;		// 400ns
		fLoEnergy 	= 0; 		// 0 MeV
		fHiEnergy 	= 99999; 	// 99999 MeV
		nExtra 		= 0;		// number of additional interactions
		_maxExtra   = 5;		// limit to the number of extra interactions you can add
		_exptiming  = false;	// use random timing as default
		_expTimingTimeWindow = false; //by default, the time window for exponential is not defined.
		_expTimingTimeWindowForce = false; //by default, the time window for exponential is not defined.
		for(int iEx=0; iEx<_maxExtra; ++iEx){
			stateStrExtra[iEx] 	= "";
			vertexGenExtra[iEx]	= 0;
			posGenExtra[iEx] 	= 0;
			fExponent[iEx]      = 0;
		}
	}

/*--------------------------------------------------------------*/
  	Coincidence_Gen::~Coincidence_Gen(){
    	delete timeGen;
    	delete posGen;
    	delete vertexGen;
		if ( messenger != 0 ){
			delete messenger;
			messenger = 0;
  		}
	}
	
/*--------------------------------------------------------------*/
	void Coincidence_Gen::GenerateEvent(G4Event* event)
	{
		G4ThreeVector 	pos;
		// We will need to keep track of how much energy has been generated if the 
		// total event energy is to be limited
		G4double		KEgen=0;
		
		// generate times for all the events
		vector<double> times;
		vector<double> MaxE;
		vector<double> MinE;
		if(_exptiming){
			// exponential timing constants have been set so choose times from them
			// first vertex is at time 0
			double lastt=0;
			times.push_back(lastt);
			for(int iEx=0, counter=0; iEx<nExtra; ++iEx){
				lastt +=CLHEP::RandExponential::shoot(fExponent[iEx]); // Each inter-arrival time has it own exponential time constant
				// if the events are forced in the window, we restart the loop as soon as they are exceeding the window
				if (_expTimingTimeWindowForce && lastt > fTimeWindow){
					counter++;
					if (counter < 100){
						times.clear();  // restarting with en empty vector
						lastt = 0;			// first tore time of previous vertex (first is at t=0)
						iEx=-1;         // Counter the ++iEx of the for loop
					}
					else {
						info << "WARNING: ---Coincidence Generator: too many trials, keeping the last one and not simulating the events outside the window---" << newline;
						break;
					}
				}
				times.push_back(lastt); // Place the time in the loop
			}
		}else{
 			// each event has random time in event window
			// the first one will be the start of the event so  keep track of that
			double firsttime = fTimeWindow; // set to latest possible value to begin		
			for(int iEx=0; iEx<=nExtra; ++iEx){	
				double t = G4UniformRand()*fTimeWindow;
				times.push_back(t);	// for the extra interactions
				if(t<firsttime)firsttime=t;
			}
			// now subtract this first time from every time
			for(int iEx=0; iEx<=nExtra; ++iEx){	
				times[iEx] -= firsttime;
			}
		}
		// now access the maxima and minima energy each extra generator can provide
		for(int iEx=0; iEx<nExtra; ++iEx){	
			MaxE.push_back(vertexGenExtra[iEx]->EMaximum());
			MinE.push_back(vertexGenExtra[iEx]->EMinimum());
		}
		// But if more than one extra generator, we actually want a cumulative total of the other energy limits
		float sumLo = 0;
		float sumHi = 0;
		for(int iEx=nExtra-1; iEx>=0; --iEx){	
			sumLo += MinE[iEx];
			sumHi += MaxE[iEx];
			MinE[iEx] = sumLo;
			MaxE[iEx] = sumHi;
		}
		
		// A test vertex and particle (used to extract event energy)
		G4PrimaryVertex* vtemp=0;
		G4PrimaryParticle* PP;
		// A temporary event
		G4Event* etemp;
		// first position
		posGen->GeneratePosition(pos);
		// the vertex generators add a primary vertex to event - ready to addirst one
		// if we need to limit the upper energy for this vertex, do so
		float lo,hi;
		if(fLoEnergy>0||fHiEnergy<99999){
			lo = fLoEnergy-sumHi;
			hi = fHiEnergy-sumLo;
			if(lo<0)lo = 0;
			if(hi<0){
				G4cerr << "Coincidence_Gen: cannot achieve selected energy range with these generators." << G4endl;					
			}
			if(vertexGen->ELimitable()){
				vertexGen->LimitEnergies(lo,hi);
				vertexGen->GeneratePrimaryVertex(event, pos, times[0]);
			}else{
				// we will have to apply trial and error so generate temporary event and test it
				G4double testE = -999*CLHEP::MeV;
				int count=1;
				while(count<100&&!(testE/CLHEP::MeV>lo&&testE/CLHEP::MeV<hi)){
					etemp = new G4Event();		// need to reset the temporary event
					vertexGen->GeneratePrimaryVertex(etemp, pos, times[0]);
					vtemp = etemp->GetPrimaryVertex();
					testE = 0;
					for(int iP=0;iP<vtemp->GetNumberOfParticle(); ++iP){
						PP = vtemp->GetPrimary(iP);
						testE += sqrt(PP->GetMomentum().mag2()+pow(PP->GetMass(),2))-PP->GetMass();
						// NOTE !!! problem if the particle is short lived
						// how do I access decay particle energy???
					}
					count++;
				}
				if(count==100){
					// Jump out of what could have been infinite loop
					G4cerr << "No luck finding correct energy ! " << G4endl; 
				}
				// got a good vertex so add it to the real event 
				event->AddPrimaryVertex(vtemp);	
			}
			// now access event, primary vertex and extract energy
			vtemp = event->GetPrimaryVertex();
			for(int iP=0;iP<vtemp->GetNumberOfParticle(); ++iP){
				PP = vtemp->GetPrimary(iP);
				KEgen += sqrt(PP->GetMomentum().mag2()+pow(PP->GetMass(),2))-PP->GetMass();
			}
		}else{
			// No limits on the energy so just add the vertex
			vertexGen->GeneratePrimaryVertex(event, pos, times[0]);
		}

		// Now loop through extra vertices
		for(int iEx=0; iEx<nExtra; ++iEx){
			if (_expTimingTimeWindow && times[iEx+1] > fTimeWindow) continue; // if the time window is defined, reject the events starting after the window is closed
			posGenExtra[iEx] -> GeneratePosition(pos);
			if(fLoEnergy>0||fHiEnergy<99999){
				if(iEx<nExtra-1){
					lo = fLoEnergy-MaxE[iEx+1]-KEgen/CLHEP::MeV;
					hi = fHiEnergy-MinE[iEx+1]-KEgen/CLHEP::MeV;
				}else{
					lo = fLoEnergy-KEgen/CLHEP::MeV;
					hi = fHiEnergy-KEgen/CLHEP::MeV;
				}
				if(vertexGenExtra[iEx]->ELimitable()){
					// Set limits on energy and add the vertex
					vertexGenExtra[iEx]->LimitEnergies(lo,hi);
					vertexGenExtra[iEx]->GeneratePrimaryVertex(event, pos, times[iEx+1]);				
				}else{
					// we will have to apply trial and error so generate temporary event and test it
					G4double testE = -999*CLHEP::MeV;
					int count=1;
					while(count<100&&!(testE/CLHEP::MeV>lo&&testE/CLHEP::MeV<hi)){
						etemp = new G4Event();		// need to reset the temporary event
						vertexGenExtra[iEx]->GeneratePrimaryVertex(etemp, pos, times[iEx+1]);
						vtemp = etemp->GetPrimaryVertex();
						testE = 0;
						for(int iP=0;iP<vtemp->GetNumberOfParticle(); ++iP){
							PP = vtemp->GetPrimary(iP);
							testE += sqrt(PP->GetMomentum().mag2()+pow(PP->GetMass(),2))-PP->GetMass();
						}
						count++;
					}
					if(count==100||count==1){
						// Jump out of what could have been infinite loop
						G4cerr << "No luck finding correct energy ! " << G4endl; 
					}
					// got a good vertex so add it to the real event 
					event->AddPrimaryVertex(vtemp);				
				}
				// and increment our energy sum (if not last event)
				if(iEx<(nExtra-1)){
					vtemp = event->GetPrimaryVertex(iEx+1);
					for(int iP=0;iP<vtemp->GetNumberOfParticle(); ++iP){
						PP = vtemp->GetPrimary(iP);
						KEgen += sqrt(PP->GetMomentum().mag2()+pow(PP->GetMass(),2))-PP->GetMass();
					}	
				}				
			}else{
				// No limits on the energy so just add the vertex
				vertexGenExtra[iEx]->GeneratePrimaryVertex(event, pos, times[iEx+1]);				
			}
		}
	}

/*--------------------------------------------------------------*/
	void Coincidence_Gen::ResetTime(double offset)
	{
		// This applies to the overall event time (not time between vertices)
		double eventTime = timeGen->GenerateEventTime();
		nextTime = eventTime + offset;
	}

/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetState(G4String state)
	{
		// arguments are V1:P1:T
		// V1 = first vertex type, required 
		// P1 = first vertex position, required
		// T  = overall time generator, optional, default = poisson

		state = util_strip_default(state);

		vector<string> parts = util_split(state, ":");

		try {
			switch (parts.size()) {
				case 3:
					// last is optional time generator
					delete timeGen; timeGen = 0; // In case of exception in next line
					timeGen = RAT::GlobalFactory<GLG4TimeGen>::New(parts[4]);
				case 2:
					delete posGen; posGen = 0;
					posGen = RAT::GlobalFactory<GLG4PosGen>::New(parts[1]);
					delete vertexGen; vertexGen = 0;
					vertexGen = RAT::GlobalFactory<GLG4VertexGen>::New(parts[0]);
					break;
				default:
				        G4Exception(__FILE__, "Invalid Parameter", FatalException, ("Coincidence generator syntax error: "+state).c_str());
					break;
			}
			stateStr = state; // Save for later call to GetState()
		} catch (RAT::FactoryUnknownID &unknown) {
			G4cerr << "Unknown generator \"" << unknown.id << "\"" << G4endl;
		}
	}
    
	G4String Coincidence_Gen::GetState() const
	{
		return stateStr;
	}

/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetEnergyRange(G4String newValues)
	{
		newValues = util_strip_default(newValues);
		if (newValues.length() == 0) {		
			// Print help and current state
			G4cout 	<< " Current limits on Coincidence_Gen generated energy range = " 
				   	<< fLoEnergy << " - " << fHiEnergy << " MeV\n" 
				    << " To change use syntax:  Elo  Ehi \n"
					<< " Elo = lower energy limit in MeV, Ehi = upper energy limit in MeV"
					<< " !!!! But note that this option will be very slow for vertex generators whose energy cannot be limited !!! " 
					<< G4endl;
		}
 		G4std::istringstream is(newValues.c_str());	
		double Elo, Ehi;
		is >> Elo >> Ehi;
		if(Elo<Ehi){
			fLoEnergy = Elo;
			fHiEnergy = Ehi;
			G4cout  << "Coincidence_Gen: Restricting generated energy range to " << Elo << " - " << Ehi << " MeV\n" 
					<< " !!!! But note that this option will be very slow for vertex generators whose energy cannot be limited !!!! " 
					<< " !!!! And does not work when event energy comes from radioactive decay !!!! "
					<< G4endl;
		}else{
			G4cerr << "Coincidence_Gen error: Elo must be less than Ehi!" << G4endl;
		}
	}

/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetTimeState(G4String state)
	{
		if (timeGen){
			timeGen->SetState(state);
		}else{
			G4cerr << "Coincidence_Gen error: Cannot set time state, no time generator selected" << G4endl;
		}
	}

/*--------------------------------------------------------------*/
	G4String Coincidence_Gen::GetTimeState() const
	{
		if (timeGen){
			return timeGen->GetState();
		}else{
			return G4String("Coincidence_Gen error: no time generator selected");
		}
	}

/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetPosState(G4String state)
	{
		// Set the position generator for the first interaction type
		if (posGen){
			posGen->SetState(state);
		}else{
			G4cerr << "Coincidence_Gen error: Cannot set position state, no position generator selected" << G4endl;
		}
	}

/*--------------------------------------------------------------*/
	G4String Coincidence_Gen::GetPosState() const
	{
		// Get the position generator for the first interaction type
		if (posGen){
			return posGen->GetState();
		}else{
			return G4String("Coincidence_Gen error: no pos generator selected");
		}
	}	
	
/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetVertexState(G4String state)
	{
		// Set the vertex generator for the first interaction
		if (vertexGen){
			vertexGen->SetState(state);
		}else{
			G4cerr << "Coincidence_Gen error: Cannot set vertex state, no vertex generator selected" << G4endl;
		}
	}

/*--------------------------------------------------------------*/
	G4String Coincidence_Gen::GetVertexState() const
	{
		// Get the name of the vertex generator for the first interaction
		if (vertexGen){
			return vertexGen->GetState();
		}else{
			return G4String("Coincidence_Gen error: no vertex generator selected");
		}
	}	

/*--------------------------------------------------------------*/
	void Coincidence_Gen::AddExtra(G4String state){
		// Add an extra vertex to the same event
		// arguments are V:P
		// V = vertex type, required
		// P = vertex position, required

		state = util_strip_default(state);

		vector<string> parts = util_split(state, ":");

		try {
			switch (parts.size()) {
				case 2:
					G4cout << "adding new interaction " << nExtra+1 << " " << parts[0] << " "<<
					parts[1] << G4endl;
					
					delete posGenExtra[nExtra]; posGenExtra[nExtra] = 0;
					posGenExtra[nExtra] = RAT::GlobalFactory<GLG4PosGen>::New(parts[1]);
					delete vertexGenExtra[nExtra]; vertexGenExtra[nExtra] = 0;
					vertexGenExtra[nExtra] = RAT::GlobalFactory<GLG4VertexGen>::New(parts[0]);
					// successfully added so increment number of extra states
					nExtra++;
					break;
		         	default:
				        G4Exception(__FILE__, "Invalid Parameter", FatalException, ("Coincidence generator syntax error: "+state).c_str());
					break;
			}
			stateStrExtra[nExtra-1] = state; // Save for later call to GetState()
		} catch (RAT::FactoryUnknownID &unknown) {
			G4cerr << "Unknown generator \"" << unknown.id << "\"" << G4endl;
		}
	}

/*--------------------------------------------------------------*/
	G4String Coincidence_Gen::GetExtraState(int nint) const
	{
		if (nint<=nExtra){	// check we have added an extra interaction
			return stateStrExtra[nint-1];
		}else{
			return G4String("Coincidence_Gen error: that extra interaction has not been selected");			
		}	
	}

/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetExtraPosState(G4String state)
	{
		// set the position generator name for the most recently added extra interaction
		if (nExtra>0){	// check we have added an extra interaction
			posGenExtra[nExtra-1]->SetState(state);
			G4cout << "Setting extra interaction " << nExtra << ", position type to " << state << G4endl;
		}else{
			G4cerr << "Coincidence_Gen error: Cannot set extra position state, no extra interaction selected" << G4endl;
		}
	}

/*--------------------------------------------------------------*/
	G4String Coincidence_Gen::GetExtraPosState(int nint) const
	{
		// return the position generator name for extra state nint
		if (nint<=nExtra){	// check we have added an extra interaction
			return posGenExtra[nint-1]->GetState();
		}else{
			return G4String("Coincidence_Gen error: that extra interaction has not been selected");
		}
	}	

/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetExtraVertexState(G4String state)
	{
		// Set the vertex generator fo the most recently added in interaction
		if (nExtra>0){
			vertexGenExtra[nExtra-1]->SetState(state);
			G4cout << "Setting extra interaction " << nExtra << ", vertex type to " << state << G4endl;
		}else{
			G4cerr << "Coincidence_Gen error: Cannot set extra vertex state, no extra interaction selected" << G4endl;
		}
	}

/*--------------------------------------------------------------*/
	G4String Coincidence_Gen::GetExtraVertexState(int nint) const
	{
		if (nint<=nExtra){
			return vertexGenExtra[nint-1]->GetState();
		}else{
			return G4String("Coincidence_Gen error: that extra interaction has not been selected");
		}
	}	

/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetExponentials(G4String newValues)
	{
		/* Set exponential time constants to separate decays. 
		* Default is to give each vertex a random time in the event window but if this option is called
		* User must set a time constant for each added vertex. 
		* First vertex is at t - 0, the rest have times selected from exponentials
		* each exponential applies to the time since last vertex */
  		
		_exptiming = true;
		G4std::istringstream is(newValues.c_str());
		for(int i=0; i<nExtra; ++i){
		 	is >> fExponent[i];
			if(is.good()){
				G4cout << "Coincidence_Gen: Setting extra interaction " << i+1 
					   << " exponential time constant to " <<  fExponent[i] << G4endl;
			}else{
				G4cerr << "Coincidence_Gen error: "
				       << "Exponential timing selected, but insufficient time constants provided " 
					   << G4endl;
			}
		}
	}
	
	/*--------------------------------------------------------------*/
	void Coincidence_Gen::SetExpoForceWindow(G4bool newValues)
	{
		/* Set Force the events to be all in the time window. If set to false, the event not inside the window
		 will not be simulated.It is valable for all the coincidence interactions, with the time window starting
		 with the first event.*/
		
		_expTimingTimeWindowForce = newValues;
	}
}
	

