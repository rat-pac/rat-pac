# -*- mode: Makefile -*-

#CC = clang++
CC = g++

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --libs` -lGeom -lMinuit
RAT_INCLUDES = -I./../include
RAT_LIBS = -L./../lib -lRATEvent -lrat
LOCAL_INC = -I. 
#LOCAL_INC += -I/Users/twongjirad/software/gperftools-2.4/include


#CFLAGS = -g -fPIC
CFLAGS = -O3 -fPIC
CFLAGS += $(ROOT_INCLUDES) $(RAT_INCLUDES) $(LOCAL_INC)
LDFLAGS += `root-config --ldflags` 
LDLIBS = $(RAT_LIBS) $(ROOT_LIBS) -lstdc++
#LDLIBS += -L/Users/twongjirad/software/gperftools-2.4/lib -lprofiler 

all: libKPFit.so run_kpfit

run_kpfit: run_kpfit.cc libKPFit.so
	$(CC) $(CFLAGS) -c -o run_kpfit.o run_kpfit.cc
	$(CC) $(LDFLAGS) -o run_kpfit run_kpfit.o libKPFit.so $(LDLIBS)

kpfit.o: kpfit.hh kpfit.cc
	$(CC) $(CFLAGS) -c -o kpfit.o kpfit.cc

libKPFit.so: kpfit.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)
	cp libKPFit.so ../lib/

clean: 
	rm *.o libKPFit.so run_kpfit