from load import *

def fillHistograms(inFile,a1,t1,h,cover,ii,locj,covPCT):
    # Obtain logarithmic binnings
    nbins, xbins, ybins = logx_logy_array()
    print arguments["--fv"]

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
                    a = 1


    distanceEff = 0.004
    for iii,value in enumerate(pct):
        x   = float(value)
        oY  = EG[_scal_acc].Eval(x)
        nY  = 0.0001/timeS*oY*oY
        EG[_scal_acc1].SetPoint(iii,x,nY)
        EG[_scal_acc_notFV1].SetPoint(iii,x,nY*distanceEff)
    return EG

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
