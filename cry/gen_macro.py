import os,sys

def gen_macro( outfile, infile, jobid, nevents_per_job ):
    part1 = """#/glg4debug/glg4param omit_muon_processes  0.0
#/glg4debug/glg4param omit_hadronic_processes  0.0

/rat/db/set DETECTOR experiment \"kpipe\"
/rat/db/set DETECTOR geo_file \"kpipe/kpipe_wsipms.geo\"

/run/initialize

# BEGIN EVENT LOOP
/rat/proc count
/rat/procset update 10

# Use IO.default_output_filename
/rat/proclast outroot
#END EVENT LOOP
"""
    offset = jobid*nevents_per_job
    gen_str = "/generator/add external external:uniform:\"/net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/cry/./cosmic_interface %s %d |\"" % (infile, nevents_per_job)

    run_str = "/run/beamOn %d"%(nevents_per_job)
    fout = open( outfile, 'w' )
    print >> fout,part1
    print >> fout,""
    print >> fout,gen_str
    print >> fout,""
    print >> fout,run_str
    fout.close()
    

if __name__=="__main__":
    if len( sys.argv)==1:
        gen_macro( "test.mac", "cry_gen.root", 1, 10000 )
    else:
        gen_macro( sys.argv[2], sys.argv[1], int(sys.argv[3]), int(sys.argv[4]) )
