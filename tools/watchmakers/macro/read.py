#import watchmakers as PR
from ROOT import kOrange as kO,kBlue as kB,kGreen as kG
from ROOT import kMagenta as kM,kAzure as kA,kRed as kR
from ROOT import TCanvas,TLine, TLatex
from ROOT import sqrt, TH2D

t = 'day'

def drange(start, stop, step):
	rii= start
	while rii<stop:
		yield rii
		rii+=step



def accidentalCanvas(Graphs):

    proc    = []
    loca    = []
    type    = []
    color   = []
    lineS   = []
    acc     = []

    proc    += ['234Pa','214Pb','214Bi','210Bi','210Tl']
    loca    += ['PMT','PMT','PMT','PMT','PMT']
    type    += ['si','si','si','si','si']
    color   += [kO+0,  kO+1, kO+2, kO+3, kO-2]

    proc    +=['232Th','228Ac','212Pb','212Bi','208Tl']
    loca    +=['PMT','PMT','PMT','PMT','PMT']
    type    += ['si','si','si','si','si']
    color   += [kM+0,  kM-6, kM+2, kM-3, kM-7]

    proc    +=['214Pb','214Bi','210Bi','210Tl']
    loca    +=['FV','FV','FV','FV']
    type    += ['ei','ei','ei','ei']
    color   += [kB+0,  kB+1, kB+2, kB+3]

    proc    +=['214Pb','214Bi','210Bi','210Tl']
    loca    +=['FV','FV','FV','FV']
    type    += ['si','si','si','si']
    color   += [kG+0,  kG+1, kG+2, kG+3]

    proc    +=['QGSP_BERT_EMV','QGSP_BERT_EMX','QGSP_BERT','QGSP_BIC',\
    'QBBC','QBBC_EMZ','FTFP_BERT','QGSP_FTFP_BERT']
    loca    +=['FN','FN','FN','FN','FN','FN','FN','FN']
    type    +=['si','si','si','si','si','si','si','si']
    color   += [kO+2,kO+4,kO+2,kO+4,kO+2,kO+4,kO+2,kO+4]




def siteCanvas(Graphs,site,cut,hist):
    proc        = []
    loca        = []
    type        = []
    color       = []
    lineS       = []
    acc         = []
    scale       = []

    #fast neutrons
    proc        += ['QGSP_BERT_EMV','QGSP_BERT_EMX','QGSP_BERT','QGSP_BIC',\
    'QBBC','QBBC_EMZ','FTFP_BERT','QGSP_FTFP_BERT']
    _t          = 'FN%s' % (site)
    loca        += [_t,_t, _t,_t,_t,_t,_t,_t]
    type        += ['si','si','si','si','si','si','si','si']
    acc         += ['corr','corr','corr','corr','corr','corr','corr', 'corr']
    color       += [kO+6,kO+6,kO+6,kO+6,kO+6,kO+6,kO+6,kO+6]
    lineS       += [2,2,2,2,2,2,2,2]
    scale       += [1./8.,1./8.,1./8.,1./8.,1./8.,1./8.,1./8.,1./8.]

    #ibds
    if site == 'boulby':
        proc    += ['boulby','boulby','neutron']
    else:
        proc    += ['imb','imb','neutron']
    loca        += ['S','S','N%s'%(site)]
    type        += ['ei','ei','ei']
    acc         += ['di','corr','corr']
    color       += [kA+0,kA-0,kA-0]
    lineS       += [1,2,2]
    scale       += [-1.0,0.0,0.0]

    proc        += ['16006','17006','18006','17007','18007','8002',\
    '9003','11003']
    _t          =  'RN%s' % (site)
    loca        += [_t,_t,_t,_t,_t,_t,_t,_t]
    type        += ['ei','ei','ei','ei','ei','ei','ei','ei']
    acc         += ['di','di','di','di','di','di','di','di']
    color       += [kG+3, kG+3,kG+3, kG+3,kG+3,kG+3,kG+3,kG+3]
    lineS       += [1,1,1,1,1,1,1,1]
    scale       += [1.,1.,1.,1.,1.,1.,1.,1.]


    for PC in range(10,41):
        S = 0.
        B = Graphs['all_cut_accidental'].Eval(PC)

        for i,arr in enumerate(proc):
            _str = 'scaled_%s_%s_%s_1_abs_%s' %(type[i],proc[i],loca[i],acc[i])
            if scale[i] > 0.:
                B+=Graphs[_str].Eval(PC)*scale[i]
            elif scale[i] < 0:
                S+=Graphs[_str].Eval(PC)
#        print cut,S,B,S/B,S/sqrt(B+S)
        hist.Fill(PC,cut,S/sqrt(B+S))

    C1 = TCanvas('C1','%s'%(t),1200,800)

    Graphs['all_cut_accidental'].Draw('AC')
    Graphs['all_cut_accidental'].SetLineColor(2)
    Graphs['all_cut_accidental'].SetLineWidth(3)
    Graphs['all_cut_accidental'].SetTitle('doubles rate (all cuts)')
    Graphs['all_cut_accidental'].GetYaxis().SetTitle('event rate [%s^{-1}]'%(t))
    Graphs['all_cut_accidental'].GetXaxis().SetTitle('photo-coverage [%]')
    if site == 'boulby':
        Graphs['all_cut_accidental'].GetYaxis().SetRangeUser(0,70/31.)
    else:
        Graphs['all_cut_accidental'].GetYaxis().SetRangeUser(0,400/31.)

    for i,arr in enumerate(proc):
        _str = 'scaled_%s_%s_%s_1_abs_%s' %(type[i],proc[i],loca[i],acc[i])
        Graphs[_str].Draw('same')
        Graphs[_str].SetLineColor(color[i])
        Graphs[_str].SetLineStyle(lineS[i])
        if loca[i] == 'PMT':
            _descrpt  = '%s %s glass'%(proc[i],loca[i])
            Graphs[_str].SetTitle(_descrpt)
        elif loca[i] == 'FV':
            _descrpt  = '%s %s in water (%s)'%(proc[i],loca[i],type[i])
            Graphs[_str].SetTitle(_descrpt)
        else:
            if proc[i]=='imb':
                Graphs[_str].SetTitle('Prompt-positron at Perry')
            if proc[i]=='boulby':
                Graphs[_str].SetTitle('Prompt-positron at Boulby')
        Graphs[_str].SetMarkerStyle(1)

    t1 = TLatex()
    if site == 'boulby':
        line = TLine(7, 4.,42, 4)
        t1.DrawLatex( 10,65., 'pe > %d.0'%(cut))
    else:
        line = TLine(7, 40,42, 40)
        t1.DrawLatex( 10,363., 'pe > %d.0'%(cut))

    line.SetLineColor(4)
    line.Draw('same')
    C1.SetGridy()
    C1.Update()

#    C1.SaveAs('gif/pc_%s_%d.gif'%(site,cut))
    C1.SaveAs('gif/pc_%s.gif+'%(site))
    return hist




def sensitivityMap(site,hist):
    proc        = []
    loca        = []
    type        = []
    color       = []
    lineS       = []
    acc         = []
    scale       = []
    #fast neutrons
    proc        += ['QGSP_BERT_EMV','QGSP_BERT_EMX','QGSP_BERT','QGSP_BIC',\
    'QBBC','QBBC_EMZ','FTFP_BERT','QGSP_FTFP_BERT']
    _t          = 'FN%s' % (site)
    loca        += [_t,_t, _t,_t,_t,_t,_t,_t]
    type        += ['si','si','si','si','si','si','si','si']
    acc         += ['corr','corr','corr','corr','corr','corr','corr', 'corr']
    color       += [kO+6,kO+6,kO+6,kO+6,kO+6,kO+6,kO+6,kO+6]
    lineS       += [2,2,2,2,2,2,2,2]
    scale       += [1./8.,1./8.,1./8.,1./8.,1./8.,1./8.,1./8.,1./8.]
    #radionuclides
    proc        += ['16006','17006','18006','17007','18007','8002',\
                    '9003','11003']
    _t          =  'RN%s' % (site)
    loca        += [_t,_t,_t,_t,_t,_t,_t,_t]
    type        += ['ei','ei','ei','ei','ei','ei','ei','ei']
    acc         += ['di','di','di','di','di','di','di','di']
    color       += [kG+3, kG+3,kG+3, kG+3,kG+3,kG+3,kG+3,kG+3]
    lineS       += [1,1,1,1,1,1,1,1]
    scale       += [1.,1.,1.,1.,1.,1.,1.,1.]
    #ibds
    if site == 'boulby':
        proc    += ['boulby','boulby','neutron']
    else:
        proc    += ['imb','imb','neutron']
    loca        += ['S','S','N%s'%(site)]
    type        += ['ei','ei','ei']
    acc         += ['di','corr','corr']
    color       += [kA+0,kA-0,kA-0]
    lineS       += [1,2,2]
    scale       += [-1.0,0.0,0.0]


    for PC in range(10,41):
#        print 'processing photocoverage ',PC,'% :',
        S_tmp           = 0.
        B_tmp           = 0.1
        SoverS_B        = 0.
        SoverS_B_sys20  = 0.
        cut_tmp     = 0.
        for cut in range(4,40):
            Graphs =PR.obtainAbsoluteEfficiency('processed_data_watchman.root',\
            timeScale=t,cut=cut)

            S       = 0.
            BFN     = 0.
            BRN     = 0.
            BAC     = 0.
            _str    = 'all_cut_accidental'
            BAC = Graphs[_str].Eval(PC)
#            B = Graphs[_str].Eval(PC)
#            if  cut > 9 and cut < 11:
#                print '\n',PC, cut, _str,B,Graphs[_str].Eval(PC)
#            print ''
            for i,arr in enumerate(proc):
                _str = 'scaled_%s_%s_%s_1_abs_%s' %(type[i],proc[i],loca[i],acc[i])
                if scale[i] > 0.:
#                    B+=Graphs[_str].Eval(PC)*scale[i]
                    if loca[i] == 'FNboulby':
                        BFN += Graphs[_str].Eval(PC)*scale[i]
                    elif loca[i] == 'RNboulby':
                        BRN += Graphs[_str].Eval(PC)*scale[i]
#                    if cut > 9 and cut < 11:
#                        print loca[i],B,Graphs[_str].Eval(PC)*scale[i]
                elif scale[i] < 0:
                    S =Graphs[_str].Eval(PC)
#                    if cut > 9 and cut <11:
                    B = BAC+BRN+BFN
                    if cut < 12 or 9*(S+2*(S+B))/S/S < 600.:
                        print PC,cut,S,S/1.53615649315,BAC,BRN,BFN,B,S/sqrt(S+S+B),9*(S+2*(S+B))/S/S
#            print ''
    #        print cut,S,B,S/B,S/sqrt(B+S)
            if S/sqrt(B+S) >  SoverS_B:
                    S_tmp       = S
                    B_tmp       = B
                    SoverS_B    = S/sqrt(S+B)
                    cut_tmp     = cut
#        print '\n',PC,cut_tmp,S_tmp,B_tmp,SoverS_B,'\n'
#        print '10% :',
#        for value in drange(.2,12.,0.2):
#            print "%4.2f "%(S_tmp*sqrt(value)/sqrt(S_tmp+B_tmp + pow(B_tmp*0.10,2)*value)),
#        print '\n20% :',
#        for value in drange(.2,12.,0.2):
#            print "%4.2f "%(S_tmp*sqrt(value)/sqrt(S_tmp+B_tmp + pow(B_tmp*0.20,2)*value)),
#        print '\n30% :',
#        for value in drange(.2,12.,0.2):
#            print "%4.2f "%(S_tmp*sqrt(value)/sqrt(S_tmp+B_tmp + pow(B_tmp*0.30,2)*value)),
#        print ''
    return hist


def runSensitivity():
    hBoulby = TH2D('hBoulby','hBoulby',50,0.5,50.5,50,0.5,50.5)
    print 'Boulby:'
    for cut in range(4,40):
        Graphs = PR.obtainAbsoluteEfficiency('processed_data_watchman.root',timeScale=t,cut=cut)
        hBoulby =  siteCanvas(Graphs,'boulby',cut,hBoulby)
    hBoulby.SaveAs('hBoulby.C')


    hFairport = TH2D('hFairport','hFairport',50,0.5,50.5,50,0.5,50.5)
    print 'Fairport:'
    for cut in range(4,40):
        Graphs = PR.obtainAbsoluteEfficiency('processed_data_watchman.root',timeScale=t,cut=cut)
        hFairport =  siteCanvas(Graphs,'',cut,hFairport)
    hFairport.SaveAs('hFairport.C')

    return 0


#hBoulby = TH2D('hBoulby','hBoulby',50,0.5,50.5,50,0.5,50.5)
#hBoulby = sensitivityMap('boulby',hBoulby)
