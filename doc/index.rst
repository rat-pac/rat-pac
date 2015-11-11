RAT (is an Analysis Tool) User's Guide
======================================
This manual describes how to configure and run RAT for simulation and analysis.
Those who wish to modify the source code of RAT should first be familiar with
this guide, then read the Programmer Guide.

RAT-PAC (RAT, Plus Additional Codes) is hosted on
`GitHub <https://github.com/rat-pac/rat-pac>`_. For information on accessing
and working with the code using Git and GitHub, see
:download:`Using GitHub with RAT-PAC Code <github_guide/ratpac_github.pdf>`.

User's Guide
------------

.. toctree::
   :maxdepth: 1

   overview
   installation
   tutorials
   command_interface
   producers_processors
   random
   data_structure
   database
   generators
   geometry
   processes
   pmt
   producers
   processors
   user_processors
   root_analysis
   tracks
   rattest
   wired
   tools

Programmer's Guide
------------------

.. toctree::
   :maxdepth: 1

   programmer_ds
   programmer_db
   programmer_log
   programmer_processors
   programmer_generators
   programmer_utils
   programmer_style

Authors
-------

* Tim Bolton
* Dan Gastler
* Josh Klein
* Hugh Lippincott
* Andy Mastbaum
* James Nikkel
* Gabriel Orebi Gann
* Michael Akashi-Ronquest
* Stan Seibert
* Stephen Sekula
* William Seligman
* Chris Tunnell
* Matthew Worcester

*YOUR NAME COULD BE HERE!*

Acknowledgements
----------------

* The SNO Collaboration -- Much of the design of RAT is inspired by SNOMAN, the SNO Monte Carlo and ANalysis program.
* Glenn Horton-Smith -- RAT uses GLG4sim as the basis for its Monte Carlo processor.
* The Double-CHOOZ Collaboration -- We use their Gd capture simulation in the Monte Carlo

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

