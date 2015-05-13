g++ -O3 -I $RATROOT/include/ -I $ROOTSYS/include/ -L $RATROOT/lib/ -L $ROOTSYS/lib/ -o event_dump `root-config --libs` -lRATEvent event_dump.cpp
