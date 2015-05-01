# CRY "Interface"

We choose to first generate CRY events in a ROOT file,
then load them into kpipe-RAT much in the same way as kdar_muon events are generated.

We provide a python script here to generate events. (Move to cluster?)
Then we use the 'external' RAT generator to call 'import_cry_events' to 
give RAT the primary particles to simulate. 'import_cry_events' can be used to
implement filters for the type of particles to use.

### Environment Variables Needed

* CRYHOME: set to base directoy of CRY
* CRYDATAPATH: set to data files.  typically $CRYHOME/data.
* LD_LIBRARY_PATH=${CRYHOME}/lib:$LD_LIBRARY_PATH
