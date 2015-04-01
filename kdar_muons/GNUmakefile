# -*- mode: Makefile -*-

CC = clang++

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --glibs` -lGeom
LOCAL_INC = -I. 


CFLAGS = -g
#CFLAGS = -O3
CFLAGS += $(ROOT_INCLUDES) $(LOCAL_INC)
LDFLAGS += `root-config --ldflags` 
LDLIBS = $(ROOT_LIBS) -lstdc++

all: gen_kdar_event

GenGeomDAR.o: GenGeomDAR.cc GenGeomDAR.h
	$(CC) $(CFLAGS) -c -o GenGeomDAR.o GenGeomDAR.cc

tree3.o: tree3.cc tree3.h
	$(CC) $(CFLAGS) -c -o tree3.o tree3.cc

gen_kdar_event.o: gen_kdar_event.cc
	$(CC) $(CFLAGS) -c -o gen_kdar_event.o gen_kdar_event.cc

gen_kdar_event: gen_kdar_event.o tree3.o GenGeomDAR.o
	$(CC) $(LDFLAGS) -o gen_kdar_event $^ $(LDLIBS)
clean: 
	rm *.o gen_kdar_event