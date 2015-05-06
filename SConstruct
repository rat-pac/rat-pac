#-*-python-*-
# Main build script for RAT.

# Do not copy this to make your own customized RAT applications!
# Instead see user/SConstruct for a simpler example.

#### Add new source module names here ####

modules = Split('''
cmd core cut daq db fit gen geo io physics stlplus util
''') # ds handled separately


############### You should not need to edit below this line ############

import os
import glob
from buildhelp import RATENVFILE, src_module, build_list

# Get build environment for making RAT applications
env = SConscript(RATENVFILE)

# Collect list of object files from source directories and prepare
# headers for copying.

# src_module() returns list for each directory.
ratobj = []
for m in modules:
    ratobj += src_module(env, m)
ratobj += src_module(env, 'ds', header_subdir='DS')

#### Target: Copy RAT header files to $RATROOT/include
# Define an alias so one can just type "scons headers" to only copy headers.
copy_headers = env.Alias('headers', env['RATHEADERS'])
env.Default(copy_headers)

#### Target: CINT dictionary for data structure classes
cint_cls= Split('DS/Root DS/MC DS/EV DS/MCParticle DS/MCPhoton '
                'DS/MCPMT DS/MCSummary DS/PMT '
                'DS/RunStore DS/Run '
                'DS/PosFit DS/PMTInfo '
                'DS/MCTrack DS/MCTrackStep DS/Calib '
                'DS/Centroid DS/PathFit '
                'DSReader DSWriter TrackNav TrackNode TrackCursor DB DBLink '
                'DBTextLoader DBJsonLoader HTTPDownloader Log ObjInt ObjDbl '
                'DS/LinkDef')


cint_headers = ['include/RAT/' + clsname for clsname in cint_cls]
if bool(ARGUMENTS.get('cppjailbreak', True)):
    jailbreak_env = env.Clone(CPPDEFINES=[('protected','public')])
    ratdict = jailbreak_env.RootDict(os.path.join(jailbreak_env['BUILDDIR'], 'RAT_Dict_jailbreak.cc'),
                                     cint_headers)
else:
    ratdict = env.RootDict(os.path.join(env['BUILDDIR'], 'RAT_Dict.cc'),
                           cint_headers)

#### Target: RAT Library
ratlib = env.StaticLibrary(env['RATLIB'], ratobj + ratdict)

#### Target: Main RAT program
ratbin = env.RATApp(env['RATBIN'], [])
env.Default(ratbin)

#### Target: Shared library for ROOT
cint_source = Split('ds/Root io/DSReader io/TrackNav io/TrackCursor '
                    'ds/MCPMT '
                    'io/DSWriter '
                    'db/DB db/DBLink db/DBTextLoader db/DBTable '
                    'db/DBJsonLoader util/ReadFile db/HTTPDownloader '
                    'ds/RunStore '
                    'core/Log stlplus/fileio stlplus/multiio '
                    'stlplus/string_utilities stlplus/dprintf stlplus/textio '
                    'stlplus/stringio '
                    'stlplus/exceptions stlplus/debug stlplus/file_system '
                    'db/json')

solib_obj = env.SharedObject([os.path.join(env['BUILDDIR'], clsname+'.cc')
                              for clsname in cint_source])
ratsolib = env.SharedLibrary(env['RATSOLIB'], solib_obj + ratdict)
env.Default(ratsolib)

#### Target: Shared library to make RooFit stop printing that stupid banner
nobanner_obj = env.SharedObject([os.path.join(env['BUILDDIR'], 'util', 'SilenceRooFitBanner.cc')])
nobanner_so = env.SharedLibrary(os.path.join(env['RATROOT'], 'lib', 'libSilenceRooFitBanner.so'), nobanner_obj)
env.Default(nobanner_so)

#### Target: Custom ROOT which automatically loads rootinit.C from $RATROOT
rootbin = env.Program('bin/root', env.Object(os.path.join(env['BUILDDIR'], 'root.cc')))
env.Default(rootbin)

#### Target: GEANT4 neutron capture data for Gd
installdata = env.Command('installdata', '#data/neutron/load_data.csh',
                          'csh data/neutron/load_data.csh')

#### Target: Documentation
tar_wiki = env.Command('doc/wiki/WikiStart', 'doc/wiki.tar.gz',
                       'tar xvzf $SOURCE -C doc')
tar_html = env.Command('doc/html/index.html', 'doc/html.tar.gz',
                       'tar xvzf $SOURCE -C doc')
dox = env.Command('does_not_exist3', 'dox/Doxyfile',
                  'doxygen dox/Doxyfile && cd dox/html'
                  ' && ./installdox -lgeant4.tag@http://nu.ph.utexas.edu/rat/geant4/')

env.Alias('doc', [tar_wiki, tar_html, dox])

