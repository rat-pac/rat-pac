// This file is part of the GenericLAND software library.
// $Id: GLG4TestSolid.cc,v 1.1 2005/08/30 19:55:23 volsung Exp $
//
/*
  class GLG4TestSolid

  Encapsulates routines for testing a Geant4 solid for consistency.

  G4String  Test(const G4VSolid &s, G4int npair)
    calls CalculateExtent() for null affine transform for each axis
    calls GetExtent and checks for consistency, < kInfinity on all axes
    chooses pairs of points and calls TestRay for vectors between these points
       using PickRandomPoint (see below)
    repeats for npair pairs
    returns empty string if passed all tests, error message otherwise

  G4String  TestRay(const G4VSolid &s,
                 const G4ThreeVector &p, const G4ThreeVector &v)
    calls Inside(p) -- should be kOutside, kInside, or kSurface
    if kOutside,
      calls DistanceToIn(p)   -- save value as "safe," should be <= p.abs()
      calls DistanceToIn(p,v) -- dist should be >= "safe"
    else if kInside
      calls DistanceToOut(p)  -- save value as "safe," should be <= max extent
      calls DistanceToOut(p,v) -- dist should be >= "safe", <= max extent
        (max extent checked below) -- also check normal
    else
      dist= 0.0;
    end
    compute an AffineTransform a which makes v be (0,0,1) and p be (0,0,0)
    calls CalculateExtent(kZaxis,
                          [+/-10*kRadTol limits on X and Y],
                          a.inverse,
			  din_first, dout_last )
    if p is not within the maximum extent, should have din_first >= 0.0
    calls TestAtSurface(p+dist*v)


  G4String  TestAtSurface(const G4VSolid &s,
                       const G4ThreeVector &p1)
    calls Inside(p1) -- should be "kSurface"
    calls SurfaceNormal(p1) --> norm, should have non-zero magnitude,
    calls Inside(p1 + 2.0*kRadTol*norm) -- should be "kOutside"
    calls Inside(p1 - 2.0*kRadTol*norm) -- should be "kInside"
    forms e1 and e2, perpendicular to norm and each other
    calls Inside(p1 [+/-] 2.0*kRadTol*e[1/2]) -- should be "kSurface"
    calls DistanceToIn(p1 + 2.0*kRadTol*norm + i*2.0*kRadTol*e1, -norm)
       for i=-1,0,+1
         -- should be (1.9 to 2.1)*kRadTol for i=0
	 -- should be (1.0 to 3.0)*kRadTol for i=+/-1
	 -- should be same number (+/- 0.01*kRadTol) for i=+1 and i=-1
	    (by definition of "norm")
	 -- also calls non-directional "DistanceToIn" and checks safe
    repeat above for e2
    repeat above using p1 - 2.0*kRadTol*norm and "DistanceToOut" functions
         -- do all above tests, plus test returned normal for validity
    call DistanceToIn(p1, -norm)
         -- should return small number
    call DistanceToOut(p1, norm)
         -- should return small number

  G4ThreeVector PickRandomPoint(G4VisExtent &ve, G4int itype= -1)
    3 point choice styles: uniform in box volume, uniform on box surface,
                           equal prob to each box surface, uniform on surface
     -- itype=-1 means equal probability for each of the three styles 

  Author: Glenn Horton-Smith (Tohoku) 1999.12.09
  Last modified: 2000.08.09 (G.H-S.)
*/

#include "globals.hh"

#include "GLG4TestSolid.hh"
#include "G4VSolid.hh"
#include "G4VisExtent.hh"
#include "G4VoxelLimits.hh"
#include "G4AffineTransform.hh"
#include "G4GeometryTolerance.hh"

#include "local_g4compat.hh"

#include <sstream>

#define GLG4TESTSOLID_DEBUG 1

// #if defined(__GNUC__) && (__GNUC__ < 3)
// [rayd] renamed hypot as glg4_hypot, to avoid problems with gcc 2.96
static inline double glg4_hypot(double x, double y) { return sqrt(x*x+y*y); }
// #endif

static G4std::ostream& myenderr(G4std::ostream& outs)
{
  return outs << G4endl;
}

static G4String MakeErrString(G4std::ostringstream & errstr)
{
  errstr << G4std::ends;
  std::string st; 
  st = errstr.str(); 
  const char* cp = st.c_str(); 
  if (cp == NULL) {
    return G4String("");
  }
  else {
    G4String rv(cp);
    return rv;
  }
}

static G4VoxelLimits voxel_limit_pencil;

G4String  GLG4TestSolid::Test(const G4VSolid &s, G4int npair)
{
  double radTolerance = G4GeometryTolerance::GetInstance()->GetRadialTolerance();

  G4std::ostringstream errmsg;
  G4AffineTransform a;
  G4VoxelLimits vl;
  G4double xmin, xmax, ymin, ymax, zmin, zmax;
  G4VisExtent extent;

  ResetRandomPointSeed(0);

  // get extents from CalculateExtent and GetExtent
  s.CalculateExtent(kXAxis, vl, a, xmin, xmax);
  s.CalculateExtent(kYAxis, vl, a, ymin, ymax);
  s.CalculateExtent(kZAxis, vl, a, zmin, zmax);
  extent= s.GetExtent();

  // check for validity
  if (xmax < xmin || ymax < ymin || zmax < zmin)
    errmsg << "CalculateExtent internal inconsistency" << myenderr;
  if (extent.GetXmax() < extent.GetXmin()
      || extent.GetYmax() < extent.GetYmin() 
      || extent.GetZmax() < extent.GetZmin() )
    errmsg << "GetExtent internal inconsistency" << myenderr;
  if (xmax < extent.GetXmin() || xmin > extent.GetXmax())
    errmsg << "CalculateExtent vs. GetExtent incompatibility X" << myenderr;
  if (ymax < extent.GetYmin() || ymin > extent.GetYmax())
    errmsg << "CalculateExtent vs. GetExtent incompatibility Y" << myenderr;
  if (zmax < extent.GetZmin() || zmin > extent.GetZmax())
    errmsg << "CalculateExtent vs. GetExtent incompatibility Z" << myenderr;
  if (errmsg.str().size() > 0)
    return MakeErrString(errmsg);

  // widen extent to encompass all
  if (xmin < extent.GetXmin()) extent.SetXmin(xmin);
  if (ymin < extent.GetYmin()) extent.SetYmin(ymin);
  if (zmin < extent.GetZmin()) extent.SetZmin(zmin);
  if (xmax > extent.GetXmax()) extent.SetXmax(xmax);
  if (ymax > extent.GetYmax()) extent.SetYmax(ymax);
  if (zmax > extent.GetZmax()) extent.SetZmax(zmax);

  // check for finiteness
  G4double dx= xmax-xmin;
  G4double dy= ymax-ymin;
  G4double dz= zmax-zmin;
  if (dx < 0.0 || dx >= kInfinity)
    errmsg << "Bad X extent" << myenderr;
  if (dy < 0.0 || dy >= kInfinity)
    errmsg << "Bad Y extent" << myenderr;
  if (dz < 0.0 || dz >= kInfinity)
    errmsg << "Bad Z extent" << myenderr;
  if (errmsg.str().size() > 0)
    return MakeErrString(errmsg);

  // set myCheckTolerance
  G4double myCheckTolerance= G4std::max(radTolerance,
					1e-6*G4std::max(dx,G4std::max(dy,dz)));

  // pick pairs of points
  G4int nerr= 0;
  G4int ipair;
  G4int lastpcount= 0;
  for (ipair=0; ipair<npair && nerr < 10; ipair++)
    {
      G4ThreeVector p1,p2,v;

      p1= PickRandomPoint(extent);
      p2= PickRandomPoint(extent);

      if (p1 == p2) {
	p1= PickRandomPoint(extent);
	p2= PickRandomPoint(extent);
	if (p1 == p2) {
	  errmsg << "Successive points are identical -- RNG error???" << myenderr;
	  nerr++;
	  break;
	}
      }

      v= (p2-p1).unit();

      errmsg << TestRay(s, p1, v, myCheckTolerance);
      
      if ((int) errmsg.str().size() > lastpcount) {
	errmsg << "Above errors going from " << p1 << " to " << p2
	       << "\n\t x=" << p1.x() << "; y=" << p1.y() << "; z=" << p1.z()
	       << "\n\t dx="<< v.x()  << ";dy=" << v.y()  << ";dz=" << v.z()
	       << "\n\t ipair=" << ipair << "(a)\n" << myenderr;
	nerr++;
	lastpcount= errmsg.str().size();
      }

      v= -v;
      errmsg << TestRay(s, p2, v, myCheckTolerance);

      if ((int) errmsg.str().size() > lastpcount) {
	errmsg << "Above errors going from " << p2 << " to " << p1
	       << "\n\t x=" << p2.x() << "; y=" << p2.y() << "; z=" << p2.z()
	       << "\n\t dx="<< v.x()  << ";dy=" << v.y()  << ";dz=" << v.z()
	       << "\n\t ipair=" << ipair << "(b)\n" << myenderr;	  
	nerr++;
	lastpcount= errmsg.str().size();
      }
      
    }
  if (ipair < npair) {
    errmsg << "Aborted after " << ipair << " pairs, " << nerr << " errors\n";
  }
  else
    errmsg << "Finished " << npair << " loops with " << nerr << " errors\n";

  return MakeErrString(errmsg);
}


G4String  GLG4TestSolid::TestRay(const G4VSolid &s,
			       const G4ThreeVector &p,
			       const G4ThreeVector &v,
			       G4double myCheckTolerance)
{
  G4std::ostringstream errmsg;
  EInside whereami;
  G4double safe=0.0;
  G4double dist=0.0;

  whereami= s.Inside(p);
  switch (whereami)
    {
    case kOutside:
      safe= s.DistanceToIn(p);
      dist= s.DistanceToIn(p,v);
      // if (safe > p.mag())
      //   errmsg << "\"safe\" DistanceToIn(p) > p.mag() !" << myenderr;
      break;

    case kInside:
      safe= s.DistanceToOut(p);
      if (safe >= kInfinity)
	errmsg << "\"safe\" DistanceToOut(p) >= kInfinity !" << myenderr;
      {
	G4ThreeVector norm;
	G4bool validNorm;
	dist= s.DistanceToOut(p,v,true,&validNorm, &norm);
	// if norm is valid, should have some comp. along incidence vector
	if (validNorm && norm*v <= 0.0) {
	  errmsg << "DistanceToOut returned norm flagged as valid which "
	    "points opposite to ray direction vector!" << myenderr;
	}
      }
      break;

    case kSurface:
      safe= 0.0;
      dist= 0.0;
      break;

    default:
      errmsg << "Invalid return value from Inside!" << myenderr;
      break;
    }

  // check validity
  if (dist < safe)
    errmsg << "DistanceToIn(p,v) < \"safe\" DistanceToIn(p) !"
	   << " (dist=" << dist << " safe=" << safe << ")"
	   << myenderr;

  if (errmsg.str().size() > 0)
    return MakeErrString(errmsg);

  // comput affine transform
  G4RotationMatrix rot;
  G4double rho= glg4_hypot(v.x(), v.y());
  if (rho > 0.0)
    rot.rotateZ(-atan2(v.y(),v.x()));
  rot.rotateY(-atan2(rho,v.z()));

  G4AffineTransform a(rot, p);

#ifdef GLG4TESTSOLID_DEBUG
  G4AffineTransform ai= a.Inverse();
  G4ThreeVector pt= ai.TransformPoint(p);
  G4ThreeVector vt= ai.TransformAxis(v);
  if (pt.mag() > myCheckTolerance || fabs(vt.mag()-1.0) > 0.001
      || fabs(vt.z()-1.0) > 0.001) {
    G4cerr << "ERROR, failed internal test in GLG4TESTSOLID_DEBUG" << myenderr;
    G4cerr.flush();
  }
#endif

  // call CalculateExtent for defined transform
  G4bool intersect_ce;
  G4double din_ce, dout_ce;
  if (!voxel_limit_pencil.IsLimited()) {
    voxel_limit_pencil.AddLimit(kXAxis, -8*myCheckTolerance, 8*myCheckTolerance);
    voxel_limit_pencil.AddLimit(kYAxis, -8*myCheckTolerance, 8*myCheckTolerance);
  }
  intersect_ce= s.CalculateExtent(kZAxis,
				voxel_limit_pencil,
				a.Inverse(),
				din_ce,
				dout_ce);

  // check validity
  if (intersect_ce == false) {
    if (whereami == kInside || whereami == kSurface) {
      errmsg << "CalculateExtent shows no intersection for region containing"
	" known interior point!" << myenderr;
    }
    else if (dist < kInfinity) {
      errmsg << "CalculateExtent shows no intersection for pencil beam "
	"when DistanceToIn shows intersection for ray!" << myenderr;
    }
  }
  else {
    if (whereami == kInside || whereami == kSurface) {
      // if we're inside, expect din_ce <= 0.0 and dout_ce >= dist
      if (din_ce > 0.0) {
	errmsg << "CalculateExtent range does not include interior point!" << myenderr;
      }
      if (dist > dout_ce) {
	errmsg << "CalculateExtent range does not include surface point"
	  " given by DistanceToOut!" << myenderr;
      }
    }
    else if (dist < kInfinity) {
      // if we're outside and have an intersection from DistanceToIn,
      // then expect dout_ce >= dist and din_ce <= dist
      // also expect dout_ce < kInfinity
      if (dist > dout_ce || din_ce > dist) {
	errmsg << "CalculateExtent range does not include surface point"
	  " given by DistanceToIn!" << myenderr;
      }
      if (dout_ce >= kInfinity) {
	errmsg << "CalculateExtent claims intersection, but range extends to infinity!" << myenderr;
      }
    }
    else {
      // if we're outside and have no intersection from DistanceToIn
      // but do have an intersection from CalculateExtent,
      // maybe that's okay, but check to make sure
      if (din_ce < 0.0)
	din_ce= 0.0;
      G4double dd= (dout_ce-din_ce)/32.;
      for (G4double d=din_ce+dd/2.0; d<dout_ce; d+=dd) {
	if ( s.Inside(p+d*v) == kInside ) {
	  errmsg << "DistanceToIn found no intersection, but I found an "
		 << "interior point at " << p+d*v << myenderr;
	  break;
	}
      }
    }
    //    if (errmsg.str().size() > 0)
    //      return MakeErrString(errmsg);

    if (dist < kInfinity)
      errmsg << TestAtSurface(s, p+dist*v, myCheckTolerance);
  }

  return MakeErrString(errmsg);
}


G4String  GLG4TestSolid::TestAtSurface(const G4VSolid &s,
				     const G4ThreeVector &p1,
				     G4double myCheckTolerance)
{
  double radTolerance = G4GeometryTolerance::GetInstance()->GetRadialTolerance();
  G4std::ostringstream errmsg;
  G4ThreeVector norm;

  if (s.Inside(p1) != kSurface) {
    errmsg << "TestAtSurface point is not at surface!" << myenderr;
    return MakeErrString(errmsg);
  }

  norm= s.SurfaceNormal(p1);
  if ( fabs(norm.mag2()-1.0) > 1e-6 ) {
    if (norm.mag2() <= 1e-6) {
      errmsg << "SurfaceNormal returned zero-magnitude vector!" << myenderr;
      return MakeErrString(errmsg);
    }
    errmsg << "SurfaceNormal returned un-normalized vector." << myenderr;
    norm = norm.unit();
  }

  if (s.Inside(p1 + 2*myCheckTolerance*norm) != kOutside) {
    errmsg << "Following normal doesn't get you outside!" << myenderr;
  }
  if (s.Inside(p1 - 2*myCheckTolerance*norm) != kInside) {
    errmsg << "Following -normal doesn't get you inside!" << myenderr;
  }

  G4ThreeVector e[2];
  e[0]= norm.orthogonal().unit();
  e[1]= norm.cross(e[0]);

  for (int idir=0; idir<=1; idir++) {
    G4double disti[3], disto[3];
    if (s.Inside(p1 + 2*radTolerance*e[idir]) != kSurface) {
      errmsg << "Moved outside surface traveling perpindicular to normal!" << myenderr;
    }
    if (s.Inside(p1 - 2*radTolerance*e[idir]) != kSurface) {
      errmsg << "moved outside surface traveling perpindicular to normal!" << myenderr;
    }
    // find distances to surface for various points near this one
    for (int ioffs=-1; ioffs<=1; ioffs++) {
      const G4double surfCheckDisplFactor= 8.0;
      G4ThreeVector pos= p1 + surfCheckDisplFactor*myCheckTolerance*norm
			    + ioffs*surfCheckDisplFactor*myCheckTolerance*e[idir];
      if (s.Inside(pos) != kOutside) {
	errmsg << "Not outside when expected in surface-checking loop" << myenderr;
      }
      disti[ioffs+1]= s.DistanceToIn(pos, -norm);
      
      pos= p1 - surfCheckDisplFactor*myCheckTolerance*norm + ioffs*surfCheckDisplFactor*myCheckTolerance*e[idir];
      if (s.Inside(pos) != kInside) {
	errmsg << "Not inside when expected in surface-checking loop" << myenderr;
      }      
      disto[ioffs+1]= s.DistanceToOut(pos, norm);
      // check these distances for consistency
      if ( fabs(disti[ioffs+1]-surfCheckDisplFactor*myCheckTolerance)
	   > (ioffs ? 1.5 : 0.25)*myCheckTolerance ) {
	errmsg << "Distance to in just outside surface is " << disti[ioffs+1]
	       << ", ioffs=" << ioffs << myenderr;
      }
      if ( fabs(disto[ioffs+1]-surfCheckDisplFactor*myCheckTolerance)
	   > (ioffs ? 1.5 : 0.25)*myCheckTolerance ) {
	errmsg << "Distance to out just inside surface is " << disto[ioffs+1]
	       << ", ioffs=" << ioffs << myenderr;
      }
      if ( fabs(disto[ioffs+1]+disti[ioffs+1]-2.0*surfCheckDisplFactor*myCheckTolerance)
	   > 0.125*myCheckTolerance ) {
	errmsg << "Sum of disti and disto on opposite sides of surface is"
	       " inconsistent!" << myenderr;
      }
    }
    // distance to surface should be same on opposite tangent offsets of normal
    if (fabs(disto[0]-disto[2]) > myCheckTolerance/32.) {
      errmsg << "Normal doesn't look like interior normal!" << myenderr;
    }
    if (fabs(disti[0]-disti[2]) > myCheckTolerance/32.) {
      errmsg << "Normal doesn't look like exterior normal!" << myenderr;
    }
  }

  // test for already-inside / already-outside cases
  {
    G4double dist0;
    dist0= s.DistanceToIn(p1, -norm);
    if (fabs(dist0) > myCheckTolerance)
      errmsg << "Failed already-inside test at surface, " << dist0 << myenderr;
//    dist0= s.DistanceToIn(p1 - 2.0*myCheckTolerance*norm, -norm);
//    if (dist0 != 0.0)
//	errmsg << "Failed already-inside test inside, " << dist0 << myenderr;
    dist0= s.DistanceToOut(p1, norm);
    if (fabs(dist0) > myCheckTolerance)
      errmsg << "Failed already-outside test at surface, " << dist0 <<myenderr;
//    dist0= s.DistanceToOut(p1 + 2.0*myCheckTolerance*norm, norm);
//    if (dist0 != 0.0)
//      errmsg << "Failed already-outside test at outside, " << dist0 <<myenderr;
  }
  
  if (errmsg.str().size() > 0)
    errmsg << "\t-- TestAtSurface(solid, " << p1 << ", " <<  myCheckTolerance
	   << ")" << myenderr;

  return MakeErrString(errmsg);
}

#include <limits.h>
#if (UINT_MAX != 4294967295U)
#error "Sorry, but I can't handle a compiler where ints are not 32-bit"
#endif

static unsigned int rndqd32_ulseed=0;
static const double rnd2tom32=2.32830643654e-10;
static inline double rndqd32(void)
{ return ((double)(rndqd32_ulseed=1664525U*rndqd32_ulseed+1013904223U)
	  *rnd2tom32);
}


void GLG4TestSolid::ResetRandomPointSeed(G4int newiseed)
{
  rndqd32_ulseed= (unsigned int)(newiseed)|1U;
  for (int i=0; i<37; i++)
    rndqd32();
}

G4ThreeVector GLG4TestSolid::PickRandomPoint(G4VisExtent &ve,
					   G4int itype)
{
  G4double dx, dy, dz;
  G4ThreeVector rv;
  int isurf=0;
  
  if (itype < 0) {
    itype= (int)(rndqd32()*3.0);
  }
  
  dx= (ve.GetXmax()-ve.GetXmin());
  dy= (ve.GetYmax()-ve.GetYmin());
  dz= (ve.GetZmax()-ve.GetZmin());
  
  switch (itype%3) {
  case 0: // random in volume
    rv= G4ThreeVector( rndqd32()*dx + ve.GetXmin(),
		       rndqd32()*dy + ve.GetYmin(),
		       rndqd32()*dz + ve.GetZmin() );
    return rv;
  case 1: // random on surface, same density on all surfaces
    {
      G4double axy= dx*dy;
      G4double a_xy_yz= axy+dy*dz;
      G4double atot= a_xy_yz + dz*dx;
      G4double au= atot*rndqd32();
      if (au < axy)
	isurf= (au < 0.5*axy) ? 0 : 3;
      else if (au < a_xy_yz)
	isurf= (au < 0.5*(axy+a_xy_yz)) ? 1 : 4;
      else
	isurf= (au < 0.5*(a_xy_yz+atot)) ? 2 : 5;
      break;
    }
  case 2: // random on random surface, same total probability for each face
    isurf= (int)(rndqd32()*6.0);
    break;
  }
  switch (isurf%3) {
  case 0: // xy
    rv= G4ThreeVector( rndqd32()*dx + ve.GetXmin(),
		       rndqd32()*dy + ve.GetYmin(),
		       (isurf/3)*dz + ve.GetZmin());
    break;
  case 1: // yz
    rv= G4ThreeVector( (isurf/3)*dx + ve.GetXmin(),
		       rndqd32()*dy + ve.GetYmin(),
		       rndqd32()*dz + ve.GetZmin());
    break;
  case 2: // zx
    rv= G4ThreeVector( rndqd32()*dx + ve.GetXmin(),
		       (isurf/3)*dy + ve.GetYmin(),
		       rndqd32()*dz + ve.GetZmin());
    break;
  }
  return rv;
}
