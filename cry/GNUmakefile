#
# make       = run short test
# make all   = run all tests (including root)
# make runRoot  = compile and run specified test
#
# Doug Wright

include ${CRYHOME}/Makefile.common  #....define compiler and any other common settings

CXX = clang++
EXE = testMain
CXXFLAGS += -I${CRYHOME}/src

ROOT_INCLUDES = `root-config --cflags`
ROOT_LIBS = `root-config --libs --glibs` -lGeom
CXXFLAGS += ${ROOT_INCLUDES}

all: testMain

#....rule to make test executables
test%: test%.cc ${CRYHOME}/lib/libCRY.a
	$(CXX) $(CXXFLAGS) -o $@ $@.cc -L${CRYHOME}/lib -lCRY ${ROOT_LIBS}
 
#....testRoot is different
testRoot: testRoot.cc ${CRYHOME}/lib/libCRY.a
	$(CXX) $(CXXFLAGS) -I$(ROOTSYS)/include -o $@ $@.cc -L${CRYHOME}/lib -lCRY ${ROOT_LIBS}

clean:
	rm -f *.o $(EXE)

#....make sure these executables are not deleted as intermediate files
.PRECIOUS: $(EXE)