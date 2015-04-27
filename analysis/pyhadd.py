import os

addlist = ""
for n in xrange(0,100):
    #addlist += " /net/nudsk0001/d00/scratch/taritree/scrape_out/output_scrape_%d.root"%(n)
    if n<100:
        addlist += " /net/nudsk0001/d00/scratch/taritree/trg_out/output_analysis_%d.root"%(n)
    else:
        os.system( "rm /net/nudsk0001/d00/scratch/taritree/trg_out/output_analysis_%d.root"%(n))
os.system("hadd run0_100_analysis.root %s"%(addlist))

