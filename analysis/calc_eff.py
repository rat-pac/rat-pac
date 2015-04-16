import os,sys
from ROOT import *

gStyle.SetOptStat(0)

#f = TFile("run0_100_scraped.root") # cherenkov+scintillation
f = TFile("run300_399_scraped.root") # cherenkov+scintillation+reflections
mcdata = f.Get("mcdata")

tcut = 1000.0
qe = 0.30
ntotal = mcdata.GetEntries("mumomv>0 && rv<140.0 && abs(z)<5000.0")

heff = TH2D("heff",";nprompt;nlate",10,0,10,10,0,30)
for nprompt in xrange(0,10):
    for nlate in xrange(0,30):
        nevents =  mcdata.GetEntries("mumomv>0 && rv<140.0 && abs(z)<5000.0 && tave_dcye>%.2f && npe_prompt*%.2f>=%d && npe_late*%.2f>=%d"
                                     %(tcut,qe, nprompt, qe, nlate))
        eff = float(nevents)/float(ntotal)
        heff.SetBinContent(nprompt+1,nlate+1,eff)

c = TCanvas("c","c",1200,600)
c.Draw()
heff.Draw("COLZ")
c.Update()
raw_input()
        
