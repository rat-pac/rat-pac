/**
 * @class BNLOpWLSData
 *
 * Data loading for the BNLOpWLS model.
 *
 * @author A. Mastbaum, L. Bignell
 *
 * This factors out the ROOT file loading, since Geant and ROOT classes
 * do not interact well with -Wshadow enabled.
 * 
 */
#ifndef __BNLOpWLSData__
#define __BNLOpWLSData__

#include <string>
#include <vector>

class TBranch;

class BNLOpWLSData {
public:
  BNLOpWLSData(const std::string fname);
  virtual ~BNLOpWLSData();

  // Arguments: File name, Tree name containing Ex/Em data, Branch names for
  // excitation wavelength and emission intensity. Bins are assumed to be nm.
  //
  // Currently, the data are stored in the ExEmData vector, with the exciting
  // wavelength for event i stored in ExEmData.at(i).at(0).at(0), the emitting
  // wavelengths stored in ExEmData.at(i).at(1).at(:), and the normalized 
  // stored at ExEmData.at(i).at(2).at(:).
  // These data are read into memory to speed things up.
  void SetExEmData(std::string fname);

  // Method to get a value from a tree. Arguments; pointer to branch, event #,
  // name of branch.
  void* GetPointerToValue(TBranch*, int, const char*);

  // The Ex/Em data
  std::vector<std::vector<std::vector<double> > > ExEmData;
};

#endif  // __BNLOpWLSData__

