#!/usr/bin/env ipython
import numpy as np
import subprocess
''' Need to add -headerpad_max_install_names when linking the library, then we can change the path manually.'''
''' Easiest way is to delete lib/libRatEvent.so and see the command line to modify, -header needs to go before -o'''

array = np.array(["libG4Tree.dylib","libG4FR.dylib","libG4GMocren.dylib",\
	"libG4visHepRep.dylib",\
	"libG4RayTracer.dylib",\
	"libG4VRML.dylib",\
	"libG4vis_management.dylib",\
	"libG4modeling.dylib",\
	"libG4interfaces.dylib",\
	"libG4persistency.dylib",\
	"libG4analysis.dylib",\
	"libG4error_propagation.dylib",\
	"libG4readout.dylib",\
	"libG4physicslists.dylib",\
	"libG4run.dylib",\
	"libG4event.dylib",\
	"libG4tracking.dylib",\
	"libG4parmodels.dylib",\
	"libG4processes.dylib",\
	"libG4digits_hits.dylib",\
	"libG4track.dylib",\
	"libG4particles.dylib",\
	"libG4geometry.dylib",\
	"libG4materials.dylib",\
	"libG4graphics_reps.dylib",\
	"libG4intercoms.dylib",\
	"libG4global.dylib",\
	"libG4clhep.dylib",\
	"libG4zlib.dylib"])


relink = '''clang++  -headerpad_max_install_names  -o lib/libRATEvent.so -pthread -Wl,-rpath,/Users/bergevin1/software/root_build/lib -dynamiclib build/macosx64/ds/Root.os build/macosx64/io/DSReader.os build/macosx64/io/TrackNav.os build/macosx64/io/TrackCursor.os build/macosx64/ds/MCPMT.os build/macosx64/io/DSWriter.os build/macosx64/db/DB.os build/macosx64/db/DBLink.os build/macosx64/db/DBTextLoader.os build/macosx64/db/DBTable.os build/macosx64/db/DBJsonLoader.os build/macosx64/util/ReadFile.os build/macosx64/db/HTTPDownloader.os build/macosx64/ds/RunStore.os build/macosx64/core/Log.os build/macosx64/stlplus/fileio.os build/macosx64/stlplus/multiio.os build/macosx64/stlplus/string_utilities.os build/macosx64/stlplus/dprintf.os build/macosx64/stlplus/textio.os build/macosx64/stlplus/stringio.os build/macosx64/stlplus/exceptions.os build/macosx64/stlplus/debug.os build/macosx64/stlplus/file_system.os build/macosx64/db/json.os build/macosx64/RAT_Dict_jailbreak.os -Llib -L/Users/bergevin1/software/root_build/lib -L/Users/bergevin1/software/geant4.10.2-install/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lpthread -lm -ldl -lSpectrum -lFoam -lPyROOT -lMinuit2 -lG4Tree -lG4FR -lG4GMocren -lG4visHepRep -lG4RayTracer -lG4VRML -lG4vis_management -lG4modeling -lG4interfaces -lG4persistency -lG4analysis -lG4error_propagation -lG4readout -lG4physicslists -lG4run -lG4event -lG4tracking -lG4parmodels -lG4processes -lG4digits_hits -lG4track -lG4particles -lG4geometry -lG4materials -lG4graphics_reps -lG4intercoms -lG4global -lG4clhep -lG4zlib -lcurl -lbz2 -lz '''
print relink
for i in array:
	subP =  " install_name_tool -change @rpath/%s $SOFT/geant4.10.2-install/lib/%s lib/libRATEvent.so " %(i,i)
	# process = subprocess.Popen(subP.split(), stdout=subprocess.PIPE)
	# output, error = process.communicate()
	print subP
