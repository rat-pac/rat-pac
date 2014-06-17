Example Cylindrical Detector
============================
The tables in this directory define a simple cylindrical detector consisting
of 1094 8" Hamamatsu R1408 PMTs immersed in water, with a diameter and height
of 6 m.

`PMTINFO.ratdb` lists the PMT positions and directions, and was generated with
the utility at `$RATROOT/util/cylinder_pmts.py`. `cylinder.geo` is a RAT
geometry definition.

These and any other experiment-specific database tables in this directory
(e.g. a special noise rate in a DAQ table) are loaded when you put this in
your macro:

    /rat/db/set DETECTOR experiment "cylinder"
    /rat/db/set DETECTOR geo_file "cylinder/cylinder.geo"

