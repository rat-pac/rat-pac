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

fitpath
```````

The ``fitpath`` processor is an implementation (still a work in progress) of the 
successful PathFitter algorithm used in SNO. It fits position, time, and direction 
for cherenkov events using a maximum likelihood fit of hit time residuals while 
taking into account different paths the hit could have taken. For "direct" light 
(i.e. neither reflected nor scattered) an angular distribution of cherenkov light 
is taken into account to fit the direction. All other light is considered "other"
and does not contribute to the direction fit.

Minimization is done in three stages:
0) Hit time residuals are minimized directly using :ref:`simulated-annealing` from a static seed. 
1) PathFitter likelihood is minimized with :ref:`simulated-annealing` from stage 0's result.
2) PathFitter likelihood is minimized with Minuit2 from stage 1's result.

Command
'''''''

::

    /rat/proc fitpath

Parameters
''''''''''

None required from macro. ``fitpath`` reads parameters from a table ``FTP`` containing
the following fields:

=========================   ==========================  ===================
**Field**                   **Type**                    **Description**
=========================   ==========================  ===================
``num_cycles``              ``int``                     Number of annealing iterations (times to lower temp)
``num_evals``               ``int``                     Number of evaluations per iteration (evals per temp)
``alpha``                   ``double``                  Controls the rate of cooling in :ref:`simulated-annealing`

``seed_pos``                ``double[3]``               Static position seed to stage 0
``pos_sigma0``              ``double``                  Size of initial stage 0 simplex in position coordinates
``seed_time``               ``double``                  Static time seed to stage 0
``time_sigma0``             ``double``                  Size of initial stage 0 simplex in time
``temp0``                   ``double``                  Initial temperature of :ref:`simulated-annealing` for stage 0

``seed_theta``              ``double``                  Static theta (detector coordinates) seed to stage 1
``theta_sigma``             ``double``                  Size of initial stage 1 simplex in theta
``seed_phi``                ``double``                  Static phi (detector coordinates) seed to stage 1
``phi_sigma``               ``double``                  Size of initial stage 1 simplex in phi
``pos_sigma1``              ``double``                  Size of initial stage 1 simplex in position coordinates
``time_sigma1``             ``double``                  Size of initial stage 1 simplex in time
``temp1``                   ``double``                  Initial temperature of :ref:`simulated-annealing` for stage 1

``cherenkov_multiplier``    ``double``                  Number of cherenkov photons generated per hits detected
``light_speed``             ``double``                  Speed of light in material in mm/ns 
``direct_prob``             ``double``                  Fraction of direct detected light
``other_prob``              ``double``                  Fraction of late detected light
``photocathode_area``       ``double``                  Area of photocathode mm^2

``direct_time_first``       ``double``                  Time (ns) of first entry in ``direct_time_prob``
``direct_time_step``        ``double``                  Time step (ns) between entries in ``direct_time_prob``
``direct_time_prob``        ``double[]``                Probability (need not be normalized) of being "direct" light with a certain time residual

``other_time_first``        ``double``                  Time (ns) of first entry in ``other_time_prob``
``other_time_step``         ``double``                  Time step (ns) between entries in ``other_time_prob``
``other_time_prob``         ``double[]``                Probability (need not be normalized) of being "other" light with a certain time residual

``cosalpha_first``          ``double``                  Cos(alpha) of first entry in ``cosalpha_prob``
``cosalpha_step``           ``double``                  Cos(alpha) step between entries in ``cosalpha_prob``
``cosalpha_prob``           ``double[]``                Probability (need not be normalized) of Cherenkov light being emitted at a certain cos(alpha) w.r.t. particle direction
=========================   ==========================  ===================


Fit information in DS
'''''''''''''''''''''

In the ``EV`` branch the ``PathFit`` class contains Get/Set methods for the following data:

======================  ==========================  ===================
**Field**               **Type**                    **Description**
======================  ==========================  ===================
``Time0``               ``double``                  Time seed from simple hit time residual minimization
``Pos0``                ``TVector3``                Position seed from simple hit time residual minimization
``Time``                ``double``                  Time resulting from final stage of minimization
``Position``            ``TVector3``                Position resulting from final stage of minimization
``Direction``           ``TVector3``                Direction resulting from final stage of minimization
======================  ==========================  ===================

``PathFit`` implementes ``PosFit`` under the name ``fitpath``.

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
