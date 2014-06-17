Data Structure
--------------

The event data structure is a tree of information about a particular event. Event producers (like [wiki:UserGuideGsim Gsim]) create an instance of the the data structure for each event, and processors can then operate on this structure, transforming it as desired.

*A point of clarification:* Unfortunately, "event" is an ambiguous term.  There is what you might call a ''physics event,'' which represents a single interaction, like nu-bar + p -> e+ + n.  The simulated detector, however, will probably turn that into at least two ''detector events.''  Unless specified, assume "event" means "physics event."

Data Structure Definition
`````````````````````````

Brackets in the name indicate that item is an array of the listed type.  The brackets are not valid ROOT syntax, so you should drop them when plotting, as in::

    T->Draw("mc.particle.x");


General Information
```````````````````

============== ==== =======
name           type description
============== ==== =======
lastProcResult int  Result code of last processor, OK = 0, FAIL=1, ABORT=2
ratversion     int  Version of RAT which produced this event, ver 0.1 = 100, ver 1.2 = 1200, and so on
============== ==== =======

Monte Carlo Information
```````````````````````

============== ====== =======
name           type   description
============== ====== =======
mc             MC     Contains all Monte Carlo information
mc.runID       int    Run number
mc.eventID     int    Event number
mc.UT          double Time of this event (units, origin?)
============== ====== =======

Monte Carlo Particles
'''''''''''''''''''''

Monte Carlo PMT Information
'''''''''''''''''''''''''''

Monte Carlo Track Information
'''''''''''''''''''''''''''''

Detected Event Information
``````````````````````````

Event Reconstruction
''''''''''''''''''''

