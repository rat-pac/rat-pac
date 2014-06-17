#!/usr/bin/env python
import sys
import os.path

def file_subst(fname_in, fname_out, subst):
    fin = file(fname_in)
    contents = fin.read()
    fin.close()
    
    for key in subst.keys():
        contents = contents.replace("%"+key+"%", subst[key])

    fout = file(fname_out, "w")
    fout.write(contents)
    fout.close()

def generate_jobs(template="template.mac.in", start=0, stop=2):
    params = {}
    jobdir = os.path.dirname(template)

    for jobnum in range(start,stop):
        params["jobnum"] = "%03d" % jobnum
        mac_file = os.path.join(jobdir, "job%03d.mac" % jobnum)
        file_subst(template, mac_file, params)
        print "./rat.sh %s" % (mac_file,)

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print "job [template file] [start num] [stop num]"
    else:
        generate_jobs(template=sys.argv[1],
                      start=int(sys.argv[2]), stop=int(sys.argv[3]))

