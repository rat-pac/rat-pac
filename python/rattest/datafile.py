import sys, glob, os.path, re, time, shelve

def load_local_config():
    '''load variables from local_config file and return as dict.'''
    local_config = {}
    try:
        exec file('local_config') in globals(), local_config
    except IOError, e:
        if e.errno == 2:
            print '''Local configuration file does not exist.  Please run:

    cp local_config.orig local_config

and edit it to specify your system settings.'''
        else:
            print 'Error accessing local_config:', e
        sys.exit()
    
    return local_config

def load_datasets(search_dir='data'):
    'Collect dataset descriptions'
    datasets = {}
    for ds_descfile in glob.glob(os.path.join(search_dir, '*', 'Dataset')):
        # Extract short name from path
        ds_name = re.match('data/(.+)/Dataset', ds_descfile).group(1)

         # load variables from file into ds_param
        ds_param = {}
        exec file(ds_descfile) in globals(), ds_param

        # Add directory
        ds_param['dir'] = os.path.dirname(ds_descfile)

        datasets[ds_name] = ds_param

    return datasets

def load_runfile(filename, create=False):
    '''Load test run database.
    
    If create==False, this function will test for the existence of the file and
    return None if it is not present.

    If create==True and filename==None, a new filename is generated to contain
    the testrun data.  Otherwise, filename is returned.'''

    if not filename:
        if create:
            filename = time.strftime('log/%Y%m%d-*')
            candidates = glob.glob(filename)
            candidates.sort()

            if len(candidates) == 0:
                filename = time.strftime('log/%Y%m%d-00')
            else:
                last_index = int(candidates[-1][-2:])
                filename = time.strftime('log/%Y%m%d') + '-%02d'%(last_index+1)
            print 'Creating test run', filename, '...'
        else:
            filename = time.strftime('log/*')
            candidates = glob.glob(filename)
            candidates.sort()

            if len(candidates) == 0:
                print 'No default file found! Use -n option to create new one'
                sys.exit(1)
            else:
                filename = candidates[-1]
            print 'Opening test run', filename, '...'

    elif len(glob.glob(filename+'*')) == 0:
        print 'Run file', filename, 'does not exist.'
        sys.exit(1)


    runfile = shelve.open(filename, writeback=True)

    if create:
        runfile['ctime'] = time.time()
        runfile['rev'] = -1
        runfile['filename'] = filename

    runfile['atime'] = time.time()

    runfile.sync()

    return runfile

