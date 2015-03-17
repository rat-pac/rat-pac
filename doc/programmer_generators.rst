Adding a New Gsim Generator
---------------------------
If you find that the existing event generators are insufficient for your
needs, then you will have to write a new event generator for Gsim.

First you need to ask yourself: Do you need to control some combination of
momentum, position or timing simultaneously?  If not, then you will be able
to use the combo generator as your top-level generator, and just implement
a vertex, position or time generator as described below.  If your problem
does not factorize in those independent parts, then you will need to
implement a new top-level generator.

Writing a top-level generator
`````````````````````````````
Top level generators are subclasses of GLG4Gen. By convention, the name of
the subclass should be RAT::Gen_XXXX, where XXXX is some short identifier
of your generator.

You will need to implement the following methods:

* IsRepeatable() - Returns true if this event generator should be used to
  generate more than one event.  This is almost certainly the case for any
  generator you write.  If this is false, the generator will be used to
  generate one event in the simulation and then deleted.
* SetState()/GetState() - Read a string passed by the user for general
  configuration when they run /generator/add.  GetState() should return a
  string in the same form that SetState() accepts.
* ResetTime(double offset) - Pick a new time for the event to occur relative
  to the given offset.  offset should be considered ''now'', and the new time
  is selected relative to it.  It must be >= offset.  Store the new time in
  the protected member variable nextTime.
* GenerateEvent() - This adds G4PrimaryVertex objects which contain the
  G4PrimaryParticle objects for this event.  Called once at the beginning
  of each event.

Optionally, you can also override:

* SetVertexState() - called when user runs /generator/vtx/set
* SetPosState() - called when user runs /generator/pos/set
* SetTimeState() - called when user runs /generator/rate/set

This is not required if no further customization is needed beyond your normal SetState() method.


Next, you will need to edit RAT::Gsim::Init() to register your new generator
with the ''generator factory''::

    GLG4Factory<GLG4Gen>::Register("cosmic",
                                 new GLG4AllocImpl<GLG4Gen, Gen_Cosmic>);

Don't forget to include your class header file at the top of Gsim.cc.

Now you will be able to invoke your generator with the command::

    /generator/add cosmic parameters etc etc

Writing a vertex generator
``````````````````````````
Vertex generators pick the particles and their momenta and polarization.
The assumption is that some other generator has been used to pick the position
and time.

Vertex generators are subclasses of GLG4VertexGen.  By convention, the name
of the subclass should be RAT::VertexGen_XXXX, where XXXX is some short
identifier of your generator.  You will need to implement the methods:

* GeneratePrimaryVertex(G4Event* argEvent, G4ThreeVector& dx, G4double dt) -
  Add vertices to argEvent with position and time offset to dx and dt.
* SetState()/GetState() - control the generator config, usually called by
  /generator/vtx/set.

The new generator class is registered with the vertex generator factory in
RAT::Gsim::Init()::

    GLG4Factory<GLG4VertexGen>::Register("betadecay",
        new GLG4AllocImpl<GLG4VertexGen, RAT::VertexGen_BetaDecay>);

Writing a position generator
````````````````````````````
Position generators pick the locations of events in the detector.

Position generators are subclasses of GLG4PosGen.  By convention, the name
of the subclass should be RAT::PosGen_XXXX, where XXXX is some short identifier
of your generator.  You will need to implement the methods:

* GeneratePosition(G4ThreeVector& argResult) - Assign a new event position
  to argResult.
* SetState()/GetState() - control the generator config, usually called by
  /generator/pos/set.

The new generator class is registered with the position generator factory in
RAT::Gsim::Init()::

    GLG4Factory<GLG4PosGen>::Register("plane",
        new GLG4AllocImpl<GLG4PosGen, RAT::PosGen_Plane>);


Writing a time generator
````````````````````````
Time generators pick the time interval between events. (Only from the same
generator instance, time between events from different instances cannot be
controlled).

Time generators are subclasses of GLG4TimeGen.  By convention, the name of
the subclass should be RAT::TimeGen_XXXX, where XXXX is some short identifier
of your generator.  You will need to implement the methods:

* GenerateEventTime(G4double offset) - Return a new time until the next event
  (in ns).  Offset defines "now" so the returned time should never be less
  than offset.
* SetState()/GetState() - control the generator config, usually called by
  /generator/rate/set.

The new generator class is registered with the time generator factory in
RAT::Gsim::Init()::

    GLG4Factory<GLG4TimeGen>::Register("burst",
        new GLG4AllocImpl<GLG4TimeGen, RAT::TimeGen_Burst>);

