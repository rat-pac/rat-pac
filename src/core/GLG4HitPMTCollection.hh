#ifndef __GLG4HitPMTCollection_hh__
#define __GLG4HitPMTCollection_hh__
/** @file GLG4HitPMTCollection.hh
    Declares GLG4HitPMTCollection class and helper functions.
    
    This file is part of the GenericLAND software library.
    $Id: GLG4HitPMTCollection.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
    
    @author Glenn Horton-Smith, December 2004
*/

#include "GLG4HitPMT.hh"
#include <vector>
#include <map>

/** GLG4HitPMTCollection stores GLG4HitPMT objects.

    The general contract for GLG4HitPMTCollection is as follows:
      - efficiently keep a bunch of GLG4HitPMTs with GLG4HitPhotons
      - provide Clear, DetectPhoton, SortTimeAscending, GetEntries, GetPMT,
        and Print(ostream &) functions
      - note the GLG4HitPhotons passed to DetectPhoton will become the
        "owned" by the GLG4HitPMTCollection and its GLG4HitPMTs, and they
	will take care of deletion.

    This is almost the same "general contract" that was implemented
    for KLG4sim's KLHitPMTCollection by O. Tajima and G. Horton-Smith,
    but the code was rewritten for GLG4sim in December 2004.

    @author Glenn Horton-Smith
*/

class GLG4HitPMTCollection {
public:
  GLG4HitPMTCollection();
  virtual ~GLG4HitPMTCollection();

  void Clear();
  void DetectPhoton(GLG4HitPhoton*);
  void SortTimeAscending();
  int GetEntries() const;
  GLG4HitPMT* GetPMT(int i) const;
  GLG4HitPMT* GetPMT_ByID(int id) const;

  void Print(std::ostream &) const;

private:
  std::vector<GLG4HitPMT*> fPMT;
  std::map<int, GLG4HitPMT*> fHitmap;  
};


#endif // __GLG4HitPMTCollection_hh__
