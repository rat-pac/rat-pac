import ROOT
ROOT.gSystem.Load('libRATEvent')
from ROOT.RAT import DSReader
from root_numpy import root2array
import numpy as np
import pandas as pd
from math import sqrt

pmtinfo = pd.DataFrame( root2array('../data/kpipe/PMTINFO.root', 'pmtinfo' ) )
pmtinfo = pmtinfo.set_index('opdetid')

#reader = DSReader('kpipeout_test.root')
#reader = DSReader('../output_kdar_test.root')
reader = DSReader("output_kpipe_200.root")
nevents = reader.GetTotal()

out = ROOT.TFile('output_test.root','recreate')

c = ROOT.TCanvas('c','',1200,1200)
c.Divide(1,3)
c.cd(2).Divide(2,1)
c.cd(3).Divide(2,1)

time_cut = 500.0

hz = ROOT.TH1D('hz','',200,-5000,5000)
hxy = ROOT.TH2D('hxy','',150,-150,150,150,-150,150)
hz1 = ROOT.TH1D('hz1','',200,-5000,5000)
hxy1 = ROOT.TH2D('hxy1','',150,-150,150,150,-150,150)
hz2 = ROOT.TH1D('hz2','',200,-5000,5000)
hxy2 = ROOT.TH2D('hxy2','',150,-150,150,150,-150,150)
htime = ROOT.TH1D("ht","",100, 0, 20000)

for iev in xrange(0,nevents):
    dsroot = reader.NextEvent()
    mc = dsroot.GetMC()
    npes = mc.GetNumPE()
    npmts = mc.GetMCPMTCount()

    npe_prompt = 0
    npe_late = 0

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
    hz1.Reset()
    hxy1.Reset()
    hz2.Reset()
    hxy2.Reset()
    htime.Reset()
    if npes>0:
        weight_mean = [0.0,0.0]
        weight_mean1 = [0.0,0.0]
        weight_mean2 = [0.0,0.0]
        quad_pos = [0.0,0.0]
        npe_tot = 0.0
        for ipmt in xrange(0,npmts):
            pmt = mc.GetMCPMT( ipmt )
            npe_pmtid = pmt.GetMCPhotonCount()
            pmtid = pmt.GetID()
            hoopid = int(pmtid)/100
            pmt_hoop = int(pmtid)%100
            quadid = int(pmt_hoop)/25
            x = pmtinfo.loc[pmtid]['x']
            y = pmtinfo.loc[pmtid]['y']
            z = pmtinfo.loc[pmtid]['z']
            weight_mean[0] += x*float(npe_pmtid)
            weight_mean[1] += y*float(npe_pmtid)
            qhpos = 0.707*135.0*float(npe_pmtid)
            if quadid==0:
                quad_pos[0] += qhpos
                quad_pos[1] += qhpos
            elif quadid==1:
                quad_pos[0] -= qhpos
                quad_pos[1] += qhpos
            elif quadid==2:
                quad_pos[0] -= qhpos
                quad_pos[1] -= qhpos
            elif quadid==3:
                quad_pos[0] += qhpos
                quad_pos[1] -= qhpos
                
            hz.Fill( z, float(npe_pmtid) )
            hxy.Fill( x,y, float(npe_pmtid) )
            npe_tot += float(npe_pmtid)
            for ihit in xrange(0,npe_pmtid):
                htime.Fill( pmt.GetMCPhoton(ihit).GetHitTime() )
                if pmt.GetMCPhoton(ihit).GetHitTime()<time_cut:
                    npe_prompt += 1
                    hz1.Fill( z, float(npe_pmtid) )
                    hxy1.Fill( x,y, float(npe_pmtid) )
                    weight_mean1[0] += x
                    weight_mean1[1] += y
                else:
                    npe_late += 1
                    hz2.Fill( z, float(npe_pmtid) )
                    hxy2.Fill( x,y, float(npe_pmtid) )
                    weight_mean2[0] += x
                    weight_mean2[1] += y
            #print "PMT %d Hit. NPES=%f. Pos=(%.2f,%.2f,%.2f)"%(pmtid, npe_pmtid, x, y, z)
        markrecon = ROOT.TMarker( weight_mean[0]/npe_tot,  weight_mean[1]/npe_tot, 30 )
        markrecon.SetMarkerColor(4)
        print "quad pos: ",quad_pos[0]/npe_tot,quad_pos[1]/npe_tot
        markquad = ROOT.TMarker( quad_pos[0]/npe_tot,  quad_pos[1]/npe_tot, 30 )
        markquad.SetMarkerColor(7)
        if npe_prompt>0:
            markrecon1 = ROOT.TMarker( weight_mean1[0]/npe_prompt,  weight_mean1[1]/npe_prompt, 30 )
            markrecon1.SetMarkerColor(4)
        if npe_late>0:
            markrecon2 = ROOT.TMarker( weight_mean2[0]/npe_late,  weight_mean2[1]/npe_late, 30 )
            markrecon2.SetMarkerColor(4)
        print "EARLY PE: ",npe_prompt
        print "LATE PE: ",npe_late
    # Truth
    markpos = ROOT.TMarker( vertex.X()*0.1, vertex.Y()*0.1, 28 )
    markpos.SetMarkerColor( 6 )
    markz = ROOT.TLine( vertex.Z()*0.1, 0, vertex.Z()*0.1, hz.GetMaximum() )
    markz.SetLineColor(2)

    c.cd(1)
    htime.Draw()

    c.cd(2).cd(1)
    hz1.Draw()
    markz.Draw()
    c.cd(2).cd(2)
    hxy1.Draw("COLZ")
    markpos.Draw()
    if npe_prompt>0:
        markquad.Draw()
        markrecon1.Draw()

    c.cd(3).cd(1)
    hz2.Draw()
    markz.Draw()
    c.cd(3).cd(2)
    hxy2.Draw("COLZ")
    markpos.Draw()
    if npe_late>0:
        markquad.Draw()
        markrecon2.Draw()
    
    c.Update()
    if npes>0:
        raw_input()

hz.Scale(1.0/100.0)
hxy.Scale(1.0/100.0)
c.cd(1)
hz.Draw()
c.cd(2)
hxy.Draw("COLZ")
c.Update()
raw_input()
