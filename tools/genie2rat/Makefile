SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

# Include machine specific flags and locations (inc. files & libs)
#
include $(GENIE)/src/make/Make.include

GENIE_LIBS  = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)
RAT_LIBS = -L$(RATROOT)/lib -lRATEvent

LIBRARIES  := $(GENIE_LIBS) $(LIBRARIES) $(CERN_LIBRARIES) $(RAT_LIBS)

INCLUDES := $(INCLUDES) -I$(RATROOT)/include

all: genie2rat

genie2rat: FORCE
	$(CXX) -g -o genie2rat $(CXXFLAGS) $(LDFLAGS) genie2rat.cc $(INCLUDES) $(LIBRARIES)

#################### CLEANING

purge: FORCE
	$(RM) *.o *~ core 

clean: FORCE
	$(RM) *.o *~ core 
	$(RM) genie2rat


FORCE:

# DO NOT DELETE
