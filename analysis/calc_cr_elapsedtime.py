import os,sys
from ROOT import *
# must match with cygen file to have truth info for analysis

pair_list = {}
elaspsed_time_tot = 0
for p in xrange(0,500):
    cryfile = "/net/nudsk0001/d00/scratch/taritree/cry_gen/cry_events_%d.root"%(p)
    anafile = "/net/nudsk0001/d00/scratch/taritree/cr_trg_out/output_cr_analysis_%d.root"%(p)
    rcryfile = TFile( cryfile )
    ranafile = TFile( anafile )
    
    mcdata = ranafile.Get("mcdata")
    nevents = 0
    try:
        nevents = mcdata.GetEntries()
    except:
        nevents = 0

    if nevents==100:
        cry = rcryfile.Get( "crytree" )
        cry.GetEntry(99)
        dt = cry.telapsed_sec[0]
        elaspsed_time_tot += dt
        pair_list[p] = { "cryfile":cryfile, "anafile":anafile, "dt":dt }
    rcryfile.Close()
    ranafile.Close()

print "COMPLETE PAIRS: ",len(pair_list)
print "ELAPSED TIME: ",elaspsed_time_tot,"secs"

