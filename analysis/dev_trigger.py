import os,sys
import ROOT
ROOT.gSystem.Load('libRATEvent')
from ROOT.RAT import DSReader
#from root_numpy import root2array
#import numpy as np
#import pandas as pd
from math import sqrt,exp

#pmtinfo = pd.DataFrame( root2array('../data/kpipe/PMTINFO.root', 'pmtinfo' ) )
#pmtinfo = pmtinfo.set_index('opdetid')

inputfile = "output_kpipe_0.root"
#reader = DSReader('kpipeout_test.root')
#reader = DSReader('../cry/crkpipe.root')
#reader = DSReader("output_kpipe_cryevents_2.root")

if len(sys.argv)==2:
    inputfile = sys.argv[1]
    
reader = DSReader(inputfile)
nevents = reader.GetTotal()

out = ROOT.TFile('output_test.root','recreate')

c = ROOT.TCanvas('c','',1200,1200)
c.Divide(1,2)

window = 5
thresh = 5
tave = 10

#hz = ROOT.TH1D('hz','',200,-5000,5000)
ht = ROOT.TH1D("htraw",'',10000,0,10000) # 10 usec with 1 ns bins
ht2 = ht.Clone("htana")
hthresh = ht.Clone("hthresh")
hthresh.SetLineColor(ROOT.kGreen+4)

# =====================================
# FILL HISTOGRAMS
for iev in xrange(0,nevents):
    dsroot = reader.NextEvent()
    mc = dsroot.GetMC()
    npes = mc.GetNumPE()
    npmts = mc.GetMCPMTCount()

    #if iev not in [83,90,137,274,335,472,724,742,778,943]:
    #    continue

    npe_prompt = 0
    npe_late = 0
    print "============================================================"
    print "EVENT ",iev
    for ipart in xrange(0,mc.GetMCParticleCount()):
        pmom = 0.
        mom = mc.GetMCParticle(ipart).GetMomentum()
        ke = mc.GetMCParticle(ipart).GetKE()
        pmom = mom.X()*mom.X()  + mom.Y()*mom.Y() + mom.Z()*mom.Z()
        pmom = sqrt(pmom)
        pos = mc.GetMCParticle(ipart).GetPosition()
        posr = sqrt( pos.X()*pos.X()  + pos.Y()*pos.Y() )*0.1
        print "  ",ipart,") pdg=",mc.GetMCParticle(ipart).GetPDGCode()," Mom=",pmom,"=",mom.X(),",",mom.Y(),",",mom.Z()," ke=",ke," pos=",pos.X()*0.1,pos.Y()*0.1,pos.Z()*0.1," cm, r=",posr," cm"
    vertex = mc.GetMCParticle(0).GetPosition()
    print "  TOTAL NPEs in event: ",npes
    print "  TOTAL NPMTs hit: " ,npmts
    ht.Reset()
    ht2.Reset()
    hthresh.Reset()

    # Fill Time Bin
    id_pes = 0
    od_pes = 0
    for ipmt in xrange(0,npmts):
        pmt = mc.GetMCPMT( ipmt )
        npe_pmtid = pmt.GetMCPhotonCount()
        pmtid = pmt.GetID()
        if ( pmtid>=90000 ):
            od_pes += npe_pmtid
            continue
        id_pes += npe_pmtid
        for ihit in xrange(0,npe_pmtid):
            mchit = pmt.GetMCPhoton(ihit)
            ht.Fill( mchit.GetHitTime() )

    print "  ID PEs: ",id_pes
    print "  OD PEs: ",od_pes

    # Analyze
    pulses = {}
    npulses = 0
    thresh_adjust = 0
    active_pulses = []
    for ibin in xrange(window,ht.GetNbinsX()-tave):

        hits_window = 0
        for i in xrange( ibin-window, ibin ):
            hits_window += ht.GetBinContent(i+1)

        ave_window = 0.0
        nbinsum = 0
        for i in xrange( max(ibin-tave,0), ibin+tave ):
            ave_window += ht.GetBinContent(i+1)
            nbinsum += 1
        ave_window /= float( nbinsum )
        #ht2.SetBinContent( ibin, hits_window )
        ht2.SetBinContent( ibin, ave_window )

        if len(active_pulses)==0:
            # looking for new pulse
            if hits_window>(thresh+thresh_adjust):
                # start new pulse
                active_pulses.append( npulses )
                pulses[ npulses ] = { "tstart":ibin, "nhits":hits_window, "end":False, "last_level":float(hits_window)/float(window), "nfalling":0 }
                print "PULSE FOUND (",npulses,"): ",ibin,hits_window
                npulses += 1
            hthresh.SetBinContent( ibin, thresh )
        else:

            # look for secondary peak
            mod_thresh = 0
            onerising = False
            for pulse in active_pulses:
                if "peak" not in pulses[pulse]:
                    # we still have a rising peak. make threshold impossble
                    mod_thresh += 2.0*hits_window
                    onerising = True
                else:
                    expecthits = pulses[pulse]["peak"]*exp( -0.6*( ibin-pulses[pulse]["tpeak"] )/45.5 - 0.4*( ibin-pulses[pulse]["tpeak"] )/120.0 )
                    mod_thresh += expecthits + 3.0*sqrt(expecthits)
                    onefall = True
            hthresh.SetBinContent( ibin, mod_thresh )
            # second pulse thresh
            #if hits_window/float(window)>mod_thresh:
            if onerising==False and ave_window>mod_thresh:
                # start new pulse
                active_pulses.append( npulses )
                pulses[ npulses ] = { "tstart":ibin, "nhits":hits_window, "end":False, "last_level":float(hits_window)/float(window), "nfalling":0 }
                print "PULSE FOUND (",npulses,") (overlap): ",ibin,hits_window
                npulses += 1
            #if onefall:
            #    print "   mod thresh: ",mod_thresh, "hit_win/win=",float(hits_window)/float(window)," ave_win=",ave_window


            # in pulse. looking for peak and end
            for pulse in active_pulses:
                if "peak" not in pulses[pulse]:
                    # now looking for peak
                    if ave_window<pulses[pulse]["last_level"]:
                        pulses[pulse]["nfalling"] += 1
                    else:
                        pulses[pulse]["nfalling"] = 0
                        pulses[pulse]["last_level"] = ave_window                        

                    if pulses[pulse]["nfalling"]>3:
                        print "Found peak: ",ibin-5
                        pulses[pulse]["tpeak"] = ibin-5
                        pulses[pulse]["peak"] = ave_window
                else:
                    if ibin >pulses[pulse]["tpeak"]+8*45.0:
                        print "End of pulse %d found: "%(pulse),ibin
                        pulses[pulse]["tend"] = pulses[pulse]["tpeak"]+8*45.0 # for now

        for ipulse,pulseinfo in pulses.items():
            if "tend" in pulseinfo and ipulse in active_pulses:
                print "Remove pulse=",ipulse
                active_pulses.remove(ipulse)
                
    if len(pulses)<2:
        continue

    c.cd(1)
    ht.Draw()
    ipulses = pulses.keys()
    ipulses.sort()
    lines = []
    fits = []
    for pulse in ipulses:
        pulseinfo = pulses[pulse]
        s = ROOT.TLine( pulseinfo['tstart'], 0, pulseinfo['tstart'], 50 )
        if "tpeak" in pulseinfo:
            p = ROOT.TLine( pulseinfo['tpeak'], 0, pulseinfo['tpeak'], 50 )
            lines.append( s )
            lines.append( p )
            s.Draw()
            p.Draw()
            f = ROOT.TF1("fit%d"%(pulse),"[0]*TMath::Exp( -(x-[1])/[2] )", pulseinfo['tpeak'], pulseinfo['tpeak']+45*8 )
            f.SetParameter(0, pulseinfo['peak'] )
            f.SetParameter(1, pulseinfo['tpeak'] )
            f.SetParameter(2, 45.0 )
            f.Draw("Rsame")
            fits.append(f)

    c.cd(2)
    #ht2.Scale(1.0/float(window))
    ht2.Draw()
    hthresh.Draw("same")
    for l in lines:
        l.Draw()
    for f in fits:
        f.Draw("Rsame")

    c.Update()
    if npes>0:
        raw_input()

raw_input()
