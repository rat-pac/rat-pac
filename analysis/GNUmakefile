# -*- mode: Makefile -*-

CC = clang++
#CC = g++

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --glibs` -lGeom
RAT_INCLUDES = -I./../include
RAT_LIBS = -L./../lib -lRATEvent -lrat
LOCAL_INC = -I. 


CFLAGS = -g
#CFLAGS = -O3
CFLAGS += $(ROOT_INCLUDES) $(RAT_INCLUDES) $(LOCAL_INC)
LDFLAGS += `root-config --ldflags` 
LDLIBS = $(RAT_LIBS) $(ROOT_LIBS) -lstdc++

all: scrape_data

scrape_data.o: scrape_data.cc
	$(CC) $(CFLAGS) -c -o $@ $^

scrape_data: scrape_data.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

clean: 
	rm *.o scrape_data