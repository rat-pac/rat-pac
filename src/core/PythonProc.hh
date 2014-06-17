/** @class PythonProc
 *  A RAT processor that is implemented as a Python class.
 *
 *  @author Stan Seibert <sseibert@hep.upenn.edu>
 *
 *  This class wraps arbitrary Python classes enabling
 *  them to be used as RAT processors.
 *
 *  The Python class needs to implement 3 basic methods:
 *
 *     def __init__(self):  # Constructor
 *     def dsevent(self, ds): # Called once per event
 *     def finish(self): # Called at the end of the job
 *
 */

#ifndef __RAT_PythonProc__
#define __RAT_PythonProc__

#include <RAT/Processor.hh>

namespace RAT {

class PythonProc : public Processor {
public:  
  /** Create new Python processor */
  PythonProc();

  /** Destroy count processor. */
  virtual ~PythonProc();

  /** */
  virtual void SetS(std::string param, std::string value);

  virtual Processor::Result DSEvent(DS::Root *ds);

protected:
  static int fgProcCounter;

  std::string fPyProcName;
};

} // namespace RAT

#endif
