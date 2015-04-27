import os,sys
from ROOT import *
# must match with cygen file to have truth info for analysis

pair_list = {}

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
        pair_list[p] = { "cryfile":cryfile, "anafile":anafile }
    rcryfile.Close()
    ranafile.Close()

print "COMPLETE PAIRS: ",len(pair_list)

cry_merge = "crygen_merged.root"
ana_merge = "crana_merged.root"
cry_add = ""
ana_add = ""
for p in xrange(0,len(pair_list)/100+1):
    ana_temp = "temp_cranalysis_f%d.root"%(p)
    cry_temp = "temp_crygen_f%d.root"%(p)
    cry_add += " "+cry_temp
    ana_add += " "+ana_temp
    cry_addlist = ""
    ana_addlist = ""
    for n in xrange(0,100):
        fnum = 100*p + n
        g = "/net/nudsk0001/d00/scratch/taritree/cry_gen/cry_events_%d.root"%(n)
        f = "/net/nudsk0001/d00/scratch/taritree/cr_trg_out/output_cr_analysis_%d.root"%(fnum)
        if os.path.exists(f)==True and os.path.exists(g)==True:
            ana_addlist += " "+f
            cry_addlist += " "+g
    os.system("hadd %s %s"%(ana_temp,ana_addlist))
    os.system("hadd %s %s"%(cry_temp,cry_addlist))

os.system( "hadd %s %s"%(cry_merge,cry_add))
os.system( "hadd %s %s"%(ana_merge,ana_add))

