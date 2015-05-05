EventManip
==========

Overview
--------

This event visualizer uses Mathematica to render fancy 3D plots that are easy to
view, rotate, and zoom. It also incorporates a powerful (though low level) 
ability to analyze the event tree, i.e. a tree of tracks with the primary 
particle track at the top. One can select subtrees based on criteria such as 
particle type, creating event, destroying event, and sub-tree information given 
sufficient cleverness. Any subset of the event tree can be rendered in whatever 
style one chooses, and a few examples are included in ```EventManip.nb``` along 
with the code necessary for getting the data from a RAT output file into 
Mathematica as a tree.

Compiling
---------

First the ```event_dump``` sub program needs to be built. This is the part that
parses the RAT datastructure and provides Mathematica with an event tree. With
RAT and ROOT properly sourced simply run `./build_event_dump` in the current 
directory.

Running
-------

```event_dump``` requires RAT and ROOT libraries to function, and since it is
launched from Mathematica, so does Mathematica. The ```LaunchEventManip.sh```
script exists to source RAT and ROOT then launch Mathematica to open 
```EventManip.nb```, but you will have to edit ```LaunchEventManip.sh``` to 
source the proper scripts for RAT and ROOT in your local setup. You may also
have to include the full path to Mathematica depending on your setup.
