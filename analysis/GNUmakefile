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

all: scrape_data libkptrigger.so analyze_data

scrape_data.o: scrape_data.cc
	$(CC) $(CFLAGS) -c -o $@ $^

kptrigger.o: kptrigger.cc kptrigger.h
	$(CC) $(CFLAGS) -c -o $@ kptrigger.cc

libkptrigger.so: kptrigger.o
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)
	cp $@ ../lib/

scrape_data: scrape_data.o libkptrigger.so
	$(CC) $(LDFLAGS) -o $@ scrape_data.o $(LDLIBS) -lkptrigger

analyze_data.o: analyze_data.cc
	$(CC) $(CFLAGS) -c -o $@ $^

analyze_data: analyze_data.o libkptrigger.so
	$(CC) $(LDFLAGS) -o $@ analyze_data.o $(LDLIBS) -lkptrigger

clean: 
	rm *.o scrape_data libkptrigger.so analyze_data