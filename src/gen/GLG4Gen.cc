#include "GLG4Gen.hh"
#include "GLG4VertexGen.hh"
#include "GLG4PosGen.hh"
#include "GLG4TimeGen.hh"
#include "GLG4StringUtil.hh"

#include <RAT/Factory.hh>

#include <G4Event.hh>
#include <G4Track.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4ThreeVector.hh>

using namespace std;

GLG4Gen_Combo::GLG4Gen_Combo() : stateStr(""), vertexGen(), posGen(0)
{
  // Default time generator
  timeGen = new GLG4TimeGen_Poisson();
}

GLG4Gen_Combo::~GLG4Gen_Combo()
{
  delete timeGen;
  delete vertexGen;
  delete posGen;
}

void GLG4Gen_Combo::GenerateEvent(G4Event *event)
{
  G4ThreeVector pos;
  posGen->GeneratePosition(pos);
  G4double t0 = NextTime();
 
  vertexGen->GeneratePrimaryVertex(event, pos, t0);
}

void GLG4Gen_Combo::ResetTime(double offset)
{
  nextTime = timeGen->GenerateEventTime() + offset;
}
 
void GLG4Gen_Combo::SetState(G4String state)
{
  state = util_strip_default(state);

  vector<string> parts = util_split(state, ":");

  try {
    switch (parts.size()) {
    case 3:
      // last is optional time generator
      delete timeGen; timeGen = 0; // In case of exception in next line
      timeGen = RAT::GlobalFactory<GLG4TimeGen>::New(parts[2]);
    case 2:
      delete vertexGen; vertexGen = 0;
      vertexGen = RAT::GlobalFactory<GLG4VertexGen>::New(parts[0]);
      delete posGen; posGen = 0;
      posGen = RAT::GlobalFactory<GLG4PosGen>::New(parts[1]);
      break;
    default:
      G4Exception(__FILE__, "Invalid Parameter", FatalException, ("Combo generator syntax error: "+state).c_str());
      break;
    }

    stateStr = state; // Save for later call to GetState()
  } catch (RAT::FactoryUnknownID &unknown) {
    G4cerr << "Unknown generator \"" << unknown.id << "\"" << G4endl;
  }
}

G4String GLG4Gen_Combo::GetState() const
{
  return stateStr;
}

void GLG4Gen_Combo::SetTimeState(G4String state)
{
  if (timeGen)
    timeGen->SetState(state);
  else
    G4cerr << "GLG4Gen_Combo error: Cannot set vertex state, no vertex generator selected" << G4endl;
}

G4String GLG4Gen_Combo::GetTimeState() const
{
  if (timeGen)
    return timeGen->GetState();
  else
    return G4String("GLG4Gen_Combo error: no time generator selected");
}

void GLG4Gen_Combo::SetVertexState(G4String state)
{
  if (vertexGen)
    vertexGen->SetState(state);
  else
    G4cerr << "GLG4Gen_Combo error: Cannot set vertex state, no vertex generator selected" << G4endl;
}

G4String GLG4Gen_Combo::GetVertexState() const
{
  if (vertexGen)
    return vertexGen->GetState();
  else
    return G4String("GLG4Gen_Combo error: no vertex generator selected");
}

void GLG4Gen_Combo::SetPosState(G4String state)
{
  if (posGen)
    posGen->SetState(state);
  else
    G4cerr << "GLG4Gen_Combo error: Cannot set position state, no position generator selected" << G4endl;
}


G4String GLG4Gen_Combo::GetPosState() const
{
  if (posGen)
    return posGen->GetState();
  else
    return G4String("GLG4Gen_Combo error: no pos generator selected");
}


////////////////////////////////////////////////////////////////


GLG4Gen_DeferTrack::GLG4Gen_DeferTrack(const G4Track *track)
{
  vertex = new G4PrimaryVertex( track->GetPosition(),
			       track->GetGlobalTime() );

  G4ThreeVector mom(track->GetMomentum());
  G4ThreeVector pol(track->GetPolarization());
  G4PrimaryParticle* particle;
  particle=
    new G4PrimaryParticle(track->GetDefinition(),// particle code
			  mom.x(), mom.y(), mom.z() );// momentum
  particle->SetPolarization(pol.x(), pol.y(), pol.z()); // polarization
  particle->SetMass(track->GetDefinition()->GetPDGMass()); // Geant4 is silly.
  
  vertex->SetPrimary( particle );

  nextTime = vertex->GetT0();
}

GLG4Gen_DeferTrack::~GLG4Gen_DeferTrack()
{
}

void GLG4Gen_DeferTrack::GenerateEvent(G4Event *event)
{
  vertex->SetT0(nextTime);
  event->AddPrimaryVertex(vertex);
}

void GLG4Gen_DeferTrack::SetState(G4String /*state*/)
{
  // No-op
}

G4String GLG4Gen_DeferTrack::GetState() const
{
  return G4String("");
}


////////////////////////////////////////////////////////////////

GLG4Gen_External::GLG4Gen_External() : stateStr(""), timeGen(0), vertexGen(), posGen(0)
{
}

GLG4Gen_External::~GLG4Gen_External()
{
  delete timeGen;
  delete vertexGen;
  delete posGen;
}

void GLG4Gen_External::GenerateEvent(G4Event *event)
{
  // G4ThreeVector is a CLHEP::ThreeVector, which is initialized to (0,0,0) on creation
  G4ThreeVector pos;
  if(!dynamic_cast<GLG4VertexGen_HEPEvt*>(vertexGen)->GetUseExternalPos()){
    posGen->GeneratePosition(pos);
  }
  G4double t0 = NextTime();
 
  vertexGen->GeneratePrimaryVertex(event, pos, t0);
}

void GLG4Gen_External::ResetTime(double offset)
{
  nextTime = timeGen->GenerateEventTime() + offset;
}
 
void GLG4Gen_External::SetState(G4String state)
{
  state = util_strip_default(state);

  vector<string> parts = util_split(state, ":");

  try {
    switch (parts.size()) {
    case 3:
      delete timeGen; timeGen = 0; // In case of exception in next line
      timeGen = RAT::GlobalFactory<GLG4TimeGen>::New(parts[1]);
      delete vertexGen; vertexGen = 0;
      vertexGen = RAT::GlobalFactory<GLG4VertexGen>::New("HEPEvt");
      vertexGen->SetState(parts[2]);
      delete posGen; posGen = 0;
      if(parts[0].compare("external")==0) {
	dynamic_cast<GLG4VertexGen_HEPEvt*>(vertexGen)->SetUseExternalPos(true);
      } else {
	posGen = RAT::GlobalFactory<GLG4PosGen>::New(parts[0]);
	dynamic_cast<GLG4VertexGen_HEPEvt*>(vertexGen)->SetUseExternalPos(false);
      }
      break;
    default:
      G4Exception(__FILE__, "Invalid Parameter", FatalException, ("External generator syntax error: "+state).c_str());
      break;
    }

    stateStr = state; // Save for later call to GetState()
  } catch (RAT::FactoryUnknownID &unknown) {
    G4cerr << "Unknown generator \"" << unknown.id << "\"" << G4endl;
  }
}

G4String GLG4Gen_External::GetState() const
{
  return stateStr;
}

void GLG4Gen_External::SetTimeState(G4String state)
{
  if (timeGen)
    timeGen->SetState(state);
  else
    G4cerr << "GLG4Gen_External error: Cannot set vertex state, no vertex generator selected" << G4endl;
}

G4String GLG4Gen_External::GetTimeState() const
{
  if (timeGen)
    return timeGen->GetState();
  else
    return G4String("GLG4Gen_External error: no time generator selected");
}

void GLG4Gen_External::SetVertexState(G4String state)
{
  if (vertexGen)
    vertexGen->SetState(state);
  else
    G4cerr << "GLG4Gen_External error: Cannot set vertex state, no vertex generator selected" << G4endl;
}

G4String GLG4Gen_External::GetVertexState() const
{
  if (vertexGen)
    return vertexGen->GetState();
  else
    return G4String("GLG4Gen_External error: no vertex generator selected");
}

void GLG4Gen_External::SetPosState(G4String state)
{
  if (posGen)
    posGen->SetState(state);
  else
    G4cerr << "GLG4Gen_External error: Cannot set position state, no position generator selected" << G4endl;
}


G4String GLG4Gen_External::GetPosState() const
{
  if (posGen)
    return posGen->GetState();
  else
    return G4String("GLG4Gen_External error: no pos generator selected");
}
