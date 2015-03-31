# -*- mode: Makefile -*-

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --glibs`
LOCAL_INC = -I. 


CFLAGS = -g
#CFLAGS = -O3
CFLAGS += $(ROOT_INCLUDES) $(LOCAL_INC)
LDFLAGS += `root-config --ldflags` 
LDLIBS = $(ROOT_LIBS) -lstdc++

all: gen_kdar_event

gen_kdar_event.o: gen_kdar_event.cc
	g++ $(CFLAGS) -c -o gen_kdar_event.o gen_kdar_event.cc

gen_kdar_event: gen_kdar_event.o
	g++ $(LDFLAGS) -o gen_kdar_event gen_kdar_event.o $(LDLIBS)
