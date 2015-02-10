Gsim Geometry
-------------

The detector geometry used in Gsim is controlled by the GEO tables stored in RATDB.  Each piece of the detector is represented by a GEO table that gives the name of the element, the shape, position, material, and color (for visualization purposes).  This allows simple changes to the detector configuration to be made without having to edit the RAT source code.

Unlike other RATDB files, geometry files end in .geo rather than .ratdb.  Because of this, when RAT starts, no GEO tables are loaded into memory.  Instead, the program waits until the /run/initialize command is issued, and then loads the geometry file listed in the DETECTOR.geo_file field.  It also optionally loads the geometry file given in the DETECTOR.veto_file field if it exists.  Once these files have been loaded into the database, the detector is constructed from all of the GEO tables currently in memory.

Customizing the Geometry in a Macro File
````````````````````````````````````````

There are several ways to customize the detector geometry:

Change DETECTOR.geo_file
''''''''''''''''''''''''

If you want to make drastic changes to the detector, you should start by copying one of the geometry files (like simple.geo) and editing it.  Then you can add a command like::

    /rat/db/set DETECTOR geo_file "mydetector.geo"

to the top of your macro (before /run/initialize).  None of the default geometry will be loaded, just your new file.  Note that this file should be placed in the data directory.

Load an additional geometry file
''''''''''''''''''''''''''''''''

Since geometry files are just RATDB files, you can load additional GEO tables using a command like::

    /rat/db/load "calibration_source.geo"

This file can contain either completely new detector pieces, or it can override parts of the default geometry.  Make sure you set the validity ranges on the tables to put them in the user plane, otherwise they will be overwritten by the defaults when they are loaded.  Any extra GEO tables you create will be built right along with the defaults when /run/initialize is executed.

Alter individual fields
'''''''''''''''''''''''

For a very small change, like changing just a few numbers, you can use the commands::

    /rat/db/set GEO[av] r_max 2800.0
    /rat/db/set GEO[scint] r_max 2700.0

Unfortunately, you cannot change array fields using the set command yet.

GEO Table Fields
````````````````

GEO tables can contain a wide variety of fields to control the properties of the volume.  The common fields shared by all tables:

======================  ======================  ===================
**Field**               **Type**                **Description**
======================  ======================  ===================
``index``               ``string``              Name of the volume.  To conform with RATDB standards, it should follow identifier conventions (no spaces).
``mother``              ``string``              Name of the mother volume.  The mother volume should fully contain this volume.  The world volume has the mother "".
``enable``              ``int`` (optional)      If set to zero, this volume is skipped and not constructed.
``type``                ``string``              Shape of this volume, see below for list.
``sensitive_detector``  ``string`` (optional)   Name of sensitive detector if this volume should register hits.  Limited to ''/mydet/pmt/inner'' and ''/mydet/veto/genericchamber''
======================  ======================  ===================

Allowed types:
 * box - Rectangular solid
 * tube - Cylindrical solid (or section over limited phi range)
 * ptube - Cylindrical solid with circular perforations along z cut out
 * sphere - Spherical solid (or section over limited theta and phi range)
 * psphere - Spherical solid with circular perforations radially cut out
 * revolve - Solid of revolution defined by (r_max, r_min, z)
 * AV - Generic spherical acrylic vessel
 * tubearray - Array of tubes
 * lgarray - Array of tubes where one end has the PMT face cut out 
 * pmtarray - Array of PMTs
 * waterboxarray - Array of standard cubitainer water boxes
 * extpolyarray - Array of extruded polygonal solids
 * bubble - Collection of bubbles

All types except "pmtarray", "waterboxarray", and "bubble"  have these additional fields:

======================  ==========================  ===================
**Field**               **Type**                    **Description**
======================  ==========================  ===================
``material``            ``string``                  Material filling this volume.  See the MaterialList for details.
``color``               ``float[3|4]`` (optional)   Color to be used for this element in visualization.  Either RGB or RGBA (A=alpha transparancy) components ranging from 0.0 to 1.0.
``invisible``           ``int``                     If set to 1, mark this volume as invisible during visualization
``position``            ``float[3]`` (optional)     X, Y, Z (mm) components of the position of the volume center, 'in coordinate system of the mother volume'.  Default position is the center.
``rotation``            ``float[3]`` (optional)     X, Y, Z axis rotations (deg) of element about its center.  Rotations are applied in X, Y, Z order. Default is no rotation.
``replicas``            ``int`` (optional)          Replicate this volume N times inside the mother (position and rotation are ignored if this is set)
``replica_axis``        ``string`` (optional)       Axis along which to replicate volume: x, y, z
``replica_spacing``     ``float`` (optional)        Distance (mm) between replicas
======================  ==========================  ===================

Box Fields:

======================  ==========================  ===================
**Field**               **Type**                    **Description**
======================  ==========================  ===================
``size``                ``float[3]``                X, Y, Z half-lengths (mm) of box (perpendicular distance from center to each face) 
======================  ==========================  ===================


Tube Fields:

======================  ==========================  ===================
**Field**               **Type**                    **Description**
======================  ==========================  ===================
``r_max``               ``float``                   Outer radius of tube (mm) 
``r_min``               ``float`` (optional)        Inner radius of tube (mm) Default is 0.0 (solid)
``size_z``              ``float``                   Half-height of tube (mm)
``phi_start``           ``float`` (optional)        Angle (deg) where tube segment starts.  Default is 0.0
``phi_delta``           ``float`` (optional)        Angle span (deg) of tube segment.  Default is 360.0
======================  ==========================  ===================

Sphere Fields:

======================  ==========================  ===================
**Field**               **Type**                    **Description**
======================  ==========================  ===================
``r_max``               ``float``                   Outer radius of sphere (mm)
``r_min``               ``float``                   Inner radius of sphere (mm) Default is 0.0 (solid)
``theta_start``         ``float`` (optional)        Polar angle (deg) where sphere segment starts.  Default is 0.0
``theta_delta``         ``float`` (optional)        Polar angle span (deg) of sphere segment.  Default is 180.0
``phi_start``           ``float`` (optional)        Azimuthal angle (deg) where sphere segment starts.  Default is 0.0
``phi_delta``           ``float`` (optional)        Azimuthal angle span (deg) of sphere segment.  Default is 360.0
======================  ==========================  ===================

PMTArray Fields:

======================  ==========================  ===================
**Field**               **Type**                    **Description**
======================  ==========================  ===================
``pmt_model``           ``string``                  Serves as the index for ``PMT``, ``PMTCHARGE``, and ``PMTTRANSIT`` tables giving the geometry, charge response, and time response models.
``pos_table``           ``string``                  Specifies the table containing position (and direction) arrays specifying how to place PMTs
``start_idx``           ``int`` (optional)          Index to start building PMTs in the ``PMTINFO`` table specified (inclusive, defaults to 0)
``end_idx``             ``int`` (optional)          Index to stop building PMTs in the ``PMTINFO`` table specified (inclusive, defaults to length-1)
``orientation``         ``string``                  Method of determining PMT direction.  "point" will aim all PMTs at a point in space.  "manual" requires that the position table also contain dir_x, dir_y, and dir_z fields which define the direction vector for each PMT.
``orient_point``        ``float[3]`` (optional)     Point (mm) in mother volume to aim all PMTs toward.
``rescale_radius``      ``float`` (optional)        Assumes all PMTs are spherically arranged around the center of the mother volume and rescales their positions to a particular radius.  By default, no rescaling is done.
======================  ==========================  ===================

Creating a parameterized geometry
`````````````````````````````````		
Using a ``DetectorFactory`` one can build a DB defined geometry on the fly (less useful),		
or modify a normal DB defined geometry template (more useful) before the geometry itself is built. 		
Using only ``.geo`` files there is no nice way to have a property of a geometry component defined 		
as a formula (a function of other geometry parameters), and no nice way to algorithmically define 		
components of a scalable geometry, e.g. PMT positions for various photocathode coverage fractions. 		
		
The DetectorFactory to use is specified by name in the `DETECTOR` table under the field ``detector_factory`` 		
and supersedes the ``geo_file`` field if used. If no ``DetectorFactory`` is specified, the ``geo_file`` specified 		
is loaded as described above. A DetectorFactory should define tables in the DB in the same way a ``.geo`` 		
file would and make use of ``GeoFactory`` components. 		
::	
    /rat/db/set DETECTOR experiment "Watchman"		
    /rat/db/set DETECTOR geo_file "Watchman/Watchman.geo"		
		
v.s.		
::		
    /rat/db/set DETECTOR experiment "Watchman"		
    /rat/db/set DETECTOR detector_factory "Watchman"		
		
Example usage would be to load a normal (statically defined) ``.geo`` file into the DB and modify		
it as necessary for the dynamic functionality. See the ``WatchmanDetectorFactor`` for example use.

