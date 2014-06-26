Example Spherical Detector
==========================
The tables in this directory define a simple spherical detector consisting
of 5586 inward-looking Hamamatsu R11780 PMTs immersed in water at a radius of
8 m, for coverage of about 50%.

`PMTINFO.ratdb` lists the PMT positions and directions, and was generated with
the utility at `$RATROOT/util/sphere_pmts.py`. `sphere.geo` is a RAT
geometry definition.

These and any other experiment-specific database tables in this directory
(e.g. a special noise rate in a DAQ table) are loaded when you put this in
your macro:

    /rat/db/set DETECTOR experiment "sphere"
    /rat/db/set DETECTOR geo_file "sphere/sphere.geo"

