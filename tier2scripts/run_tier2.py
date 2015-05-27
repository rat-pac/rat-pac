#!/bin/env python
import os,sys
import StringIO
import json
import time
try:
    import argparse
except:
    sys.path.append( "/net/hisrv0001/home/taritree/software/argparse-1.3.0" )
    import argparse

# argument parser
parser = argparse.ArgumentParser(description='Manage Tier 2 Submissions')
parser.add_argument( "setup_file",type=str,help="json file with setup of submission" )
parser.add_argument( "--check",default=False,help="only checks if all files made",action='store_true')
parser.add_argument( "--submit",default=False,help="submit jobs (does not resubmit completed files)",action='store_true')
parser.add_argument( "--redo",default=False,help="submit jobs (will resubmit completed files)",action='store_true')
parser.add_argument( "--test",default=False,help="test mode. runs until submission, which it will skip.",action='store_true')
parser.add_argument( "--limit",default=-1,type=int,help="limit the number of jobs made",nargs=1 )

# tier 2 running utilities for KPipe rat-pac

def generate_condor_submtfile( jobid, outfilename, inputfiles, outputfiles, script, arguments, njobs=1 ):
    condorsub = StringIO.StringIO()
    print >> condorsub,"# Generated Tier 2 KPipe Condor submission file"
    print >> condorsub,"# --------------------------------------------"
    print >> condorsub,"Universe                = vanilla"
    print >> condorsub,"Environment             =\"HOSTNAME=$HOSTNAME\""
    print >> condorsub,"Requirements            = UidDomain == \"cmsaf.mit.edu\" && Arch == \"X86_64\" && HasFileTransfer"
    print >> condorsub,"Executable              = ",script
    print >> condorsub,"Arguments               = ",arguments
    print >> condorsub,"Input                   = /dev/null"
    print >> condorsub,"Output                  = condor_out/condor_job%d.out" % ( jobid )
    print >> condorsub,"Error                   = condor_err/condor_job%d.err" % ( jobid )
    print >> condorsub,"Log                     = condor.log"
    inputstr = ""
    iinput = 0
    for arg,inputfile in inputfiles.items():
        inputstr += inputfile
        if iinput!=len(inputfiles)-1:
            inputstr += ", "
        iinput += 1
    ioutput = 0
    outstr = ""
    for arg,outputfile in outputfiles.items():
        outstr += outputfile
        if ioutput!=len(outputfiles)-1:
            outstr += ", "
        ioutput += 1
    print >> condorsub,"transfer_input_files    = ",inputstr
    print >> condorsub,"transfer_output_files = ",outstr
    print >> condorsub,"should_transfer_files   = YES"
    print >> condorsub,"when_to_transfer_output = ON_EXIT"
    print >> condorsub,"+AccountingGroup = \"group_mitlns.taritree\""
    print >> condorsub,"Queue"
    print >> condorsub,"# --------------------------------------------"

    out = open( outfilename, 'w' )
    print >> out, condorsub.getvalue()

def generate_script_wrapper( outfilename, inputfiles, outputfiles, job_json ):
    script_wrapper = StringIO.StringIO()
    print script_wrapper >> "#!/bin/sh"
    print script_wrapper >> "Transfer input files"
    return

def parse_job_file( jsonfile ):
    f = open( jsonfile, 'r' )
    rawdata = f.read()
    parsed = json.loads( rawdata )
    return parsed

def make_arg_lists( job_json ):
    nargs = int(job_json["job"]["nargs"])
    start = int(job_json["job"]["startjob"])
    njobs = int(job_json["job"]["njobs"])

    inputlists = {}
    outputlists = {}
    arglists = {}

    for jobid in xrange(start,start+njobs):
        inputlists[jobid] = {}
        outputlists[jobid] = {}
        arglists[jobid] = ""
        for arg in job_json["job"]["args"]:
            if int(arg["njobids"])>0:
                jobarg = ( jobid*int(arg["njobids"]) )
                inputfile = arg[ "value" ] % jobarg
            else:
                inputfile = arg[ "value" ]
            if arg["type"]=="input":
                if arg["transfer"]=="True":
                    inputlists[jobid][ arg["argname"] ] = inputfile
                    arglists[jobid] += " "+os.path.basename(inputfile)
                else:
                    inputlists[jobid][ arg["argname"] ] = inputfile
                    arglists[jobid] += " "+inputfile
            elif arg["type"]=="output":
                outputlists[jobid][ arg["argname"] ] = inputfile
                arglists[jobid] += " "+inputfile
            else:
                arglists[jobid] += " "+inputfile
        for packagefile in job_json["job"]["packagefiles"]:
            inputlists[ jobid ][ os.path.basename(packagefile) ] = packagefile

    return inputlists,outputlists,arglists

def launch_jobs( job_json, inputlists, outputlists, arglists, testmode=False ):
    joblist = inputlists.keys()
    joblist.sort()
        
    script  = job_json["job"]["script"]
    os.system("mkdir -p condor_scripts")
    os.system("mkdir -p condor_out")
    os.system("mkdir -p condor_err")
    for jobid in joblist:
        condorfile = "condor_scripts/condor_submit_jobid%d.condor"%(jobid)
        script_wrapper = "condor_scripts/wrapper_job%d_%s"%(jobid, script)
        inputs = inputlists[ jobid ]
        outputs = outputlists[ jobid ]
        generate_condor_submtfile( jobid, condorfile, inputs, outputs, job_json["job"]["script"], arglists[jobid] )
        command = "condor_submit %s"%(condorfile)
        if not testmode:
            print "[SUBMIT]", command
            os.system( command )
            time.sleep(0.1)
        else:
            print "[TEST] ",command

def check_output( outputlists ):
    joblist = outputlists.keys()
    joblist.sort()
    status = {}
    nbad = 0
    for jobid in joblist:
        outputfiles = outputlists[jobid]
        ok = True
        for key,item in outputfiles.items():
            if os.path.exists( item )==False or os.stat( item ).st_size<10e3:
                print item," not ok"
                ok = False
        if not ok:
            nbad += 1
        status[jobid] = ok
    if nbad==0:
        print "All output files checkout as OK."
    return status

if __name__ == "__main__":

    args = parser.parse_args()

    print "RUNNING TIER 2 JOBS FROM SETUP FILE: ",args.setup_file
    job_json = parse_job_file( args.setup_file )
    if not args.check and not args.submit and not args.redo:
        print " no command switches given. parsing job file only."
        sys.exit(-1)

    if args.submit and args.redo:
        print " can only run in either submit or redo mode"
        sys.exit(-1)
        
    inputlists, outputlists, arglists = make_arg_lists( job_json )
    jobstatus = check_output( outputlists )

    if args.submit:
        for jobid,status in jobstatus.items():
            if status==True:
                inputlists.pop( jobid, None )
                outputlists.pop( jobid, None )
                arglists.pop( jobid, None )

        launch_jobs( job_json, inputlists, outputlists, arglists, testmode=args.test )
    elif args.redo:
        launch_jobs( job_json, inputlists, outputlists, arglists, testmode=args.test )
    
