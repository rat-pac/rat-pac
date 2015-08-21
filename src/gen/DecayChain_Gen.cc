// RAT::DecayChain_Gen
// 10-Jan-2006 WGS

// See comments in RATDecayChain_Gen.hh

#include <RAT/DecayChain_Gen.hh>
#include <RAT/DecayChain.hh>

#include <RAT/GLG4PosGen.hh>
#include <RAT/GLG4TimeGen.hh>
#include <RAT/Factory.hh>
#include <RAT/GLG4StringUtil.hh>

#include <G4Event.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4IonTable.hh>
#include <G4Electron.hh>
#include <G4Gamma.hh>
#include <G4Alpha.hh>
#include <G4ThreeVector.hh>
#include <G4UnitsTable.hh>

#include <cstring>

#undef DEBUG

namespace RAT {

  DecayChain_Gen::DecayChain_Gen() : 
    stateStr(""), 
    posGen(0),
    fDecayChain(0)
  {
    // As in the combo generator, use a default time generator if the
    // user does not supply one.
    timeGen = new GLG4TimeGen_Poisson();
  }

  DecayChain_Gen::~DecayChain_Gen()
  {
    delete timeGen;
    delete posGen;
  }

  void DecayChain_Gen::GenerateEvent(G4Event* event)
  {
    if (fDecayChain == 0) 
      {
	G4cerr << "RAT::DecayChain_Gen::GenerateEvent: No decay chain found; "
	       << "please use /generator/add decaychain ISOTOPE:POSITION[:TIME]" << G4endl;
	return;
      }

    // Generate the position of the isotope.  Note that, for now, we
    // don't change the position of the isotope as it decays.
    G4ThreeVector position;
    posGen->GeneratePosition(position);
 
    // Generate the chain of decay products for this isotope.
    fDecayChain->GenerateFullChain();
    G4int nPrimaries = fDecayChain->GetNGenerated();

#ifdef DEBUG
    G4cout << "RAT::DecayChain_Gen::GenerateEvent: "
	   << nPrimaries << " primaries in decay chain "
	   << fDecayChain->GetChainName() << G4endl;
#endif

    for ( G4int iPrimary = 0; iPrimary < nPrimaries; iPrimary++ ) 
      {
	// particle type
	G4int pid = fDecayChain->GetEventID(iPrimary); // not the PDG code!

	G4ParticleDefinition* particleDef = 0;
	if ( pid >= 100000000 ) 
	  {
	    G4int A = (pid-100000000)/1000;
	    G4int Z = (pid-100000000) - A*1000;
	    G4double excitationEnergy = 0.0; // assume all in ground state
	    particleDef = G4IonTable::GetIonTable()->GetIon(Z, A, excitationEnergy);
	  }
	else {
	  if( pid == DecayBeta ) 
	    {
	      particleDef = G4Electron::Electron();
	    }
	  else if ( pid == DecayGamma ) 
	    {
	      particleDef = G4Gamma::Gamma();
	    }
	  else if( pid == DecayAlpha ) 
	    {
	      particleDef = G4Alpha::Alpha();
	    }
	}
	if ( particleDef == 0 ) 
	  {
	    G4cerr << "RAT::DecayChain_Gen::GenerateEvent: "
		   << "didn't know how to handle particle with ID " << pid << G4endl;
	    continue;
	  }

	// Get the particle's information (momentum and time)
	DecayChain::ParticleInfo_t particleInfo = fDecayChain->GetParticleInfo(iPrimary);
	G4double time = NextTime() + fDecayChain->GetEventTime(iPrimary);

	// generate a vertex with a primary particle
	G4PrimaryVertex* vertex = new G4PrimaryVertex(position, time);
	G4PrimaryParticle* particle = new G4PrimaryParticle(particleDef, 
							    particleInfo.vector.px(), 
							    particleInfo.vector.py(), 
							    particleInfo.vector.pz());
	particle->SetMass(particleDef->GetPDGMass()); // Apparently this is useful in IBD code.
	vertex->SetPrimary(particle);
	event->AddPrimaryVertex(vertex);

#ifdef DEBUG
	G4cout << "RAT::DecayChain_Gen::GenerateEvent: "
	       << "Primary " << iPrimary << " of " << nPrimaries 
	       << ", pid=" << pid 
	       << ", name=" << particleDef->GetParticleName() << G4endl;
	G4cout << "    time=" << G4BestUnit(time,"Time")
	       << ", position=" << G4BestUnit(position,"Length")
	       << ", momentum=" << G4BestUnit(particleInfo.vector,"Energy")
	       << G4endl;
#endif

      } // for each primary
  }

  void DecayChain_Gen::ResetTime(double offset)
  {
    double eventTime = timeGen->GenerateEventTime();
    nextTime = eventTime + offset;
#ifdef DEBUG
    G4cout << "RAT::DecayChain_Gen::ResetTime:"
	   << " eventTime=" << G4BestUnit(eventTime,"Time")
	   << ", offset=" << G4BestUnit(offset,"Time")
	   << ", nextTime=" << G4BestUnit(nextTime,"Time")
	   << G4endl;
#endif
  }
 
  void DecayChain_Gen::SetState(G4String state)
  {
#ifdef DEBUG
    G4cout << "RAT::DecayChain_Gen::SetState called with state='"
	   << state << "'" << G4endl;
#endif

    // Break the argument to the this generator into sub-strings
    // separated by ":".
    state = util_strip_default(state);
    std::vector<std::string> parts = util_split(state, ":");
    size_t nArgs = parts.size();

#ifdef DEBUG
    G4cout << "RAT::DecayChain_Gen::SetState: nArgs="
	   << nArgs << G4endl;
#endif

    try {

      if ( nArgs >= 3 )
	{
	  // The last argument is an optional time generator
	  delete timeGen; timeGen = 0; // In case of exception in next line
	  timeGen = GlobalFactory<GLG4TimeGen>::New(parts[2]);
	}

      if ( nArgs >= 2 )
	{
	  // The first argument is the isotope that starts the decay
	  // chain.
	  std::string isotope = parts[0];

	  // Don't bother building a new decay chain if we already have
	  // one for that isotope.
	  if ( fDecayChain == 0  ||
	       isotope != fDecayChain->GetChainName() )
	    {
	      delete fDecayChain; 
	      fDecayChain = new DecayChain(isotope);
#ifdef DEBUG
	      fDecayChain->SetVerbose(true);
#endif
	      bool found = fDecayChain->ReadInputFile(fDecayChain->GetChainName());
	      if (!found) 
		{
		  G4cerr << "RAT::DecayChain_Gen::SetState: couldn't find data for isotope " 
			 << isotope << G4endl;
		  delete fDecayChain; fDecayChain=0;
		}
	      G4cout << "RAT::DecayChain_Gen::SetState: successfully created decay chain for "
		     << fDecayChain->GetChainName() << G4endl;
#ifdef DEBUG
	      fDecayChain->Show();
#endif
	    }

	  // The second argument is a position generator.
	  delete posGen; posGen = 0;
	  posGen = GlobalFactory<GLG4PosGen>::New(parts[1]);
	}
      else
	{
	  G4Exception(__FILE__, "Invalid Parameter", FatalException, ("decaychain generator syntax error: '"+
								      state+
								      "' does not have a position generator").c_str());
	}

      stateStr = state; // Save for later call to GetState()
    } catch (FactoryUnknownID &unknown) {
      G4cerr << "Unknown generator \"" << unknown.id << "\"" << G4endl;
    }
  }

  G4String DecayChain_Gen::GetState() const
  {
    return stateStr;
  }

  void DecayChain_Gen::SetTimeState(G4String state)
  {
    if (timeGen)
      timeGen->SetState(state);
    else
      G4cerr << "DecayChain_Gen error: Cannot set time state, no time generator selected" << G4endl;
  }

  G4String DecayChain_Gen::GetTimeState() const
  {
    if (timeGen)
      return timeGen->GetState();
    else
      return G4String("DecayChain_Gen error: no time generator selected");
  }

  void DecayChain_Gen::SetPosState(G4String state)
  {
    if (posGen)
      posGen->SetState(state);
    else
      G4cerr << "DecayChain_Gen error: Cannot set position state, no position generator selected" << G4endl;
  }

  G4String DecayChain_Gen::GetPosState() const
  {
    if (posGen)
      return posGen->GetState();
    else
      return G4String("DecayChain_Gen error: no pos generator selected");
  }

} // namespace RAT
