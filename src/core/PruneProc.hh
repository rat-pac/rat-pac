/** @class PruneProc
 *  Prunes the data structure
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  PruneProc clears branches of the data structure _DS.  This
 *  is useful if you want to eliminate parts of the event you don't
 *  need in order to reduce the amount of space the event occupies on
 *  disk.  The prune processor can eliminate most of the lists in the
 *  data structure (like PMT hits and particle tracks).  See SetS()
 *  for a detailed list of branch names.
 *
 *  There is little benefit to pruning an event except just before
 *  writing it to disk.  Events are never copied in memory, so their
 *  size does not affect the speed of other processors.
 */

#ifndef __RAT_PruneProc__
#define __RAT_PruneProc__

#include "Processor.hh"
#include <vector>

namespace RAT {


class PruneProc : public Processor {
public:
  /** Create a new prune processor which prunes nothing by default. */
  PruneProc();

  /** Destroy the processor */
  virtual ~PruneProc();

  /** Eliminate branches from @p ds as specified in SetS(). */
  virtual Processor::Result DSEvent(DS::Root *ds);

  /** Set string parameter.
   *
   *   - "prune" : Adds a branch to the list to prune.  Allowed values
   *               include @p "mc.particle", @p "mc.track",
   *               @p "mc.hit", @p "mc.pmt", @p "mc.pmt.photon",
   *               @p "ev", @p "ev.pmt".  If you wish to remove
   *               only a specific kind of particle, like optical
   *               photons, you can use the for @p "mc.track:opticalphoton".
   */
  virtual void SetS(std::string param, std::string value);

  /** Set the pruning state of a branch.
   *
   *  @param  item  Name of branch.  Valid names listed in SetS().
   *  @param  state if true, that branch will be deleted.
   */
  void SetPruneState(std::string item, bool state);

  /** Returns true if the branch name @p item will be cut.
   *
   *  Valid branch names listed in SetS().
   */
  bool GetPruneState(std::string item);

protected:
  bool mc;                    /**< Cut Monte Carlo information entirely */
  bool mc_particle;           /**< Cut Monte Carlo particles? */
  bool mc_track;              /**< Cut Monte Carlo tracks?  True even if only
                                   certain particle types will be cut. */
  bool mc_pmt;                /**< Cut Monte Carlo list of hit PMTs? */
  bool mc_pmt_photon;         /**< Cut Monte Carlo list of photoelectrons? */
  bool mc_pmt_pulse;          /**< Cut Monte Carlo list of PMT pulses? */
  bool ev;                    /**< Cut triggered events? */
  bool ev_pmt;                /**< Cut list of hit PMTs from triggered events? */
  bool ev_board_raw_header;   /**< Cut raw from each board */
  bool ev_channel_raw_block;  /**< Cut raw blocks from each cheannel */
  bool ev_channel_raw_wave;   /**< Cut raw waveforms from each channel */
  bool ev_channel_raw_int;    /**< Cut raw integrals from each channel */
  bool ev_pmt_wave;            /**< Cut calibrated waveforms from each ev.pmt */
  bool ev_channel;            /**< Cut digitizer channel hits? */
  bool ev_board;              /**< Cut digitizers */

  bool ev_pmt_block;         /**< Cut entire calibrated blocks of samples */
  bool ev_calsum;            /**< Cut calibrated waveform sum */

  /** Names of particles to eliminate from tracks.
   *
   *  If this vector is empty and @c mc_track is true, cut all tracks.
   */
  std::vector<std::string> track_cut;
};

} // namespace RAT

#endif
