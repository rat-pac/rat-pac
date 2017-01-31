from load import *


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
