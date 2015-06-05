DAQ Process
-----------

*A point of clarification:* Unfortunately, "event" is an ambiguous term.  There is what you might call a ''physics event,'' which represents a single interaction, like nu-bar + p -> e+ + n.  The simulated detector, however, will probably turn that into at least two ''detector events.''  Unless specified, assume "event" means "physics event."

In the context of this processor, an event is considered a "physics event" while a subevent is considered a "detector event". This processor splits "physical events" into "detector events".

Data Structure Definition
`````````````````````````
The "physics events" and "detector events" are both kept in the data structure. For example, when using the processor the line:

CountProc: Event 100 (181 triggered events)

indicates that 100 "physics events" have been processed while 181 "detector events" have been observed. This case was for pi+ events provided in the "mac/event_splitter_example.mac" routine, these events tend eventually decay into michel electrons.

Default configuration
`````````````````````
There is currently no default configuration. The user is encouraged to enter (and change) the following flags in the macro:
/rat/proc eventsplitterdaq
/rat/procset offsetToTimeWindowNanoSec 100
/rat/procset collectionTimeWindowNanoSec 400
/rat/procset nhitThreshold 15

General Information
```````````````````
When using this processor it is important to keep in mind that the PMT timing information has been modified. For example, if a neutron has been capture 30,000 ns after the start of the geant4 event, the PMT timing information will registered has a series of PMT hit with time ranging from 100 ns to 500 ns (offsetToTimeWindowNanoSec+collectionTimeWindowNanoSec). This is made to prevent rouding off errors for events in the higher time ranges.

It is also important to note that for low energy studies, the user should consider changing the value to a lower threshold, but should not go lower than a value of 4 PMT hit if he wishes to use BONSAI.


Examples of analysis uses
'''''''''''''''''''''''''
There are three parameters that are added to the data structure (ds) and are available to the user for analysis. These are:

*triggerTime*: time of the first PMT hit in the subevent
*nSubevents*:  Number of subevents found in the physical event
*subevent*: Subevent index. This is required to access the PMT information of the correct subevent

Additionally, two other parameters are used in this DAQ processor:

*id*: Incremental counter that registers that there is a subevent present
*qTotal*: Charge collected within the sub-event window





