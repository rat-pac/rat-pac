import os,sys

folder = "/net/nudsk0001/d00/scratch/taritree/cosmic_events/"
folder_ana = "/net/nudsk0001/d00/scratch/taritree/cr_trg_out/"
rm_range = [500,1000]
for i in xrange(rm_range[0],rm_range[1]):
    f = folder + "/output_kpipe_cryevents_%d.root"%(i)
    f2 = folder_ana + "/output_cr_analysis_%d.root"%(i)
    print f,f2
    os.system("rm -f %s %s"%(f,f2))
