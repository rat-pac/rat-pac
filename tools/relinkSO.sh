#!/bin/sh
## Code created to fix a certain link error on certain version of mac os. Kept here as a reference, may not be useable as is.

clang++  -headerpad_max_install_names  -o lib/libRATEvent.so -pthread -Wl,-rpath,/Users/bergevin1/software/root_build/lib -dynamiclib build/macosx64/ds/Root.os build/macosx64/io/DSReader.os build/macosx64/io/TrackNav.os build/macosx64/io/TrackCursor.os build/macosx64/ds/MCPMT.os build/macosx64/io/DSWriter.os build/macosx64/db/DB.os build/macosx64/db/DBLink.os build/macosx64/db/DBTextLoader.os build/macosx64/db/DBTable.os build/macosx64/db/DBJsonLoader.os build/macosx64/util/ReadFile.os build/macosx64/db/HTTPDownloader.os build/macosx64/ds/RunStore.os build/macosx64/core/Log.os build/macosx64/stlplus/fileio.os build/macosx64/stlplus/multiio.os build/macosx64/stlplus/string_utilities.os build/macosx64/stlplus/dprintf.os build/macosx64/stlplus/textio.os build/macosx64/stlplus/stringio.os build/macosx64/stlplus/exceptions.os build/macosx64/stlplus/debug.os build/macosx64/stlplus/file_system.os build/macosx64/db/json.os build/macosx64/RAT_Dict_jailbreak.os -Llib -L/Users/bergevin1/software/root_build/lib -L/Users/bergevin1/software/geant4.10.2-install/lib -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lpthread -lm -ldl -lSpectrum -lFoam -lPyROOT -lMinuit2 -lG4Tree -lG4FR -lG4GMocren -lG4visHepRep -lG4RayTracer -lG4VRML -lG4vis_management -lG4modeling -lG4interfaces -lG4persistency -lG4analysis -lG4error_propagation -lG4readout -lG4physicslists -lG4run -lG4event -lG4tracking -lG4parmodels -lG4processes -lG4digits_hits -lG4track -lG4particles -lG4geometry -lG4materials -lG4graphics_reps -lG4intercoms -lG4global -lG4clhep -lG4zlib -lcurl -lbz2 -lz 
 install_name_tool -change @rpath/libG4Tree.dylib $SOFT/geant4.10.2-install/lib/libG4Tree.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4FR.dylib $SOFT/geant4.10.2-install/lib/libG4FR.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4GMocren.dylib $SOFT/geant4.10.2-install/lib/libG4GMocren.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4visHepRep.dylib $SOFT/geant4.10.2-install/lib/libG4visHepRep.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4RayTracer.dylib $SOFT/geant4.10.2-install/lib/libG4RayTracer.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4VRML.dylib $SOFT/geant4.10.2-install/lib/libG4VRML.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4vis_management.dylib $SOFT/geant4.10.2-install/lib/libG4vis_management.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4modeling.dylib $SOFT/geant4.10.2-install/lib/libG4modeling.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4interfaces.dylib $SOFT/geant4.10.2-install/lib/libG4interfaces.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4persistency.dylib $SOFT/geant4.10.2-install/lib/libG4persistency.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4analysis.dylib $SOFT/geant4.10.2-install/lib/libG4analysis.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4error_propagation.dylib $SOFT/geant4.10.2-install/lib/libG4error_propagation.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4readout.dylib $SOFT/geant4.10.2-install/lib/libG4readout.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4physicslists.dylib $SOFT/geant4.10.2-install/lib/libG4physicslists.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4run.dylib $SOFT/geant4.10.2-install/lib/libG4run.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4event.dylib $SOFT/geant4.10.2-install/lib/libG4event.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4tracking.dylib $SOFT/geant4.10.2-install/lib/libG4tracking.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4parmodels.dylib $SOFT/geant4.10.2-install/lib/libG4parmodels.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4processes.dylib $SOFT/geant4.10.2-install/lib/libG4processes.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4digits_hits.dylib $SOFT/geant4.10.2-install/lib/libG4digits_hits.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4track.dylib $SOFT/geant4.10.2-install/lib/libG4track.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4particles.dylib $SOFT/geant4.10.2-install/lib/libG4particles.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4geometry.dylib $SOFT/geant4.10.2-install/lib/libG4geometry.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4materials.dylib $SOFT/geant4.10.2-install/lib/libG4materials.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4graphics_reps.dylib $SOFT/geant4.10.2-install/lib/libG4graphics_reps.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4intercoms.dylib $SOFT/geant4.10.2-install/lib/libG4intercoms.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4global.dylib $SOFT/geant4.10.2-install/lib/libG4global.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4clhep.dylib $SOFT/geant4.10.2-install/lib/libG4clhep.dylib lib/libRATEvent.so 
 install_name_tool -change @rpath/libG4zlib.dylib $SOFT/geant4.10.2-install/lib/libG4zlib.dylib lib/libRATEvent.so 
