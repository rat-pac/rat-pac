Tools
-----

Genie2Rat
`````````

Genie2Rat allows vertices generated with GENIE (www.genie-mc.org) to be processed with RAT. It converts the root output of GENIE's gevgen_atmo atmospheric neutrino simulation tool and converts it into RAT root format. Note that gevgen_atmo requires a ROOT geometry for your detector to create vertices in it. Once the GENIE file is created, this tool is then run using the syntax ::

  genie2rat -i [input genie filename] -o [output root filename] (-N [number of events to process])

and will output a RAT root file.
