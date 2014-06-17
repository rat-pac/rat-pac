RAT User's Guide: Installation
------------------------------
Prerequisites
`````````````
These software packages should be installed before you attempt to build RAT.  Take note of the version numbers as many of these packages make incompatible changes between releases.

 * [http://proj-clhep.web.cern.ch/proj-clhep/DISTRIBUTION/ CLHEP 2.0.4.5] - Utility Classes for High Energy Physics. This package is required by GEANT4.

 * [http://geant4.web.cern.ch/geant4/support/download.shtml GEANT 4.9.3.p01] - Toolkit used by the Monte Carlo simulation.  When configuring GEANT4, make sure these options are selected:
    * Copy all headers in one directory
    * Make static libraries
    * For using the OpenGL event viewer, enable linking with OpenGL
    * For nicer visualization with Wired 3/4, enable "Link with ZLIB" to produce compressed HEPREP files, much smaller than normal HEPREP.
    * Don't forget to download the interaction cross-section files from the GEANT4 site! (Below the source code links)

 * [http://root.cern.ch/drupal/content/production-version-526 ROOT 5.26.00] - Used for object serialization and network processors.  As of revision [467], you must add the --enable-minuit2 option when you run the ROOT ./configure script.  If installing on Mac OS X 10.5 on 64bit intel, configure with macosx, not macosx64.  OS X 10.6 requires no special settings.

Build Steps
```````````

Make sure that you have setup your environment variables:

* source the ``geant4.sh`` or ``geant4.csh`` created by GEANT4
* source the ``thisroot.sh`` created by ROOT

* Download RAT from GitHub::

    git clone git@github.com:TBD/rat  # with SSH keys set up on GitHub

or::

    git clone https://github.com/TDB/rat  # Anonymous

* Change directory::

    cd rat

* Run the configure script::

    ./configure

* Source the newly generated ``env.sh`` or ``env.csh`` into your environment.
(You will also need to do this in the future before running RAT.)

* Build everything::

    scons


Test Drive
``````````

*  Run an interactive session by just typing "rat".  The rat command line has tcsh-like line editing commands and a history.  Here's a sample of what you might see::

    This is RAT, version 0.3
    Status messages enabled: info 
    DB: Loading /work/rat/rat/data/ANGLE_RESPONSE.ratdb ... Success!
    DB: Loading /work/rat/rat/data/CONCENTRATOR.ratdb ... Success!
    DB: Loading /work/rat/rat/data/DAQ.ratdb ... Success!
    DB: Loading /work/rat/rat/data/DETECTOR.ratdb ... Success!
    DB: Loading /work/rat/rat/data/ELEMENTS.ratdb ... Success!
    DB: Loading /work/rat/rat/data/FIT_POISSION.ratdb ... Success!
    DB: Loading /work/rat/rat/data/IBD.ratdb ... Success!
    DB: Loading /work/rat/rat/data/LED.ratdb ... Success!
    DB: Loading /work/rat/rat/data/MATERIALS.ratdb ... Success!
    DB: Loading /work/rat/rat/data/MATERIALS_CLEAN.ratdb ... Success!
    DB: Loading /work/rat/rat/data/MC.ratdb ... Success!
    DB: Loading /work/rat/rat/data/OPTICS.ratdb ... Success!
    DB: Loading /work/rat/rat/data/PMTLOCATION.ratdb ... Success!
    DB: Loading /work/rat/rat/data/PMTTRANSIT.ratdb ... Success!
    
    *************************************************************
     Geant4 version Name: geant4-08-01-patch-01    (27-July-2006)
                          Copyright : Geant4 Collaboration
                          Reference : NIM A 506 (2003), 250-303
                                WWW : http://cern.ch/geant4
    *************************************************************
    
    Visualization Manager instantiating...
    Visualization Manager initialising...
    Registering graphics systems...
    
    You have successfully chosen to use the following graphics systems.
    Current available graphics systems are:
      ASCIITree (ATree)
      DAWNFILE (DAWNFILE)
      GAGTree (GAGTree)
      G4HepRepFile (HepRepFile)
      G4HepRep (HepRepXML)
      RayTracer (RayTracer)
      VRML1FILE (VRML1FILE)
      VRML2FILE (VRML2FILE)
      FukuiRenderer (DAWN)
      OpenGLImmediateX (OGLIX)
      OpenGLStoredX (OGLSX)
      OpenGLImmediateXm (OGLIXm)
      OpenGLStoredXm (OGLSXm)
      VRML1 (VRML1)
      VRML2 (VRML2)
    
    You have successfully registered the following graphics systems.
    Current available graphics systems are:
      ASCIITree (ATree)
      DAWNFILE (DAWNFILE)
      GAGTree (GAGTree)
      G4HepRepFile (HepRepFile)
      G4HepRep (HepRepXML)
      RayTracer (RayTracer)
      VRML1FILE (VRML1FILE)
      VRML2FILE (VRML2FILE)
      FukuiRenderer (DAWN)
      OpenGLImmediateX (OGLIX)
      OpenGLStoredX (OGLSX)
      OpenGLImmediateXm (OGLIXm)
      OpenGLStoredXm (OGLSXm)
      VRML1 (VRML1)
      VRML2 (VRML2)
    
    Registering model factories...
    G4VisManager: No model factories registered with G4VisManager.
    G4VisManager::RegisterModelFactories() should be overridden in derived
    class. See G4VisExecutive for an example.
    
    You have successfully registered the following model factories.
    Registered model factories:
      None
    
    Registered models: 
      None
    
    Registered filter factories:
      None
    
    Registered filters:
      None
    
    macro file <prerun.mac> could not open.
    PreInit> 

* Run a macro job::

    rat mac/electron_demo_cyliner.mac -o test.root

This will simulate 10 MeV electrons in a cylindrical detector. 

* Now you can start ROOT to analyze the events you just created. The rootlogon.C file in the RAT directory will load the RATEvent shared library::

    $ root test.root 
    root [0] 
    Attaching file test.root as _file0...
    root [1] T->Draw("mc.particle.pos.fX")

You should get a plot of particle x coordinates.

