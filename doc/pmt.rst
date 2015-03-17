PMT Simulation
--------------

RAT uses a custom PMT simulation extracted from GLG4Sim.

Q/T Response
````````````
Gsim checks the database for single photoelectron charge and transit time PDFs 
automatically for PMT models that are added to the geometry. These PDFs are 
stored in tables named ``PMTCHARGE`` and ``PMTTRANSIT`` respectively, where the 
index corresponds to a ``pmt_model`` field used in ``GEO`` tables. These PDFs
are sampled whenever a photon is absorbed by the photocathode to create a 
realistic Q/T response automatically for PMTs independent of any DAQ processor.
If no tables are defined for a ``pmt_model`` the time defaults to approximately
zero spread from photoelectron absorption time and the charge defaults to a
phenomenological model used by MiniCLEAN.

``PMTCHARGE`` fields:
 * ``charge`` - "x" values of the charge PDF (arbitrary units)
 * ``charge_prob`` - "y" values of the charge PDF (will be normalized)
 
 
``PMTTRANSIT`` fields:
 * ``cable_delay`` - constant offset applied to all PMTs of this model (nanoseconds)
 * ``time`` - "x" values of the time PDF (nanoseconds)
 * ``time_prob`` - "y" values of the time PDF (will be normalized)

Dark Current
````````````

PMTs have an intrinsic noise rate, or "dark current", which results from thermal excitation at the
single electron level.  These thermal electrons can exactly mimic a photoelectron from the PMT's
photocathode and, thus, noise hits cannot be distinguished from 'true' hits caused by incident photons.

On the upside, this makes the noise hits fairly simple to simulate: we can draw from the same charge
spectrum as is used for regular PMT hits.  The only subtleties are in the timing distribution of the
hits, and the rates at which noise is generated.  This document describes the inclusion of noise hits in 
RAT.

There old noise processor (source:rat/src/daq/NoiseProc.cc) had several problems with the implementation, in particular the
hits were incorrectly distributed in time (generated for one sampling window width from the start of the
simulated event, therefore not extending throughout the event window), and noise was defined in terms of a number of hits per event
instead of a rate (which is what we are more likely to measure).  
In addition, under the principle of "apathy should lead to 
the most realistic simulation possible" (- Dr Stanley Seibert), it was decided to incorporate noise
hits into the default event simulation, rather than retaining the optional processor.  This avoids the
possibility of noise not being included either through forgetfulness, or because the noise processor was
run in the wrong order, for example after the DAQ processors.  As a result,
running RAT will now include PMT noise hits by default, unless they are switched off.  Details on
how to do so follow.

Control
'''''''

There are three options for the inclusion of noise, as follows:

0: No noise hits simulated.

1: Average noise rate read from 'noise_rate' in DAQ.ratdb.

2: Tube-by-tube noise rates read from 'PMTnoise' in DAQ.ratdb.

These options are controlled by the use of the 'noise_flag',
in the DAQ.ratdb file.  This flag can be include' in RAT macros as follows::

    /rat/db/set DAQ noise_flag x

where x = 0, 1, or 2, depending on the noise model chosen.

The noise is included in the simulation after the physics event has been propagated (all particles
followed to extinction, and PMT hits recorded) but before the DAQ, which runs as a separate processor. 
All noise hits are flagged with the 'isNoise' bit in the MCPhoton class (set to 1 for noise hits, and 0
for regular PMT hits).

Timing Distribution
'''''''''''''''''''

Noise hits are generated uniformly in time, throughout a pre-defined 'noise window'.  The DAQ records data beginning from a predefined time before a trigger.  This time period (pretrigger time) is given by a fixed number (currently 1350) of sampling windows (4ns each).  We want noise to be simulated throughout any recorded waveform.  The noise window therefore begins at a time before the first photon hit given by this pretrigger time.  To allow for a noise hit in coincidence with the last recorded photon to cause a trigger, the noise window continues until the width of the event window, plus the width of a single discriminator pulse, past the last photon hit time.

Speed and file size Comparison
''''''''''''''''''''''''''''''

To determine the effect of including noise in the default simulation, I generated 1000 isotropic 20keV 
electrons at the centre of the detector using each noise model.  Both the average and the individual tube noise 
rates were set to 2000Hz, to emphasise any impact of including noise hits (default is 500Hz).  The results, in terms of CPU usage (output file size), were as follows:

Noise model 0: 2280.91 s (46M)

Noise model 1: 2285.77 s (48M)

Noise model 2: 2341.45 s (48M)

So including noise in the simulation increases the processing time by 0.2%, and simulating noise tube-by-tube increases it by a further 2.4%.

The file size increased by ~5% when noise was included in the simulation.

Parameters
''''''''''
All are stored in DAQ.ratdb

noise_rate: 500.0d, // The mean noise rate across all PMTs, in Hz

PMTnoise: [], // an array with 92 entries: individual noise rates per PMT, in Hz

noise_flag: 0, // the flag to determine which noise model you use (default is to turn noise off completely)

