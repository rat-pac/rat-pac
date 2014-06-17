Overview
--------

Goals
`````

The Reactor Analysis Tool (RAT) is intended to be a framework that combines both Monte Carlo simulation of the Braidwood detector with event-based analysis tasks, like reconstruction. The primary goals are:

* Make it easy to analyze Monte Carlo-generated events as well as data from disk using the same software with only a few command changes. Even in the proposal/R&D phase, where there is no real data, this is still useful for dumping Monte Carlo events to disk to be analyzed by another job. When there is real data, being able to do the analysis with the same code path as was used on Monte Carlo is very reassuring.
* Allow for a modular, user-controlled analysis of events. This includes allowing the user to selected which analysis tasks to perform (different fitters, pruning unneeded data from the event data structure, etc.). It should also be relatively straightforward for users to introduce their own code into the analysis process.
* Separate analysis into small tasks which can be developed asynchronously by different people, yet integrated with minimal (or perhaps zero) pain.
* Integrate into existing GEANT4 and GLG4sim efforts with a minimum of code duplication. As much as possible, RAT should be designed incorporate upgrades of these packages just by relinking. No cut and paste of code (mainly a question with GLG4sim).

Design
``````

The overall design of RAT is much like SNOMAN: View analysis as a big loop, iterated through once for each event. The body of the loop is assembled by the user in their macro file as a list of "processors." A processor is a self-contained module that takes an event as input and does some work on the event, possibly altering the contents of the event data structure. A fitting processor would add reconstruction information to the event structure, and an I/O processor would write the event to disk, but leave the data structure in memory unchanged. (The SNOMAN event loop actually allows for branching, which has not yet been implemented in RAT.)

Processors can read and modify existing events, but where do the events originally come from? This is the job of event "producers." A producer can be something like a Monte Carlo simulation. We might decide to simulate the following reaction:

  nu-bar + p -> n + e+

Given the delay between the observation of the positron and the neutron, this single physics event will be detected as (at least) two separate detector events. The job of the event producer is ultimately to generate physics events and hand them over to the processors selected by the user, one event at a time.  (Other processors may convert the physics event into detector events.  There is a place in the data structure to put multiple detector events.)

Other event generators are possible. Generators which read events from disk or over the network would function in a similar manner, creating event data structures in memory and handing them to the event processors one by one.

The event-sequential nature of this computation model is both powerful and simple, but can be awkward for certain kinds of analyses. Multi-pass analyses, which must go through a list of events more than once, can be implemented in RAT without much difficulty as long as each pass is sequential. Time-correlation processors can also be implemented if the processor buffers some data internally. A processor that needs full random access to the event stream cannot be implemented efficiently in RAT. The events should be dumped to disk and analyzed in some other program (such as ROOT).

Relationships with Other Software
`````````````````````````````````

RAT makes use of several other software packages to do the heavy-lifting:

* CLHEP - CLHEP is a library containing classes useful to physics software, such as 3D vectors and random number generators. It is also used by GEANT4.
* GEANT4 - While GEANT4 is intended to simulate particle interactions in detectors, RAT does not use it directly for that purpose, delegating that to GLG4sim. Instead, RAT makes direct use of the GEANT4 command interpreter to provide a language for both interactive use and executing macro files. RAT also uses the GEANT4 compilation system and makefiles.
* ROOT - ROOT is used to load and save objects to/from disk and over the network.
* GLG4sim - This package is a generalized version of the KamLAND Monte Carlo, intended to simulate KamLAND-like neutrino experiments with GEANT4. GLG4sim is a self-contained application, but RAT just uses the classes defined in GLG4sim to create a Monte Carlo event producer that integrates with the rest of the RAT framework. The way in which this was done allows the RAT application to be completely backward compatible with the GLG4sim application (for now, anyway).

In order to control the version of GLG4sim used by RAT, we include the entire GLG4sim source tree. Updates to the RAT copy of GLG4sim are made from the KSU GLG4sim repository manually as needed.  Long term, we are planning to slowly modify the GLG4sim classes to integrate with the RAT environment.
