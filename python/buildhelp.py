import os
import sys
import glob
import re

# Useful constants
try:
    RATROOT=os.environ["RATROOT"]
except KeyError:
    RATROOT=""
RATCONFIGDIR = os.path.join(RATROOT,"config")
RATENVFILE = os.path.join(RATCONFIGDIR, "RAT.scons")
ROOTARCH = os.popen("root-config --arch").read().strip()

# Discover version info for RAT
if os.path.isdir(os.path.join(RATROOT,'.hg')):
    svn_info = os.popen('hg svn info '+RATROOT).read()
else:
    svn_info = os.popen('svn info '+RATROOT).read()
svn_info_match = re.search('Revision: (\d+)', svn_info)
if svn_info_match is None:
    # Maybe someone is using git-svn
    git_info = os.popen('git log -n 100').read()
    git_info_match = re.search('git-svn-id:.*@(\d+)', git_info)
    if git_info_match is None:

        # Maybe someone is using hg svn
        
        print 'Cannot discover RAT version.  Is $RATROOT set?'
        sys.exit(1)
    else:
        RATVERSION = int(git_info_match.group(1))
else:
    RATVERSION = int(svn_info_match.group(1))

RATVERSIONSTR = 'r'+str(RATVERSION)

def testenv(envname):
    'Test for the presence of an environment var and if it set to 1.'
    return os.environ.has_key(envname) and os.environ[envname] == '1'

# Construct a list of build targets
def build_list(srcpath, build_dir):
    return [os.path.join(build_dir, os.path.basename(item)) for item
            in glob.glob(srcpath)]

# Create targets from a source directory, and add headers to the build list.
def src_module(env, module_name, header_subdir=""):
    modbuilddir = os.path.join(env['BUILDDIR'], module_name)
    srcdir = os.path.join('src', module_name)
    env.BuildDir(modbuilddir, srcdir, duplicate=0)

    # 09-Aug-2006 WGS: Add a "builder" for the headers so that only
    # those headers that are needed or revised are copied.

    headers = glob.glob(srcdir+'/*.hh') + glob.glob(srcdir+'/*.hpp') + \
              glob.glob(srcdir+'/*.tpp') + glob.glob(srcdir+'/*.icc') + \
              glob.glob(srcdir+'/*.h')
    env['INCLUDE_SUBDIR'] = header_subdir
    for h in headers:
        env.Append(RATHEADERS=env.RATHeader(h))

    src_list = build_list(os.path.join(srcdir,'*.cc'), modbuilddir)
    if 'CPPFLAGS' in env and 'CUDA' in env['CPPFLAGS']:
        src_list += build_list(os.path.join(srcdir,'*.cu'), modbuilddir)

    return env.Object(src_list)
