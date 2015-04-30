//  VertexFile_Gen.cc
// Contact person: R. Bonventre <rbonventre@lbl.gov>
// See  VertexFile_Gen.hh for more details
//———————————————————————//

#include <RAT/VertexFile_Gen.hh>
#include <RAT/Log.hh>
#include <RAT/PrimaryVertexInformation.hh>
#include <RAT/Factory.hh>
#include <RAT/GLG4StringUtil.hh>
#include <RAT/GLG4TimeGen.hh>
#include <RAT/GLG4PosGen.hh>
#include <RAT/DS/Root.hh>
#include <RAT/DS/MC.hh>
#include <RAT/DS/MCParticle.hh>

#include <G4Event.hh>
#include <G4PrimaryVertex.hh>
#include <G4PrimaryParticle.hh>
#include <G4ParticleTable.hh>
#include <G4ThreeVector.hh>
#include <G4RunManager.hh>

#include <TTree.h>
#include <TFile.h>
#include <TROOT.h>
#include <TTimeStamp.h>
#include <TVector3.h>

#include <stdexcept>
#include <vector>
#include <string>
using namespace std;

namespace RAT {

void VertexFile_Gen::GenerateEvent(G4Event *event)
{
  fTTree->GetEntry(fCurrentEvent);
  DS::MC* mc = fDS->GetMC();

  G4PrimaryVertex *vertex = NULL;
  PrimaryVertexInformation *vertinfo = new PrimaryVertexInformation();

  bool vertexset = false;

  // add any parents
  for (int i=0;i<mc->GetMCParentCount();i++){
    const DS::MCParticle* mcp = mc->GetMCParent(i);
    G4PrimaryParticle* particle = new G4PrimaryParticle(mcp->GetPDGCode(),
        mcp->GetMomentum().X(),mcp->GetMomentum().Y(),mcp->GetMomentum().Z());

    if (!vertexset){
      G4ThreeVector pos(mcp->GetPosition().X(),mcp->GetPosition().Y(),mcp->GetPosition().Z());
      vertex = new G4PrimaryVertex(pos, NextTime() + mcp->GetTime());
      vertexset = true;
    }

    vertinfo->AddNewParentParticle(particle);
  }

  // add all particles
  for (int i=0;i<mc->GetMCParticleCount();i++){
    const DS::MCParticle* mcp = mc->GetMCParticle(i);
    G4PrimaryParticle* particle = new G4PrimaryParticle(mcp->GetPDGCode(),
        mcp->GetMomentum().X(),mcp->GetMomentum().Y(),mcp->GetMomentum().Z());

    if (!vertexset){
      G4ThreeVector pos(mcp->GetPosition().X(),mcp->GetPosition().Y(),mcp->GetPosition().Z());
      vertex = new G4PrimaryVertex(pos, NextTime() + mcp->GetTime());
      vertexset = true;
    }

    vertex->SetPrimary(particle);
  }

  if (vertexset){
    vertex->SetUserInformation(vertinfo);
    event->AddPrimaryVertex(vertex);
  }

  fLastEventTime = mc->GetUTC();
  fCurrentEvent++;

  if (fCurrentEvent >= fNumEvents || (fMaxEvent > 0 && fCurrentEvent >= fMaxEvent)){
    // we are out of events, stop the simulation after this one
    G4RunManager::GetRunManager()->AbortRun(/*softabort*/ true);
  }
}

void VertexFile_Gen::ResetTime(double offset)
{
  if (fCurrentEvent < fNumEvents){
    if (fTimeGen){
      double eventTime = fTimeGen->GenerateEventTime();
      nextTime = eventTime + offset;
    }else{
      fTTree->GetEntry(fCurrentEvent);
      DS::MC* mc = fDS->GetMC();
      nextTime = (mc->GetUTC().GetSec()-fLastEventTime.GetSec())*1e9 + (mc->GetUTC().GetNanoSec()-fLastEventTime.GetNanoSec()) + offset;
    }
  }else{
    nextTime = 1e9;
  }
}

void VertexFile_Gen::SetState(G4String state)
{
  // Break the argument to the this generator into sub-strings
  // separated by ":".
  state = util_strip_default(state);
  std::vector<std::string> parts = util_split(state, ":");
  size_t nArgs = parts.size();

  std::string filename;
  if (nArgs >= 5){
    int num_skip;
    istringstream (parts[4]) >> num_skip;
    fCurrentEvent = num_skip;
  }
  if (nArgs >= 4){
    istringstream (parts[3]) >> fMaxEvent;
    if (fMaxEvent > 0)
      fMaxEvent += fCurrentEvent;
  }
  if (nArgs >= 3){
    if (parts[2] == "default"){
      fTimeGen = 0;
    }else{
      fTimeGen = GlobalFactory<GLG4TimeGen>::New(parts[2]);
    }
  }
  if (nArgs >= 2){
    if (parts[1] == "default"){
      fPosGen = 0;
    }else{
      fPosGen = GlobalFactory<GLG4PosGen>::New(parts[1]);
    }
  }
  if (nArgs >= 1){
    filename = parts[0];
  }else{
    G4Exception(__FILE__, "Invalid Parameter", FatalException, ("vertexfile generator syntax error: '"+
          state+"' does not have a filename").c_str());
  }

  fStateStr = state;
  TFile *file = new TFile(filename.c_str());
  gROOT->cd(0);
  fTTree = (TTree*) ((TTree *) file->Get("T"))->CloneTree();
  fTTree->SetDirectory(0);

  file->Close();

  fNumEvents = fTTree->GetEntries();
  if (!fNumEvents)
    G4Exception(__FILE__, "Invalid Parameter", FatalException, ("File '"+
          filename+"' is empty").c_str());

  info << "VertexFile_Gen: Reading from " << filename << newline;

  fDS = new DS::Root();
  fTTree->SetBranchAddress("ds", &fDS);

  fNumEvents = (int) fTTree->GetEntries();
}

G4String VertexFile_Gen::GetState() const
{
  return fStateStr;
}

void VertexFile_Gen::SetTimeState(G4String state)
{
  if (fTimeGen)
      fTimeGen->SetState(state);
  else
      warn << "VertexFile_Gen error: Cannot set time state, no time generator selected\n";
}

G4String VertexFile_Gen::GetTimeState() const
{
  if (fTimeGen)
     return fTimeGen->GetState();
  else
     return G4String("VertexFile_Gen error: no time generator selected");
}

void VertexFile_Gen::SetPosState(G4String state)
{
  if (fPosGen)
     fPosGen->SetState(state);
  else
     warn << "VertexFile_Gen error: Cannot set position state, no position generator selected\n";
}

G4String VertexFile_Gen::GetPosState() const
{
  if (fPosGen)
     return fPosGen->GetState();
  else
     return G4String("VertexFile_Gen error: no pos generator selected");
}



} // namespace RAT
