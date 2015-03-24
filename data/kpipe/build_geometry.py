import os,sys
from math import sin,cos,pi
from build_pmtinfo import build_pmtinfo

# This script generated GDML geometry files for KPIPE. It places the SiPMs parametrically.
# the generated portion is sandwiched between two parts (part1 and part2) below.

part1="""<?xml version="1.0" encoding="UTF-8"?>
<gdml xmlns:gdml="http://cern.ch/2001/Schemas/GDML"	
      xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
      xsi:noNamespaceSchemaLocation="http://service-spi.web.cern.ch/service-spi/app/releases/GDML/schema/gdml.xsd" >

<define>
  <position name="sipm_offset" unit="mm" x="0" y="1.0" z="0"/>
  <rotation name="identity"/>
</define>

<materials>
  <element name="bromine" formula="Br" Z="35"> <atom value="79.904"/> </element>
  <element name="hydrogen" formula="H" Z="1">  <atom value="1.0079"/> </element>
  <element name="nitrogen" formula="N" Z="7">  <atom value="14.0067"/> </element>
  <element name="oxygen" formula="O" Z="8">  <atom value="15.999"/> </element>
  <element name="aluminum" formula="Al" Z="13"> <atom value="26.9815"/>  </element>
  <element name="silicon" formula="Si" Z="14"> <atom value="28.0855"/>  </element>
  <element name="carbon" formula="C" Z="6">  <atom value="12.0107"/>  </element>
  <element name="potassium" formula="K" Z="19"> <atom value="39.0983"/>  </element>
  <element name="chromium" formula="Cr" Z="24"> <atom value="51.9961"/>  </element>
  <element name="iron" formula="Fe" Z="26"> <atom value="55.8450"/>  </element>
  <element name="nickel" formula="Ni" Z="28"> <atom value="58.6934"/>  </element>
  <element name="calcium" formula="Ca" Z="20"> <atom value="40.078"/>   </element>
  <element name="magnesium" formula="Mg" Z="12"> <atom value="24.305"/>   </element>
  <element name="sodium" formula="Na" Z="11"> <atom value="22.99"/>    </element>
  <element name="titanium" formula="Ti" Z="22"> <atom value="47.867"/>   </element>
  <element name="argon" formula="Ar" Z="18"> <atom value="39.9480"/>  </element>
  
  <material Z="1" formula=" " name="Vacuum">
    <D value="1.e-25" unit="g/cm3"/>
    <atom value="1.0079"/>
  </material>

  <material name="stainless_steel" formula="stainless_steel">
    <D value="7.9300" unit="g/cm3"/>
    <fraction n="0.0010" ref="carbon"/>
    <fraction n="0.1792" ref="chromium"/>
    <fraction n="0.7298" ref="iron"/>
    <fraction n="0.0900" ref="nickel"/>
  </material>
  
  <material formula=" " name="air">
    <D value="0.001205" unit="g/cc"/>
    <fraction n="0.781154" ref="nitrogen"/>
    <fraction n="0.209476" ref="oxygen"/>
    <fraction n="0.00937" ref="argon"/>
  </material>
  
  <material formula=" " name="Dirt">
    <D value="1.7" unit="g/cc"/>
    <fraction n="0.438" ref="oxygen"/>
    <fraction n="0.257" ref="silicon"/>
    <fraction n="0.222" ref="sodium"/>
    <fraction n="0.049" ref="aluminum"/>
    <fraction n="0.019" ref="iron"/>
    <fraction n="0.015" ref="potassium"/>
  </material>
  
  <material formula=" " name="mineral_oil">
    <D value="0.77" unit="g/cc"/>
    <fraction n="0.8563" ref="carbon"/>
    <fraction n="0.1437" ref="hydrogen"/>
  </material>

  <material formula=" " name="pseudocumene">
    <D value="0.8758" unit="g/cc"/>
    <fraction n="0.8994" ref="carbon"/>
    <fraction n="0.1006" ref="hydrogen"/>
  </material>
  
  <material formula=" " name="ppo">
    <D value="1.06" unit="g/cc"/>
    <fraction n="0.8142" ref="carbon"/>
    <fraction n="0.0501" ref="hydrogen"/>
    <fraction n="0.0633" ref="nitrogen"/>
    <fraction n="0.0723" ref="oxygen"/>
  </material>
  
  <material formula=" " name="scintillator">
    <D value="0.78" unit="g/cc"/>
    <fraction n="0.996984" ref="mineral_oil"/>
    <fraction n="0.001919" ref="pseudocumene"/>
    <fraction n="0.001097" ref="ppo"/>
  </material>

  <material formula=" " name="chipsilicon">
    <D value="2.3" unit="g/cc"/>
    <fraction n="1.0" ref="silicon"/>
  </material>
</materials>

<solids>

  <box name="world"
       lunit="m"
       x="40.0"
       y="40.0"
       z="200.0" />
  <box name="dirt"
       lunit="m"
       x="16.0"
       y="16.0"
       z="199.0"/>
  <tube name="outerpipe"
	lunit="m"
	rmin="0.0"
	rmax="1.920"
	z="101.04"
	deltaphi="360"
	aunit="deg"/>
  <tube name="vetoregion"
	lunit="m"
	rmin="0.0"
	rmax="1.900"
	z="101.0" 
	deltaphi="360"
	aunit="deg"/>
  <tube name="innerpipe"
	lunit="m"
	rmin="0.0"
	rmax="1.4200"
	z="100.04" 
	deltaphi="360"
	aunit="deg"/>
  <tube name="targetvolume"
	lunit="m"
	rmin="0.0"
	rmax="1.400"
	z="100.00" 
	deltaphi="360"
	aunit="deg"/>

  <box name="sipm_active"
       lunit="mm"
       x="6"
       y="2"
       z="6" />
  <box name="sipm_active_sub"
       lunit="mm"
       x="6"
       y="2"
       z="6" />
  <box name="sipm_package"
       lunit="mm"
       x="10"
       y="4"
       z="10" />
  <subtraction name="sipm_inactive">
       <first ref="sipm_package"/>
       <second ref="sipm_active_sub"/>
       <positionref ref="sipm_offset"/>
       <rotationref ref="identity"/>
  </subtraction>
</solids>

<structure>
  <!-- building the world inside-out -->
  <volume name="volActiveSiPM">
    <materialref ref="chipsilicon"/>
    <solidref ref="sipm_active"/>
  </volume>
  <volume name="volInactiveSiPM">
    <materialref ref="chipsilicon"/>
    <solidref ref="sipm_inactive"/>
  </volume>
"""

part2 = """
  <volume name="volInnerPipe">
    <materialref ref="stainless_steel"/>
    <solidref ref="innerpipe"/>
    <physvol>
      <volumeref ref="volTarget"/>
      <position name="posTargetVolume" unit="m" x="0" y="0" z="0"/>
    </physvol>
  </volume>

  <volume name="volVetoRegion">
    <materialref ref="scintillator"/>
    <solidref ref="vetoregion"/>
    <physvol>
      <volumeref ref="volInnerPipe"/>
      <position name="posInnerPipe" unit="m" x="0" y="0" z="0"/>
    </physvol>
  </volume>

  <volume name="volOuterPipe">
    <materialref ref="stainless_steel"/>
    <solidref ref="outerpipe"/>
    <physvol>
      <volumeref ref="volVetoRegion"/>
      <position name="posVetoRegion" unit="m" x="0" y="0" z="0"/>
    </physvol>
  </volume>

  <volume name="volDirt">
    <materialref ref="Dirt"/>
    <solidref ref="dirt"/>
  </volume>

  <volume name="volWorld">
    <materialref ref="air"/>
    <solidref ref="world"/>
    <physvol>
      <volumeref ref="volDirt"/>
      <position name="posDirt" unit="m" x="0" y="-8.0-1.92" z="0"/>
    </physvol>
    <physvol>
      <volumeref ref="volOuterPipe"/>
      <position name="posOuterPipe" unit="m" x="0" y="0" z="0"/>
    </physvol>
  </volume>

</structure>

<setup name="Default" version="1.0">
  <world ref="volWorld" />
</setup>

</gdml>
"""

def generate_gdml_file( gdml_filename, pmtinfo_filename, ip_nsipms_per_ring, ip_nrings, op_nsipms_per_ring, op_nrings, ip_radius_cm=135.0, op_radius_cm=155.0 ):
    """ Generates KPipe GDML file. Populates detector with rings of SiPMs.

    For each SiPM, we lay down two components, the active and inactive SiPMs.
    We do this for Chroma. All units in cm.
    """
    pipelength = 9990.0
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
        targetvol+="      <position name=\"posVolActiveSiPM%d\" unit=\"cm\" x=\"%.4f\" y=\"%.4f\" z=\"%.4f\"/>\n"%(key, 
                                                                                                                   transform[4], 
                                                                                                                   transform[5], 
                                                                                                                   transform[2] )
        targetvol+="      <rotation name=\"rotVolActiveSiPM%d\" x=\"0.0\" y=\"0.0\" z=\"%.2f\"/>\n"%(key,  transform[3]*pi/180.0)
        targetvol+="    </physvol>\n"
        # inactive
        targetvol+="    <physvol name=\"InactiveSiPM%d\">\n"%(key)
        targetvol+="      <volumeref ref=\"volInactiveSiPM\"/>\n"
        targetvol+="      <position name=\"posVolInactiveSiPM%d\" unit=\"cm\" x=\"%.4f\" y=\"%.4f\" z=\"%.4f\"/>\n"%(key, transform[0], transform[1], transform[2] )
        targetvol+="      <rotation name=\"rotVolInactiveSiPM%d\" x=\"0.0\" y=\"0.0\" z=\"%.2f\"/>\n"%(key,  transform[3]*pi/180.0)
        targetvol+="    </physvol>\n"
    targetvol+="  </volume>\n"
    
    fgdml = open( gdml_filename, 'w' )
    print >> fgdml, part1+"\n"+targetvol+"\n"+part2+"\n"
    fgdml.close()
    
    pmtinfo = build_pmtinfo( isipm, ip_sipmdict )
    fpmtinfo = open( pmtinfo_filename, 'w' )
    print >> fpmtinfo, pmtinfo+'\n'
    fpmtinfo.close()

    return part1+"\n"+targetvol+"\n"+part2+"\n"


if __name__=="__main__":
    generate_gdml_file( "kpipe.gdml", "PMTINFO.ratdb", 100, 1000, 0, 0 )
        

    
