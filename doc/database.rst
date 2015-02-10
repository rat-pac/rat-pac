The RAT Database (RATDB)
------------------------

RATDB is the database of constants that will be used by RAT for all adjustable parameters.  "Parameters" in this sense includes many things, such as:
 * physical properties of materials (density, optics, etc.)
 * geometric configuration of parts of the detector (dimensions and locations of the acrylic vessel, PMTs, ...)
 * calibration constants
 * lookup tables
 * control parameters which specify how processing should be done

Note that RATDB is NOT intended to hold actual event information, just everything else.

Despite the suggestive name, RATDB is not really a proper Database, like MySQL or Oracle.  Rather it is just a simple set of C++ classes used to read parameters from various sources (more on that later) and make the data easily available to other parts of the RAT application.

How is the data organized?
``````````````````````````

Data in RATDB is organized in a two-level space.  At the top level are ''tables'', which contain groups of ''fields''.  So, every item in the RATDB is located by these two things.

Tables
''''''

Tables are identified by names written in all capital letters which follow the C++ rules for identifiers.  Only letters, numbers, and the underscore are allowed, and the name must start with a non-digit.  Tables also carry an index, which also follows identifier rules (mix case allowed).  The convention when writing out table names is to put the index in brackets right after the name, with the exception that it may be left off if the index is "" (the empty string).

Some examples of valid table names are::

  CALIB, DAQ_CHANNELS, THETA13, MEDIA[vacuum], MEDIA[water], MEDIA[acrylic]

The index is intended to allow several tables with the same fields to exist.  For example, you might standardize a MEDIA table that holds properties for all the materials in the detector.  All materials will have the same set of properties (density, index of refraction, etc), and each material will be given a different index.  Many tables will not need this functionality, so you are free to ignore the index when dealing with them, and the index will be implicitly understood to be empty by RATDB.

Fields
''''''

Every table contains zero or more fields.  Similar to table names, field names also follow the C++ identifier convention, but use only lowercase letters.  Each field has associated with it a piece of data of a distinct type.  The currently allowed types are: integers, floats, doubles, strings, integer arrays, float arrays, double arrays, and string arrays.

Arrays will only contain elements of the same type, and there must be at least one element in an array.  The length of the array, however, is not specified and is allowed to change.

Planes
''''''

Normally, tables and fields are all you have to think about, but RATDB also addresses an additional complication: overriding constants.  It is a common use case to have default values of constants, values which are only valid in certain time intervals and can change (like the optical properties of the scintilator), and user-specified values which are intended to override everything.

RATDB handles this by internally grouping tables into three ''planes''.  The name is intended to suggest a stack of layers where you start at the top, and keep going down until you find your answer.  The RATDB planes are (from highest to lowest priority):

 * the user plane
 * the time plane
 * the default plane

When an item is requested, RATDB will attempt to locate it in the user plane first, then the time plane, and finally the default plane.  Note that this is all handled in the background.  You simply request the index_of_refraction field in the MEDIA[acrylic] table, and RATDB figures out the appropriate plane from which to retreive the data.

How do I load data into RATDB?
``````````````````````````````

RATDB has the potential to read data from a variety of sources (such as real SQL databases), but right now only supports reading data in the RATDB text format.  Read the [wiki:RATDB_TextFormat RATDB text format] page for instructions on how to compose such a file.

Once you have your text file, you have two options for loading it:

* Give it the .ratdb extension and place it into the $GLG4DATA directory.  This is usually the same as $RATROOT/data.  All .ratdb files in that directory are automatically loaded when RAT first starts.
* Manually load the file in your macro using a command like::

    /rat/db/load myfile.ratdb


You can also set the value of individual fields in the database inside the your macro using commands like::

    /rat/db/set MEDIA[acrylic] index_of_refraction 1.52
    /rat/db/set GEO_DETECTOR av_radius 6.2

The /rat/db/set command alters fields in the user plane, so they will override any other values set on the time or default planes.

Note that these changes are not saved when you exit RAT.  If you want permanently change the value of a field, you need to edit the relevant .ratdb file in the data/ directory.  Also note that /rat/db/set will create new tables in memory if they do not already exist.

Limitations
```````````

RATDB is not finished by any means, but is complete enough to address immediate needs.  Current limitations include:

* Tables cannot be added to the time plane.  Properly implementing the time plane will be non-trivial and require implementing caching and load-on-demand semantics for tables.  At the moment, only the user and default planes exist, but adding the time plane will not require any changes to user code or macros.  We don't need it now, so there is no sense in investing the time to implement it yet.
* Array fields cannot be altered using the /rat/db/set command.

