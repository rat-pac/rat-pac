import os,sys
import StringIO

part1 = """
{
name: "GEO",
valid_begin: [0, 0],
valid_end: [0, 0],
gdml_file: "kpipe.gdml",
opdet_lv_name: "volActiveSiPM",
}

{
name: "GEO",
index: "border_target_inner",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"border",
type:"border",
volume1:"pvTarget",
volume2:"pvInnerPipe",
surface:"stainless_steel",
reverse: 1,
}

{
name: "GEO",
index: "border_veto_inner",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"border",
type:"border",
volume1:"pvVetoRegion",
volume2:"pvInnerPipe",
surface:"stainless_steel",
reverse: 1,
}

{
name: "GEO",
index: "border_veto_outer",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"border",
type:"border",
volume1:"pvVetoRegion",
volume2:"pvOuterPipe",
surface:"stainless_steel",
reverse: 1,
}
"""

sipm_template = """
{
name: "GEO",
index: "border_target_simp%d",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"border",
type:"border",
volume1:"pvTarget",
volume2:"SiPM%d",
surface:"chip_silicon",
reverse: 1,
}
"""

def build_geo_file( name, nsipms ):
    out = StringIO.StringIO()
    print >> out,part1
    #for isipm in xrange(0,nsipms):
    #    sipmout = sipm_template%(isipm,isipm)
    #    print >> out,sipmout
    fout = open(name,'w')
    print >> fout, out.getvalue()
    fout.close()


if __name__ == "__main__":
    build_geo_file( "kpipe_wsipms.geo", 100*1000)
