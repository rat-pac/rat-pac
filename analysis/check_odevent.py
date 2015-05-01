import sys, os
import ROOT
ROOT.gSystem.Load('libRATEvent')
from ROOT.RAT import DSReader, TrackNav, TrackCursor
from math import sqrt

def check_odevents( input_rat, input_trg ):
    """ check the truth for events with ID vertex, two pulses, but with OD event pe.

    special files required as truth needed to be saved.
    these files are in /net/nudsk0001/d00/scratch/taritree/kpipe_out/with_truth: files 300 to 319.
    they are much larger.
    still need an analysis file pair.
    """
    print "analyze: ",input_rat
    fin_trg = ROOT.TFile( input_trg )
    mcdata = fin_trg.Get("mcdata")
    
    reader = DSReader(input_rat)

    nevents = reader.GetTotal()

    for iev in xrange(0,nevents):
        dsroot = reader.GetEvent(iev)
        mcdata.GetEntry(iev)
        
        nid = mcdata.idpe
        nod = mcdata.odpe
        rv = mcdata.rv
        zv = mcdata.zv

        analyze = False
        #print iev,nid,nod,rv,zv,mcdata.npulses
        if nid>0 and nod>0 and rv<150.0 and mcdata.npulses==2:
            analyze = True

        if not analyze:
            continue

        mc = dsroot.GetMC()
        print "analyze event: ",iev,"nid=",nid,"nod=",nod,rv,zv,mcdata.pulsepe[0],mcdata.pulsepe[1]," t:",mcdata.ttrig[0],mcdata.ttrig[1],mc.GetNumPE()
        npmts = mc.GetMCPMTCount()
        nod2 = 0
        for ipmt in xrange(0,npmts):
            pmt = mc.GetMCPMT(ipmt)
            if pmt.GetID()<90000:
                pass
            else:
                print "od pmtid=",ipmt,": ",pmt.GetID()
                nod2 += 1
        print " nod2=",nod2
        #nav = TrackNav( dsroot )

        #c = nav.Cursor(True)
        #c.GoChild(0)
        #c.GoParent()
        #c.GoChild(1)
        raw_input()

if __name__ == "__main__":

    for x in xrange(300,320):
        input1 = "/net/nudsk0001/d00/scratch/taritree/kpipe_out/output_kpipe_%d.root"%(x)
        input2 = "/net/nudsk0001/d00/scratch/taritree/trg_out/output_analysis_%d.root"%(x)
        if os.path.exists(input1) and os.path.exists(input2):
            check_odevents( input1, input2 )
    
