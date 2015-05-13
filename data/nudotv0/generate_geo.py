import os,sys
import StringIO
from math import cos,sin,pi
from gdml_parts import part1,part2
from ROOT import *
from array import array

def gen_photocathod_pos_hexpattern( radius ):

    rowy = []
    yradius = cos(30.0*(pi/180.0))*radius 
    ystep = 0.5*yradius
    for i in xrange(0,5):
        y = yradius - float(i)*ystep
        rowy.append(y)

    nx = [3,4,6,4,3]
    thx = [30.0, 60.0, 90.0, 60.0, 30.0 ]
    posxy = []
    for n in xrange(0,5):
        y = rowy[n]
        maxx = sin(thx[n]*pi/180.0)*radius
        xstep = 2.0*maxx/float(nx[n]-1)
        for ix in xrange(0,nx[n]):
            x = maxx - float(ix)*xstep
            posxy.append( (x,y) )
    return posxy

def gen_gdml_snippet( posxy ):
    pmttxt = StringIO.StringIO()
    for n,pos in enumerate(posxy):
        print >> pmttxt, "    <physvol name=\"PMTPC%02d\">"%(n)
        print >> pmttxt, "      <volumeref ref=\"volPC\"/>"
        print >> pmttxt, "      <position name=\"posPMTPC%02d\" unit=\"cm\" x=\"%.04f\" y=\"%.04f\" z=\"%.04f\" />" % ( n, pos[0], pos[1], 0.0 )
        print >> pmttxt, "    </physvol>"
    return pmttxt.getvalue()

def gen_pmtinfo( posxy ):
    f = open("PMTINFO.ratdb",'w')
    pmtdb = StringIO.StringIO()
    posx = StringIO.StringIO()
    posy = StringIO.StringIO()
    posz = StringIO.StringIO()
    typepmt = StringIO.StringIO()
    for pos in posxy:
        print >> posx," %.4f,"%(pos[0]),
        print >> posy," %.4f,"%(pos[1]),
        print >> posz," 0.0,",
        print >> typepmt, " 1,",
    print >> f,"{"
    print >> f,"  name:\"PMTINFO\","
    print >> f,"  valid_begin: [0,0],"
    print >> f,"  valid_end: [0,0],"
    print >> f,"  x: [",posx.getvalue()[:-1],"],"
    print >> f,"  y: [",posy.getvalue()[:-1],"],"
    print >> f,"  z: [",posz.getvalue()[:-1],"],"
    print >> f,"  type: [",typepmt.getvalue()[:-1],"],"
    print >> f,"}"
    f.close()

def save_as_roottree( posxy ):
    r = TFile('PMTINFO.root', 'recreate')
    t = TTree('pmtinfo','PMT Positions')
    opdetid = array('i',[0])
    x = array('f',[0.0])
    y = array('f',[0.0])
    z = array('f',[0.0])
    t.Branch( "opdetid", opdetid, "opdetid/I" )
    t.Branch( "x", x, "x/F" )
    t.Branch( "y", y, "y/F" )
    t.Branch( "z", z, "z/F" )
    for ipmt,pos in enumerate(posxy):
        opdetid[0] = ipmt
        x[0] = pos[0]
        y[0] = pos[1]
        z[0] = 0.0
        t.Fill()
    r.Write()

def gen_gdml( posxy ):
    pmttxt = gen_gdml_snippet( posxy )
    gdmlout = StringIO.StringIO()
    print >> gdmlout, part1
    print >> gdmlout, pmttxt
    print >> gdmlout, part2

    f = open("nudotv0.gdml",'w')
    print >> f,gdmlout.getvalue()
    f.close()

if __name__ == "__main__":
    posxy = gen_photocathod_pos_hexpattern( 15.0 )
    gen_gdml( posxy )
    gen_pmtinfo( posxy )
    save_as_roottree( posxy )
