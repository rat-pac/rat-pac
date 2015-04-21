#include "CRYGenerator.h"
#include "CRYSetup.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>  // For Ubuntu Linux
#include <assert.h>

#include "TRandom3.h"
#include "CRYParticle.h"

/* ---------------------------------------------------
   This program generates candidate CR events
   using the CRY package. It filters out the events
   based on which particles will pass through a box
   surrounding the KPIPE geometry.
   --------------------------------------------------- */

TRandom3* __gRANDOM = new TRandom3();

double myrandom() {
  return __gRANDOM->Rndm();
}

double getCRParticleMass( CRYParticle::CRYId id ) {
  double mass; 
  switch ( id ) {
  case CRYParticle::Neutron :
    mass = 939.6; // mev
    break;
  case CRYParticle::Proton :
    mass = 938.3; // mev
    break;
  case CRYParticle::Pion:
    mass = 139.6;
    break;
  case CRYParticle::Kaon:
    mass = 493.7;
    break;
  case CRYParticle::Muon:
    mass = 105.7;
    break;
  case CRYParticle::Electron:
    mass =0.511;
    break;
  case CRYParticle::Gamma:
    mass = 0;
    break;
  default:
    assert(false);
    break;
  }
  return 0;
}

bool intersectKPIPEbox( double pos[], double dir[], double intesectpt[] ) {

  // our box around the kpipe detector is a centered, axis-aligned bouning box, which reduces 
  // the intersection test to a bunch of if-statements.
  // we also are able to describe the box with 1 point, the max extent of the box.
  // we assume that pos is in meters and that dir is in the coordinate frame of the kpipe world
  double box[3] = { 2.0, 2.0, 5.0 }; // meters
  
  // first, check if rays point away from box
  for (int i=0; i<3; i++) {
    if ( pos[i]>box[i] && dir[i]>0 ) return false;
    if ( pos[i]<-box[i] && dir[i]<0 ) return false;
  }

  // next, check if along axis
  for (int i=0; i<3; i++) {
    if ( dir[i]==1.0 )


}

int main( int argc, const char *argv[]) {

  int nEv=1000; //....default number of cosmic-ray events to produce
  int seed=-1;

  if ( argc < 3 ) {
    std::cout << "usage " << argv[0] << " <setup file name> <output file> <N events> <seed>\n";
    std::cout << "output file: ROOT file" << std::endl;
    std::cout << "N events = " << nEv << " by default\n";
    return 0;
  }

  if ( argc > 3 ) {
    nEv=atoi(argv[3]);
    if (argc>=5) {
      seed = atoi(argv[4]);
    }
  }

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
  CRYSetup *setup=new CRYSetup(setupString,-1,datapath.c_str());
  if ( seed>=0 ) {
    __gRANDOM->SetSeed( seed );
    setup->setRandomFunction( &myrandom );
  }


  // Setup the CRY event generator
  CRYGenerator gen(setup);

  // we need to rotate vectors to KPIPE geometry coordinates: detector along z-axis (source at negative end), dirt at -Y
  // Need active rotation of -90 deg on x-axis
  double rotX[3][3] = { {1.0, 0.0, 0.0},
			{0.0, 0.0, 1.0},
			{0.0, -1.0, 0.0} };
  

  // Generate the events
  std::vector<CRYParticle*> *ev=new std::vector<CRYParticle*>;
  for ( int i=0; i<nEv; i++) {
    ev->clear();
    gen.genEvent(ev);

    // now we filter the events. we keep events that intersect kpipe box around detector:
    // dx,dy,dz = ( 5.0 m, 5.0 m, 110 m )
    
    

    // ======================================================================
    // PRINT OUT KINEMATICS FOR RAT

//     NHEP
//       ISTHEP IDHEP JDAHEP1 JDAHEP2 PHEP1 PHEP2 PHEP3 PHEP5 DT X Y Z PLX PLY PLZ
//       ISTHEP IDHEP JDAHEP1 JDAHEP2 PHEP1 PHEP2 PHEP3 PHEP5 DT X Y Z PLX PLY PLZ
//       ISTHEP IDHEP JDAHEP1 JDAHEP2 PHEP1 PHEP2 PHEP3 PHEP5 DT X Y Z PLX PLY PLZ
//       ... [NHEP times]
    
    std::cout << ev->size() << std::endl;
    for ( unsigned j=0; j<ev->size(); j++) {
      CRYParticle* p = (*ev)[j];
      double mass = getCRParticleMass( p->id() );
      double E = p->ke() + mass;
      double pnorm = sqrt( E*E - mass*mass )*0.001; // GeV
      double pmomv[3] = { 0, 0, 0 };
      pmomv[0] = pnorm*p->u();
      pmomv[1] = pnorm*p->v();
      pmomv[2] = pnorm*p->w();

      std::cout << 1 << " " // ISTEHP: Status code
		<< p->PDGid() << " " // IDHEP: PDG
		<< 0 << " " << 0 << " " // JDAHEP1, JDAHEP2: first, last daughter
		<< pmomv[0] << " " << pmomv[1] <<  " " << pmomv[1] << " " //  PHEP1 PHEP2 PHEP3: momentum GeV
		<< mass*0.001 << " " // PHEP5: mass GeV
		<< 0 << " " // DT: delta time
		<< p->x() << " " << p->y() << " " << p->z() << " " //  X Y Z: position in mm
		<< std::endl;
    }
	

  }

  //std::cout << "Run completed.\n";
  //std::cout << "Total time simulated: " << gen.timeSimulated() << " seconds\n";

  return 0;
}
