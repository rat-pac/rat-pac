#include <iostream>
#include <RAT/string_utilities.hpp>
#include <RAT/dprintf.hpp>

#include <RAT/TrackNode.hh>
#include <RAT/TrackCursor.hh>
#include <CLHEP/Units/PhysicalConstants.h>

using namespace std;
using namespace CLHEP;

namespace RAT {

TrackNode *TrackCursor::TrackStart() const
{
  TrackNode *cur = fCur;
  while (!cur->IsTrackStart()) cur = cur->GetPrev();
  return cur;
}

TrackNode *TrackCursor::TrackEnd() const
{
  TrackNode *cur = fCur;
  while (!cur->IsTrackEnd()) cur = cur->GetNext();
  return cur;  
}

TrackNode *TrackCursor::Step(int i) const
{
  TrackNode *cur = TrackStart();
  while (cur->GetStepID() != i && !cur->IsTrackEnd()) cur = cur->GetNext();

  if (cur->GetStepID() == i)
    return cur;
  else
    return 0;
}

TrackNode *TrackCursor::Child(int i) const
{
  if ( (int)fCur->child.size() > i) return fCur->child[i];
  else return 0;
}

int TrackCursor::TrackChildCount() const {
  TrackNode *cur = TrackStart();
  int count = cur->child.size();
  while (!cur->IsTrackEnd()) {
    cur = cur->GetNext();
    count += cur->child.size();
  }
  
  return count;
}

TrackNode *TrackCursor::TrackChild(int i) const
{
  TrackNode *cur = TrackStart();
  while (cur != 0 && i >= (int) cur->child.size()) {
    i -= cur->child.size();
    cur = cur->GetNext();
  }
  
  if (cur == 0)
    return 0; // Hit end of list before found child
  else
    return cur->child[i];
}

TrackNode *TrackCursor::Parent() const
{
  return TrackStart()->GetPrev();
}

double TrackCursor::TrackLength() const
{
  TrackNode *cur = TrackStart();
  double length = cur->GetLength(); // Should be zero for track start
  while (!cur->IsTrackEnd()) {
    cur = cur->GetNext();
    length += cur->GetLength();
  }
  return length;
}

void TrackCursor::Go(TrackNode *node) 
{ 
  fCur = node;
  if (fVerbose)
    PrintTrack();
}

TrackNode *TrackCursor::GoTrackStart()
{
  TrackNode *newCur = TrackStart();
  if (newCur != 0)
    Go(newCur);
  return newCur;
}

TrackNode *TrackCursor::GoPrev()
{
  TrackNode *newCur = Prev();
  if (newCur != 0)
    Go(newCur);
  return newCur;
}

TrackNode *TrackCursor::GoNext()
{
  TrackNode *newCur = Next();
  if (newCur != 0)
    Go(newCur);
  return newCur;
}

TrackNode *TrackCursor::GoTrackEnd()
{
  TrackNode *newCur = TrackEnd();
  if (newCur != 0)
    Go(newCur);
  return newCur;
}


TrackNode *TrackCursor::GoStep(int i)
{
  TrackNode *newCur = Step(i);
  if (newCur != 0)
    Go(newCur);
  return newCur;
}


TrackNode *TrackCursor::GoChild(int i)
{
  TrackNode *newCur = Child(i);
  if (newCur != 0)
    Go(newCur);
  return newCur;
}

TrackNode *TrackCursor::GoTrackChild(int i)
{
  TrackNode *newCur = TrackChild(i);
  if (newCur != 0)
    Go(newCur);
  return newCur;
}
 
TrackNode *TrackCursor::GoParent()
{
  TrackNode *newCur = Parent();
  if (newCur != 0)
    Go(newCur);
  return newCur;
}

// Pretty printing
void TrackCursor::Print() const
{
  cout << Print(fCur);
}

void TrackCursor::PrintTrack() const
{
  cout << PrintTrack(fCur);
}

std::string TrackCursor::PrintTrackIDs(TrackNode *node)
{
  set<int> trackIDs;
  vector<string> trackIDStrings;
  TrackCursor cursor(node);
  TrackNode *current = cursor.GoTrackStart();
  
  while(current != 0) {
    if (trackIDs.count(current->GetTrackID()) == 0) {
      trackIDs.insert(current->GetTrackID());
      trackIDStrings.push_back(::to_string(current->GetTrackID()));
    }
    current = cursor.GoNext();
  }

  return join(trackIDStrings, "/");
}

std::string TrackCursor::Print(TrackNode */*node*/)
{
  return string("");
}

std::string TrackCursor::PrintTrack(TrackNode *node)
{
  if (node == 0)
    return "null\n";

  TrackCursor cursor(node);
  TrackNode *cur = cursor.GoTrackStart();
  string output;

  // Track level information
  output += dformat("Track %s: %s", PrintTrackIDs(cur).c_str(), cur->GetParticleName().c_str());
  TrackNode *parent = cursor.Parent();
  if (parent != 0)
    output += dformat("  parent: %s(%s)\n", parent->GetParticleName().c_str(), 
		      PrintTrackIDs(parent).c_str());
  else
    output += "\n";

  string ecol;
  if (cur->GetParticleName() == "opticalphoton") 
    ecol = "  nm  ";
  else
    ecol = "  MeV ";

  output += "-----------------------------------------------------------------------------------\n";
  output += " # |          position                |"+ecol+"|     process    |   subtracks\n";
  output += "-----------------------------------------------------------------------------------\n";

  // Step information
  while (cur != 0) {
    string id = (cur == node ? "*" : " ") + dformat("%2d. ", cur->GetStepID());

    string pos = dformat("(%6.1f,%6.1f,%6.1f) %10s ", 
              cur->GetEndpoint().x(), cur->GetEndpoint().y(), cur->GetEndpoint().z(),
			        cur->GetVolume().c_str());

    string ene;
    if (cur->GetParticleName() == "opticalphoton") 
      ene = dformat("%3.0f nm ", (h_Planck*c_light / cur->GetKE()) / nm);
    else if (cur->GetKE() < 0.001)
      ene = "<0.001 ";
    else
      ene = dformat("%6.3f ", cur->GetKE());

    string process = dformat("%15s ", cur->GetProcess().c_str());

    string others;
    if (cur->child.size() == 0)
      others = "";
    else if (cur->child.size() < 3) {
      others = "->";
      vector<string> otherNames;
      for (unsigned i=0; i < cur->child.size(); i++)
	otherNames.push_back(dformat("%s(%s)", 
				     cur->child[i]->GetParticleName().c_str(),
				     PrintTrackIDs(cur->child[i]).c_str()));

      others += join(otherNames, ",");
    } else
      others = dformat("->%d tracks", cur->child.size());

    output += id + pos + ene + process + others + "\n";

    cur = cursor.GoNext();
  }

  return output;
}


TrackNode *TrackCursor::FindNextTrack()
{
  // Depth-first iteration through the track tree, visiting intermediate tracks 
  // "on the way down"
  if (TrackChildCount() > 0)
    return GoTrackChild(0); // Next track down tree
  else
    return FindNextTrackNoDescend(); // Next track back up (and down another branch perhaps)
}

TrackNode *TrackCursor::FindNextTrackNoDescend()
{
  // Depth-first iteration, going back up the tree
  if (Parent() == 0)
    return 0; // Nothing left
  else {
    TrackNode *origChild = TrackStart();
    fCur = Parent(); // Don't use GoParent() since we don't want to trigger verbose output

    int totalChildren = TrackChildCount();
    int i; // Declare outside so we can break out of loop and keep value
    for (i=0; i < totalChildren; i++) {
      if (TrackChild(i) == origChild)
	break;
    }

    if (i+1 < totalChildren)
      return GoTrackChild(i+1); // Done, go down next child branch
    else
      return FindNextTrackNoDescend();  // Recurse back up tree
  }
}

TrackNode *TrackCursor::FindNextTrack(TrackTest *predicate)
{
  // Disable verbose printing while searching
  TrackNode *origLocation = Here();
  bool verboseRemember = fVerbose;
  fVerbose = false;
  
  TrackNode *candidate = FindNextTrack();
  while(candidate != 0 && !(*predicate)(candidate))
    candidate = FindNextTrack();

  fVerbose = verboseRemember;
  if (candidate != 0)
    Go(candidate);
  else {
    Go(origLocation);
    if (fVerbose)
      cout << "Unable to find track.  Returned to original location.\n";
  }

  return candidate;
}


TrackNode *TrackCursor::FindNextParticle(const std::string &particleName)
{
  TrackTest_Particle test(particleName);
  return FindNextTrack(&test);
}


}  // namespace RAT
