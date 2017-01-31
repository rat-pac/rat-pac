#!/usr/bin/python

from watchmakers.load import *
from watchmakers.io_operations import *
from watchmakers.analysis import *
from watchmakers.sensitivity import *

fv_rad = 5.42

######################## Start of main function ###########################

if __name__ == "__main__":
    
    if arguments['-D']:
        deleteAllWorkDirectories()

    if arguments['-m']:
        generateMacros(int(arguments['-N']),int(arguments['-e']))

    if arguments['-j']:
        generateJobs(int(arguments['-N']),arguments)

    if arguments['-n']:
        extractNtuple(arguments)

    if arguments['-M']:
        mergeFiles()

    if arguments['-a']:
        g,h = {},{}
        g,h = runAnalysisProcess(arguments["-f"],g,h)
        writeResultsToFile(arguments["-o"],g,h)

    if arguments['-R']:
        readAnalysisResult(arguments["-o"])



######################## Waba Luba Dub Dub!! ###########################
