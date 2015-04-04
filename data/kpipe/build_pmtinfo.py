import StringIO
from ROOT import *
from array import array

def save_as_roottree( npmts, pmtdict ):
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
    for ipmt in xrange(0,npmts):
        opdetid[0] = ipmt
        x[0] = pmtdict[ipmt][0]
        y[0] = pmtdict[ipmt][1]
        z[0] = pmtdict[ipmt][2]
        t.Fill()
    r.Write()

def build_pmtinfo(npmts,pmtdict=None):
    """generates PMTINFO.db.
    
    This is needed because we hacked RAT.  It stil thinks we are using PMTs,
    so we have to give it pseudo-data. Fixing this is on a to-do list somewhere.
    We also take the opportunity to build a look up table of channel and position.

    """
    pmtinfo = StringIO.StringIO()
    print >> pmtinfo, "{"
    print >> pmtinfo, "  name:\"PMTINFO\","
    print >> pmtinfo, "  valid_begin: [0,0],"
    print >> pmtinfo, "  valid_end: [0,0],"
    xposlist = "["
    yposlist = "["
    zposlist = "["
    typelist = "["
    for n in xrange(0,npmts):
        if pmtdict is None:
            xposlist +=" 0.0"
            yposlist +=" 0.0"
            zposlist +=" 0.0"
        else:
            xposlist += " %.4f"%(pmtdict[n][0])
            yposlist += " %.4f"%(pmtdict[n][1])
            zposlist += " %.4f"%(pmtdict[n][2])
        typelist += "1"
        if n!=npmts-1:
            xposlist+=","
            yposlist+=","
            zposlist+=","
            typelist+=","
    xposlist += "]"
    yposlist += "]"
    zposlist += "]"
    typelist += "]"
    print >> pmtinfo, " x:",xposlist,","
    print >> pmtinfo, " y:",yposlist,","
    print >> pmtinfo, " z:",zposlist,","
    print >> pmtinfo," type:",typelist,","
    print >> pmtinfo,"}"
    if pmtdict is not None:
        save_as_roottree( npmts, pmtdict )
    return pmtinfo.getvalue()

if __name__ == "__main__":
    pmtinfo = build_pmtinfo( 200000 )
    print pmtinfo
