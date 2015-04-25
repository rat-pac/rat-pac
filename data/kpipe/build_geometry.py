import os,sys
from math import sin,cos,pi
from gdml_parts import part1, part2, part3
from build_pmtinfo import build_pmtinfo

# This script generated GDML geometry files for KPIPE. It places the SiPMs parametrically.
# the generated portion is sandwiched between two parts (part1 and part2) below.


def generate_gdml_file( gdml_filename, pmtinfo_filename, ip_nsipms_per_ring, ip_nrings, op_nsipms_per_strings, op_nstrings, pipelength, ip_radius_cm=149.0, op_radius_cm=153.0 ):
    """ Generates KPipe GDML file. Populates detector with rings of SiPMs.

    For each SiPM, we lay down two components, the active and inactive SiPMs.
    We do this for Chroma. All units in cm.
    """

    # ID SIPMS
    zdist_ring_gap = pipelength/float(ip_nrings+1)
    phi_sipm_gap = 360.0/float(ip_nsipms_per_ring)
    ip_sipmdict = {} # dict: key=(sipmid,ringid,ring_sipmindex), value=(x,y,z,phi), where x,y,z is position, phi is rotation around z axis.
    isipm = 0
    for iring in xrange(0,ip_nrings):
        zpos = -0.5*pipelength + zdist_ring_gap*iring
        for iring_sipm in xrange(0,ip_nsipms_per_ring):
            phi = iring_sipm*phi_sipm_gap
            x = (ip_radius_cm)*cos( phi*pi/180.0 )
            y = (ip_radius_cm)*sin( phi*pi/180.0 )
            x2 = (ip_radius_cm-0.1)*cos( phi*pi/180.0 )
            y2 = (ip_radius_cm-0.1)*sin( phi*pi/180.0 )
            rotphi = 90.0 + phi
            ip_sipmdict[ isipm ] = ( x, y, zpos, -rotphi, x2, y2, iring )
            isipm += 1

    # OD SIPMS
    op_string_delta_phi = 360.0/float(op_nstrings)
    op_string_delta_z = pipelength/float(op_nsipms_per_strings-1)
    op_sipmdict = {}
    for istring in xrange(0, op_nstrings):
        phi = istring*op_string_delta_phi
        for istring_sipm in xrange( 0, op_nsipms_per_strings):
            z = -0.5*pipelength + istring_sipm*op_string_delta_z
            x = (op_radius_cm)*cos( phi*pi/180.0 )
            y = (op_radius_cm)*sin( phi*pi/180.0 )
            x2 = x+(0.1)*cos( phi*pi/180.0 )
            y2 = y+(0.1)*sin( phi*pi/180.0 )
            rotphi = -90.0 + phi
            op_sipmdict[isipm] = ( x, y, z, -rotphi, x2, y2, istring )
            isipm += 1
            
    targetvol = "  <volume name=\"volTarget\">\n"
    targetvol+= "    <materialref ref=\"scintillator\"/>\n"
    targetvol+= "    <solidref ref=\"targetvolume\"/>\n"
    keys = ip_sipmdict.keys()
    keys.sort()
    for key in keys:
        transform = ip_sipmdict[key]
        # active
        targetvol+="    <physvol name=\"SiPM%d\">\n"%(key)
        targetvol+="      <volumeref ref=\"volActiveSiPM\"/>\n"
        targetvol+="      <position name=\"posVolActiveSiPM%d\" unit=\"cm\" x=\"%.8f\" y=\"%.8f\" z=\"%.8f\"/>\n"%(key, 
                                                                                                                   transform[4], 
                                                                                                                   transform[5], 
                                                                                                                   transform[2] )
        targetvol+="      <rotation name=\"rotVolActiveSiPM%d\" x=\"0.0\" y=\"0.0\" z=\"%.8f\"/>\n"%(key,  transform[3]*pi/180.0)
        targetvol+="    </physvol>\n"
        # inactive
        targetvol+="    <physvol name=\"InactiveSiPM%d\">\n"%(key)
        targetvol+="      <volumeref ref=\"volInactiveSiPM\"/>\n"
        targetvol+="      <position name=\"posVolInactiveSiPM%d\" unit=\"cm\" x=\"%.8f\" y=\"%.8f\" z=\"%.8f\"/>\n"%(key, transform[0], transform[1], transform[2] )
        targetvol+="      <rotation name=\"rotVolInactiveSiPM%d\" x=\"0.0\" y=\"0.0\" z=\"%.2f\"/>\n"%(key,  transform[3]*pi/180.0)
        targetvol+="    </physvol>\n"
    targetvol+="  </volume>\n"

    targetvol_nosipms = "  <volume name=\"volTarget\">\n"
    targetvol_nosipms+= "    <materialref ref=\"scintillator\"/>\n"
    targetvol_nosipms+= "    <solidref ref=\"targetvolume\"/>\n"
    targetvol_nosipms+= "  </volume>\n"

    # VETO
    vetovol  = "  <volume name=\"volVetoRegion\">\n"
    vetovol += "    <materialref ref=\"scintillator\"/>\n"
    vetovol += "    <solidref ref=\"vetoregion\"/>\n"
    vetovol += "    <physvol name=\"pvInnerPipe\">\n"
    vetovol += "      <volumeref ref=\"volInnerPipe\"/>\n"
    vetovol += "      <position name=\"posInnerPipe\" unit=\"m\" x=\"0\" y=\"0\" z=\"0\"/>\n"
    vetovol += "    </physvol>\n"
    okeys = op_sipmdict.keys()
    okeys.sort()
    for okey in okeys:
        transform = op_sipmdict[okey]
        # active
        vetovol+="    <physvol name=\"SiPM%d\">\n"%(okey)
        vetovol+="      <volumeref ref=\"volActiveSiPM\"/>\n"
        vetovol+="      <position name=\"posVolActiveSiPM%d\" unit=\"cm\" x=\"%.4f\" y=\"%.4f\" z=\"%.4f\"/>\n"%(okey, 
                                                                                                                 transform[4], 
                                                                                                                 transform[5], 
                                                                                                                 transform[2] )
        vetovol+="      <rotation name=\"rotVolActiveSiPM%d\" x=\"0.0\" y=\"0.0\" z=\"%.4f\"/>\n"%(okey,  transform[3]*pi/180.0)
        vetovol+="    </physvol>\n"
        # inactive
        vetovol+="    <physvol name=\"InactiveSiPM%d\">\n"%(okey)
        vetovol+="      <volumeref ref=\"volInactiveSiPM\"/>\n"
        vetovol+="      <position name=\"posVolInactiveSiPM%d\" unit=\"cm\" x=\"%.4f\" y=\"%.4f\" z=\"%.4f\"/>\n"%(okey, transform[0], transform[1], transform[2] )
        vetovol+="      <rotation name=\"rotVolInactiveSiPM%d\" x=\"0.0\" y=\"0.0\" z=\"%.4f\"/>\n"%(okey,  transform[3]*pi/180.0)
        vetovol+="    </physvol>\n"
    vetovol+="  </volume>\n"
        
    vetovol_nosipms  = "  <volume name=\"volVetoRegion\">\n"
    vetovol_nosipms += "    <materialref ref=\"scintillator\"/>\n"
    vetovol_nosipms += "    <solidref ref=\"vetoregion\"/>\n"
    vetovol_nosipms += "    <physvol name=\"pvInnerPipe\">\n"
    vetovol_nosipms += "      <volumeref ref=\"volInnerPipe\"/>\n"
    vetovol_nosipms += "      <position name=\"posInnerPipe\" unit=\"m\" x=\"0\" y=\"0\" z=\"0\"/>\n"
    vetovol_nosipms += "    </physvol>\n"
    vetovol_nosipms += "  </volume>\n"


    #fgdml = open( gdml_filename, 'w' )
    #print >> fgdml, part1+"\n"+targetvol+"\n"+part2+"\n"+vetovol+"\n"+part3+"\n"
    #fgdml.close()

    #fgdml = open( gdml_filename.replace(".gdml","_nosipms.gdml"), 'w' )
    #print >> fgdml, part1+"\n"+targetvol_nosipms+"\n"+part2+"\n"+vetovol_nosipms+"\n"+part3+"\n"
    #fgdml.close()

    pmtinfo = build_pmtinfo( isipm, ip_sipmdict, op_sipmdict )
    fpmtinfo = open( pmtinfo_filename, 'w' )
    print >> fpmtinfo, pmtinfo+'\n'
    fpmtinfo.close()

    #return part1+"\n"+targetvol+"\n"+part2+"\n"


if __name__=="__main__":
    ip_sipms_per_hoop = 100
    ip_hoops = 900
    op_sipms_per_string = 100
    op_strings = 10
    pipelength = 8990.0 # cm
    generate_gdml_file( "kpipe.gdml", "PMTINFO.ratdb", ip_sipms_per_hoop, ip_hoops, op_sipms_per_string, op_strings, pipelength )
        

    
