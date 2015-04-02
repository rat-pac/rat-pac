/** @file GLG4HitPMT.cc
    For GLG4HitPMT class.
    
    This file is part of the GenericLAND software library.
    $Id: GLG4HitPMT.cc,v 1.2 2005/08/31 17:47:54 volsung Exp $

    @author Glenn Horton-Smith, December 2004
*/

#include "GLG4HitPMT.hh"
#include <algorithm>
#include <limits>
#include <G4ios.hh>

#ifdef G4DEBUG
#define IFDEBUG(A) A
#else
#define IFDEBUG(A)
#endif

/// controls when to start trying to merge HitPhotons
/// WARNING: Set to huge number to avoid suprising people
const size_t GLG4HitPMT::kApproxMaxIndividualHitPhotonsPerPMT 
    = std::numeric_limits<size_t>::max();
    
/// hit merging window in ns
const double GLG4HitPMT::kMergeTime= 1.0;

GLG4HitPMT::GLG4HitPMT(unsigned long ID)
{
  fID= ID;
}

GLG4HitPMT::~GLG4HitPMT()
{
  Clear();
}

/** clear out AND DELETE HitPhotons that were detected, resetting this
    HitPMT to have no HitPhotons */
void GLG4HitPMT::Clear()
{
  for (unsigned int i=0; i<fPhotons.size(); i++)
    delete fPhotons[i];
  fPhotons.clear();
}

/** Add HitPhoton, or try to merge with another HitPhoton when number
    of HitPhotons is bigger than kApproxMaxIndividualHitPhotonsPerPMT.
    
    If number of HitPhotons stored is less than
    kApproxMaxIndividualHitPhotonsPerPMT, just add it to the list.
    
    If number of HitPhotons stored is greater than or equal to
    kApproxMaxIndividualHitPhotonsPerPMT, sort the vector, then find
    the HitPhoton that immediately preceeds it in sorted vector.  If
    this HitPhoton's time is within kMergeTime of the preceeding
    HitPhoton, then add this HitPhoton's count to that closest
    preceeding HitPhoton.  Otherwise, look at following HitPhoton and
    merge with it if time is is within kMergeTime.  Otherwise, insert
    a new HitPhoton, allowing the size of the vector to grow beyond
    kApproxMaxIndividualHitPhotonsPerPMT.

    The time of "merged HitPhotons" is set to the time of the earliest
    HitPhoton in the merged set, because of the importance of the
    leading edge in the electronics and the analysis.  Due to the
    smallness of kMergeTime (1 ns), the effect is expected to be
    small.

    Note that GLG4HitPMT immediately "owns" any HitPhoton passed to
    DetectPhoton, and can delete the HitPhoton object at will.  In
    particular, the GLG4HitPhoton pointed to by the new_photon
    argument is deleted if merging with an existing HitPhoton occurs.

    @param new_photon  New HitPhoton to add (or merge).
*/  
void GLG4HitPMT::DetectPhoton(GLG4HitPhoton* new_photon) {
  if (fPhotons.size() < kApproxMaxIndividualHitPhotonsPerPMT) {
    fPhotons.push_back(new_photon);
  }
  else {
    SortTimeAscending();
    std::vector<GLG4HitPhoton*>::iterator it2, it1;
    it2= std::lower_bound(fPhotons.begin(), fPhotons.end(), new_photon,
			  Compare_HitPhotonPtr_TimeAscending);
    it1= it2;
    if ( it1 == fPhotons.begin() ) {
      // photon is earlier than any recorded so far -- always insert!
      fPhotons.insert(it1, new_photon);
    }
    else {
      it1--; // it1 photon should be earlier than photon to be added
      if ( new_photon->GetTime() - (*it1)->GetTime() < kMergeTime ) {
	// close to earlier photon -- merge with earlier photon
	IFDEBUG(if (new_photon->GetTime() - (*it1)->GetTime() < 0.0) G4cerr << "GLG4HitPMT STRANGE merge " << new_photon->GetTime() << " with non-earlier photon " << (*it1)->GetTime() << G4endl );
	(*it1)->AddCount( new_photon->GetCount() );
	delete new_photon;
	new_photon= 0;
      }
      else if ( it2 != fPhotons.end()
		&& (*it2)->GetTime() - new_photon->GetTime() < kMergeTime ) {
	// not after last photon, and close to later photon
	IFDEBUG(if ((*it2)->GetTime() - new_photon->GetTime() < 0.0) G4cerr << "GLG4HitPMT STRANGE merge " << new_photon->GetTime() << " with non-later photon " << (*it2)->GetTime() << G4endl);
	(*it2)->AddCount( new_photon->GetCount() );
	(*it2)->SetTime( new_photon->GetTime() );
	delete new_photon;
	new_photon= 0;
      }
      else {
	// after last photon or not close to any photon
	fPhotons.insert(it2, new_photon);
      }
    }
  }
  
}

/// sort HitPhotons so earliest are first
void GLG4HitPMT::SortTimeAscending() {
  std::sort(fPhotons.begin(), fPhotons.end(),
	    Compare_HitPhotonPtr_TimeAscending);
}

/// print out HitPhotons.
void GLG4HitPMT::Print(std::ostream &os, bool fullDetailsMode)
{
  os << " PMTID= " << fID
     << "  number of HitPhotons = " << fPhotons.size() << G4endl;
  if (fullDetailsMode==false) {
    for (size_t i=0; i<fPhotons.size(); i++)
      os << "  Hit time= " << fPhotons[i]->GetTime()
	 << " count= " << fPhotons[i]->GetCount() << G4endl;
  }
  else {
    for (size_t i=0; i<fPhotons.size(); i++) {
      os << "  Hit time= " << fPhotons[i]->GetTime() << G4endl;
      os << "      count= " << fPhotons[i]->GetCount() << G4endl;
      os << "      wavelength= " << fPhotons[i]->GetWavelength() << G4endl;
      double x,y,z;
      fPhotons[i]->GetPosition(x,y,z);
      os << "      position= " << x << " " << y << " " << z << G4endl;
      fPhotons[i]->GetMomentum(x,y,z);
      os << "      momentum= " << x << " " << y << " " << z << G4endl;
      fPhotons[i]->GetPolarization(x,y,z);
      os << "      polarization= " << x << " " << y << " " << z << G4endl;
    }
  }    
}


