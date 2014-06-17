// This file is part of the GenericLAND software library.
// $Id: GLG4PosGen.hh,v 1.2 2005/11/19 17:59:39 volsung Exp $
//
// GenericLAND global position generator for primary events,
// by G.Horton-Smith, August 3, 2001
// (See note on GenericLAND generators for more information.)
#ifndef __GLG4PosGen_h__
#define __GLG4PosGen_h__ 1

#include "globals.hh"
class G4PrimaryVertex;
class G4VPhysicalVolume;
class G4Material;

#include <vector>
#include <G4ThreeVector.hh>

class GLG4PosGen {
public:
  GLG4PosGen(const char *arg_dbname="pos") : _dbname(arg_dbname) { }
  virtual ~GLG4PosGen() { }

  virtual void GeneratePosition( G4ThreeVector &argResult ) = 0;
  // Select position for event
  
  virtual void SetState( G4String newValues ) = 0;
  // sets filename or other information needed by global position generator
  
  virtual G4String GetState() const = 0;
  // returns the current state information in a form that can be understood
  // by SetState (and, hopefully, a well-informed human)

protected:
  G4String _dbname; // used for GLG4param key prefix
};

class GLG4PosGen_Point : public GLG4PosGen {
public:
  GLG4PosGen_Point(const char *arg_dbname="point");
  virtual void GeneratePosition( G4ThreeVector &argResult );
  // Generates a position either at a fixed point in the global coordinates
  void SetState( G4String newValues );
  // newValues == x y z coordinates in mm (separated by white space),
  G4String GetState() const;
  // returns current state in format above
protected:
  G4ThreeVector  _fixedPos;
};

class GLG4PosGen_Paint : public GLG4PosGen {
public:
  GLG4PosGen_Paint(const char *arg_dbname="paint");
  virtual void GeneratePosition( G4ThreeVector &argResult );
  // Generates a position uniformly over the surface of the volume
  // which containst he given point.
  // (This approach to specifying the volume isn't my favorite, but
  // it is strongly motivated by subtleties in Geant4's geometry code.)
  void SetState( G4String newValues );
  // newValues == x y z coordinates in mm (separated by white space),
  // optionally followed by name of physical volume expected at that position,
  // optionally followed by thickness of coat of "paint" (external to volume).
  // optionally followed by name of material to which to restrict "paint".
  G4String GetState() const;
  // returns current state in format above
protected:
  G4ThreeVector _pos;
  G4double _thickness;
  G4String _pVolumeName;
  G4VPhysicalVolume* _pVolume;
  G4String _materialName;
  G4Material* _material;
  int _ntried;
  int _nfound;
  G4double _boundingBoxVolume;
  std::vector<G4ThreeVector> _intercepts;

};

class GLG4PosGen_Fill : public GLG4PosGen {
public:
  GLG4PosGen_Fill(const char *arg_dbname="fill");
  virtual void GeneratePosition( G4ThreeVector &argResult );
  // Generates a position uniformly filling the volume which contains 
  // the given point.
  // (This approach to specifying the volume isn't my favorite, but
  // it is strongly motivated by subtleties in Geant4's geometry code.)
  // - A random point in a compact physical volume is pretty fast.
  // - A volume which only sparsely fills its geometric "extent" may
  //   require many iterations to find an internal point -- this will be slow.
  void SetState( G4String newValues );
  // newValues == x y z coordinates in mm (separated by white space),
  // optionally followed by name of physical volume expected at that position;
  G4String GetState() const;
  // returns current state in format above
protected:
  G4ThreeVector  _pos;
  G4String _pVolumeName;
  G4VPhysicalVolume* _pVolume;
  G4String _materialName;
  G4Material* _material;
  int _ntried;
  int _nfound;
  G4double _boundingBoxVolume;
};



class GLG4PosGen_Cosmic : public GLG4PosGen {
public:
  GLG4PosGen_Cosmic(const char *arg_dbname);
  virtual void GenerateVertexPositions( G4PrimaryVertex *argVertex,
					double max_chain_time,
					double event_rate,
					double dt=0.0
					);
  // external flux uniformly distributed over area normal to
  // incident direction of first track in vertex
  void GeneratePosition(G4ThreeVector &);  // (not used)
  void SetState( G4String newValues );
  // newValues == "width height"
  //  width == width of rectangular area normal to incident direction (mm)
  //  height == height of rectangular area normal to incident direction (mm)
  // Appropriate values for GenericLAND would be 20000 33000.
  // The rectangle is rotated so that the "width" direction vector lies in
  // the XY plane for non-zero polar angle of the incident track.
  // If a track is generated which completely misses the detector,
  // the PDG code of the vertex tracks are modified to make them have
  // an impossible effective ISTHEP codes of 1, so Geant4 does not attempt
  // to track them.
  // This means the generated external flux in Hz/mm**2 is always
  //     flux = (rate/(width*height)),
  // regardless of the geometry of the detector, where "rate" is the rate
  // set via /generators/rate.  The "rate" must be chosen appropriately
  // for the area of the rectangle.
  G4String GetState() const;
  // returns current state in format above
private:
  G4double _width, _height;
};


#endif
