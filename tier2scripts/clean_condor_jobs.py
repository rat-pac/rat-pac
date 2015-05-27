#!/bin/env python

import os,sys

holdpipe = os.popen("condor_q taritree -hold")
holdlines = holdpipe.readlines()
for l in holdlines:
    info = l.strip().split()
    try:
        jobid = info[0]
        #print jobid,": ",info
        if info[4]=="Error":
            command = "condor_rm "+jobid
            print command
            os.system(command)
    except:
        pass


    
