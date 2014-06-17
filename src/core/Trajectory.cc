#include <RAT/Trajectory.hh>
#include <G4StepPoint.hh>
#include <G4VProcess.hh>
#include <G4VPhysicalVolume.hh>
#include <RAT/Log.hh>
#include <RAT/Gsim.hh>
#include <RAT/TrackInfo.hh>

namespace RAT {


G4Allocator<Trajectory> aTrajectoryAllocator;
bool  Trajectory::fgDoAppendMuonStepSpecial=false;

Trajectory::Trajectory() : G4Trajectory()
{
  ratTrack = new DS::MCTrack;
}

Trajectory::Trajectory(const G4Track *aTrack) : G4Trajectory(aTrack),
						      creatorProcessName("start")
{
  ratTrack = new DS::MCTrack;
  
  ratTrack->SetID(GetTrackID());
  ratTrack->SetParentID(GetParentID());
  ratTrack->SetPDGCode(GetPDGEncoding());
  ratTrack->SetParticleName(GetParticleName());
  
  const G4VProcess *creatorProcess = aTrack->GetCreatorProcess();
  if (creatorProcess)
    creatorProcessName = creatorProcess->GetProcessName();
  // override with extra TrackInfo if present
  const TrackInfo *trackInfo = dynamic_cast<TrackInfo*>(aTrack->GetUserInformation());
  if (trackInfo && trackInfo->GetCreatorProcess() != "")
    creatorProcessName = trackInfo->GetCreatorProcess();

  ratTrack->SetLength(0.0);
}

Trajectory::~Trajectory()
{
  delete ratTrack;
}

void Trajectory::AppendStep(const G4Step* aStep)
{
  if (ratTrack->GetMCTrackStepCount() == 0) {
    // Add initial step at very beginning of track
    DS::MCTrackStep *initStep = ratTrack->AddNewMCTrackStep();
    G4StepPoint *initPoint = aStep->GetPreStepPoint();
    FillStep(initPoint, aStep, initStep, 0.0);
  }

  // Check if we are storing truncated stepping info for muons
  G4String particleName = aStep->GetTrack()->GetDefinition()->GetParticleName();
  if(fgDoAppendMuonStepSpecial==true && (particleName=="mu-" || particleName=="mu+")){
    G4String processName = aStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName();
    // would also like to store steps where neutron was created...but not yet
    if(processName=="Transportation"){
      DS::MCTrackStep *ratStep = ratTrack->AddNewMCTrackStep();
      G4StepPoint *endPoint = aStep->GetPostStepPoint();
      FillStep(endPoint, aStep, ratStep, aStep->GetStepLength());
      // Update total track length
      //ratTrack->SetLength(ratTrack->GetLength() + ratStep->GetLength());
      // previous line won't work if we're only keeping a subset of steps
      // so let's just set it to -1 for now to prevent misuse of ratTrack->GetLength()
      // instead, will have to use position of steps to determine length
      ratTrack->SetLength(-1.);
    }
    return;
  }
  
  DS::MCTrackStep *ratStep = ratTrack->AddNewMCTrackStep();
  G4StepPoint *endPoint = aStep->GetPostStepPoint();
  FillStep(endPoint, aStep, ratStep, aStep->GetStepLength());  
  // Update total track length
  ratTrack->SetLength(ratTrack->GetLength() + ratStep->GetLength());
  if(Gsim::GetFillPointCont())
    G4Trajectory::AppendStep(aStep);
}

void Trajectory::FillStep(const G4StepPoint *point, const G4Step *step,
                          DS::MCTrackStep *ratStep, double stepLength)
{
  G4StepPoint *startPoint = step->GetPreStepPoint();

  ratStep->SetLength(stepLength);

  const G4ThreeVector &pos = point->GetPosition();
  ratStep->SetEndpoint( TVector3(pos.x(), pos.y(), pos.z()) );
  ratStep->SetGlobalTime(point->GetGlobalTime());
  ratStep->SetLocalTime(point->GetLocalTime());
  ratStep->SetProperTime(point->GetProperTime());

  G4ThreeVector mom = point->GetMomentum();
  ratStep->SetMomentum( TVector3(mom.x(), mom.y(), mom.z()) );
  ratStep->SetKE(point->GetKineticEnergy());

  const G4VProcess *process = point->GetProcessDefinedStep();
  if (process == 0)
    ratStep->SetProcess(creatorProcessName); // Assume first step
  else
    ratStep->SetProcess(process->GetProcessName());

  G4VPhysicalVolume *volume = startPoint->GetPhysicalVolume();
  if(volume == NULL){
    detail<<"\nTrajectory encountered a NULL volume.  Continuing...\n";
    ratStep->SetVolume("NULL");
  }
  else{
    ratStep->SetVolume(volume->GetName());
  }
}


void Trajectory::MergeTrajectory(G4VTrajectory* secondTrajectory)
{
  G4Trajectory::MergeTrajectory(secondTrajectory);

  Trajectory *secondTraj = dynamic_cast<Trajectory*>(secondTrajectory);
  if (secondTraj) {
    for (int i=1; i < secondTraj->ratTrack->GetMCTrackStepCount(); i++)
      *ratTrack->AddNewMCTrackStep() = *secondTraj->ratTrack->GetMCTrackStep(i);
    ratTrack->SetLength(ratTrack->GetLength() + secondTraj->ratTrack->GetLength());
    secondTraj->ratTrack->PruneMCTrackStep();
  }
}


} // namespace RAT
