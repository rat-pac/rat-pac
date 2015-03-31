# -*- mode: Makefile -*-

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --glibs` -lGeom
LOCAL_INC = -I. 


CFLAGS = -g
#CFLAGS = -O3
CFLAGS += $(ROOT_INCLUDES) $(LOCAL_INC)
LDFLAGS += `root-config --ldflags` 
LDLIBS = $(ROOT_LIBS) -lstdc++

all: gen_kdar_event

tree3.o: tree3.cc tree3.h
	g++ $(CFLAGS) -c -o tree3.o tree3.cc

gen_kdar_event.o: gen_kdar_event.cc
	g++ $(CFLAGS) -c -o gen_kdar_event.o gen_kdar_event.cc

gen_kdar_event: gen_kdar_event.o tree3.o
	g++ $(LDFLAGS) -o gen_kdar_event $^ $(LDLIBS)
clean: 
	rm *.o gen_kdar_event