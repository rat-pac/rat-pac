/** @class ProcBlock
 *  A logical group of processors to be executed sequentially.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 * 
 *  A processor block is analagous to a C++ block of statements.  It
 *  consists of a list of processors which it runs sequentially when
 *  DSEvent() is called.  The result code returned by a block when
 *  processing an event is roughly the logical AND of the result codes
 *  from the processors it contains.  See DSEvent() for more details.
 * 
 *  New processors are added by calling AddProcessor() and the list is
 *  emptied when Clear() is called.  Note that when a processor is
 *  added to a ProcBlock, the block takes ownership of the
 *  processor object and will delete it when the list is clear or the
 *  block is deleted.
 *
 */

#ifndef __RAT_ProcBlock__
#define __RAT_ProcBlock__

#include <vector>
#include <RAT/Processor.hh>
#include <TStopwatch.h>

namespace RAT {

class ProcBlock : public Processor {
public:
  /** Create an empty block of processors. */
  ProcBlock();
  
  /** Delete this block, and delete all processors in this block. */
  virtual ~ProcBlock();

  /** Remove all processors from this block.
   *
   *  The destructors of all the processors are called, allowing them
   *  to clean up, close files, write final status messages, etc.
   */
  virtual void Clear();

  /** Append a processor to the end of this block.
   *
   *  Note that the block takes ownership of the processor at this
   *  point.  You should not delete the processor yourself later!
   *
   */
  virtual void AddProcessor(Processor *proc);


  /** Defer the append of a processor to the end of this block.
   *
   *  Processors added through DeferAppend() will be added to the
   *  end of the processor block when the first event is processed.
   *  This allows final analysis processors to be created prior
   *  to the population of the event loop, say from the command line.
   *
   *  Note that the block takes ownership of the processor at this
   *  point.  You should not delete the processor yourself later!
   *
   */
  virtual void DeferAppend(Processor *proc);

  /** Process this event sequentially through every processor in this
   *  block.
   *  
   *  The @p ds object is passed to the DSEvent() method for every
   *  processor in the order they were added to this block.  If one of
   *  the processors requests that the event be aborted, ABORT is
   *  returned immediately.  In that case, not all processors will
   *  have a chance to process the event.
   *
   *  @retval Processor::OK if all processors succeed.
   *  @retval Processor::FAIL if one processor failed the event,
   *  but none requested an abort.
   *  @retval Processor::ABORT if one processor requested that the
   *  event be aborted.  
   */
  virtual Processor::Result DSEvent(DS::Root *ds);
                                                   
protected:
  /** List of pointers to processors in this block.  The ProcBlock
   *  object owns these Processor objects. */
  std::vector<Processor *> fProcessorList;  

  /** Total time in seconds spent in each processor in this block */
  std::vector<double> fProcessorTime;

  /** Total number of times each processor is run */
  std::vector<int> fProcessorExecutionCount;

  /** Has at least one event been processed yet? */
  bool fSeenFirstEvent;

  /** Timer used to figure out how long the event source is taking */
  TStopwatch fSourceTimer;
  
  /** Total time in seconds spent on whatever is producing events for this block */
  double fSourceTime;
  
  /** Number of events counted in fSourceTime */
  int fSourceCount;

  /** List of processors to append to processor list as soon as the first event
      comes in */
  std::vector<Processor *> fDeferredAppendList;
};


} // namespace RAT

#endif
