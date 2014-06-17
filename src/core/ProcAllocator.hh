/** @class ProcAllocator
 *  Abstract base class for factory that construct a Processor.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  See ProcAllocatorTmpl for an explanation of what this is for.
 */

#ifndef __RAT_ProcAllocator__
#define __RAT_ProcAllocator__

namespace RAT {


class Processor;

class ProcAllocator {
public:
  /** Returns new instance of a Processor.
   *
   *  This must be overridden by subclass to return a particular kind
   *  of processor.
   */
  virtual Processor *operator() ()=0;

  virtual ~ProcAllocator() { };
};



/** @class ProcAllocatorTmpl
 *  Template for creating Processor factories.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  This class exists because there is no notion of a "virtual
 *  constructor" in C++.  In particular, it is used in
 *  ProcBlockManager::ProcBlockManager() to create a map which
 *  lets you take a string and construct a particular processor.  The
 *  usage in that class is:
@verbatim
procAllocators["daq"]= new ProcAllocatorTmpl<DAQProc>;
procAllocators["singlepe"]= new ProcAllocatorTmpl<SinglePEProc>;
// etc, etc...
@endverbatim
 *
 *  And later, it can then create an instance of FrontEndProc by
 *  doing this:
@verbatim
proc = (*procAllocators[procname])();
@endverbatim
 *
 *  If this does not quite make sense, don't worry, you probably don't
 *  need this class.
 *
 *  @param  T  Processor class instantiated by this factory.  Must be
 *             subclass of Processor.
 */

template <class T>
class ProcAllocatorTmpl : public ProcAllocator {
public:

  /** Create new instance of processor subclass T */
  virtual Processor *operator() () { return new T; }

  virtual ~ProcAllocatorTmpl() { };
};


} // namespace RAT

#endif
