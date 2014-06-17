/** @class CountProc
 *  Count number of events and print status updates.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  This processor counts events and periodically prints a status
 *  message showing the number of physics events and the number of
 *  triggered events so far.  In order for the count of triggered
 *  events to be accurate, this processor must be run after a DAQ
 *  processor which simulates the trigger.
 *
 *  The "update" parameter controls the update interval.  See SetI()
 *  for details.
 *
 */

#ifndef __RAT_CountProc__
#define __RAT_CountProc__

#include <RAT/Processor.hh>

namespace RAT {

class CountProc : public Processor {
public:
  
  /** Create new count processor.
   *
   *  Default update interval is to print a status line for every
   *  physics event.
   */
  CountProc();

  /** Destroy count processor. */
  virtual ~CountProc();

  /** Set integer parameter.
   *
   *  - "update" : Number of physics events per display of status
   *               line.  Must be >= 1.
   */
  virtual void SetI(std::string param, int value);

  /** Increment event counters, print message if number of physics
   * events is divisible by update interval.
   */
  virtual Processor::Result DSEvent(DS::Root *ds);

protected:
  int dscount;        ///< Number of physics events
  int evcount;        ///< Number of triggered events
  int updateInterval; ///< Number of physics events per update line
};

} // namespace RAT

#endif
