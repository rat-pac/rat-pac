Chroma Interface
----------------

This fork offers an interface to Chroma, a package which performs photon propagation using GPGPUs.

Activate it using::

	/chroma/useChroma 1

Dependencies
````````````

The Chroma interface requires a number of different packages to work:

- Chroma, of course;
- G4DAE, which converts Geant4 geomtries into triangle meshes that Chroma can understand. It's now included in Geant4 as one the persistancy modules;
- ZeroMQ, which is a network socket package that provides the means of commuincation between RAT and Chroma;
- Google protobufs, which serializes the data that is passed between RAT and Chroma over the zmq sockets; and
- RATChromaServer, which is the layer in between Chroma and RAT that handles passing of messages between multiple RAT clients and an instance of Chroma. (Eventually, this should move into this fork of RAT.)


Setting up you geometry for Chroma
``````````````````````````````````

Integration of Geant4/RAT geometry, unfortunately, is not automatic and seemless.  Preparing a geometry description requires the user to perform a number of steps.

- Export Geant4 geometry using G4DAE tool (by S.C. Blythe).  This is now a feature in Geant4.10. But you can also build it into your own Geant4 copy
- Use the Chroma tool, genUserGeant4GeometryDesciption.py, to generate a template python class.  The user must then fill in class methods which provide a way for
Chroma to learn about the geometries materials, sensitive detectors, etc.  There are also functions to help the user smooth over the transition from Geant4 geometry to Chroma triangle mesh.
- Set the parameters in CHROMA.ratdb to configure interface

Interface settings available in CHROMA.ratdb
````````````````````````````````````````````

Stuff

