Particle Tracks
---------------

The Monte Carlo simulation has the ability to store the entire track for every particle, including optical photons, generated in an event.  This can consume a great deal of disk space (tens of megabytes per event, depending on energy) and even slow down the event loop by bogging down the [wiki:UserGuideOutRoot outroot] processor.  For this reason, track storage is ''disabled'' by default.

However, if track information in the event loop is desired, it can be enabled with the command::

    /tracking/storeTrajectory 1


Track Representation: Lists, Trees, and "Reality"
`````````````````````````````````````````````````

Deciding how to represent particle tracks given the diverse selection of particles and interactions is not a simple question.  There are several ways used in different parts of RAT, which are reviewed below.

"Reality"
'''''''''

In GEANT4's version of reality, a particle track consists of a list of discrete points in spacetime.  You may assume that the particle travels in a straight line between each point.  There are some subtleties here since GEANT4 can also simulate multiple scattering 'within' a single track segment in order to boost performance.  However, we don't understand when and where that happens quite yet, so we will ignore it.

Each point, except the initial point, in the track has associated with it a "process".  (Not to be confused with RAT "processors, " which are entirely unrelated!)  A process represents some sort of interaction, like ionization, absorption, etc.  While some of the process names are straightforward enough, like "eIoni" and "eBrem," others like "Attenuation" are misleading (implements both absorption '''and''' Rayleigh scattering).  As a result of the interaction:
 * The energy and momentum of the particle can be altered.
 * The particle can be destroyed.  (AKA "the track is killed")
 * New particles can be created.
However, the type of particle (electron, positron, Gd-153 nucleus, etc) cannot change during a track.  New particles always get new tracks.

In principle, new tracks created by a process can start anywhere, but in practice they either (a) always start at the interaction point (such as in a discrete process like pair production) or (b) are distributed along the track segment between the current point and the previous one (such as in a continuous process like ionization).

For extra confusion, there is a special "post-process" that isn't really a process at all.  This process is run specially by the GLG4sim code after all other processes have been run, but only if the track segment is contained in a scintillator region.  The post-process does two things to the current track:
 1. If the track is an optical photon and it was killed (i.e. "absorbed"), then throw a random number and check if it should be re-emitted by the wavelength shifter in the scintillator.  The new photon will have a different wavelength in general, and be placed in a new track.
 2. Otherwise, if energy was deposited in the scintillator (such as via an ionization process), generate scintillation photons according to the light yield, and distribute them along the track segment.

Lists
'''''

The most simple-minded way to pack all this information into the data structure that is written to disk is to make a list of tracks.  Each track in turn contains a list of track steps, one for each point along the track.  The nested lists are easy and quick to generate as the simulation runs, and they are easy to store in classes without the use of pointers, which is a big plus. This is how tracks are stored in the ROOT files on disk.

Trees
'''''

Of course, this list format is not very convenient for browsing the tree, or answering questions which concern the relationships between tracks.  For these sorts of studies, we need a data structure that allows us to find the parent and child tracks.

To do this, we use a separate class (RATTrackNav) to convert the lists in the ROOT file into a full tree (in the graph theory sense) in memory.  Each node represents a particle at a given point in its track.  A node has a time, position, momentum, energy, particle type, process, etc.  A node also has a pointer to the "previous" and "next" node.  The previous node is either the previous point in the same track, or if this is the first point in a given track, the previous node will be a the node in the parent track where this track was created.  Similarly, if new tracks are created at a given point by the active process (like ionization), then the node will have pointers to them in a separate "child" list.

This model is a direct representation of "reality" in the case of discrete processes which always create new tracks at the point of interaction.  Continuous processes, like scintillation, which distributes new tracks between interaction points create tracks with ambiguous parent nodes (but not ambiguous parent tracks).  Do we assign the parent to be the node at the beginning or end of the parent segment?  Since the tracks were created in the GEANT4 simulation by the process acting on the point at the '''end''' of the segment, that is the convention we have adopted in the tree structure.

Working with Tracks in ROOT
```````````````````````````

To construct the track tree for an event, you first need to load an event into memory.  The easiest way to do that is to use the RATDSReader class in your ROOT macro::

    RAT::DSReader r("testIBDgen.root");
    RAT::DS::Root *ds = r.NextEvent();


Once you have a data structure object, you can convert the tracks into a tree using the RATTrackNav class, and get a "cursor" which represents a location in the tree::

    RAT::TrackNav nav(ds);
    RAT::TrackCursor c = nav.Cursor(true);

The boolean argument to the Cursor() method selects whether you want the cursor to print out a summary of the current track every time you move the cursor.  This is very useful when interactively browsing a tree.  However, for a macro which is moving around the tree frequently, it is a nuisance, so by default, a RATTrackCursor prints nothing to the screen.

Now that you have a cursor, you can move around the tree using methods which start with "Go".  Here is an example, with the verbose output from the cursor being displayed::

    root [16] RATTrackCursor c = nav.Cursor(true);
    Track 0: TreeStart
    -----------------------------------------------------------------------------------
     # |          position                |  MeV |     process    |   subtracks
    -----------------------------------------------------------------------------------
    * 0. (   0.0,   0.0,   0.0)      _____ <0.001                 ->e+(1),neutron(2)
    root [17] c.GoChild(0)
    Track 1: e+  parent: TreeStart(0)
    -----------------------------------------------------------------------------------
     # |          position                |  MeV |     process    |   subtracks
    -----------------------------------------------------------------------------------
    * 0. ( 485.6,-543.3, 439.7)      scint  1.770           start ->17 tracks
      1. ( 485.6,-543.4, 439.7)      scint  1.757           eIoni ->107 tracks
      2. ( 485.5,-543.8, 439.6)      scint  1.693           eIoni ->613 tracks
      3. ( 485.4,-545.8, 440.1)      scint  1.408           eIoni ->855 tracks
      4. ( 485.5,-547.4, 440.1)      scint  1.212           eIoni ->501 tracks
      5. ( 485.3,-547.8, 440.0)      scint  1.145           eIoni ->683 tracks
      6. ( 484.4,-549.0, 438.6)      scint  0.748           eIoni ->735 tracks
      7. ( 483.5,-549.2, 438.1)      scint  0.572           eBrem ->515 tracks
      8. ( 483.1,-549.9, 438.1)      scint  0.440           eIoni ->111 tracks
      9. ( 483.1,-550.0, 438.0)      scint  0.415           eIoni ->166 tracks
     10. ( 483.1,-550.1, 437.9)      scint  0.398           eIoni ->987 tracks
     11. ( 483.0,-551.9, 438.8)      scint  0.120           eIoni ->259 tracks
     12. ( 483.1,-552.1, 439.1)      scint  0.024           eIoni ->21 tracks
     13. ( 483.1,-552.1, 439.1)      scint <0.001           eIoni
     14. ( 483.1,-552.1, 439.1)      scint <0.001         annihil ->44 tracks
    (class RATTrackNode*)0x565aea0
    root [18] c.GoParent()
    Track 0: TreeStart
    -----------------------------------------------------------------------------------
     # |          position                |  MeV |     process    |   subtracks
    -----------------------------------------------------------------------------------
    * 0. (   0.0,   0.0,   0.0)      _____ <0.001                 ->e+(1),neutron(2)
    (class RATTrackNode*)0x4f6e2a0
    root [19] c.GoChild(1)
    Track 2: neutron  parent: TreeStart(0)
    -----------------------------------------------------------------------------------
     # |          position                |  MeV |     process    |   subtracks
    -----------------------------------------------------------------------------------
    * 0. ( 485.6,-543.3, 439.7)      scint  0.011           start
      1. ( 498.8,-536.3, 439.7)      scint  0.008        LElastic ->proton(3)
      2. ( 503.5,-529.5, 437.0)      scint  0.005        LElastic ->proton(6)
      3. ( 513.9,-525.1, 438.0)      scint  0.004        LElastic ->proton(9)
      4. ( 516.1,-524.2, 437.5)      scint  0.001        LElastic ->proton(10)
      5. ( 526.0,-528.4, 423.1)      scint  0.001        LElastic ->C12[0.0](13)
      6. ( 524.8,-528.8, 425.1)      scint <0.001        LElastic ->proton(14)
      7. ( 530.1,-520.9, 438.2)      scint <0.001        LElastic ->proton(16)
      8. ( 530.9,-518.2, 439.0)      scint <0.001        LElastic ->proton(17)
      9. ( 492.1,-526.1, 450.3)      scint <0.001 NeutronDiffusionAndCapture ->5 tracks
    (class RATTrackNode*)0x5f0ba80

The asterisk on the left shows you which step in the current track your cursor is pointing at.

Also, you'll notice that the Go methods return a RATTrackNode pointer in addition to moving the cursor.  With this pointer, you can get information about the current node, like the energy/momentum/position/etc.  If fact, you can get the pointer to the current node at any time using the Here() method on the cursor::

    root [36] RAT::TrackNode *n = c.Here();
    root [37] n->GetParticleName()
    (string 0x5f0bae8)"neutron"

Names of the node attributes can be found in the [source:RAT/trunk/include/RATTrackNode.hh#latest RATTrackNode header file] and the 
[source:RAT/trunk/include/RAT_MCTrackStep.hh#latest RAT_MCTrackStep header file].

Other RATTrackCursor methods can be found by looking at the [source:RAT/trunk/include/RATTrackCursor.hh#latest header file].

Iterating through the Tree
''''''''''''''''''''''''''

Once you have the tree in memory, you will probably want to be able to step through every track in a loop.  A depth-first iteration algorithm has been provided for you via the FindNextTrack() method.  This will step through tracks, starting from the current cursor location, going up and down the tree in a pattern that will ensure you visit every track once and only once.  Since FindNextTrack() is concerned with visiting each ''track'' and not each node, it returns the first node of each track, and none of the later nodes.  When no more tracks exist to check, it returns 0.

::

    RAT::TrackCursor c = nav.Cursor(false);
    RAT::TrackNode *n = c.Here();
    while (n != 0) {
        // Do something with n

        n = c.FindNextTrack();
    }

Searching the Tree
''''''''''''''''''

A common task is to iterate through the tree, stopping at nodes which match some sort of criteria.  For example, you may want to stop at each neutron track and ignore all the other particles.  The generic way to do something like this is to write a ''boolean functor'' that recognizes the node you want to stop on.  For example, this is the functor that tests particle type::

    class RAT::TrackTest_Particle : public RAT::TrackTest {
      std::string fParticleName;
    public:
      RAT::TrackTest_Particle(const std::string &particleName) : fParticleName(particleName) { };
      virtual bool operator() (RATTrackNode *c) { return fParticleName == c->particleName; };
    };

Notice this functor uses a constructor to customize the type of particle it tests for.  A functor to find electrons would be created with::

    RAT::TrackTest *t = new RAT::TrackTest_Particle("e-")
    RAT::TrackNode n = c.FindNextTrack(t);

and a positron test would look like::

    RAT::TrackTest *t = new RAT::TrackTest_Particle("e+")
    RAT::TrackNode n = c.FindNextTrack(t);

Other tests can be implemented by subclassing RATTrackTest in a similar fashion.

Search by particle type is such a common operation, that a shortcut method has been provided::

    RAT::TrackCursor c = nav.Cursor(false);
    RAT::TrackNode *n = c.FindNextParticle("e-");

You can call the FindNextTrack()/FindNextParticle() methods over and over again with the same test to iterate over just the tracks you are interested it.

Dealing with Optical Photons
````````````````````````````

By far, the bulk of the tracks generated by most events are composed of optical photons.  However, for many studies the optical photons are of no interest at all (beyond perhaps the hits they register on the PMTs).  In these situations, you can add a [wiki:UserGuidePrune prune processor] to your event loop to remove just the optical photons::

    /rat/proc prune
    /rat/procset prune "mc.track:opticalphoton"

You can use any other particle name in place of "opticalphoton" as well, and typing just "mc.track" will prune all tracks from the data structure.  Note that this has no impact on the PMT hits.  The photons are propagated to the PMTs no matter what, but the prune processor lets you delete them after they are no longer needed.

