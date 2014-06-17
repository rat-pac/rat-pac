#ifndef __RAT_TrackNav__
#define __RAT_TrackNav__

#include <RAT/DS/Root.hh>
#include <RAT/TrackNode.hh>
#include <RAT/TrackCursor.hh>
#include <map>

namespace RAT {

class TrackNav {
public:
  TrackNav(RAT::DS::Root *ds, bool verbose=false)
      { Load(ds->GetMC(), verbose); };
  TrackNav(RAT::DS::MC *mc, bool verbose=false)
      { Load(mc, verbose); };
  void Load(RAT::DS::MC *mc, bool verbose=false);
  void Clear();

  TrackNode *Head() { return fHead; };
  TrackCursor Cursor(bool verbose=false) { return TrackCursor(fHead, verbose); };

  TrackNode *FindID(int trackID);
  TrackNode *FindParticle(const std::string &particleName, bool verbose=false);

protected:
  TrackNode *fHead;
  std::map<int,TrackNode *> fTracks; // Access by Track ID
                                     // point to first node of each track
  
};


} // namespace RAT

#endif
