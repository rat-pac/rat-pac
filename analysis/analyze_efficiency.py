import ROOT
ROOT.gSystem.Load('libRATEvent')
from ROOT.RAT import DSReader
from root_numpy import root2array
import numpy as np
import pandas as pd
from math import sqrt

pmtinfo = pd.DataFrame( root2array('data/kpipe/PMTINFO.root', 'pmtinfo' ) )
pmtinfo = pmtinfo.set_index('opdetid')

#reader = DSReader('kpipeout_test.root')
reader = DSReader('output_kdar_test.root')
nevents = reader.GetTotal()

out = ROOT.TFile('output_test.root','recreate')

c = ROOT.TCanvas('c','',1200,600)
c.Divide(2,1)
c.cd(1)

hz = ROOT.TH1D('hz','',200,-5000,5000)
hxy = ROOT.TH2D('hxy','',150,-150,150,150,-150,150)

for iev in xrange(0,nevents):
    dsroot = reader.NextEvent()
    mc = dsroot.GetMC()
    npes = mc.GetNumPE()
    npmts = mc.GetMCPMTCount()

    print "EVENT ",iev
    for ipart in xrange(0,mc.GetMCParticleCount()):
        pmom = 0.
        mom = mc.GetMCParticle(ipart).GetMomentum()
        pmom = mom.X()*mom.X()  + mom.Y()*mom.Y() + mom.Z()*mom.Z()
        pmom = sqrt(pmom)
        pos = mc.GetMCParticle(ipart).GetPosition()
        print "  ",ipart,") pdg=",mc.GetMCParticle(ipart).GetPDGCode()," Mom=",pmom,"=",mom.X(),",",mom.Y(),",",mom.Z()," pos=",pos.X(),pos.Y(),pos.Z()
    vertex = mc.GetMCParticle(0).GetPosition()
    print "NPEs in event: ",npes
    print "NPMTs hit: " ,npmts
    hz.Reset()
    hxy.Reset()
    if npes>0:
        weight_mean = [0.0,0.0]
        npe_tot = 0.0
        for ipmt in xrange(0,npmts):
            pmt = mc.GetMCPMT( ipmt )
            npe_pmtid = pmt.GetMCPhotonCount()
            pmtid = pmt.GetID()
            x = pmtinfo.loc[pmtid]['x']
            y = pmtinfo.loc[pmtid]['y']
            z = pmtinfo.loc[pmtid]['z']
            weight_mean[0] += x*float(npe_pmtid)
            weight_mean[1] += y*float(npe_pmtid)
            hz.Fill( z, float(npe_pmtid) )
            hxy.Fill( x,y, float(npe_pmtid) )
            npe_tot += float(npe_pmtid)
            #print "PMT %d Hit. NPES=%f. Pos=(%.2f,%.2f,%.2f)"%(pmtid, npe_pmtid, x, y, z)
        markrecon = ROOT.TMarker( weight_mean[0]/npe_tot,  weight_mean[1]/npe_tot, 30 )
        markrecon.SetMarkerColor(4)
    # Truth
    markpos = ROOT.TMarker( vertex.X()*0.1, vertex.Y()*0.1, 28 )
    markpos.SetMarkerColor( 6 )
    markz = ROOT.TLine( vertex.Z()*0.1, 0, vertex.Z()*0.1, hz.GetMaximum() )
    markz.SetLineColor(2)

    c.cd(1)
    hz.Draw()
    markz.Draw()
    c.cd(2)
    hxy.Draw("COLZ")        
    markpos.Draw()
    if npes>0:
        markrecon.Draw()
    c.Update()
    raw_input()

hz.Scale(1.0/100.0)
hxy.Scale(1.0/100.0)
c.cd(1)
hz.Draw()
c.cd(2)
hxy.Draw("COLZ")
c.Update()
raw_input()
