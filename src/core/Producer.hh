/** @class Producer
 *  Base class for classes which create events and pass them through a
 *  block of processors.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  A Producer creates new events and then passes them to the main
 *  block of processors for analysis.  Unlike a Processor, a
 *  Producer does not operate upon existing events, but constructs
 *  them in memory, extracting them from various sources like files, the
 *  network, or the void (as good a description of Monte Carlo as
 *  any).
 *
 *  The event loop is controlled by a Producer, and initiated by
 *  the user in response to a some command.  This is why Producer
 *  inherits from G4UImessenger.  If you make a new producer
 *  subclass, you will need to create some instances of G4UIcommand in
 *  your constructor so that users can start your producer running in
 *  their macros.  See InROOTProducer for an example of this.
 *  (Note that Gsim is a special producer which is wired into
 *  GEANT4'event management loop, so it is unusual in this respect.)
 *
 *  Another consideration for producers is graceful handling of a
 *  user's Ctrl-C termination request.  If a user presses Ctrl-C once,
 *  execution is not interrupted, but a flag is set and can be
 *  retrieved from SignalHandler::IsTermRequested().  This gives
 *  the currently running producer a chance to let the current event
 *  finish and then terminate the event loop.  Your implementation of
 *  the event loop should probably look something like this:

@verbatim
  for (Int_t i=0; i < num_events && !SignalHandler::IsTermRequested(); i++) {
    // Create/Retreive the i'th event
    mainBlock->DSEvent(ds);
  }
@endverbatim

 *  All producers will be constructed once in the main() function and
 *  destroyed at the end of program execution.  If you create a new
 *  Processor, be sure to edit main() accordingly.
 *  
 */

#ifndef __RAT_Producer__
#define __RAT_Producer__

#include <G4UImessenger.hh>

namespace RAT {


class ProcBlock;

class Producer : public G4UImessenger 
{
public:

  /** Create a new producer with no block.
   *
   *  It is not allowed to do anything further with this object until
   *  SetMainBlock() is called.
   */
  Producer();

  /** Create a new producer which will pass events to @p theBlock when
   *  it is running.
   *
   *  Most likely, other Producer objects have a pointer to @p
   *  theBlock as well, so we do not take ownership of the object.
   */
  Producer(ProcBlock *theBlock);

  /** Destroy this producer.  Does not destroy the main block. */
  virtual ~Producer() { /* Do nothing */ };

  /** Get a pointer to the main block used by this producer. */
  ProcBlock *GetMainBlock();

  /** Set the main block used by this producer. */
  void SetMainBlock(ProcBlock *block);

protected:
  /** Run events on this block of processors when created. */
  ProcBlock *mainBlock; 
};


} // namespace RAT

#endif
