
# KPIPE RAT Fork

Simulation code for KPIPE.

## Checking out

* using git flow pattern (more info: https://www.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow)
* git clone https://github.com/twongjirad/ratpac-kpipe.git
* git checkout develop (this moves you to the develop branch)

## Dependencies
* ROOT 5
* Geant4 (checked things worked with geant4.6.p04)
* scons

## building
* instructions in doc/installation.rst
* make sure ROOT and geant4 environment variables set
* ./configure
* scons

## Running
* rat
* /control/execute mac/kpipe.mac (will bring up and initialize geometry)

## Adding a feature
* git checkout develop
* git checkout -b [username]_[featurename]
* do something awesome
* to put your code onto develop: git checkout develop; git merge [username]_[featurename]
* delete the branch

## Geometry

* in data/kpipe/kpipe.geo
* also have COLLADA format file for chroma: data/kpipe/kpipe.dae

![alt tag](https://raw.github.com/twongjirad/ratpac-kpipe/develop/data/kpipe/kpipe_daeview.png)


# RAT (is an Analysis Tool), Public Edition
-----------------------------------------
RAT is a simulation and analysis package built with GEANT4, ROOT, and C++,
originally developed by S. Seibert for the Braidwood Collaboration. Versions
of RAT are now being used and developed by several particle physics
experiments.

RAT combines simulation and analysis into a single framework, which allows
analysis code to trivially access the same detector geometry and physics
parameters used in the detailed simulation.

RAT follows the "AMARA" principle: As Microphysical as Reasonably Achievable.
Each and every photon is tracked through an arbitrarily detailed detector
geometry, using standard GEANT4 or custom physics processes. PMTs are fully
modeled, and detected photons may be propagated to a simulation of front-end
electronics and DAQ.

This generic version is intended as a starting point for collaborations
looking for an easy-to-learn, extensible detector simulation and analysis
package that works out of the box. Once acquainted with RAT, it is easy to
customize the geometry elements, physics details, data structure, analysis
tools, etc., to suit your experiment's needs.

