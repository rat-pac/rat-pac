#include <RAT/TrackNav.hh>
#include <G4ThreeVector.hh>
#include <iostream>
#include <set>
#include <map>
#include <stack>

using namespace std;

// TrackNode has no source, so put impl in this file
ClassImp(RAT::TrackNode)

namespace RAT {

void TrackNav::Load(DS::MC *mc, bool verbose) {
  // Build tables to map trackID to list index and to get the list
  // of daughter tracks for a given trackID
  map<int, int> idToIndex;
  multimap<const int, int> idToDaughter;
  typedef multimap<const int, int>::iterator MMIter;
  set<int> idYetToLoad; // This is really just for sanity check

  for (int i=0; i < mc->GetMCTrackCount(); i++) {
    DS::MCTrack *track = mc->GetMCTrack(i);
    int trackID = track->GetID();
    int parentID = track->GetParentID();
    
    if (verbose)
      cerr << "\rReading in track " << i << " (id " <<  trackID << ")     ";

    if (idYetToLoad.count(trackID) == 0) {
      idYetToLoad.insert(trackID);
      idToIndex[trackID] = i;
      idToDaughter.insert(pair<const int,int>(parentID, trackID));
    } else
      cerr << "TrackNav: TrackID " << trackID << " duplicated!" << endl;
  }

  if (verbose)
    cerr << endl;

  // Set head node with appropriate markers
  fHead = new TrackNode();
  fHead->SetTrackID(0);
  fHead->SetStepID(0);
  fHead->SetParticleName("TreeStart");
  fHead->SetVolume("_____");

  // Initialize tree-building data structures
  stack<int> readyToAdd; // Track IDs we can add since parent exists

  // Tracks with no parent are children of head node, so we are ready
  // to add all tracks with parentID == 0
  fTracks[0] = fHead;
  pair<MMIter, MMIter> mmipair = idToDaughter.equal_range(0); // I hate C++.
  for (MMIter mmi = mmipair.first; mmi != mmipair.second; mmi++)
    readyToAdd.push(mmi->second);
  
  // Loop until nothing left in readyToAdd.  If track list is not corrupted
  // then in principle all tracks will be reached.
  while (readyToAdd.size() != 0) {
    // Fetch next track
    int trackID = readyToAdd.top();

    if (verbose)
      cerr << "\rAdding " << trackID << " to tree.";

    readyToAdd.pop();
    DS::MCTrack *track = mc->GetMCTrack(idToIndex[trackID]);

    // Create initial node
    TrackNode *trackHead = new TrackNode;
    *trackHead = *track->GetMCTrackStep(0); // Copy over initial step
    trackHead->SetTrackID(track->GetID());
    trackHead->SetStepID(0);
    trackHead->SetPDGCode(track->GetPDGCode());
    trackHead->SetParticleName(track->GetParticleName());

    fTracks[trackHead->GetTrackID()] = trackHead;

    // Add remaining steps
    TrackNode *last = trackHead;
    for (int istep = 1; istep < track->GetMCTrackStepCount(); istep++) {
      TrackNode *node = new TrackNode;
      *node = *track->GetMCTrackStep(istep); // Copy over initial step
      node->SetTrackID(track->GetID());
      node->SetStepID(istep);
      node->SetPDGCode(track->GetPDGCode());
      node->SetParticleName(track->GetParticleName());

      last->AddNext(node);
      last = node;
    }

    // Find parent of this node.  Not totally straightforward
    // since this node could connect on any step of the parent track,
    // or even be generated between steps.
    // Only way to reconnect is to compare positions.
    TrackNode *parent = fTracks[track->GetParentID()];
    TVector3 nodePos = trackHead->GetEndpoint();
    // Start from end.  If several steps with identical positions
    // we will connect to last one
    while(!parent->IsTrackEnd()) parent = parent->GetNext();
    TVector3 parentPos = parent->GetEndpoint();

    TrackNode *bestParent = parent;
    double closestDist = (nodePos - parentPos).Mag();
    const double tolerance = 1e-5;
    while (parent->GetPrev() != 0 && 
            parent->GetPrev()->GetTrackID() == parent->GetTrackID()) {
      parent = parent->GetPrev();
      parentPos = parent->GetEndpoint();
      double dist = (nodePos - parentPos).Mag();
      // Want the fractional improvement to be better than just float rounding errors
      if ( (dist - closestDist)/closestDist < -tolerance) {
        closestDist = dist;
        bestParent = parent;
      }
    }

    // This node is either a good match, or the only possibility
    // Special case test: optical photons are frequently absorbed and reemitted,
    // it which case it is more useful to append them to the parent track rather
    // than register them as a child.
    if (bestParent->IsTrackEnd()                               // only append
	&& bestParent->GetParticleName() == trackHead->GetParticleName() // to same kind of particle
	&& idToDaughter.count(bestParent->GetTrackID()) == 1) {      // if we are the only child
      
      // Need to renumber the steps
      TrackCursor bestParentCur(bestParent);
      int stepnum = bestParentCur.StepCount();
      TrackNode *temp = trackHead;
      while (temp != 0) {
        temp->SetStepID(stepnum);
        stepnum++;
        temp = temp->GetNext();
      }

      bestParent->AddNext(trackHead);
    } else
      bestParent->AddChild(trackHead);
    
    // Add this track's children to the available stack
    pair<MMIter, MMIter> mmipair2 = idToDaughter.equal_range(track->GetID());
    for (MMIter mmi = mmipair2.first; mmi != mmipair2.second; mmi++)
      readyToAdd.push(mmi->second);
    
    // Finally, remove this track from the yet-to-load set (for checking at end)
    idYetToLoad.erase(track->GetID());
  }

  // All done, let's make sure we are really done
  if (idYetToLoad.size() != 0) {
    cerr << "TrackNav: Error! Not all tracks were reachable from initial particles.\n"
      "             Still " << idYetToLoad.size() << " left in queue." << endl;
  }
}

void TrackNav::Clear() {
  delete fHead;
}

TrackNode *TrackNav::FindID(int trackID) {
  return fTracks[trackID];
}

TrackNode *TrackNav::FindParticle(const std::string &particleName, bool verbose) {
  TrackCursor c = Cursor(false);
  c.SetVerbose(verbose);
  return c.FindNextParticle(particleName);
}


} // namespace RAT

