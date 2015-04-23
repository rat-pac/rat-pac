#include "CRYGenerator.h"
#include "CRYSetup.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <stdlib.h>  // For Ubuntu Linux
#include <assert.h>

#include "TFile.h"
#include "TTree.h"
#include "TRandom3.h"
#include "CRYParticle.h"

/* ---------------------------------------------------
   This program generates candidate CR events
   using the CRY package. It filters out the events
   based on which particles will pass through a box
   surrounding the KPIPE geometry.
   --------------------------------------------------- */

//#define __DEBUG_ME__ 1

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
  return mass;
}

bool intersectKPIPEbox( double pos[], double dir[], double intersectpt[] ) {

  // our box around the kpipe detector is a centered, axis-aligned bouning box, which reduces 
  // the intersection test to a bunch of if-statements.
  // we also are able to describe the box with 1 point, the max extent of the box.
  // we assume that pos is in meters and that dir is in the coordinate frame of the kpipe world
  double box[3] = { 2.0, 2.0, 50.0 }; // meters
  
  // ------------------------------------------
  // first, check if rays point away from box
  for (int i=0; i<3; i++) {
    if ( pos[i]>box[i] && dir[i]>0 ) {
#ifdef __DEBUG_ME__
      std::cout << "miss: ray in wrong direction (" << i << ")" << std::endl;
#endif
      return false;
    }
    if ( pos[i]<-box[i] && dir[i]<0 ) {
#ifdef __DEBUG_ME__
      std::cout << "miss: ray in wrong direction (" << i << ")" << std::endl;
#endif
      return false;
    }
  }

  // ------------------------------------------
  // next, check if along axis
  int aci[3][3] = { {0,1,2}, {1,0,2}, {2,0,1} }; // axis check indices
		    
  for (int a=0; a<3; a++) {
    if ( dir[aci[a][0]]==1.0 ) {
      if ( pos[aci[a][0]]>-box[aci[a][0]] && pos[aci[a][0]]<box[aci[a][0]] 
	   && fabs(pos[aci[a][1]])<box[aci[a][1]] && fabs(pos[aci[a][2]])<box[aci[a][2]] ) {
	// along +x, inside box
	intersectpt[aci[a][0]] = box[aci[a][0]];
	intersectpt[aci[a][1]] = pos[aci[a][1]];
	intersectpt[aci[a][2]] = pos[aci[a][2]];
#ifdef __DEBUG_ME__
	std::cout << "hit: axis-aligned: " << a << std::endl;
#endif
	return true;
      }
      else if ( pos[aci[a][0]]<-box[aci[a][0]] && fabs(pos[aci[a][1]])<box[aci[a][1]] && fabs(pos[aci[a][2]])<box[aci[a][2]] ) {
	// along +x, outside box
	intersectpt[aci[a][0]] = -box[aci[a][0]];
	intersectpt[aci[a][1]] = pos[aci[a][1]];
	intersectpt[aci[a][2]] = pos[aci[a][2]];
#ifdef __DEBUG_ME__
	std::cout << "hit: axis-aligned: " << a << std::endl;
#endif
	return true;
      }
#ifdef __DEBUG_ME__      
      std::cout << "miss: axis-aligned: " << a << std::endl;
#endif
      return false;
    }
    else if ( dir[aci[a][0]]==-1.0 ) {
      if ( pos[aci[a][0]]<box[aci[a][0]] && pos[aci[a][0]]>-box[aci[a][0]] 
	   && fabs(pos[aci[a][1]])<box[aci[a][1]] && fabs(pos[aci[a][2]])<box[aci[a][2]] ) {
	// along -x, inside box
	intersectpt[aci[a][0]] = -box[aci[a][0]];
	intersectpt[aci[a][1]] = pos[aci[a][1]];
	intersectpt[aci[a][2]] = pos[aci[a][2]];
#ifdef __DEBUG_ME__
	std::cout << "hit: axis-aligned: " << a << std::endl;
#endif
	return true;
      }
      else if ( pos[aci[a][0]]>box[aci[a][0]] && fabs(pos[aci[a][1]])<box[aci[a][1]] && fabs(pos[aci[a][2]])<box[aci[a][2]] ) {
	// along +x, outside box
	intersectpt[aci[a][0]] = box[aci[a][0]];
	intersectpt[aci[a][1]] = pos[aci[a][1]];
	intersectpt[aci[a][2]] = pos[aci[a][2]];
#ifdef __DEBUG_ME__
	std::cout << "hit: axis-aligned: " << a << std::endl;
#endif
	return true;
      }
#ifdef __DEBUG_ME__
      std::cout << "miss: axis-aligned: " << a << std::endl;
#endif
      return false;
    }
  }// loop over axes

  // ------------------------------------------
  
  // now do intersection
  bool intersect = false;
  double shortest = -1.0;
  double testpt[3];

  // find shortest dist to plane in units of direction
  for (int i=0; i<3; i++) {
    if ( dir[i]==0 ) continue; // no intersection possible
    // each index, we check distance to each plane
    double distp = (box[i]-pos[i])/dir[i];
    double distn = (-box[i]-pos[i])/dir[i];
    double dist = -1;
    if ( distp>0 && distn>0 ) {
      if ( distp<distn ) dist = distp;
      else dist = distn;
    }
    else if ( distp>0 )
      dist = distp;
    else if (distn>0 )
      dist = distn;

    if ( dist<0 )
      continue;

    // now propagate
    for (int j=0; j<3; j++) {
      testpt[j] = pos[j] + dist*dir[j];
    }
    
    // check if on box
    bool plane_intersect = true;
    for (int k=0; k<3; k++) {
      if ( k==i ) continue;
      if ( fabs(testpt[k])>box[k] ) { 
	plane_intersect = false;
      }
    }
#ifdef __DEBUG_ME__
    if ( !plane_intersect )
      std::cout << "intersection missed (planes normal to axis=" << i << "): testpt=" << testpt[0] << " " << testpt[1] << " " << testpt[2] << std::endl;
    else
      std::cout << "intersection made (planes normal to axis=" << i << "): testpt=" << testpt[0] << " " << testpt[1] << " " << testpt[2] << std::endl;
#endif

    if ( plane_intersect && ( dist<shortest || shortest<0 ) ) {
      shortest=dist;
      intersect = true;
    }
  }
  
  for ( int i=0; i<3; i++)
    intersectpt[i] = pos[i] + shortest*dir[i];

#ifdef __DEBUG_ME__
  if ( intersect )
    std::cout << "testing hit: shortest=" << shortest << std::endl;
  else
    std::cout << "testing miss: shortest=" << shortest << std::endl;
#endif

  return intersect;
  
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
  CRYSetup *setup=new CRYSetup(setupString,datapath.c_str());
  //CRYSetup *setup=new CRYSetup(setupString,datapath.c_str());
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
  

  // SETUP OUTPUT
  std::string outputFile = argv[2];
  TFile* outfile = new TFile( outputFile.c_str(), "recreate" );
  TTree* crytree = new TTree("crytree", "CRY EVENTS" );
  int nparticles;
  std::vector< int > status;
  std::vector< int > pdg;
  std::vector< double > momx_gev;
  std::vector< double > momy_gev;
  std::vector< double > momz_gev;
  std::vector< double > mass_gev;
  std::vector< double > posx_mm;
  std::vector< double > posy_mm;
  std::vector< double > posz_mm;
  std::vector< double > hitx_mm;
  std::vector< double > hity_mm;
  std::vector< double > hitz_mm;
  std::vector< double > telapsed_sec;
  std::vector< double > delta_time_ns;
  crytree->Branch( "nparticles", &nparticles, "nparticles/I" );
  crytree->Branch( "status", &status );
  crytree->Branch( "pdg", &pdg );
  crytree->Branch( "momx_gev", &momx_gev );
  crytree->Branch( "momy_gev", &momy_gev );
  crytree->Branch( "momz_gev", &momz_gev );
  crytree->Branch( "mass_gev", &mass_gev );
  crytree->Branch( "posx_mm", &posx_mm );
  crytree->Branch( "posy_mm", &posy_mm );
  crytree->Branch( "posz_mm", &posz_mm );
  crytree->Branch( "hitx_mm", &hitx_mm );
  crytree->Branch( "hity_mm", &hity_mm );
  crytree->Branch( "hitz_mm", &hitz_mm );
  crytree->Branch( "telapsed_sec", &telapsed_sec);
  crytree->Branch( "delta_time_ns", &delta_time_ns );



  // Generate the events
  std::vector<CRYParticle*> *ev=new std::vector<CRYParticle*>;
  int nkeep = 0;
  int nreject = 0;
  double t_last_keep = 0;
  while ( nkeep<nEv ) {

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

    bool keep = false;
    nparticles = 0;
    status.clear();
    pdg.clear();
    momx_gev.clear();
    momy_gev.clear();
    momz_gev.clear();
    mass_gev.clear();
    posx_mm.clear();
    posy_mm.clear();
    posz_mm.clear();
    hitx_mm.clear();
    hity_mm.clear();
    hitz_mm.clear();
    telapsed_sec.clear();
    delta_time_ns.clear();


    for ( unsigned j=0; j<ev->size(); j++) {
      CRYParticle* p = (*ev)[j];
      double mass = getCRParticleMass( p->id() ); // MeV
      double E = p->ke() + mass; // MeV
      double pnorm = sqrt( E*E - mass*mass )*0.001; // GeV
      double pmomv[3] = { 0, 0, 0 };

      double dir[3] = { p->u(), p->v(), p->w() };
      double pos[3] = { p->x(), p->y(), p->z()+10.0 };
      double pos_rot[3] = {0, 0, 0};
      double dir_rot[3] = {0, 0, 0};
      for (int v=0; v<3; v++) {
	dir_rot[v] = 0.0;
	pos_rot[v] = 0.0;
	for (int w=0; w<3; w++) {
	  dir_rot[v] += rotX[v][w]*dir[w];
	  pos_rot[v] += rotX[v][w]*pos[w];
	}
      }

      double hit[3];
      bool intersect = intersectKPIPEbox( pos_rot, dir_rot, hit );


      pmomv[0] = pnorm*dir_rot[0];
      pmomv[1] = pnorm*dir_rot[1];
      pmomv[2] = pnorm*dir_rot[2];

      // store
      if ( intersect ) {
	nparticles++;
	status.push_back(1);
	pdg.push_back( p->PDGid() );
	momx_gev.push_back( pmomv[0] );
	momy_gev.push_back( pmomv[1] );
	momz_gev.push_back( pmomv[2] );
	mass_gev.push_back( mass*0.001 );

	hitx_mm.push_back( hit[0]*1000.0 );
	hity_mm.push_back( hit[1]*1000.0 );
	hitz_mm.push_back( hit[2]*1000.0 );

	if ( fabs(pos_rot[0])<20.0 && fabs(pos_rot[1])<20.0 && fabs(pos_rot[2])<100.0 ) {
	  // use the original position
	  posx_mm.push_back( pos_rot[0]*1000 );
	  posy_mm.push_back( pos_rot[1]*1000 );
	  posz_mm.push_back( pos_rot[2]*1000 );
	  telapsed_sec.push_back( gen.timeSimulated() );	
	  delta_time_ns.push_back( 0.0 );
	}
	else {
	  // not bound in world volume! so we move the starting point up to the hit point
	  posx_mm.push_back( hit[0]*1000 );
          posy_mm.push_back( hit[1]*1000 );
          posz_mm.push_back( hit[2]*1000 );
	  telapsed_sec.push_back( gen.timeSimulated() );
	  double beta = 1.0;
	  if ( mass >0 ) {
	    double gamma = sqrt( 1.0 + (pnorm/(0.001*mass))*(pnorm/(0.001*mass)) );
	    beta = pnorm/(gamma*(0.001*mass));
	  }
	  double pos2hit = 0.;
	  for (int i=0; i<3; i++)
	    pos2hit += (hit[i]-pos_rot[i])*(hit[i]-pos_rot[i]);
	  pos2hit = sqrt(pos2hit); // distance in meters
	  double dt = pos2hit/(beta*3.0e8); // seconds
	  delta_time_ns.push_back( dt*1.0e9 ); // ns
	}

      }

      if (intersect) {
#ifdef __DEBUG_ME__
	std::cout << "INTERSECTS KPIPE BOUNDING BOX: KEEP (" << nkeep+1 << ")" << std::endl;
#endif
	keep = true;
      }
#ifdef __DEBUG_ME__
      else {
	std::cout << "INTERSECTS KPIPE BOUNDING BOX: REJECT (" << nreject+1 << ")" << std::endl;
      }
#endif

#ifdef __DEBUG_ME__
      std::cout << 1 << " " // ISTEHP: Status code
		<< p->PDGid() << " " // IDHEP: PDG
		<< 0 << " " << 0 << " " // JDAHEP1, JDAHEP2: first, last daughter
		<< pmomv[0] << " " << pmomv[1] <<  " " << pmomv[1] << " " //  PHEP1 PHEP2 PHEP3: momentum GeV
		<< mass*0.001 << " " // PHEP5: mass GeV
		<< 0 << " "; // DT: delta time
      if ( intersect )
	std::cout << hit[0] << " " << hit[1] << " " << hit[2] << " " //  X Y Z: position in mm
		  << std::endl;
      else
	std::cout << pos_rot[0] << " " << pos_rot[1] << " " << pos_rot[2] << " " //  X Y Z: position in mm
		  << std::endl;
#endif      
    }//end of loop over all particles

    // if one successful intersection
    if ( keep && nparticles>0) {
      t_last_keep = gen.timeSimulated();
      crytree->Fill();
      nkeep++;
    }
    else
      nreject++;
    
    if ( (nkeep+nreject)%1000==0 ) 
      std::cout << "generated " << nkeep+nreject << " events" << std::endl;
    
  }

  double subboxlength = setup->param( CRYSetup::subboxLength );
  std::cout << "Run completed.\n";
  std::cout << "Total time simulated: " << gen.timeSimulated() << " seconds\n";
  std::cout << "Fraction kept: " << double(nkeep)/double(nkeep+nreject) << std::endl;
  std::cout << "bounding box fraction: " << (4.0*100.0)/(subboxlength*subboxlength) << std::endl;
  crytree->Write();

  return 0;
}
