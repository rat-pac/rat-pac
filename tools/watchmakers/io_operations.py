from load import *



def testCreateDirectory(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)
    else:
        rmtree(directory)
        os.makedirs(directory)

def deleteDirectory(directory):
    if os.path.exists(directory):
        rmtree(directory)
#


def macroGenerator(percentage,isotope,location,runs,events):
    
    covPCT = {'10pct':0.1,'15pct':0.15,'20pct':0.2,\
    '25pct':0.25,'30pct':0.30,'35pct':0.35,'40pct':0.40}

    #Part of the macro that is the same for all jobs
    dir = os.getcwd()
    
    header = '''
/glg4debug/glg4param omit_muon_processes  0.0
/glg4debug/glg4param omit_hadronic_processes  0.0

/rat/db/set DETECTOR experiment "Watchman"
/rat/db/set DETECTOR detector_factory "Watchman"
/rat/db/set WATCHMAN_PARAMS photocathode_coverage %4.2f


/run/initialize

# BEGIN EVENT LOOP
/rat/proc lesssimpledaq
/rat/proc fitbonsai
/rat/proc fitcentroid
/rat/proc fitpath
/rat/proc count
/rat/procset update 1000

# Use IO.default_output_filename
/rat/proclast outroot
/rat/procset file "%s/root_files/watchman_%s_%s_%s_%d.root"
#END EVENT LOOP

''' %(covPCT[percentage],dir,isotope,percentage,location,runs)
    
    
    #Part of macro that varies with the various conditions
    if location == 'PMT':
        line1 = '''
/generator/add decaychain %s:regexfill
/generator/pos/set inner_pmts[0-9]+

/run/beamOn %d''' %(isotope,events)
    elif location == 'FV':
        line1 = '''
/generator/add decaychain %s:fill:poisson
/generator/pos/set  0 0 0
/generator/rate/set 6.43

/run/beamOn %d''' %(isotope,events)
    elif location == 'FN':
        line1 = '''
/generator/add combo fastneutron:regexfill
/generator/pos/set rock_[0-9]+
/generator/vtx/set 0 0 0
/generator/fastneutron/depth 1434.0
/generator/fastneutron/enthresh 10.0
/generator/fastneutron/sidewalls 1.0

/run/beamOn %d'''%(events)
    elif location == 'FNimb':
        line1 = '''
/generator/add combo fastneutron:regexfill
/generator/pos/set rock_[0-9]+
/generator/vtx/set 0 0 0
/generator/fastneutron/depth 1434.0
/generator/fastneutron/enthresh 10.0
/generator/fastneutron/sidewalls 1.0

/run/beamOn %d'''%(events)
    elif location == 'FNboulby':
        line1 = '''
/generator/add combo fastneutron:regexfill
/generator/pos/set rock_[0-9]+
/generator/vtx/set 0 0 0
/generator/fastneutron/depth 2805.
/generator/fastneutron/enthresh 10.0
/generator/fastneutron/sidewalls 1.0

/run/beamOn %d'''%(events)
    elif    location=='I':
        line1 = '''
/generator/add combo ibd:fill
/generator/vtx/set  1 0 0
/generator/pos/set 0 0 0

/run/beamOn %d'''%(events)
    elif location == 'S':
        line1 ='''
/generator/add combo spectrum:fill
/generator/vtx/set e+ %s
/generator/pos/set 0 0 0

/run/beamOn %d'''%(isotope,events)
    elif location =='N':
        line1 = '''
/generator/add combo gun2:fill
/generator/vtx/set %s  0 0 0 0 0.001 0.20
/generator/pos/set 0 0 0

/run/beamOn %d'''%(isotope,events)
    elif location == 'RN':
        AZ = isotope
        A =  int(int(AZ)/1000)
        Z = int(AZ) - A*1000
        line1 = '''
/generator/add combo isotope:fill
/generator/pos/set 0 0 0
/generator/vtx/set GenericIon 0 0 0
/generator/isotope/A %s.0
/generator/isotope/Z %s.0
/generator/isotope/E 0.0

/run/beamOn %d''' %(A,Z,events)
    else:
        line1 = 'A'
        print location
    return header+line1



def jobString(percentage,j,runs,models,arguments):
#    directory = "/p/lscratche/adg/Watchboy/simplifiedData/rp_sim/wm"
    directory   = os.getcwd()
    softDir     = "/usr/gapps/adg/geant4/rat_pac_and_dependency"
    ratDir      = os.environ['RATROOT']
    rootDir     = os.environ['ROOTSYS']
    g4Dir       =  os.environ['G4INSTALL']
    
    software    = "%s/bin/rat" %(ratDir)
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    
    ele =  d["%s"%(iso[j])]
    location = loc[j]
    
    goodness     = float(arguments['-g'])

    

    line1 = """#!/bin/sh
#MSUB -N WM_%s_%s_%d    #name of job
#MSUB -A adg         # sets bank account
#MSUB -l nodes=1:ppn=1,walltime=23:59:59,partition=borax  # uses 1 node
#MSUB -q pbatch         #pool
#MSUB -o %s/log/wmpc_%s_%s_%d.log
#MSUB -e %s/log/wmpc_%s_%s_%d.err
#MSUB -d %s  # directory to run from
#MSUB -V
#MSUB                     # no more psub commands

source %s/bin/thisroot.sh
source %s/../../../bin/geant4.sh
source %s/geant4make.sh
source %s/env.sh
export G4NEUTRONHP_USE_ONLY_PHOTONEVAPORATION=1\n
""" %(percentage,location,runs,\
directory,percentage,location,runs,\
directory,percentage,location,runs,\
directory,\
rootDir,g4Dir,g4Dir,ratDir)

    for mods in models:
        if location == "FN":
            line1 += "export PHYSLIST=%s\n" %(mods)
        
        line1 += "%s -l log/rat.%s_%s_%s_%d.log %s/macro_%s/run%s_%s_%d.mac\n" %(software,\
                                                      percentage,mods,location,runs,\
                                                                                 directory,percentage,mods,location,runs)
        fileN = "root_files/watchman_%s_%s_%s_%d.root" %(mods,percentage,location,runs)
        line1 += "python watchmakers.py -n -g %f -f %s\n" %(goodness,fileN)

    return line1



def generateMacros(N,e):
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
#    N = int(arguments['-N'])
    print N,e
    ##Clean or create macro directories
    for idx,cover in enumerate(coverage):
        dir = "macro_%s" %(cover)
        testCreateDirectory(dir)
#    for idx,cover in enumerate(coverage):
#        dir = "%s" %(cover)
#        testCreateDirectory(dir)

    for j in range(len(iso)):
        for ii in d["%s"%(iso[int(j)])]:
            for idx,cover in enumerate(coverage):
                for val in range(N):
                    line = macroGenerator(cover,ii,loc[j],val,e )
                    dir = "macro_%s" %(cover)

                    outfile = open("%s/run%s_%s_%d.mac" %(dir,ii,\
                    loc[j],val),"wb")
                    outfile.writelines(line)
                    outfile.close
    return 0

def removeMacrosAndDirectories():
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    for idx,cover in enumerate(coverage):
        dir = "macro_%s" %(cover)
        deleteDirectory(dir)


def generateJobs(N,arguments):
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    
    '''Find wheter the jobs folder exist: if no create, if yes clean and recreate'''
    directory = 'jobs'
    if not os.path.exists(directory):
        os.makedirs(directory)
    else:
        rmtree(directory)
        os.makedirs(directory)

    '''Find wheter the jobs folder exist: if no create, if yes clean and recreate'''
    directory = 'log'
    if not os.path.exists(directory):
        os.makedirs(directory)
    else:
        rmtree(directory)
        os.makedirs(directory)

    directory = 'root_files'
    if not os.path.exists(directory):
        os.makedirs(directory)

    directory = 'ntuple_root_files'
    if not os.path.exists(directory):
        os.makedirs(directory)

    '''Make sure that the softlink are correct for Bonsai input'''

    ratDir      = os.environ['RATROOT']

    src = ratDir+'/fit_param.dat'
    dst = os.getcwd()+'/fit_param.dat'
    if not os.path.exists(dst):
        os.symlink(src,dst)

    src = ratDir+'/like.bin'
    dst = os.getcwd()+'/like.bin'
    if not os.path.exists(dst):
        os.symlink(src,dst)

    job_list = '''#!/bin/sh
'''

    for j in range(len(iso)):
        for idx,cover in enumerate(coverage):
            models  = d["%s" %(iso[j])]
            for index in range(N):
                line = jobString(cover,j,index,models,arguments)
                stringFile = "jobs/jobs%s_%s_%s_%d.sh" %(cover,\
                                                            "%s"%(iso[int(j)]),loc[j],index)
                if index == 0:
                    job_list+= '(msub ' + stringFile +') || ./'+ stringFile + '\n'
                
                outfile = open(stringFile,"wb")
                outfile.writelines(line)
                if index < N-1:
                    stringFile1 = "(msub jobs/jobs%s_%s_%s_%d.sh || ./jobs/jobs%s_%s_%s_%d.sh)" %(cover,\
                                                                                                 "%s"%(iso[int(j)]),loc[j],index+1,cover,\
                                                                                                 "%s"%(iso[int(j)]),loc[j],index+1)
                    outfile.writelines(stringFile1)
                outfile.close
                os.chmod(stringFile,S_IRWXU)


    outfile = open('sub_jobs',"wb")
    outfile.writelines(job_list)
    outfile.close
    os.chmod('sub_jobs',S_IRWXG)
    os.chmod('sub_jobs',S_IRWXU)
    return 0

def deleteAllWorkDirectories():
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()

    dir = "log"
    if os.path.exists(dir):
        rmtree(dir)

    dir = "jobs"
    if os.path.exists(dir):
        rmtree(dir)

    for idx,cover in enumerate(coverage):
        dir = "macro_%s" %(cover)
        if os.path.exists(dir):
            rmtree(dir)

    if os.path.exists('fit_param.dat'):
        os.remove('fit_param.dat')

    if os.path.exists('like.bin'):
        os.remove('like.bin')

    if os.path.exists('sub_jobs'):
        os.remove('sub_jobs')

def writeResultsToFile(s,g,h):
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    f_root = TFile(s,"recreate")
    for gE in g:
        g["%s"%(gE)].Write()
    for hE in h:
        h["%s"%(hE)].Write()
    f_root.Close()


def mergeFiles():
    # Read external requirements
    #arguments = docopt.docopt(docstring)
    # Load internal requirements
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    trees = {}
    pathFinal = "root_files/merged_ntuple_watchman"
    for j in range(len(iso)):
        for ii in d["%s"%(iso[int(j)])]:
            for idx,cover in enumerate(coverage):
                t_name  = "data_%s_%s_%s"%(ii,cover,loc[j])
                trees[t_name] = TChain("data")
                
                if loc[j] != 'neutron':
                    s = "ntuple_%s/watchman_%s_%s_*_%s.root" %(cover,ii,cover,\
                                                               loc[j])
                    sw = "%s_%s_%s_%s.root"%(pathFinal,ii,cover,loc[j])
                else:
                    s = "ntuple_%s/watchman_%s_*_N.root" %(cover,cover)
                    sw = "%s_neutron_%s_N.root"%(pathFinal,cover)
                print "Writing ", sw,"from",s
                trees[t_name].Add(s)
                print "Number of entries ",trees[t_name].GetEntries()
                trees[t_name].Merge(sw)
                del trees[t_name]
    del trees
    return 0


def extractNtuple(arguments):
    N            = int(arguments["-N"])
    rate         = float(arguments["-r"])
    timemask     = float(arguments['-t'])*1000.0
    distancemask = float(arguments['-d'])
    goodness     = float(arguments['-g'])
    dirGoodness  = float(arguments['-G'])
    minNHIT      = float(arguments['-T'])
    file         = arguments["-f"]
    print file
    d,iso,loc,coverage,coveragePCT = loadSimulationParameters()
    photocoverageAnalysisOnlyData(file,minNHIT,goodness,dirGoodness,timemask,rate,distancemask)
