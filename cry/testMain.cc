/* 

Copyright (c) 2007-2012, The Regents of the University of California. 
Produced at the Lawrence Livermore National Laboratory 
UCRL-CODE-227323. 
All rights reserved. 
 
For details, see http://nuclear.llnl.gov/simulations
Please also read this http://nuclear.llnl.gov/simulations/additional_bsd.html
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
1.  Redistributions of source code must retain the above copyright
notice, this list of conditions and the disclaimer below.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the disclaimer (as noted below) in
the documentation and/or other materials provided with the
distribution.

3. Neither the name of the UC/LLNL nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OF
THE UNIVERSITY OF CALIFORNIA, THE U.S. DEPARTMENT OF ENERGY OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "CRYGenerator.h"
#include "CRYSetup.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>  // For Ubuntu Linux

int main( int argc, const char *argv[]) {

  int nEv=1000; //....default number of cosmic-ray events to produce

  if ( argc < 2 ) {
    std::cout << "usage " << argv[0] << " <setup file name> <N events>\n";
    std::cout << "N events = " << nEv << " by default\n";
    return 0;
  }

  if ( argc > 2 ) nEv=atoi(argv[2]);

  // Read the setup file into setupString
  std::ifstream inputFile;
  inputFile.open(argv[1],std::ios::in);
  char buffer[1000];

  std::string setupString("");
  while ( !inputFile.getline(buffer,1000).eof()) {
    setupString.append(buffer);
    setupString.append(" ");
  }

  // Parse the contents of the setup file
  std::string datapath = std::string(getenv("CRYDATAPATH"));
  CRYSetup *setup=new CRYSetup(setupString,datapath.c_str());

  // Setup the CRY event generator
  CRYGenerator gen(setup);

  // Generate the events
  std::vector<CRYParticle*> *ev=new std::vector<CRYParticle*>;
  for ( int i=0; i<nEv; i++) {
    ev->clear();
    gen.genEvent(ev);

    // Write the event information to standard output
    std::cout << "Event: " << i << std::endl;
    for ( unsigned j=0; j<ev->size(); j++) {
      CRYParticle* p = (*ev)[j];
      std::cout << "Secondary " << j 
                << " " << CRYUtils::partName(p->id()) 
                << " ke=" << p->ke() 
                << " (x,y,z)=" 
                << " " << p->x()
                << " " << p->y()
                << " " << p->z()
                << " (m)"
                << "\n"; 
      delete (*ev)[j];
    }
  }

  std::cout << "Run completed.\n";
  std::cout << "Total time simulated: " << gen.timeSimulated() << " seconds\n";

  return 0;
}
