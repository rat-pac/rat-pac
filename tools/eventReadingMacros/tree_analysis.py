#!/usr/bin/python
import math
import time
from array import *
import numpy as n
import ROOT as ROOT
from ROOT import TCanvas, TPad, TFile, TPaveText, TGraph,TGraphErrors
from ROOT import gBenchmark, gStyle, gROOT,TColor, TChain,gSystem
ROOT.gSystem.Load("libRATEvent");
ROOT.gSystem.AddIncludePath(" -I$RATROOT/include");


from sys import stdout
import pandas
from pandas import *


def drange(start, stop, step):
    rii = start
    while rii <= stop:
        yield rii
        rii += step

if __name__ == "__main__":

#    ''' Prepare a TChain object for the file'''2
#    T_10_10 =  TChain("T")
#    ''' 10 inch Monte Carlo Loading'''
#    T_10_10.Add('output.root')
#    ''' Print out an example of the variables'''
#    T_10_10.Show(0)
#
##    for event in T_10_10:
##        print "%s \n" %(event.ds.ev.qTotal)
    nageom  = ROOT.TFile( 'output_new.root' )
    runT    = ROOT.gROOT.FindObject('runT')
    T       = ROOT.gROOT.FindObject( 'T' )
#    T.Show(0)
#    runT.Show(0)

#   Load in the position of each PMT
    pmt_x,pmt_y,pmt_z = array( 'd' ),array( 'd' ),array( 'd' )
    nPMT = runT.Draw("run.pmtinfo.pos.x():run.pmtinfo.pos.y():run.pmtinfo.pos.z()","","goff")
    for event in range(nPMT):
        pmt_x.append(float(runT.GetV1()[event]))
        pmt_y.append(float(runT.GetV2()[event]))
        pmt_z.append(float(runT.GetV3()[event]))


    bfit_x,bfit_y,bfit_z = array( 'd' ),array( 'd' ),array( 'd' )
    nFITS = T.Draw("ds.ev.bonsaifits.GetPosition().x():ds.ev.bonsaifits.GetPosition().y():ds.ev.bonsaifits.GetPosition().z()","","goff")
    for event in range(nFITS):
        bfit_x.append(float(T.GetV1()[event]))
        bfit_y.append(float(T.GetV2()[event]))
        bfit_z.append(float(T.GetV3()[event]))

#    cfit_x,cfit_y,cfit_z = array( 'd' ),array( 'd' ),array( 'd' )
#    nFITS = T.Draw("ds.ev.centroid.pos.x():ds.ev.centroid.pos.y():ds.ev.centroid.pos.z()","","goff")
#    for event in range(nFITS):
#        cfit_x.append(float(T.GetV1()[event]))
#        cfit_y.append(float(T.GetV2()[event]))
#        cfit_z.append(float(T.GetV3()[event]))
#   Load int the position of the particle and the PMT time

    n = T.Draw("ds.mc.particle.pos.x():ds.mc.particle.pos.y():ds.mc.particle.pos.z()","","goff")
    partcl_x,partcl_y,partcl_z = array( 'd' ),array( 'd' ),array( 'd' )
    print n
    for event in range(n):
        partcl_x.append(float(T.GetV1()[event]))
        partcl_y.append(float(T.GetV2()[event]))
        partcl_z.append(float(T.GetV3()[event]))
#        pmt_t.append(float(T.GetV4()[event]))

#        ds.ev.pmt.id, ds.ev.pmt.charge ,  ds.ev.pmt.time
#        ds.ev.pmt.id:ds.ev.pmt.charge:ds.ev.pmt.time

    hResidual = ROOT.TH1F( 'hResidual', 'Residuals', 1000, -100, 100 )

    hReconstruction  = ROOT.TH1F( 'hReconstruction', 'hReconstruction (cm)', 1000, 0, 1000 )
    hReconstructionX = ROOT.TH1F( 'hReconstructionX', 'hReconstruction X (cm)', 1000, -1000, 1000 )
    hReconstructionY = ROOT.TH1F( 'hReconstructionY', 'hReconstruction Y (cm)', 1000, -1000, 1000 )
    hReconstructionZ = ROOT.TH1F( 'hReconstructionZ', 'hReconstruction Z (cm)', 1000, -1000, 1000 )

    hReconstruction_C  = ROOT.TH1F( 'hReconstruction_C', 'hReconstruction (cm)', 1000, 0, 1000 )
    hReconstructionX_C = ROOT.TH1F( 'hReconstructionX_C', 'hReconstruction X (cm)', 1000, -1000, 1000 )
    hReconstructionY_C = ROOT.TH1F( 'hReconstructionY_C', 'hReconstruction Y (cm)', 1000, -1000, 1000 )
    hReconstructionZ_C = ROOT.TH1F( 'hReconstructionZ_C', 'hReconstruction Z (cm)', 1000, -1000, 1000 )

    for pmtInEvent in range(n):
        pmt_cnt = T.Draw("ds.ev.pmt.id:ds.ev.pmt.charge:ds.ev.pmt.time","Entry$==%d"%(pmtInEvent),"goff")
        mcx = partcl_x[pmtInEvent]
        mcy = partcl_y[pmtInEvent]
        mcz = partcl_z[pmtInEvent]
        x_reco = bfit_x[pmtInEvent]
        y_reco = bfit_y[pmtInEvent]
        z_reco = bfit_z[pmtInEvent]
#
#        x_reco_c = cfit_x[pmtInEvent]
#        y_reco_c = cfit_y[pmtInEvent]
#        z_reco_c = cfit_z[pmtInEvent]

        Radius_reco = math.sqrt((x_reco-mcx)**2 + (y_reco-mcy)**2 + (z_reco-mcz)**2)
        hReconstructionX.Fill((x_reco-mcx)/10.0)
        hReconstructionY.Fill((y_reco-mcy)/10.0)
        hReconstructionZ.Fill((z_reco-mcz)/10.0)

        hReconstruction.Fill(Radius_reco/10.0)
#
#        Radius_reco_C = math.sqrt((x_reco_c-mcx)**2 + (y_reco_c-mcy)**2 + (z_reco_c-mcz)**2)
#        hReconstructionX_C.Fill((x_reco_c-mcx)/10.0)
#        hReconstructionY_C.Fill((y_reco_c-mcy)/10.0)
#        hReconstructionZ_C.Fill((z_reco_c-mcz)/10.0)
#        hReconstruction_C.Fill(Radius_reco_C/10.0)
#


        for index in range(pmt_cnt):
            pmtID =int(T.GetV1()[index])
            eventIs = int(pmtInEvent)
            pmt_index = int(index)
            pmtForEvent = int(pmt_cnt)
            charge = float(T.GetV2()[index])

            X_pmt = pmt_x[pmtID]
            Y_pmt = pmt_y[pmtID]
            Z_pmt = pmt_z[pmtID]
            T_pmt = float(T.GetV3()[index])
            T_flight = math.sqrt((X_pmt-mcx)**2 + (Y_pmt-mcy)**2 + (Z_pmt-mcz)**2)/21.583
            
            
            print "%4d %4d %4d  %5.3f %9.2f %9.2f %9.2f %6d %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f"%(int(pmtInEvent), int(index), int(pmt_cnt),float(T.GetV2()[index]),partcl_x[pmtInEvent],partcl_y[pmtInEvent],partcl_z[pmtInEvent],int(T.GetV1()[index]),pmt_x[pmtID],pmt_y[pmtID],pmt_z[pmtID],float(T.GetV3()[index]), T_flight, T_flight/10., T_pmt - T_flight/10.,Radius_reco)
            hResidual.Fill(T_pmt - T_flight/10.)





