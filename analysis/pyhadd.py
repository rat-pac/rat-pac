import os

addlist = ""
for n in xrange(200,300):
    addlist += " /net/nudsk0001/d00/scratch/taritree/scrape_out/output_scrape_%d.root"%(n)
os.system("hadd run200_299_scraped.root %s"%(addlist))

