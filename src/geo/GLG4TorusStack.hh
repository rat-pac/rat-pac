// This file is part of the GenericLAND software library.
// $Id: GLG4TorusStack.hh,v 1.1 2005/08/30 19:55:22 volsung Exp $
//
// This code partly derived from the intellectual property of
// the RD44 GEANT4 collaboration.
//
// By copying, distributing or modifying the Program (or any work
// based on the Program) you indicate your acceptance of this statement,
// and all its terms, whatever they may be.
//
// 
// class GLG4TorusStack
//
// A stack of torii sliced normal to the Z axis, with curved
// sides parallel to the z-axis. Each torus has a certain swept
// radius about which it is centered, and a certain cross-sectional
// radius.  A negative cross-sectional radius means to use the "inner"
// surface of the torus as the boundary of the toroid stack.
// A zero cross-sectional radius may be used to indicate a cylinder.
// The complete stack is filled and complete in phi angle.
// The torii are actually specified in terms of the z and "rho" coordinates
// of the edges of the torii and the z coordinate of each segment's plane of
// symmetry; the swept radii ("a") and torii radii ("b")
// are calculated accordingly.
//
//
// Member functions:
//
// As inherited from G4CSGSolid+
//
// GLG4TorusStack(const G4String      &pName )
//
//   Construct an empty TorusStack with the given name.
//
//
//  void SetAllParameters
//  (G4int n_,                   // number of Z-segments
//   const G4double z_edge_[ ],  // n+1 edges of Z-segments
//   const double rho_edge_[ ],  // n+1 dist. from Z-axis
//   const G4double z_o_[ ] )    // z-origins (n total)
//
//    Define number of torii in stack and the dimensions of each.
//
// ****************************************************************/
//
// Protected:
//
// G4ThreeVectorList*
// CreateRotatedVertices(const G4AffineTransform& pTransform) const
//
//   Create the List of transformed vertices in the format required
//   for G4VSolid:: ClipCrossSection and ClipBetweenSections.
//   
//
// 1999.11.22 G.Horton-Smith First version of GLG4TorusStack


#ifndef GLG4TorusStack_HH
#define GLG4TorusStack_HH

#include "G4CSGSolid.hh"

class GLG4TorusStack : public G4CSGSolid {
public:
    GLG4TorusStack(const G4String &pName);

    virtual ~GLG4TorusStack();
    
    void SetAllParameters(G4int n,                   // number of Z-segments
			  const G4double z_edge[ ],  // n+1 edges of Z-segments
			  const G4double rho_edge[ ],// n+1 dist. from Z-axis
			  const G4double z_o[ ],     // tor z-origins (n total)
			  GLG4TorusStack *inner=0);    // inner TorusStack (opt.)
 
    void ComputeDimensions(G4VPVParameterisation* p,
                           const G4int n,
                           const G4VPhysicalVolume* pRep);
			   
    G4bool CalculateExtent(const EAxis pAxis,
			   const G4VoxelLimits& pVoxelLimit,
			   const G4AffineTransform& pTransform,
			   G4double& pmin, G4double& pmax) const;

    G4double    GetN() const { return n ; }
    G4double    GetZEdge(int i) const { return z_edge[i] ; }
    G4double    GetZo(int i) const { return z_o[i] ; }
    G4double    GetA(int i) const { return a[i] ; }
    G4double    GetB(int i) const { return b[i] ; }

    EInside Inside(const G4ThreeVector& p) const;

    G4ThreeVector SurfaceNormal( const G4ThreeVector& p) const;

    G4double DistanceToIn(const G4ThreeVector& p,const G4ThreeVector& v) const;
    G4double DistanceToIn(const G4ThreeVector& p) const;
    G4double DistanceToOut(const G4ThreeVector& p,const G4ThreeVector& v,
			   const G4bool calcNorm=G4bool(false),
			   G4bool *validNorm=0,G4ThreeVector *n=0) const;
    G4double DistanceToOut(const G4ThreeVector& p) const;

    // Naming method (pseudo-RTTI : run-time type identification)
    virtual G4GeometryType  GetEntityType() const
      { return G4String("GLG4TorusStack"); }

    // Visualisation functions
    void                DescribeYourselfTo (G4VGraphicsScene& scene) const ;
    G4VisExtent         GetExtent          () const ;
    G4Polyhedron*       CreatePolyhedron   () const ;

    // other generally useful functions -- public so others can use them!

    // find first torus intersection -- s == distance from p along v
    static G4int FindFirstTorusRoot (
                      G4double a,             // swept radius
		      G4double b,             // cross-section radius
		      const G4ThreeVector& p, // start point of ray
		      const G4ThreeVector& v, // direction of ray
		      G4double smin,          // lower bracket on root
		      G4double smax,          // upper bracket on root
		      G4bool fEntering,       // true if looking for out->in 
		      G4double &sout ) ;      // distance to root, if found

    // find first root of arbitrary function in bracketed interval
    class RootFinder {
    public:
      virtual void f_and_Df(G4double x, G4double &f, G4double &Df) = 0;
      //
      // f_and_Df must be overridden by user to be function which sets
      // f and Df to the value of the function and the derivative w.r.t.
      // x of the function, respectively, for a given x
      //
      int FindRoot(G4double smin,             // lower bound on root
		   G4double smax,             // uppper bound on root
		   G4double tol,              // tolerance on root
		   G4bool   fFindFallingRoot, // true == "want dF<0 at root"
		                              // false == "want dF>0 at root"
		   G4double &sout);           // returned root
      //
      // FindRoot returns 1 if root found, 0 if no root found.
      //
    };

    // return integer i such that z[i] <= z_lu < z[i+1]
    // or -1 if z_lu < z[0] or z_lu >= z[n-1]
    static G4int FindInOrderedList(double z_lu, const double *z, int n);

    // a helpful function that doesn't hurt anything to call:
    G4int FindNearestSegment(G4double pr, G4double pz) const ;

protected:
    
    G4ThreeVectorList*
    CreateRotatedVertices(const G4AffineTransform& pTransform,
                          G4int& noPolygonVertices) const;

    EInside Inside1(const G4ThreeVector& p) const;

    void CheckABRho();
  
    int n;           // number of Z-segments
    double *z_edge;  // n+1 edges of Z-segments
    double *rho_edge;// n+1 2-d distance from Z-axis at each edge
    double *z_o;     // z-origins, one for each toroid segment (n total)
    double *a;       // swept radii, one for each toroid segment (n total)
    double *b;       // torus radii, one for each toroid segment (n total)
    double max_rho;  // maxium distance of surface from Z axis
    double myRadTolerance; // because Geant4.1.0 default is too small
    static double surfaceTolerance; // in GEANT4.9.0 it is not a global const
    static double radTolerance; // in GEANT4.9.0 it is not a global const
    static double angTolerance; // in GEANT4.9.0 it is not a global const
    GLG4TorusStack *inner;  // because G4SubtractionSolid is bad
};
   	
#endif
