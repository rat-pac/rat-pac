Event Processors
----------------

simpledaq
`````````

The SimpleDAQ processor simulates a minimal data acquisition system.  The time of each PMT hit is the time of the first photon hit plus the timing distribution of the appropriate PMT, and the charge collected at each PMT is just the sum of all charge deposited at the anode, regardless of time.  All PMT hits are packed into a single event.

Command
'''''''

::

    /rat/proc simpledaq

Parameters
''''''''''
None.

count
`````

The count processor exists mostly as a simple demonstration processor.  It also displays messages periodically showing both how many physics events and detector events have been processed. The message looks something like::

    CountProc: Event 5 (8 triggered events)


Command
'''''''

::

    /rat/proc count

Parameters
''''''''''

::

    /rat/procset update [interval]

* interval (optional, integer) - Sets number of physics events between between status update messages.  Defaults to 1 (print a message for every event).

prune
`````

The Prune processor is not a kitchen aid, but rather a processor for removing unwanted parts of the data structure to save space.  The prune processor is very useful to call before the [wiki:UserGuideOutRoot OutROOT] processor to avoid writing large amounts of data to disk.

Note that there is minimal benefit to pruning in order to save memory in the running program.  Only one data structure is present in memory at any given time, and it is never copied.  Only when lots of events are written to disk does the overhead become considerable.

Command
'''''''

::

    /rat/proc prune


Parameters
''''''''''

::

    /rat/procset prune "cutlist"

* cutlist - (required) a comma separated (no spaces) list of parts of the data structure to remove. [[BR]]The currently allowed entries are:
 * mc.particle
 * mc.pmt
 * mc.pmt.photon
 * mc.track
 * ev
 * ev.pmt

If /tracking/storeTrajectory is turned on, mc.track:particle is used, where particle is the name of the particle track you want to prune (mc.track:opticalphoton will prune optical photon tracks).

A complex example of pruning can be seen in the [source:RAT/trunk/mac/prune.mac#latest prune.mac] macro file included in the RAT source.

fitcentroid
```````````

The !FitCentroid processor reconstructs the position of detector events using the charge-weighted sum of the hit PMT position vectors.

Command
'''''''

::

    /rat/proc fitcentroid

Parameters
''''''''''

None

Position fit information in data structure
''''''''''''''''''''''''''''''''''''''''''

* name - "centroid"
* figures of merit - None

outroot
```````

The OutROOT processor writes events to disk in the ROOT format.  The events are stored in a TTree object called "T" and the branch holding the events (class [source:RAT/trunk/include/RAT_DS.hh#latest RAT_DS]) is called "ds".

Command
'''''''

::

    /rat/proc outroot

Parameters
''''''''''

::

    /rat/procset file "filename"


* filename (required, string) Sets output filename.  File will be deleted if it already exists.

outnet
``````

The !OutNet processor transmits events over the network to a listening copy of RAT which is running the [wiki:UserGuideInNet InNet] event producer.  Multiple listener hostnames may be specified, and events will be distributed across them with very simplistic load-balancing algorithm.

This allows an event loop to be split over multiple machines.  I'll leave it to your imagination to think up a use for this...

Command
'''''''

::

    /rat/proc outnet


Parameters
''''''''''

::

    /rat/procset host "hostname:port"

* hostname:port (required) Network hostname (or IP address) and port number of listening RAT process.  

=== Notes ===

The "load balancing" mentioned above distributes events by checking to see which sockets are available for writing and picking the first one that can be found.  The assumption is that busy nodes will have a backlog of events, so their sockets will be full.  In principle, this means that a few slow nodes won't hold up the rest of the group.

This processor and its [wiki:UserGuideInNet sibling event producer] have no security whatsoever.  Don't use your credit card number as a seed for the Monte Carlo.
