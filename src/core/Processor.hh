/** @class Processor
 *  Base class for all processors.
 *
 *  @author Stan Seibert <volsung@physics.utexas.edu>
 *
 *  A Processor represents a chunk of analysis code that can be
 *  placed into the event loop by the user.  It is constructed in the
 *  macro when the user calls:
 *  @verbatim /rat/proc procname @endverbatim
 *
 *  The user can then configure the processor with the macro command:
 *  @verbatim /rat/procset variable_name variable_value @endverbatim
 *  This is translated into calls to SetI(), SetF(), SetD(), or SetS()
 *  depending upon the detected type of variable_value.  If there is a
 *  problem, ParamUnknown or ParamInvalid exceptions are thrown.
 *
 *  Once the event loop starts, DSEvent() called once per physics
 *  event.  If you would rather run once per triggered event (of which
 *  there can be zero or more associated with a physics event), then
 *  override the Event() method, and the default implementation of
 *  DSEvent() will call Event() once for every triggered event.
 *
 *  To create a new processor, subclass Processor and then edit
 *  ProcBlockManager::ProcBlockManager().
 */

#ifndef __RAT_Processor__
#define __RAT_Processor__

#include <string>
#include <RAT/DS/Root.hh>

namespace RAT {


class Processor {
public:

  /** The short name of this processor. */
  std::string name;

  /** Create new processor.
   *
   *  Sets the name of the processor.  This may be used to identify
   *  processors in the future.  Should be set by all subclasses and
   *  look like a valid C++ identifier.
   *
   *  @param[in] _name Short name of processor
   */
  Processor(std::string _name) : name(_name) { /* Do nothing */ };

  /** Destroy processor. */
  virtual ~Processor() { /* Do nothing */ };


  /** Result codes returned by DSEvent() or Event(). */
  enum Result { OK=0,    /**< Processor ran successfully. */
		FAIL=1,  /**< Processor task failed, but non-fatal, so
			      execution of later processors continues. */
		ABORT=2  /**< Processing of event is immediately
		              terminated, no futher processors are run
			      on this event */
	      };
  
  /** Base class for parameter exceptions.
   *
   *  Thrown if some problem happens while setting a parameter.
   */
  class ParamError {
  public:
    /** Create parameter error.
     *
     *  @param[in]  _param  Name of parameter which triggered this
     *  error.
     */
    ParamError(std::string _param) : param(_param) { };

    std::string param; /**< Name of parameter which triggered this
			    error. */
  };


  /** Exception thrown when parameter name is not known to this
      processor. 
  */
  class ParamUnknown : public ParamError {
  public:
    /** Create unknown parameter error.
     *
     *  @param[in]  _param  Name of unknown parameter
     */
    ParamUnknown(std::string _param) : ParamError(_param) { };
  };

  /** Exception thrown when parameter name is known, but value is
   *  invalid.
   *
   *  Note that this class contains no member variable to hold the
   *  parameter value which caused the problem.  This is because we do
   *  not know the type.  You should mention the value inside the
   *  error message member variable instead.  See dformat() for a nice
   *  function to format strings, much like sprintf().
   */
  class ParamInvalid : public ParamError {
  public:
    /** Create invalid parameter value error.
     *
     *  @param[in]  _param  Name of parameter
     *  @param[in]  _msg    Message explaining problem with setting the value.
     */
    ParamInvalid(std::string _param, std::string _msg) : ParamError(_param),
							 msg(_msg)  { };

    std::string msg; /**< Message explaining problem. */
  };


  /** Set integer parameter.
   *
   *  @param[in]  param  Name of parameter.
   *  @param[in]  value  Value of parameter.
   *  @throws ParamUnknown if param is not recognized.
   *  @throws ParamInvalid if value is not allowed for param.
   */
  virtual void SetI(std::string param, int value);

  /** Set float parameter.
   *
   *  @param[in]  param  Name of parameter.
   *  @param[in]  value  Value of parameter.
   *  @throws ParamUnknown if param is not recognized.
   *  @throws ParamInvalid if value is not allowed for param.
   */
  virtual void SetF(std::string param, float value);

  /** Set double parameter
   *
   *  @param[in]  param  Name of parameter.
   *  @param[in]  value  Value of parameter.
   *  @throws ParamUnknown if param is not recognized.
   *  @throws ParamInvalid if value is not allowed for param.
   */
  virtual void SetD(std::string param, double value);

  /** Set string parameter.
   *
   *  @param[in]  param  Name of parameter.
   *  @param[in]  value  Value of parameter.
   *  @throws ParamUnknown if param is not recognized.
   *  @throws ParamInvalid if value is not allowed for param.
   */
  virtual void SetS(std::string param, std::string value);


  /** Process one physics event.
   *
   *  The processor can do whatever operation it likes on the event.
   *  The default implementation of this method calls Event() once
   *  for every event in the ds.ev list.  Do not override DSEvent()
   *  if you plan to override Event().
   *
   *  @param[in,out]  ds  Data structure for current event.
   * 
   *  @return Status code for successs/failure in processing this event.
   */
  virtual Processor::Result DSEvent(DS::Root *ds);

  /** Process one triggered event.
   *
   *  If you do not override DSEvent(), then this method is called
   *  once per triggered event.  As a convenience, a reference is
   *  also provided to the root of the data structure
   *  associated with the triggered event. 
   *
   *  Do not override this method if you also plan to override
   *  DSEvent().
   *
   *  @param[in]      ds  Root of data structure
   *  @param[in,out]  ev  Triggered event data structure
   *
   *  @return Status code for successs/failure in processing this event.
   */
  virtual Processor::Result Event(DS::Root *ds, DS::EV *ev);
};


} // namespace RAT

#endif
