import os
from ROOT import *

def search_for_odpe( input ):
    f = TFile( input, "open" )
    mcdata = f.Get("mcdata")
    events = mcdata.GetEntries( "mumomv>0 && rv<140.0 && npulses==2 && odpe>0.0" )
    return events

for n in xrange(0,100):
    input = "/net/nudsk0001/d00/scratch/taritree/trg_out/output_analysis_%d.root"%(n)
    events = search_for_odpe( input )
    if events>0:
        print "Found file with nevents=%d"%(events),": ",input
    


