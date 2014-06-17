User-Defined Processors
-----------------------

One highly-encouraged method of coding up an analysis task is to write it as a user-processor.  You can write a RAT macro file which uses your user processor (on either fresh Monte Carlo data or old data loaded from a file) to operate on the data, rather than write a separate ROOT macro.  This has some advantages:

* You have access to all the RAT classes, which you do not in ROOT.
* Your code is compiled, so it runs faster than in ROOT.  (Not a major advantage, since you can also compile ROOT macros relatively easily.)
* If your processor grows into something generally useful, you can easily convert it into an "official" processor and submit it for inclusion in RAT.

Preparation
```````````

Make sure that you (or your site administrator) has already compiled RAT and set up your environment as described in the [wiki:UserGuideInstallation installation page].

Getting started
```````````````

A good place to start is to copy the $RATROOT/user directory to some location where you will develop your processor.  You can name this new directory anything you like.  (If your copy of the user directory contains .svn/, you can remove that.)

The contents of the user directory are:

* SConstruct - Scripts the build process for making a new copy of RAT
* TestUser.cc - Contains both the declaration and definition of a new RAT user processor.
* testuser.mac - A rat macro which runs the user processor defined in !TestUser.cc.

Now compile the sample processor and run the test macro by typing::

    scons
    ./myrat testuser.mac

Note SCons created a custom version of RAT called "myrat".  This new executable contains the standard RAT application, plus the extra user processor in !TestUser.cc.

If all runs correctly, you should have a file called "nhits.root" in the current directory.  This ROOT file contains a single histogram called "nhits" which will show the number of PMTs hit in each event.

Making your own changes
```````````````````````

If you only need one user processor, you can just edit !TestUser.cc directly.  You can also add new source files to your directory (make sure they end with .cc), and they will all be compiled and linked into your custom version of RAT.

Replacing the main() function
`````````````````````````````

By default, the build script compiles and links rat.cc, which contains the standard RAT main() function.  If you would like to replace this main() function with your own, edit the SConstruct file, and change::

    mymain = False

to::

    mymain = True

Then the main() function you define in one of your source files will be used instead.
