#include <RAT/ProcBlock.hh>
#include <RAT/Log.hh>
#include <TStopwatch.h>

namespace RAT {

ProcBlock::ProcBlock() : Processor("ProcBlock"),
			 fSeenFirstEvent(false), 
			 fSourceTime(0.0), fSourceCount(0)
{
  // Nothing to do!

}
 
ProcBlock::~ProcBlock()
{
  // Display usage statistics
  info << "Processor usage statistics\n";
  info << "--------------------------\n";
  info << dformat("%25s: %1.3f sec/event\n",
		  "Event source",
		  fSourceTime / fSourceCount);
  double total = fSourceTime / fSourceCount;

  for (unsigned i=0; i < fProcessorList.size(); i++) {
    double timeperevent = fProcessorTime[i]/fProcessorExecutionCount[i];
    info << dformat("%25s: %1.3f sec/event\n",
		    fProcessorList[i]->name.c_str(),
		    timeperevent);
    total += timeperevent;
  }
  info << "--------------------------\n";
  info << dformat("Total: %1.3f sec/event\n\n", total);

  Clear(); // Used to delete processors before we clear them from the list
}

void ProcBlock::Clear()
{
  // Destroy all the processor objects to give them a chance to clean up
  // and close files, etc.
  for (unsigned i=0; i < fProcessorList.size(); i++)
    delete fProcessorList[i];

  for (unsigned i=0; i < fDeferredAppendList.size(); i++)
    delete fDeferredAppendList[i];

  fProcessorList.clear();
  fProcessorTime.clear();
  fProcessorExecutionCount.clear();
  fDeferredAppendList.clear();
  fSeenFirstEvent = false;
  fSourceTime = 0.0;
  fSourceCount = 0;
}

void ProcBlock::AddProcessor(Processor *proc)
{
  fProcessorList.push_back(proc);
  fProcessorTime.push_back(0.0);
  fProcessorExecutionCount.push_back(0);
}

void ProcBlock::DeferAppend(Processor *proc)
{
  fDeferredAppendList.push_back(proc);
}

Processor::Result ProcBlock::DSEvent(DS::Root *ds)
{
  Processor::Result retcode = Processor::OK;
  TStopwatch timer;

  // Record time since last DSEvent call to keep track of event
  // source duration
  if (!fSeenFirstEvent) {
    fSeenFirstEvent = true;

    // Append queued up processors to end of processor list
    for (unsigned int i=0; i < fDeferredAppendList.size(); i++) {
      AddProcessor(fDeferredAppendList[i]);
    }
    fDeferredAppendList.clear();
  } else {
    fSourceTimer.Stop();
    fSourceCount++;
    fSourceTime += fSourceTimer.RealTime();
  }

  for (unsigned i=0; i < fProcessorList.size(); i++) {
    timer.Start(true);
    Processor::Result procResult = fProcessorList[i]->DSEvent(ds);
    timer.Stop();
    fProcessorExecutionCount[i]++;
    fProcessorTime[i] += timer.RealTime();


    ds->AppendProcResult(fProcessorList[i]->name.c_str(), procResult); // For later processors that
                                       // want to check result, like IF

    if (procResult == Processor::ABORT) {
      // If processor tells us to abort the event, set return
      // code to ABORT in order to propagate up to enclosing block
      // and break out of loop
      retcode = Processor::ABORT;
      break;
    } else if (procResult == Processor::FAIL) {
      // success of block is defined to be logical AND
      // of individual processor success, so one failure
      // means the block failed. (But keep executing)
      retcode = Processor::FAIL;
    }
  }

  // Figure out how much time is spent generating the next event we receive
  fSourceTimer.Start(true);

  return retcode;
}


} // namespace RAT
