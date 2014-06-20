import os
import sys
import glob
import re
import subprocess

# Useful constants
try:
    RATROOT=os.environ["RATROOT"]
except KeyError:
    RATROOT="./"
RATCONFIGDIR = os.path.join(RATROOT,"config")
RATENVFILE = os.path.join(RATCONFIGDIR, "RAT.scons")
ROOTARCH = os.popen("root-config --arch").read().strip()

#FIXME: Shouldn't we fix the repositories to just git?
#       Actually, the version should not be defined by git, or the releases won't work
#       Should we add a config.h file that contains this kind of information?
# Let's define a config.h file that contains the parts relevant for the verion
## --> there is no point in supporting hg or svn any more. Hail to the git baby!
# Discover version info for RAT
git_info = subprocess.Popen(['git', 'describe', '--always', '--tag'],
                            stdout=subprocess.PIPE).communicate()[0].strip()
                            
if git_info is None:
    print 'Cannot discover RAT version.  Is $RATROOT set?'
    sys.exit(1)
else:
    print "||",git_info,"||"
    RATVERSION = git_info

## Build a config.h file that contains the version information and so we avoid
## weird macros passed through compiler options

config_tmpl = \
'''/// DO NOT edit

#include <string>

namespace RAT {

static const std::string RATVERSION = "%s";

}  // namespace RAT

'''

header = config_tmpl % (str(git_info))
f= open(RATROOT+"/src/core/Config.hh", 'w')
f.write(header)
f.close()
# No need to a string version. The original is already a string.
#RATVERSIONSTR = 'r'+str(RATVERSION)

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
