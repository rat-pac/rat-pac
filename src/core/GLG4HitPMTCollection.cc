/** @file GLG4HitPMTCollection.cc
    For GLG4HitPMTCollection class.
    
    This file is part of the GenericLAND software library.
    $Id: GLG4HitPMTCollection.cc,v 1.1 2005/08/30 19:55:22 volsung Exp $

    @author Glenn Horton-Smith, December 2004
*/

#include "GLG4HitPMTCollection.hh"
#include <algorithm>
#include <G4ios.hh>

GLG4HitPMTCollection::GLG4HitPMTCollection()
{
}

GLG4HitPMTCollection::~GLG4HitPMTCollection()
{
  Clear();
}


/** clear out AND DELETE HitPMTs (and HitPhotons) that were detected,
    resetting this HitPMTCollection to be empty */
void GLG4HitPMTCollection::Clear()
{
  for(size_t i=0; i<fPMT.size(); i++){
    fPMT[i]->Clear();
    delete fPMT[i];
  }
  fPMT.clear();
  fHitmap.clear();
}

/** find or make appropriate HitPMT, and DetectPhoton in that HitPMT */
void GLG4HitPMTCollection::DetectPhoton(GLG4HitPhoton* new_photon)
{
  GLG4HitPMT* hitpmtptr= GetPMT_ByID( new_photon->GetPMTID() );

  if( hitpmtptr != NULL ) {
    // found a HitPMT with this ID
    hitpmtptr->DetectPhoton(new_photon);
  }
  else {
    // make a HitPMT with this ID
    fPMT.push_back( new GLG4HitPMT((unsigned long)new_photon->GetPMTID()) );
    fPMT[fPMT.size()-1]->DetectPhoton(new_photon);
    fHitmap.insert(
		   std::make_pair(
				  (short)new_photon->GetPMTID(),
				  (GLG4HitPMT*)fPMT[fPMT.size()-1]
				  )
		   );
  }
}

void GLG4HitPMTCollection::SortTimeAscending()
{ 
  for(size_t i=0; i<fPMT.size(); i++)
    fPMT[i]->SortTimeAscending();
  std::sort(fPMT.begin(), fPMT.end(), Compare_HitPMTPtr_TimeAscending ); 
}

/** return the number of HitPMTs in internal collection */
int GLG4HitPMTCollection::GetEntries() const
{
  return fPMT.size();
}

/** return the i-th HitPMT in internal collection */
GLG4HitPMT* GLG4HitPMTCollection::GetPMT(int i) const
{
  return fPMT[i];
}

/** return pointer to HitPMT with given id if in collection,
    or NULL if no such HitPMT is in collection */
GLG4HitPMT* GLG4HitPMTCollection::GetPMT_ByID(int id) const
{
  std::map<short, GLG4HitPMT*> :: const_iterator p = fHitmap.find((short)id);
  if( p != fHitmap.end() ) {
    // found a HitPMT with this ID
    return p->second;
  }
  else {
    // no HitPMT
    return NULL;
  }
}

/// print out HitPMTs
void GLG4HitPMTCollection::Print(std::ostream &os) const
{
  for(size_t i=0; i<fPMT.size(); i++) {
    fPMT[i]->Print(os);
  }
}
