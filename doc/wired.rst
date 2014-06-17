Setting up GEANT4 with HEPREP visuals and viewing in Wired 3
------------------------------------------------------------

Background
``````````

Before you start, the people at KSU believe that JAS3 is the way to go for visuals, so for full disclosure, here is the link to their JAS3 how-to:
[http://neutrino.phys.ksu.edu/cgi-bin/BWKSUwiki?JAS3 KSU JAS3 how-to]

One of the advantages of GEANT4 is that it allows for visualizations of the simulation. There are two types of visualizations: live and play-back. The live ways of visualizations included OPENGL. An example of a play-back style visualization is HEPREP.

This brief article will cover using HEPREP. The advantages of using HEPREP instead of a live solution is that you can play data back at the speed you want to see it, and you can also save simulation runs. HEPREP also has a program called wired associated with it which is responsible for viewing the HEPREP file.
Setting up wired

Read the installation section and skim the rest of this site to setup WIRED:

[http://www.slac.stanford.edu/BFROOT/www/Computing/Graphics/Wired/ Wired3 Users Home Page]

Setting up GEANT4
`````````````````

In your mac directory you should create a file called vis.mac. This file will hold all of your visualization information. Here is an example::

    /glg4debug/glg4param omit_muon_processes  1.0
    /glg4debug/glg4param omit_hadronic_processes  1.0
    
    /run/initialize
    
    /process/activate Cerenkov
    
    /vis/open HepRepFile
    /vis/scene/create
    /vis/scene/add/volume
    /vis/sceneHandler/attach
    /vis/scene/add/trajectories
    /vis/scene/add/hits
    /tracking/storeTrajectory 1 # view every photon (~18000 of them)

Running wired
`````````````

After you run an event, you should have a file called G4DataX.heprep, where X is a positive integer, in the directory in which you ran your program. This file is probably very large. Wired reads in HEPREP files compressed, so you should gzip the file::

    gzip G4DataX.heprep

Load of wired on the machine you want to view the data on and then load up the .heprep.gz file by during the normal File->Open sequence. You should now be viewing your data.

Examples
````````

Examples can probably be found at::
[http://wired4.freehep.org/heprep/examples/Geant4-A01/]
