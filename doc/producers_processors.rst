Event Producers vs. Event Processors
------------------------------------

RAT distinguishes "producers" from "processors" during computation.  An event producer is an object which creates new events in memory, either out of nowhere (as in the Gsim Monte Carlo producer) or from some other external source like a ROOT file.  The producer is responsible for allocating the memory, controlling the loop over events, and calling the event loop for each event it creates.

An event processor is part of the event loop.  It does not create new events, but instead receives events one-by-one and may either change the event by adding to or altering its contents, or it may simply passively observe the event.

The only reason you need to worry about this is because they way you interact with these two entities in your macro file is very different.  Producers are ''executed immediately,'' whereas processors are ''declared.''  When the macro file reader gets to a line like::

    /rat/proc count

it creates a new instance of the [wiki:UserGuideCount Count] processor in memory and adds it to the end of the global event loop, but nothing else happens.  No computation has occurred, and no events are generated.

However, when you get to a line that invokes a producer, such as this one which starts Gsim::

    /run/beamOn 100

RAT immediate begins to simulate 100 events, and each one is passed to the event loop that has been declared thus far.  Execution of your macro file will not continue until those 100 events have been generated and processed.

This is why, as described in the [wiki:UserGuideCommandInterface Command Interface] section, you must first create your processors before calling the event producer, which procedurally looks backwards::

    # Event loop
    /rat/proc count
    /rat/procset update 5
    /rat/proc fitcentroid
    /rat/proc outroot
    /rat/procset file "fit.root"
    
    
    # Run RATGsim
    /generator/rates 3 1
    /generator/gun gamma 0 0 0  0 0 0  1.022
    /run/beamOn 10

You can also call several event producers sequentially in the same macro file if you like.  For example, you could generate events at three energies this way::

    /generator/rates 3 1
    /generator/gun gamma 0 0 0  0 0 0  1.022
    /run/beamOn 10
    /generator/gun gamma 0 0 0  0 0 0  2.461
    /run/beamOn 10
    /generator/gun gamma 0 0 0  0 0 0  8.600
    /run/beamOn 10

or even mix different event generators, though that probably isn't very useful.
