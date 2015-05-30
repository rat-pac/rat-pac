#ifndef __RAT_GeoPMTParser__
#define __RAT_GeoPMTParser__

#include <G4RotationMatrix.hh>
#include <RAT/PMTConstruction.hh>
#include <RAT/DB.hh>
#include <vector>



namespace RAT {

  /** Helper class which reads the attributes stored in a GEO table
   *  describing PMT placement and construction.  */
  class GeoPMTParser {
  public:
    GeoPMTParser(DBLinkPtr table);
    ~GeoPMTParser() { delete fConstruction; };

    /** Returns total number of PMTs described by the RATDB table. */
    int GetPMTCount() const { return fDir.size(); };

    /** Fills pmt_x, pmt_y and pmt_z with PMT positions.
     *  By convention, this is the location of point in center of plane
     *  cutting through the PMT equator. */
    const std::vector<G4ThreeVector> &GetPMTLocations() const {
      return fPos;
    };

    
    /** Return PMT orientation vectors.
     *  Gives direction of PMT frame +z axis (face of PMT).  Vectors are
     *  guaranteed to be normalized. */
    const std::vector<G4ThreeVector> &GetPMTDirections() const {
      return fDir;
    };

    /** Returns the rotation matrix required to orient PMT of index i
     *  in a G4PVPlacement.  By GEANT4 convention, this is a passive rotation.
     *  Use the invert() method to convert it to an active rotation if you
     *  want to apply it to a vector.*/
    G4RotationMatrix GetPMTRotation(int i) const;

    /** Returns PMT parameter structure describing PMT geometry */
    ToroidalPMTConstructionParams GetPMTParams() const { return fParam; };

    /** Returns pointer to ToroidalPMTConstruction object which can be used
     *  to make PMTs.  GeoPMTParser owns this object, so do not delete! */
    ToroidalPMTConstruction *GetToroidalPMTConstruction() { return fConstruction; };

  protected:
    DBLinkPtr fTable;
    std::vector<G4ThreeVector> fPos;
    std::vector<G4ThreeVector> fDir;
    ToroidalPMTConstructionParams fParam;
    ToroidalPMTConstruction *fConstruction;
  };


} // namespace RAT

#endif
