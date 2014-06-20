/**
 * @class DS::Root
 * Data Structure: Top level
 *
 * @author Stan Seibert <sseibert@hep.upenn.edu>
 *
 * This data structure is used to represent a "physics event" in memory.  The
 * precise definition of what this means depends upon how the Monte
 * Carlo is configured.  If track deferral is disabled, then the
 * "physics event" includes all the particles produced in the
 * interaction.  For example, in inverse beta decay a positron and an
 * neutron are created.  The positron produces light very quickly,
 * whereas the neutron takes some time to capture and produce a
 * gamma cascade.  Both processes are part of the same physics event,
 * but the DAQ simulation might split the early and late light into
 * separate triggered events, depending on how the DAQ is configured.
 *
 * Custom user processors are allowed to store values in the @c user*
 * variables, which are also written to disk (except @c userPtr).
 * Official processors distributed with RAT are never allowed to use
 * these variables.  New variables should be created instead.
 */

#ifndef __RAT_DS_Root__
#define __RAT_DS_Root__

#include <vector>
#include <string>
#include <utility>
#include <TObject.h>
#include <RAT/DS/MC.hh>
#include <RAT/DS/EV.hh>
#include <RAT/DS/Calib.hh>

namespace RAT {
  namespace DS {

class Root : public TObject {
public:
  Root() : TObject() {}
  virtual ~Root() {}

  /** Run number. */
  virtual int GetRunID() const { return runID; }
  virtual void SetRunID(int _runID) { runID = _runID; }
  
  /** Add a result code for a processor that was executed on this event */
  virtual void AppendProcResult(const std::string &procName, int resultCode) { 
    procResult.push_back(std::pair<std::string, int>(procName, resultCode));
  }
  
  /**
   * Return code of last time this processor was run.
   *
   *  -1 : Processor was not run
   *   0 : Success
   *   1 : Non-fatal failure
   *   2 : Abort processing of this event.
   */
  virtual int GetLastProcResult(const std::string &procName) const {
    for (int i=procResult.size()-1; i>=0; i--) {
      if (procResult[(unsigned int) i].first == procName)
        return procResult[(unsigned int) i].second;
    }
    return -1; // Processor has not been run
  }

  /**
   * Return code of the last processor to execute on this event.
   * -1 if no processors have run
   */
  virtual int GetLastProcResult() const {
    if (procResult.size() == 0) {
      return -1;
    }
    return procResult[procResult.size() - 1].second;
  }

  /**
   * Get the entire list of processor result codes as pairs:
   * (name of processor, result code)
   */
  virtual std::vector< std::pair< std::string, int> > GetProcResult() const {
    return procResult;
  }
   
  /**
   * Version number of RAT which created this event.  
   *
   *  The integer version number is the floating point version shifted
   *  by 3 decimal places.  For example, version 0.2 is recorded as
   *  200.  It is guaranteed that this number will monotonically
   *  increase with new RAT releases.
   */
  virtual std::string GetRatVersion() const { return ratVersion; }
  virtual void SetRatVersion(std::string _ratVersion) { ratVersion = _ratVersion; }

  /** Monte Carlo information for this event. */
  virtual MC* GetMC() {
    if (mc.empty()) {
      mc.resize(1);
    }
    return &mc[0];
  }
  virtual bool ExistMC() const { return !mc.empty(); }
  virtual void PruneMC() { mc.resize(0); }

  /** Calibration source used in this event. */
  virtual Calib* GetCalib() {
    if (calib.empty()) {
      calib.resize(1);
    }
    return &calib[0];
  }
  virtual bool ExistCalib() const { return !calib.empty(); }
  virtual void PruneCalib() { calib.resize(0); }

  /** List of triggered events. */
  virtual EV* GetEV(int i) { return &ev[i]; }
  virtual int GetEVCount() const { return ev.size(); }
  virtual EV* AddNewEV() {
    ev.resize(ev.size() + 1);
    return &(ev.back());
  }
  virtual bool ExistEV() const { return !ev.empty(); }
  virtual void PruneEV() { ev.resize(0); }
  virtual void PruneEV(int i) { ev.erase(ev.begin() + i); }

  /**
   * A list of user-defined doubles can store data for a particular
   * user's analysis. No official processor should ever use this.
   */
  virtual int GetUserDataCount() const { return user.size(); }
  virtual void SetUserDataCount(int count) { user.resize(count); }
  virtual double GetUserData(int i) const { return user.at(i); }
  virtual void SetUserData(int i, double val) { user.at(i) = val; }

  ClassDef(Root, 1)

protected:
  int runID;
  std::string ratVersion;
  std::vector< std::pair<std::string, int> > procResult;
  std::vector<MC> mc;
  std::vector<Calib> calib;
  std::vector<EV> ev;
  std::vector<double> user;
};

  } // namespace DS
} // namespace RAT

#endif

