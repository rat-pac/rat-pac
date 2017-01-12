#!/bin/python
#import ROOT
from ROOT import TRandom3
from ROOT import TChain,TGraph,TGraphErrors,gSystem,gROOT,TH1D,TH2D,TFile,TCanvas
from ROOT import THStack,Double
from ROOT import kRed,kBlue,kGreen,kCyan,kOrange

import os.path
from stat import S_IRWXG,S_IRWXU
from shutil import rmtree
import warnings

import numpy as np
from numpy import array as npa
from numpy import power,absolute,logical_and,column_stack,zeros,empty,append,sqrt,absolute

from math import pow,exp,log10

try:
    from root_numpy import root2rec
    from rootpy.plotting import Canvas,Hist,Hist2D,Graph
    from rootpy.plotting.style import set_style
    from rootpy.io import root_open
    #from rootpy.interactive import wait

    set_style('ATLAS')

    warnings.simplefilter("ignore")
except:
    print "Could not load in root_numpy or rootpy, they are required to run this module."

docstring = """
    Usage: watchmakers.py [options]

    Arguments:

    Options:
    -f=<ifile>          Input file [Default: root_files/merged_ntuple_watchman]
    -o=<outputfile>     Optional output file [Default: processed_watchman.root]
    -M                  Merge result files
    -m                  generate macro files
    -n                  generate ntuple from analysis files
    -N=<N>              Number of MC script that were run [Default: 3]
    -e=<runBeamEntry>   Number of entries per macro [Default: 5000]
    -a                  Do the analysis on the merged file
    -j                  Create submision scripts and macros
    -r=<rate>           rate of accidentals in hz [Default: 10.0]
    -d=<distance>       Maximal distance between two events (m) [Default: 2.0]
    -t=<time>           Maximal time between two events (micro) [Default: 100.0]
    -T=<tubes>          Minimal number of tubes hit [Default: 3.0]
    -g=<goodness>       Bonsai position goodness parameter [Default: 0.1]
    -G=<goodness>       Bonsai direction goodness parameter [Default: 0.1]
    -P                  Pick a single physics process to analyis (used for ntup)
    -C                  Pick a single coverage to analyse
    -R                  Read analyisis result
    -D                  Delete all current photocoverage directory.
    """

try:
    import docopt
    arguments = docopt.docopt(docstring)
    print 'using docopt as the user control interface'
except ImportError:
    print 'docopt is not a recognized module, it is required to run this module'



fv_rad = 5.42
gSystem.Load("libRATEvent")
gSystem.AddIncludePath(" -I$RATROOT/include")

gROOT.LoadMacro("$RATROOT/tools/photocoverageAnalysisOnlyData.C")
from ROOT import photocoverageAnalysisOnlyData



##### Define a set of functions to do analysis ##############

def fillHistograms(inFile,a1,t1,h,cover,ii,locj,covPCT):
    # Obtain logarithmic binnings
    nbins, xbins, ybins = logx_logy_array()

    #Read-in file
    try:
        s =  "%s_%s_%s_%s.root"%(inFile,ii,cover,locj)
#        data = root2array(s)
        t           = root2rec(s)

        #Apply some analysis
        r           = npa(t.reco_r<5.4,dtype=bool)
        z           = npa(absolute(t.reco_z)<5.4,dtype=bool)
        isFV        = logical_and(r,z,dtype=bool)
        notFV       = npa(isFV!=1,dtype=bool)
        isFV_t      = npa(t.FV_t==1,dtype=bool)
        notFV_t     = npa(t.FV_t!=1,dtype=bool)

        iCandidate  = npa(t.candidate==1,dtype=bool)
        totalEvtGen = npa(t.all_ev==t.all_ev_tot,dtype=bool)
        tot         = float(sum(totalEvtGen))
        totD        = float(len(t.FV))

        si          = logical_and(isFV,notFV_t,dtype=bool)
        so          = logical_and(notFV,isFV_t,dtype=bool)
        ei          = logical_and(isFV,isFV_t,dtype=bool)
        eo          = logical_and(notFV,notFV_t,dtype=bool)

        ssi         = sum(si)
        sso         = sum(so)
        sei         = sum(ei)
        seo         = sum(eo)
        print 'MC events generated',tot,', number of MC recorded',len(t.FV)
        print ssi,sso,sei,seo,', absolute: ', ssi/tot,sso/tot,sei/tot,seo/tot, \
        'relative',ssi/totD,sso/totD,sei/totD,seo/totD

        #Define set of histograms to fill out
        for subE in range(5):
            if subE ==0:
                subEv0   = npa(t.candidate==1,dtype=bool)
            else:
                subEv0   = npa(t.detected_ev==subE,dtype=bool)
            suma = sum(subEv0)
            if suma:
                print 'Sub event ',subE,' : ',suma
                totRel = float(suma)
                mask = logical_and(subEv0,si,dtype=bool)
                if sum(mask):
                    type = "si"
                    print subE,type,totRel,'(',sum(mask),')'

                    string = "%s_%s_%s_%d_abs"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/tot)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/tot)
                    string = "%s_%s_%s_%d_rel"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/totRel)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/totRel)

                    anaVar      = t.pe[mask,...]
                    s_dl        = "%s_pe_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)

                    anaVar      = t.nhit[mask,...]
                    s_dl        ="%s_nhit_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)

                    i_d         = t.inner_dist[mask,...]
                    i_t         = t.inner_time[mask,...]
                    anaVar      = column_stack((i_d,i_t/1e9))
                    s_dl       ="%s_dDdT_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist2D(xbins,ybins,name=s_dl,title=s_dl);
                    h[s_dl].SetXTitle('inter-event distance [m]')
                    h[s_dl].SetYTitle('inter-event time [s]')
                    h[s_dl].fill_array(anaVar)
                    r_r         = t.reco_r[mask,...]
                    r_z         = t.reco_z[mask,...]
                    anaVar      = column_stack((power(r_r/6.4,2),r_z/6.4))
                    s_dl       = "%s_dRdZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)
                    t_r         = t.true_r[mask,...]
                    t_z         = t.true_z[mask,...]
                    anaVar      = column_stack((power(t_r/6.4,2),t_z/6.4))
                    s_dl       = "%s_tRtZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)




                mask = logical_and(subEv0,so)
                if sum(mask):

                    type        = "so"
                    print subE,type,totRel,'(',sum(mask),')'

                    string = "%s_%s_%s_%d_abs"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/tot)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/tot)
                    string = "%s_%s_%s_%d_rel"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/totRel)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/totRel)
                    anaVar      = t.pe[mask,...]
                    s_dl        = "%s_pe_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)

                    anaVar      = t.nhit[mask,...]
                    s_dl        ="%s_nhit_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)


                    i_d         = t.inner_dist[mask,...]
                    i_t         = t.inner_time[mask,...]
                    anaVar      = column_stack((i_d,i_t/1e9))
                    s_dl        ="%s_dDdT_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist2D(xbins,ybins,name=s_dl,title=s_dl);
                    h[s_dl].SetXTitle('inter-event distance [m]')
                    h[s_dl].SetYTitle('inter-event time [s]')
                    h[s_dl].fill_array(anaVar)
                    r_r         = t.reco_r[mask,...]
                    r_z         = t.reco_z[mask,...]

                    anaVar      = column_stack((power(r_r/6.4,2),r_z/6.4))
                    s_dl        ="%s_dRdZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)
                    t_r         = t.true_r[mask,...]
                    t_z         = t.true_z[mask,...]

                    anaVar      = column_stack((power(t_r/6.4,2),t_z/6.4))
                    s_dl       = "%s_tRtZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)


                mask = logical_and(subEv0,ei)
                if sum(mask):

                    type        = "ei"
                    print subE,type,totRel,'(',sum(mask),')'


                    string = "%s_%s_%s_%d_abs"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/tot)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/tot)
                    string = "%s_%s_%s_%d_rel"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/totRel)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/totRel)
                    anaVar      = t.pe[mask,...]
                    s_dl        = "%s_pe_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)


                    anaVar      = t.nhit[mask,...]
                    s_dl= "%s_nhit_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)

                    i_d         = t.inner_dist[mask,...]
                    i_t         = t.inner_time[mask,...]
                    anaVar      = column_stack((i_d,i_t/1e9))
                    s_dl= "%s_dDdT_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist2D(xbins,ybins,name=s_dl,title=s_dl);
                    h[s_dl].SetXTitle('inter-event distance [m]')
                    h[s_dl].SetYTitle('inter-event time [s]')
                    h[s_dl].fill_array(anaVar)
                    r_r         = t.reco_r[mask,...]
                    r_z         = t.reco_z[mask,...]
                    anaVar      = column_stack((power(r_r/6.4,2),r_z/6.4))
                    s_dl= "%s_dRdZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)

                    t_r         = t.true_r[mask,...]
                    t_z         = t.true_z[mask,...]

                    anaVar      = column_stack((power(t_r/6.4,2),t_z/6.4))
                    s_dl       = "%s_tRtZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)

                mask = logical_and(subEv0,eo)
                if sum(mask):

                    type        = "eo"
                    print subE,type,totRel,'(',sum(mask),')'
                    string = "%s_%s_%s_%d_abs"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/tot)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/tot)
                    string = "%s_%s_%s_%d_rel"%(type,ii,locj,subE)
                    if string in h:
                        point = h[string].GetN()
                        h[string].SetPoint(point,covPCT,sum(mask)/totRel)
                    else:
                        h[string] = Graph()
                        h[string].SetName(string)
                        h[string].SetPoint(0,covPCT,sum(mask)/totRel)

                    anaVar      = t.pe[mask,...]
                    s_dl         = "%s_pe_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)

                    anaVar      = t.nhit[mask,...]
                    s_dl        ="%s_nhit_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]     = Hist(5000,0,500,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('photoelectrons')
                    h[s_dl].SetYTitle('counts [a.u.]')
                    h[s_dl].fill_array(anaVar)

                    i_d         = t.inner_dist[mask,...]
                    i_t         = t.inner_time[mask,...]
                    anaVar      = column_stack((i_d,i_t/1e9))
                    s_dl        ="%s_dDdT_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(xbins,ybins,name=s_dl,title=s_dl);
                    h[s_dl].SetXTitle('inter-event distance [m]')
                    h[s_dl].SetYTitle('inter-event time [s]')
                    h[s_dl].fill_array(anaVar)
                    r_r         = t.reco_r[mask,...]
                    r_z         = t.reco_z[mask,...]
                    anaVar      = column_stack((power(r_r/6.4,2),r_z/6.4))
                    s_dl       = "%s_dRdZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)
                    t_r         = t.true_r[mask,...]
                    t_z         = t.true_z[mask,...]
                    anaVar      = column_stack((power(t_r/6.4,2),t_z/6.4))
                    s_dl       = "%s_tRtZ_%s_%s_%s_%d"%(type,cover,ii,locj,subE)
                    h[s_dl]= Hist2D(100,0,1.6,200,-1.6,1.6,name=s_dl,title=s_dl)
                    h[s_dl].SetXTitle('(r / R_{pmt})^{2} [unitless]')
                    h[s_dl].SetYTitle('(z / Z_{pmt}) [unitless]')
                    h[s_dl].fill_array(anaVar)


    except:
        print "Could not read file ",s

    print ""
#        f.Close()
    return h

def logx_logy_array(nbins = 500,xmin = 1e-2,xmax = 30.,ymin = 1e-9,ymax = 1e3):
    #x-axis
    logxmin = log10(xmin)
    logxmax = log10(xmax)
    xbinwidth= (logxmax-logxmin)/float(nbins)
    xbins	= zeros(nbins,dtype=float)
    xbins[0] = xmin
    #y-axis
    logymin = log10(ymin)
    logymax = log10(ymax)
    ybinwidth= (logymax-logymin)/float(nbins)
    ybins	= zeros(nbins,dtype=float)
    ybins[0] = ymin
    for i in range(nbins):
        xbins[i] = xmin + pow(10,logxmin+i*xbinwidth)
        ybins[i] = ymin + pow(10,logymin+i*ybinwidth)
    return nbins,xbins,ybins

def loadSimulationParameters():
    #Chain and subsequent isotopes
    d = {}
    # Water and PMT contamination
    d['CHAIN_238U_NA'] =['238U','234Pa','214Pb','214Bi','210Bi','210Tl']
    d['CHAIN_232Th_NA'] = ['232Th','228Ac','212Pb','212Bi','208Tl']
    d['CHAIN_222Rn_NA'] = ['222Rn','214Pb','214Bi','210Bi','210Tl']
    # Radioisotope that should have beta-Neutron modes, (beta only generated)
    A = ['16','17','18','17','18','8','9','11']
    Z = ['6','6','6','7','7','2','3','3']
    ZA = A
    for i in range(len(A)):
        ZA[i] = str(int(A[i])*1000 +int(Z[i]))
    d['A_Z'] =  ZA
    #Oscillated spectrum at Boulby and IMB site
    d['ibd'] = ['boulby','imb']
    d['N'] = ['neutron']
    d['IBD'] = ['IBD']
    # Fast neutron contamination
    d['FN'] = ['QGSP_BERT_EMV','QGSP_BERT_EMX','QGSP_BERT','QGSP_BIC','QBBC',\
    'QBBC_EMZ','FTFP_BERT','QGSP_FTFP_BERT']
    #
    #List of all physics process clumped together
    iso = ['CHAIN_238U_NA','CHAIN_232Th_NA','CHAIN_222Rn_NA','A_Z','ibd',\
    'FN','N','IBD']
    loc = ['PMT','PMT','FV','RN','S','FN','N','I']
    #Photocoverage selected
    coverage = ['10pct','15pct','20pct','25pct','30pct','35pct','40pct']
    coveragePCT = {'10pct':9.86037,'15pct':14.887,'20pct':19.4453,\
    '25pct':24.994,'30pct':28.8925,'35pct':34.3254,'40pct':39.1385}

    return d, iso,loc,coverage,coveragePCT

def writeResultsToFile(s,g,h):
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    f_root = TFile(s,"recreate")
    for gE in g:
        g["%s"%(gE)].Write()
    for hE in h:
        h["%s"%(hE)].Write()
    f_root.Close()

def runAnalysisProcess(f,g,h):
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    aDT1 =tDT1 = a1 = t1 = 0

    for j in range(len(iso)):
        for ii in d["%s"%(iso[int(j)])]:
            for idx,cover in enumerate(coverage):
                print  coverage[idx],coveragePCT[cover],ii,loc[j]
                h  = fillHistograms(f,a1,t1,h,cover,ii,loc[j],\
                float(coveragePCT[cover]))
    return g,h

def mergeFiles():
    # Read external requirements
    #arguments = docopt.docopt(docstring)
    # Load internal requirements
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    trees = {}
    pathFinal = "root_files/merged_ntuple_watchman"
    for j in range(len(iso)):
        for ii in d["%s"%(iso[int(j)])]:
            for idx,cover in enumerate(coverage):
                t_name  = "data_%s_%s_%s"%(ii,cover,loc[j])
                trees[t_name] = TChain("data")

                if loc[j] != 'neutron':
                    s = "ntuple_%s/watchman_%s_%s_*_%s.root" %(cover,ii,cover,\
                    loc[j])
                    sw = "%s_%s_%s_%s.root"%(pathFinal,ii,cover,loc[j])
                else:
                    s = "ntuple_%s/watchman_%s_*_N.root" %(cover,cover)
                    sw = "%s_neutron_%s_N.root"%(pathFinal,cover)
                print "Writing ", sw,"from",s
                trees[t_name].Add(s)
                print "Number of entries ",trees[t_name].GetEntries()
                trees[t_name].Merge(sw)
                del trees[t_name]
    del trees
    return 0



def testCreateDirectory(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)
    else:
        rmtree(directory)
        os.makedirs(directory)

def deleteDirectory(directory):
    if os.path.exists(directory):
        rmtree(directory)

def macroGenerator(percentage,isotope,location,runs,events):
    
    covPCT = {'10pct':0.1,'15pct':0.15,'20pct':0.2,\
    '25pct':0.25,'30pct':0.30,'35pct':0.35,'40pct':0.40}

    #Part of the macro that is the same for all jobs
    dir = os.getcwd()
    
    header = '''
/glg4debug/glg4param omit_muon_processes  0.0
/glg4debug/glg4param omit_hadronic_processes  0.0

/rat/db/set DETECTOR experiment "Watchman"
/rat/db/set DETECTOR detector_factory "Watchman"
/rat/db/set WATCHMAN_PARAMS photocathode_coverage %4.2f


/run/initialize

# BEGIN EVENT LOOP
/rat/proc lesssimpledaq
/rat/proc fitbonsai
/rat/proc fitcentroid
/rat/proc fitpath
/rat/proc count
/rat/procset update 1000

# Use IO.default_output_filename
/rat/proclast outroot
/rat/procset file "%s/root_files/watchman_%s_%s_%s_%d.root"
#END EVENT LOOP

''' %(covPCT[percentage],dir,isotope,percentage,location,runs)
    
    
    #Part of macro that varies with the various conditions
    if location == 'PMT':
        line1 = '''
/generator/add decaychain %s:regexfill
/generator/pos/set inner_pmts[0-9]+

/run/beamOn %d''' %(isotope,events)
    elif location == 'FV':
        line1 = '''
/generator/add decaychain %s:fill:poisson
/generator/pos/set  0 0 0
/generator/rate/set 6.43

/run/beamOn %d''' %(isotope,events)
    elif location == 'FN':
        line1 = '''
/generator/add combo fastneutron:regexfill
/generator/pos/set rock_[0-9]+
/generator/vtx/set 0 0 0
/generator/fastneutron/depth 1434.0
/generator/fastneutron/enthresh 10.0
/generator/fastneutron/sidewalls 1.0

/run/beamOn %d'''%(events)
    elif location == 'FNimb':
        line1 = '''
/generator/add combo fastneutron:regexfill
/generator/pos/set rock_[0-9]+
/generator/vtx/set 0 0 0
/generator/fastneutron/depth 1434.0
/generator/fastneutron/enthresh 10.0
/generator/fastneutron/sidewalls 1.0

/run/beamOn %d'''%(events)
    elif location == 'FNboulby':
        line1 = '''
/generator/add combo fastneutron:regexfill
/generator/pos/set rock_[0-9]+
/generator/vtx/set 0 0 0
/generator/fastneutron/depth 2805.
/generator/fastneutron/enthresh 10.0
/generator/fastneutron/sidewalls 1.0

/run/beamOn %d'''%(events)
    elif    location=='I':
        line1 = '''
/generator/add combo ibd:fill
/generator/vtx/set  1 0 0
/generator/pos/set 0 0 0

/run/beamOn %d'''%(events)
    elif location == 'S':
        line1 ='''
/generator/add combo spectrum:fill
/generator/vtx/set e+ %s
/generator/pos/set 0 0 0

/run/beamOn %d'''%(isotope,events)
    elif location =='N':
        line1 = '''
/generator/add combo gun2:fill
/generator/vtx/set %s  0 0 0 0 0.001 0.20
/generator/pos/set 0 0 0

/run/beamOn %d'''%(isotope,events)
    elif location == 'RN':
        AZ = isotope
        A =  int(int(AZ)/1000)
        Z = int(AZ) - A*1000
        line1 = '''
/generator/add combo isotope:fill
/generator/pos/set 0 0 0
/generator/vtx/set GenericIon 0 0 0
/generator/isotope/A %s.0
/generator/isotope/Z %s.0
/generator/isotope/E 0.0

/run/beamOn %d''' %(A,Z,events)
    else:
        line1 = 'A'
        print location
    return header+line1



def jobString(percentage,j,runs,models,arguments):
#    directory = "/p/lscratche/adg/Watchboy/simplifiedData/rp_sim/wm"
    directory   = os.getcwd()
    softDir     = "/usr/gapps/adg/geant4/rat_pac_and_dependency"
    ratDir      = os.environ['RATROOT']
    rootDir     = os.environ['ROOTSYS']
    g4Dir       =  os.environ['G4INSTALL']
    
    software    = "%s/bin/rat" %(ratDir)
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    
    ele =  d["%s"%(iso[j])]
    location = loc[j]
    
    goodness     = float(arguments['-g'])

    

    line1 = """#!/bin/sh
#MSUB -N WM_%s_%s_%d    #name of job
#MSUB -A adg         # sets bank account
#MSUB -l nodes=1:ppn=1,walltime=23:59:59,partition=borax  # uses 1 node
#MSUB -q pbatch         #pool
#MSUB -o %s/log/wmpc_%s_%s_%d.log
#MSUB -e %s/log/wmpc_%s_%s_%d.err
#MSUB -d %s  # directory to run from
#MSUB -V
#MSUB                     # no more psub commands

source %s/bin/thisroot.sh
source %s/../../../bin/geant4.sh
source %s/geant4make.sh
source %s/env.sh
export G4NEUTRONHP_USE_ONLY_PHOTONEVAPORATION=1\n
""" %(percentage,location,runs,\
directory,percentage,location,runs,\
directory,percentage,location,runs,\
directory,\
rootDir,g4Dir,g4Dir,ratDir)

    for mods in models:
        if location == "FN":
            line1 += "export PHYSLIST=%s\n" %(mods)
        
        line1 += "%s -l log/rat.%s_%s_%s_%d.log %s/macro_%s/run%s_%s_%d.mac\n" %(software,\
                                                      percentage,mods,location,runs,\
                                                                                 directory,percentage,mods,location,runs)
        fileN = "root_files/watchman_%s_%s_%s_%d.root" %(mods,percentage,location,runs)
        line1 += "python watchmakers.py -n -g %f -f %s\n" %(goodness,fileN)

    return line1



def generateMacros(N,e):
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
#    N = int(arguments['-N'])
    print N,e
    ##Clean or create macro directories
    for idx,cover in enumerate(coverage):
        dir = "macro_%s" %(cover)
        testCreateDirectory(dir)
#    for idx,cover in enumerate(coverage):
#        dir = "%s" %(cover)
#        testCreateDirectory(dir)

    for j in range(len(iso)):
        for ii in d["%s"%(iso[int(j)])]:
            for idx,cover in enumerate(coverage):
                for val in range(N):
                    line = macroGenerator(cover,ii,loc[j],val,e )
                    dir = "macro_%s" %(cover)

                    outfile = open("%s/run%s_%s_%d.mac" %(dir,ii,\
                    loc[j],val),"wb")
                    outfile.writelines(line)
                    outfile.close
    return 0

def removeMacrosAndDirectories():
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    for idx,cover in enumerate(coverage):
        dir = "macro_%s" %(cover)
        deleteDirectory(dir)


def generateJobs(N,arguments):
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    
    '''Find wheter the jobs folder exist: if no create, if yes clean and recreate'''
    directory = 'jobs'
    if not os.path.exists(directory):
        os.makedirs(directory)
    else:
        rmtree(directory)
        os.makedirs(directory)

    '''Find wheter the jobs folder exist: if no create, if yes clean and recreate'''
    directory = 'log'
    if not os.path.exists(directory):
        os.makedirs(directory)
    else:
        rmtree(directory)
        os.makedirs(directory)

    directory = 'root_files'
    if not os.path.exists(directory):
        os.makedirs(directory)

    directory = 'ntuple_root_files'
    if not os.path.exists(directory):
        os.makedirs(directory)

    '''Make sure that the softlink are correct for Bonsai input'''

    ratDir      = os.environ['RATROOT']

    src = ratDir+'/fit_param.dat'
    dst = os.getcwd()+'/fit_param.dat'
    if not os.path.exists(dst):
        os.symlink(src,dst)

    src = ratDir+'/like.bin'
    dst = os.getcwd()+'/like.bin'
    if not os.path.exists(dst):
        os.symlink(src,dst)

    job_list = '''#!/bin/sh
'''

    for j in range(len(iso)):
        for idx,cover in enumerate(coverage):
            models  = d["%s" %(iso[j])]
            for index in range(N):
                line = jobString(cover,j,index,models,arguments)
                stringFile = "jobs/jobs%s_%s_%s_%d.sh" %(cover,\
                                                            "%s"%(iso[int(j)]),loc[j],index)
                if index == 0:
                    job_list+= '(msub ' + stringFile +') || ./'+ stringFile + '\n'
                
                outfile = open(stringFile,"wb")
                outfile.writelines(line)
                if index < N-1:
                    stringFile1 = "(msub jobs/jobs%s_%s_%s_%d.sh || ./jobs/jobs%s_%s_%s_%d.sh)" %(cover,\
                                                                                                 "%s"%(iso[int(j)]),loc[j],index+1,cover,\
                                                                                                 "%s"%(iso[int(j)]),loc[j],index+1)
                    outfile.writelines(stringFile1)
                outfile.close
                os.chmod(stringFile,S_IRWXU)


    outfile = open('sub_jobs',"wb")
    outfile.writelines(job_list)
    outfile.close
    os.chmod('sub_jobs',S_IRWXG)
    os.chmod('sub_jobs',S_IRWXU)
    return 0


def extractNtuple(arguments):
    N            = int(arguments["-N"])
    rate         = float(arguments["-r"])
    timemask     = float(arguments['-t'])*1000.0
    distancemask = float(arguments['-d'])
    goodness     = float(arguments['-g'])
    dirGoodness  = float(arguments['-G'])
    minNHIT      = float(arguments['-T'])
    file         = arguments["-f"]
    print file
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    photocoverageAnalysisOnlyData(file,minNHIT,goodness,dirGoodness,timemask,rate,distancemask)


#    for j in range(len(iso)):
#        for ii in d["%s"%(iso[int(j)])]:
#            for idx,cover in enumerate(coverage):
#                for val in range(N):
#                    s =  "%s/watchman_%s_%s_%s_%d.root"%(cover,ii,cover,\
#                    loc[j],val)
#                    photocoverageAnalysisOnlyData(file,minNHIT,goodness,\
#                    dirGoodness,timemask,rate,distancemask)

def pickColor(H,_loc,r_c,o_c,b_c,c_c ):
    if _loc=='PMT':
        H.SetLineColor(kOrange+o_c)
        H.SetFillColor(kOrange+o_c)
        H.SetMarkerColor(kOrange+o_c)
        o_c+=1
    if _loc=='RN':
        H.SetLineColor(7)
        H.SetMarkerColor(7)
    if _loc=='FV':
        H.SetLineColor(kRed+r_c)
        H.SetFillColor(kRed+r_c)
        H.SetMarkerColor(kRed+r_c)
        r_c+=1
    if _loc=='S':
        H.SetLineColor(kBlue+b_c)
        H.SetFillColor(kBlue+b_c)
        H.SetMarkerColor(kBlue+b_c)
        b_c+=1
    if _loc=='N':
        H.SetLineColor(kCyan+c_c)
        H.SetMarkerColor(kCyan+c_c)
        c_c+=1
    return r_c,o_c,b_c,c_c, H


def integralCoincidence(R,lowerBound,upperBound):
    low = -exp(-lowerBound) * (1+lowerBound )
    up  = -exp(-upperBound) * (1+upperBound)
    return up - low


def histIntegral(s,f,cut):
    H = {}
    H[s] = f.Get(s)
    if H[s]!= None:
        a = H[s].Integral(0,int(cut*10))
        N = H[s].Integral(0,5000)
    else:
        a = 1.0
        N = 1.0

    return (1.0 - a/N )



def obtainAbsoluteEfficiency(f,timeScale='day',cut = 10.0):

    # Default units are in sec. Conversion factor are below
    timeSec     = 1.0/365./24./3600.

    # Number of free proton
    if timeScale == 'sec':
        timeS   = 1.0
    if timeScale == 'day':
        timeS   = 24.0*3600.
    if timeScale == 'month':
        timeS   = 365.0/12.*24.0*3600.
    if timeScale == 'year':
        timeS   = 365.0*24.0*3600.

    #Mass in kilograms
    mass = 2.0
    nKiloTons   = 3.22
    FreeProtons = 0.6065
    TNU         = FreeProtons* nKiloTons *timeSec

    #Fast neutrons conversion
    #Rock mass
    volumeR         = (2.*22.5*23.8*1.0+2.*17*23.8*1.0+2.*22.5*17.*1.0)
    density         = 2.39 #from McGrath
    rockMass        = volumeR*power(100.,3)*density
    avgMuon         = npa([180.,264.])
    avgMuonNC       = power(avgMuon,0.849)
    avgNFluxMag     = 1e-6
    muonRate        = npa([7.06e-7,4.09e-8]) # mu/cm2/s
    tenMeVRatio     = npa([7.51/34.1,1.11/4.86])
    fastNeutrons    = rockMass*avgMuonNC*avgNFluxMag*muonRate*tenMeVRatio

    avgRNYieldRC    = power(avgMuon,0.73)
    skRNRate        = 0.5e-7 # 1/mu/g cm2
    avgMuonSK       = power(219.,0.73)
    skMuFlux        = 1.58e-7 #mu/cm2/sec
    radionuclideRate= (skRNRate*avgRNYieldRC/avgMuonSK)*muonRate*nKiloTons*1e9

#    print "Using ",timeScale, " as a time scale"
#    print "Fast neutron ", fastNeutrons*timeS
#    print "radionuclide ", radionuclideRate*timeS
    #Radionuclides


    covPCT      = {'9.86037':1432., '14.887':2162.,'19.4453':2824.,\
    '24.994':3558.,'28.8925':4196.,'34.3254':4985.,'39.1385':5684.}
    pct         = npa([9.86037,14.887,19.4453,24.994,28.8925,\
    34.3254,39.1385])
    pctVal      = ['10pct','15pct','20pct','25pct','30pct','35pct','40pct']

    #Cuts
    f           = TFile(f,'read')
    EG          = {}
    inta        = ['si','so','eo','ei']

    #Add the U-238 chain
    proc        = ['234Pa','214Pb','214Bi','210Bi','210Tl']
    loca        = ['PMT',  'PMT',  'PMT',  'PMT',  'PMT']
    acc         = ['acc',  'acc',  'acc',  'acc',  'acc']
    br          = [1.0,     1.0,    1.0,   1.0 ,   0.002]
    site        = ['',      '',     '',     '',     '']
    arr         = empty(5)
    arr[:]      = 0.993
    Activity    = arr
    #Add the Th-232 chain
    proc        +=['232Th','228Ac','212Pb','212Bi','208Tl']
    loca        +=['PMT'  ,'PMT',   'PMT', 'PMT',  'PMT'  ]
    acc         +=['acc'  ,'acc',   'acc', 'acc',  'acc'  ]
    br          += [1.0,     1.0,    1.0,   1.0 ,   1.0]
    site        += ['',      '',     '',     '',     '']
    arr         = empty(5)
    arr[:]      = 0.124
    Activity    = append(   Activity,arr)
    #Add the Rn-222 chain
    proc        +=['214Pb','214Bi','210Bi','210Tl']
    loca        +=['FV',   'FV',   'FV',   'FV']
    acc         +=['acc',  'acc',  'acc',   'acc']
    br          += [1.0,   1.0,   1.0,     0.002]
    site        += ['',     '',     '',     '']
    arr = empty(4)
    arr[:]      = 6.4
    Activity    = append(   Activity,arr)


    #Add the neutrino signal
    proc        +=['imb','imb','boulby','boulby']
    loca        +=['S','S',     'S',  'S']
    acc         +=['di', 'corr', 'di', 'corr']
    br          += [1.0,  1.0, 1.0 , 1.0]
    site        += [''   ,'' , '', '']
    arr         = npa([7583.*TNU,7583.*TNU,924.48*TNU,924.48*TNU])
    Activity    = append(    Activity,arr)

    # Add the neutron
    proc        +=['neutron','neutron']
    loca        +=['N',     'N']
    acc         +=['corr',  'corr']
    br          += [1.0,   1.0]
    arr         = npa([7583.*TNU,924.48*TNU])
    # print "Neutrino activity ",arr*timeS/nKiloTons
    Activity    = append(    Activity,arr)
    site        += [ '','boulby']

    # add a fast neutron at Fairport
    proc        += ['QGSP_BERT_EMV','QGSP_BERT_EMX','QGSP_BERT','QGSP_BIC',\
    'QBBC','QBBC_EMZ','FTFP_BERT','QGSP_FTFP_BERT']
    loca        +=  ['FN','FN','FN','FN','FN','FN','FN','FN']
    acc         +=  ['corr','corr','corr','corr','corr','corr','corr','corr']
    br          +=  [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]
    arr = empty(8)
    arr[:]      = fastNeutrons[0]
    Activity    = append(Activity,arr)
    site        += ['',      '','',      '','',      '','',      '']
    # add a fast neutron at Boulby
    proc        += ['QGSP_BERT_EMV','QGSP_BERT_EMX','QGSP_BERT','QGSP_BIC',\
    'QBBC','QBBC_EMZ','FTFP_BERT','QGSP_FTFP_BERT']
    loca        +=  ['FN','FN','FN','FN','FN','FN','FN','FN']
    acc         +=  ['corr','corr','corr','corr','corr','corr','corr','corr']
    br          +=  [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]
    arr = empty(8)
    arr[:]      = fastNeutrons[1]
    Activity    = append(Activity,arr)
    site        += ['boulby','boulby','boulby','boulby','boulby','boulby',\
    'boulby','boulby']

    # Read in the different radionuclide
    proc        +=  ['16006','17006','18006','17007','18007','8002','9003',\
    '11003']
    loca        +=  ['RN','RN','RN','RN','RN','RN','RN','RN']
    acc         +=  ['di','di','di','di','di','di','di','di']
    #normalised to 9Li from SK
    arr         = npa([0.02,0.001,0.001,0.59*0.002,4e-6,0.23,1.9,0.01])/1.9
    arr         *= radionuclideRate[0]
    Activity    = append(Activity,arr)
    br         +=  [.988,1.0,1.0,0.951,0.143,0.16,0.495,0.927]
    site        += ['','','','','','','','']

    # Read in the different radionuclide
    proc        +=  ['16006','17006','18006','17007','18007','8002','9003',\
    '11003']
    loca        +=  ['RN','RN','RN','RN','RN','RN','RN','RN']
    acc         +=  ['di','di','di','di','di','di','di','di']
    #normalised to 9Li from SK
    arr         = npa([ 0.02,0.001,0.001,0.59*0.002,4e-6,0.23,1.9,0.01])/1.9
    arr         *= radionuclideRate[1]
    Activity    = append(Activity,arr)
    br         +=  [.988,1.0,1.0,0.951,0.143,0.16,0.495,0.927]
    site        += ['boulby','boulby','boulby','boulby','boulby','boulby',\
    'boulby','boulby']



    x,y         = Double(0.),Double(0.)
    boolFirst   = True
    for _inta in inta:
        for ii in range(len(proc)):
            _str1               = "%s_%s_%s_1_abs" %(_inta,proc[ii],loca[ii])
            _scal_str1          = "scaled_%s_%s_%s%s_1_abs_%s"%(_inta,proc[ii],\
            loca[ii],site[ii],acc[ii])
            _strEff             = "eff_%s_%s_%s%s_1_abs" %(_inta,proc[ii],\
            loca[ii],site[ii])

            EG[_strEff]         = Graph()
            cnter = 0
            EG[_strEff].SetPoint(cnter,0.,0.)
            cnter+=1
            for cc,val in enumerate(pctVal):
                s              = "%s_pe_%s_%s_%s_1"%(_inta,val,proc[ii],\
                loca[ii])
                eff = histIntegral(s,f,cut)
                if eff>0.00:
                    EG[_strEff].SetPoint(cnter,pct[cc],eff)
                    cnter+=1
#                    if loca[ii]=="RN":
#                        print _inta,val,proc[ii],loca[ii],cnter,pct[cc],eff

            try:
                EG[_str1] = f.Get(_str1)
                EG[_scal_str1] = EG[_str1].Clone()

                for i in range(EG[_str1].GetN()):
                    EG[_str1].GetPoint(i,x,y)
                    nY      = y*Activity[ii]*timeS*br[ii]*EG[_strEff].Eval(x)
                    if loca[ii] == 'PMT':
                        nY      *= mass*covPCT["%s"%(x)]
                        EG[_scal_str1].SetPoint(i,x,nY)
                    else:
                        EG[_scal_str1].SetPoint(i,x,nY)
            except:
                a = 0

    _scal_acc,_scal_acc_notFV,_scal_acc1,_scal_acc_notFV1 ="scaled_accidental",\
    "scaled_accidental_notFV","cut_accidental","all_cut_accidental"
    EG[_scal_acc],EG[_scal_acc_notFV],EG[_scal_acc1],EG[_scal_acc_notFV1] = \
    Graph(),Graph(),Graph(),Graph()

    for i,p  in enumerate(pct):
        EG[_scal_acc].SetPoint(i ,p ,0.0)
        EG[_scal_acc_notFV].SetPoint(i,p,0.)
        EG[_scal_acc1].SetPoint(i,p,0.)
        EG[_scal_acc_notFV1].SetPoint(i,p,0.)


    for _inta in inta:
        for ii in range(len(proc)):
            _scal_str1          = "scaled_%s_%s_%s%s_1_abs_%s"%(_inta,\
            proc[ii],loca[ii],site[ii],acc[ii])
            for iii,value in enumerate(pct):
                try:
                    if acc[ii] == 'acc' and (_inta == 'si' or _inta == 'ei'):
                        x   = float(value)
                        oY  = EG[_scal_acc].Eval(x)
                        nY  = EG[_scal_str1].Eval(float(value))
                        EG[_scal_acc].SetPoint(iii,x,oY+nY)
                    if acc[ii] == 'di' and (_inta == 'si' or _inta == 'ei'):
                        x   = float(value)
                        oY  = EG['scaled_ei_neutron_Nboulby_1_abs_corr'].Eval(x)
                        oY  /= (63.21751667*12./365./24./3600.*timeS)
                        nY  = EG[_scal_str1].Eval(float(value))
#                        print _scal_str1,x,oY,nY,oY*nY
                        EG[_scal_str1].SetPoint(iii,x,oY*nY)
                    if acc[ii] == 'acc' and _inta != 'si' and _inta != 'ei':
                        x   = float(value)
                        oY  = EG[_scal_acc_notFV].Eval(x)
                        nY  = EG[_scal_str1].Eval(float(value))
                        EG[_scal_acc_notFV].SetPoint(iii,x,oY+nY)
                except:
#                    print '_scal_str1 warning:',_scal_str1
                    a = 1


    distanceEff = 0.004
    for iii,value in enumerate(pct):
        x   = float(value)
        oY  = EG[_scal_acc].Eval(x)
        nY  = 0.0001/timeS*oY*oY
        EG[_scal_acc1].SetPoint(iii,x,nY)
        EG[_scal_acc_notFV1].SetPoint(iii,x,nY*distanceEff)
    return EG

def powerTables(year):
    
    T = {}
    y           = npa([1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12.])
    
    T["2015_H1"] = npa([73.15,76.47,76.92,76.61,71.60,46.30,75.99,75.31,75.77,56.28,-0.65,-2.50])
    T["2015_H2"] = npa([79.73,79.60,39.32,70.89,77.95,77.90,78.00,0.24,-0.70,76.57,79.06,74.38])
    T["2015_P1"] = npa([99.88,93.53,21.44,14.76,100.96,99.85,100.03,100.17,98.68,101.5,101.83,101.29])
    
    T["2014_H1"] = npa([99.94,91.58,99.07,71.06,63.85,99.05,98.38,35.03,0.00,-0.54,-2.07,72.48])
    T["2014_H2"] = npa([82.98,62.00,21.44,98.24,100.23,95.00,46.97,30.11,-2.04,-1.12,11.72,78.72])
    T["2014_P1"] = npa([101.62,97.84,97.14,100.83,93.67,99.37,99.08,98.21,96.63,74.91,80.47,100.75])
    
    T["2013_H1"] = npa([78.6,99.1,98.5,94.7,66.4,47.2,75.6,80.7,98.9,85.0,66.2,30.5])
    T["2013_H2"] = npa([78.5,0.0,0.0,-2.7,22.0,95.2,99.0,99.0,73.8,50.8,101.0,84.6])
    T["2013_P1"] = npa([78.2,99.3,50.3,0.0,43.3,68.6,99.3,98.0,86.0,100.5,99.6,101.4])
    
    T["2012_H1"] = npa([80.50,100.70,100.40,65.90,95.90,60.40,36.70,89.30,99.80,100.20,57.20,100.50])
    T["2012_H2"] = npa([100.50,100.50,26.30,94.50,100.90,100.30,99.60,95.90,34.00,100.80,100.80,101.30])
    T["2012_P1"] = npa([102.00,102.70,86.30,99.80,101.10,69.40,99.10,94.30,98.10,100.40,102.00,100.40])
    
    T["2011_H1"] = npa([65.90,76.06,100.68,62.24,97.71,20.82,0.00,-0.50,96.82,100.00,92.89,4.41])
    T["2011_H2"] = npa([94.05,98.94,97.65,91.87,31.49,96.74,96.43,95.78,71.88,9.30,98.92,98.67])
    T["2011_P1"] = npa([101.62,100.19,95.17,48.90,0.00,60.82,99.57,100.48,98.03,40.48,101.97,101.29])
    
    T["2010_H1"] = npa([100.74,40.42,89.90,101.06,98.12,100.26,71.84,59.36,99.79,82.20,67.74,100.01])
    T["2010_H2"] = npa([93.46,99.47,99.33,91.57,0.00,69.79,97.98,97.68,97.82,61.98,26.12,28.04])
    T["2010_P1"] = npa([102.84,101.33,100.91,99.73,73.75,92.73,97.90,99.89,100.77,100.18,101.88,101.76])
    
    T["2009_H1"] = npa([0.00,99.20,100.80,101.00,44.50,100.90,100.20,99.00,47.90,21.50,97.00,100.10])
    T["2009_H2"] = npa([0.00,16.70,96.10,73.20,79.10,96.90,54.60,78.00,96.80,69.60,0.00,0.20])
    T["2009_P1"] = npa([102.00,78.60,0.00,0.00,42.40,75.50,98.00,100.20,100.90,47.50,90.50,73.40])
    
    T["2008_H1"] = npa([0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00])
    T["2008_H2"] = npa([0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00])
    T["2008_P1"] = npa([102.70,102.80,95.90,66.70,101.50,98.20,99.50,100.20,99.40,101.50,101.80,101.20])
    
    T["2007_H1"] = npa([100.84,89.61,100.91,68.09,94.04,98.94,96.41,95.26,15.04,0.00,0.00,0.00])
    T["2007_H2"] = npa([73.47,53.79,100.48,99.92,99.34,96.60,59.44,97.33,97.25,48.89,0.00,0.00])
    T["2007_P1"] = npa([99.93,100.54,98.02,0.99,31.78,76.31,18.22,100.51,100.18,100.30,93.33,68.31])
    
    aP  = T["%d_%s%d"%(year,'P',1)]
    aH1 = T["%d_%s%d"%(year,'H',1)]
    aH2 = T["%d_%s%d"%(year,'H',2)]
    aH  = T["%d_%s%d"%(year,'H',1)]+T["%d_%s%d"%(year,'H',2)]
    
    gP = TGraph(len(y),y,aP)
    gH1 = TGraph(len(y),y,aH1)
    gH2 = TGraph(len(y),y,aH2)
    gH  = TGraph(len(y),y,aH)
    
    return y,aP,aH1,aH2,aH,gP,gH1,gH2,gH

def powerAllKnownYears():
    
    y_15,aP_15,aH1_15,aH2_15,aH_15,gP_15,gH1_15,gH2_15,gH_15 = powerTables(2015)
    y_14,aP_14,aH1_14,aH2_14,aH_14,gP_14,gH1_14,gH2_14,gH_14 = powerTables(2014)
    y_13,aP_13,aH1_13,aH2_13,aH_13,gP_13,gH1_13,gH2_13,gH_13 = powerTables(2013)
    y_12,aP_12,aH1_12,aH2_12,aH_12,gP_12,gH1_12,gH2_12,gH_12 = powerTables(2012)
    y_11,aP_11,aH1_11,aH2_11,aH_11,gP_11,gH1_11,gH2_11,gH_11 = powerTables(2011)
    y_10,aP_10,aH1_10,aH2_10,aH_10,gP_10,gH1_10,gH2_10,gH_10 = powerTables(2010)
    y_09,aP_09,aH1_09,aH2_09,aH_09,gP_09,gH1_09,gH2_09,gH_09 = powerTables(2009)
    y_08,aP_08,aH1_08,aH2_08,aH_08,gP_08,gH1_08,gH2_08,gH_08 = powerTables(2008)
    y_07,aP_07,aH1_07,aH2_07,aH_07,gP_07,gH1_07,gH2_07,gH_07 = powerTables(2007)
    
    
    
    y = y_07
    y = append(y,y_08+12.*1)
    y = append(y,y_09+12.*2)
    y = append(y,y_10+12.*3)
    y = append(y,y_11+12.*4)
    y = append(y,y_12+12.*5)
    y = append(y,y_13+12.*6)
    y = append(y,y_14+12.*7)
    y = append(y,y_15+12.*8)
    
    p = aP_07
    p = append(p,aP_08)
    p = append(p,aP_09)
    p = append(p,aP_10)
    p = append(p,aP_11)
    p = append(p,aP_12)
    p = append(p,aP_13)
    p = append(p,aP_14)
    p = append(p,aP_15)
    
    h = aH_07
    h = append(h,aH_08)
    h = append(h,aH_09)
    h = append(h,aH_10)
    h = append(h,aH_11)
    h = append(h,aH_12)
    h = append(h,aH_13)
    h = append(h,aH_14)
    h = append(h,aH_15)
    
    h1 = aH1_07
    h1 = append(h1,aH1_08)
    h1 = append(h1,aH1_09)
    h1 = append(h1,aH1_10)
    h1 = append(h1,aH1_11)
    h1 = append(h1,aH1_12)
    h1 = append(h1,aH1_13)
    h1 = append(h1,aH1_14)
    h1 = append(h1,aH1_15)
    
    h2 = aH2_07
    h2 = append(h2,aH2_08)
    h2 = append(h2,aH2_09)
    h2 = append(h2,aH2_10)
    h2 = append(h2,aH2_11)
    h2 = append(h2,aH2_12)
    h2 = append(h2,aH2_13)
    h2 = append(h2,aH2_14)
    h2 = append(h2,aH2_15)
    
    
    gP = TGraph(len(y),y,p)
    gH = TGraph(len(y),y,h)
    gH1 = TGraph(len(y),y,h1)
    gH2 = TGraph(len(y),y,h2)
    
    return y,p/100.,h/100.,h1/100.,h2/100., gP,gH,gH1,gH2




def randomNeutrinoSampling(nuP=137,bkgdP=209.4,nuH=12.4*1.20/2.,bkgdH=28.9):
    
    y, p, h, h1, h2, gP, gH, gH1, gH2 = powerAllKnownYears()
    
    
    #For Perry
    randNum = TRandom3()
    
    entryP      = p*0.0
    signalP     = p*0.0
    bdP       = p*0.0
    
    entryH      = p*0.0
    signalH     = p*0.0
    bdH      = p*0.0
    
    
    for i in range(len(y)):
        
        sP = randNum.Poisson(nuP)*p[i]
        bP = randNum.Poisson(bkgdP)
        sH = randNum.Poisson(nuH)*h[i]
        bH = randNum.Poisson(bkgdH)
        
        entryP[i]   = sP + bP
        signalP[i]  = sP
        bdP[i]    = bP
        
        entryH[i]   = sH + bH
        signalH[i]  = sH
        bdH[i]    = bH
    

    totTP    = bkgdP+nuP*p
    dataP = TGraphErrors(len(y),y,entryP,0.*y,sqrt(entryP))
    truthP =  TGraph(len(y),y,totTP)
    
    #    entryH  = append(entryH,0.0)
    totTH    = bkgdH+nuH*h
    dataH = TGraphErrors(len(y),y,entryH,0.*y,sqrt(entryH))
    truthH =  TGraph(len(y),y,totTH)
    
    scatH = TGraphErrors(len(y),h,entryH,0.*y,sqrt(entryH))
    scatP = TGraphErrors(len(y),p,entryP,0.*y,sqrt(entryP))
    
    return dataP,truthP,scatP,dataH,truthH,scatH


def deleteAllWorkDirectories():
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()

    dir = "log"
    if os.path.exists(dir):
        rmtree(dir)

    dir = "jobs"
    if os.path.exists(dir):
        rmtree(dir)

    for idx,cover in enumerate(coverage):
        dir = "macro_%s" %(cover)
        if os.path.exists(dir):
            rmtree(dir)

#    for idx,cover in enumerate(coverage):
#        dir = "%s" %(cover)
#        if os.path.exists(dir):
#            rmtree(dir)
#
#    for idx,cover in enumerate(coverage):
#        dir = "ntuple_%s" %(cover)
#        if os.path.exists(dir):
#            rmtree(dir)

    if os.path.exists('fit_param.dat'):
        os.remove('fit_param.dat')

    if os.path.exists('like.bin'):
        os.remove('like.bin')

    if os.path.exists('sub_jobs'):
        os.remove('sub_jobs')


######################## Start of main function ###########################

if __name__ == "__main__":
    
    if arguments['-D']:
        deleteAllWorkDirectories()

    if arguments['-m']:
        generateMacros(int(arguments['-N']),int(arguments['-e']))

    if arguments['-j']:
        generateJobs(int(arguments['-N']),arguments)

    if arguments['-n']:
        extractNtuple(arguments)

    if arguments['-M']:
        mergeFiles()

    if arguments['-a']:
        g,h = {},{}
        g,h = runAnalysisProcess(arguments["-f"],g,h)
        writeResultsToFile(arguments["-o"],g,h)

    if arguments['-R']:
        readAnalysisResult(arguments["-o"])




######################## Wahduhwahduhwahdubdub!! ###########################
