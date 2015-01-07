Creating a Processor
--------------------
Creating a new processor and adding it to RAT requires only a few steps.

Create the Class Header
```````````````````````
First, you need to select a name for your processor class. By convention,
processors are named "RAT::XXXXProc" where XXXX is some short descriptive
name for what your processor does.  If the processor is a fitter, it should
be named "RAT::FitXXXXProc".  All processors are subclasses of the
RAT::Processor class, which defines the common interface for processors.
The easiest way to create a processor class which follows this interface is
to copy the CountProc files in src/core and edit them.

Next, you need to decide whether you want your processor to be invoked once
per physics event or once per detector event.  If you are interested in
Monte Carlo information primarily, or need to consider all the detector
events as a group, you should overload the DSEvent() method::

    virtual RATProcessor::Result DSEvent(RAT_DS &ds);

However, if you are writing a processor that is primarily interested in
detector events, it may be easier to instead overload the Event() method
instead (this is not done in RAT::CountProc)::

    virtual RATProcessor::Result Event(RAT_MC &mc, RAT_EV &ev);

Event() will be called once for every "detector" event, even if there are
multiple detector events in a particular physics event.  The Event() method
is only provided as a convenience, since you could implement the same
behavior by writing your own loop in DSEvent() instead.  You should only
overload DSEvent() or Event(), but NOT BOTH.

Finally, you need to decide what parameters your processor will accept at
runtime.  Many processors will not need this feature at all.  If your
processor needs constants or other external data to function, you should
put it into a RATDB table.  Users can override RATDB values using the
/rat/db/set command in their macro files. 

Instead, parameters are for the constants that should be set per "instance"
of a processor.  A particular processor can be instantiated in an event
loop more than once, and parameters are the only way to differentiate one
instance from another.  For example, the prune and outroot processors can
be used multiple times::

    /rat/proc prune
    /rat/procset prune "mc.particle"
    /rat/proc outroot
    /rat/procset file "prune_mcparticle.root"
    /rat/proc prune
    /rat/procset prune "mc.pmt.photon"
    /rat/proc outroot
    /rat/procset file "prune_mc_particle_photon.root"

If you do need parameters, you will need to select names for them and decide
what type of data you want.  Currently, processors can accept parameters in
int, float, and double, and string format by overloading the appropriate
methods::

    virtual void SetI(std::string param, int value);
    virtual void SetF(std::string param, float value);
    virtual void SetD(std::string param, double value);
    virtual void SetS(std::string param, std::string value);

Only overload the methods you need.

Write the Class Implementation
``````````````````````````````
When you implement your class, you should take a look at CountProc.cc for an
example of how to implement the DSEvent() and SetI() methods.

The return value of DSEvent() and Event() both have the same meaning:

* Processor::OK - This event was successfully processed

* Processor::FAIL - A non-fatal error has occurred.  This event will continue
  to be processed through the event loop, but a later processor may use this
  information to change its behavior.

* RATProcessor::ABORT - A non-recoverable error with this event has occurred.
  If a processor returns this value, then the processing of this event immediate
  stops, and the event loop starts over with the next event, if any.

If you encounter a big problem and want to terminate RAT immediately (this
has a way of getting the attention of the user), use the RAT::Log::Die()
method as described in the Logging section.

If you are implementing one of the parameter methods, you should use this
general pattern (stolen from CountProc.cc)::

    void CountProc::SetI(std::string param, int value) {
      if (param == "update") {
        if (value > 0) {
          updateInterval = value;
        }
        else {
          throw ParamInvalid(param, "update interval must be > 0");
        }
      }
      else {
        throw ParamUnknown(param);
      }
    }

The exceptions will be caught by the RAT command interpreter and appropriate
error messages will be shown to the user before aborting the application.

Register the Class with ProcBlockManager
````````````````````````````````````````
Finally, once you have your processor implemented, you need to edit
src/cmd/ProcBlockManager.cc to register your processor so that users can
add it to their event loops.  Include the header for your processor at the
top, then find the relevant code in the constructor::

    // Create processor allocator table
    procAllocators["count"] = new ProcAllocatorTmpl<CountProc>;
    procAllocators["outroot"] = new ProcAllocatorTmpl<OutROOTProc>;
    procAllocators["outnet"] = new ProcAllocatorTmpl<OutNetProc>;
    procAllocators["prune"] = new ProcAllocatorTmpl<PruneProc>;
    procAllocators["fitcentroid"] = new ProcAllocatorTmpl<FitCentroidProc>;
    procAllocators["fitpoisson"]= new RATProcAllocatorTmpl<FitPoissonProc>;

To add your new processor to the list, pick a short name for it.  (A good
choice is the part between "RAT" and "Proc".)  Then add a line to the end of
that list, like::

    procAllocators["test"]= new RATProcAllocatorTmpl<TestProc>;

Also, the header files seem to be included at the top of ProcBlockManager.cc.

Update Documentation
````````````````````
Finally, don't forget to document your processor by adding it to the
User's Guide in $RATROOT/doc.  Copy one of the existing manual sections for
an example of what information to include.  If your processor is particularly
complicated, it's a good idea to also add a Discussion section to the end of
the manual page that goes into detail on your method, algorithms, assumptions,
etc.  In most cases, a user should be able to figure out if your processor
does what they want without reading your source code.

