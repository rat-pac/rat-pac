import os,sys

def gen_macro( outfile, jobid, nevents_per_job ):
    part1 = """/glg4debug/glg4param omit_muon_processes  0.0
/glg4debug/glg4param omit_hadronic_processes  0.0

/rat/db/set DETECTOR experiment \"kpipe\"
/rat/db/set DETECTOR geo_file \"kpipe/kpipe_wsipms.geo\"
/tracking/storeTrajectory 1
/tracking/discardParticleTrajectory opticalphoton

/run/initialize

# BEGIN EVENT LOOP
/rat/proc count
/rat/procset update 10
#/rat/proc prune
#/rat/procset prune \"mc.track:opticalphoton,mc.track:e-\"

# Use IO.default_output_filename
/rat/proclast outroot
#END EVENT LOOP
"""
    offset = jobid*nevents_per_job
    gen_str = "/generator/add external external:uniform:\"/net/t2srv0008/app/d-Chooz/Software/kpipe/ratpac-kpipe/kdar_muons/./simple_gen_kdar_event default %d %d |\"" % (nevents_per_job, offset)

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
        gen_macro( "test.mac", 1, 1000 )
    else:
        gen_macro( sys.argv[1], int(sys.argv[2]), 1000 )
