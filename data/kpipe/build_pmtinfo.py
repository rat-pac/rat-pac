import StringIO

def build_pmtinfo(npmts,pmtdict=None):
    """generates PMTINFO.db.
    
    This is needed because we hacked RAT.  It stil thinks we are using PMTs,
    so we have to give it pseudo-data. Fixing this is on a to-do list somewhere...

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

    return pmtinfo.getvalue()

if __name__ == "__main__":
    pmtinfo = build_pmtinfo( 200000 )
    print pmtinfo
