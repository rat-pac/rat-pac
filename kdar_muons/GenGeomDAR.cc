#include "GenGeomDAR.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <exception>
#include "TGeoManager.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TMath.h"
#include "TRandom3.h"
#include "TGeoNode.h"

GenGeomDAR::GenGeomDAR( std::string gdml_file, double* source_pos, double rmin, double rmax, double planez, double facex, double facey, int seed ) 
  : fRmin( rmin ), fRmax( rmax ), fPlaneZ( planez ), fFaceX( facex ), fFaceY( facey ), fMakeDiagnosticPlots( false ), fVerbose(0) 
{
  for (int i=0; i<3; i++)
    fSourcePos[i] = source_pos[i];

  gdml = TGeoManager::Import( gdml_file.c_str() );
  if ( gdml==NULL ) {
    throw std::runtime_error( "Failed importing GDML geometry." );
  }
  int trackid = 0;
  int ntracks = gdml->AddTrack( trackid, 14 ); // this values don't matter. just need track.
  if ( seed<0 )
    rand = new TRandom3( time(NULL) );
  else
    rand = new TRandom3( seed );

  double dir[3] = { 0, 0, 1 };
  double pos[3];
  GenPosition( dir, pos, fPathRateNorm );
}

GenGeomDAR::~GenGeomDAR() {
  delete rand;
  if ( fMakeDiagnosticPlots ) {
    fDiagFile->cd();
    hRdist->Write();
    hR2dist->Write();
    hCosz->Write();
    hPhi->Write();
    hXY->Write();
    hZY->Write();
    hZX->Write();
    fDiagFile->Close();
  }
}


void GenGeomDAR::MakeDiagnosticPlots( std::string outfile ) {
  if ( fMakeDiagnosticPlots==false ) {
    fDiagFile = new TFile( outfile.c_str(), "recreate" );
    hRdist = new TH1D("hRdist", "", 1000, 0, 1.1*fRmax );
    hR2dist = new TH1D("hR2dist", "", 1000, 0, 1.1*fRmax );
    hCosz = new TH1D("hCosz", "", 1000, 0, 1.0 );
    hPhi = new TH1D("hPhi", "", 1000, 0, 2.0*TMath::Pi() );
    hXY = new TH2D("hXY","",100, -1000, 1000, 100, -1000, 1000 );
    hZY = new TH2D("hZY","",100, -10000, 10000, 100, -1000, 1000 );
    hZX = new TH2D("hZX","",100, -10000, 10000, 100, -1000, 1000 );
    fMakeDiagnosticPlots = true;
  }
}

void GenGeomDAR::GetDirection( double dir[] ) {

  // get valid direction
  // throws directions until ray intersects planar window. 
  // generates events in that solid angle, weighting by material.  
  // also restricts range of radii.
  // input:
  //   source, float[3]: position in world coordinates of neutrino from DAR
  //   facez: z-position of plane we are testing
  //   faceh: height of plane (x-direction)
  //   facew: width of plane (y-direction)
  // note that the plane-window is assumed to be tangent on a 2-sphere whose center is at source and whose radius is |facez-source[2]|
  // output:
  //   dir, float[3]

  bool hit = false;
  int nthrows = 0;
  double maxangle = atan(sqrt(fFaceX*fFaceX+fFaceY*fFaceY)/fabs( fSourcePos[2]-fPlaneZ ));

  while ( !hit ) {
    double costh = rand->Uniform(); // I can make this smarter by restricting cosz ranges
    double phi = 2.0*TMath::Pi()*rand->Uniform();
    dir[0] = sqrt(1-costh*costh)*cos(phi);
    dir[1] = sqrt(1-costh*costh)*sin(phi);
    dir[2] = costh;
    if ( costh<0 ) // wrong direction
      continue;

    // check if intersects plane
    double dz = fPlaneZ-fSourcePos[2]; // distance between face and source
    //double dhmax = sqrt( dz*dz + faceh*faceh ); // face is centered on z-axis
    //double dwmax = sqrt( dz*dz + facew*facew );
    double tz = fabs(dz)/dir[2];
    if ( fabs(dir[0]*tz + fSourcePos[0])<fFaceX && fabs(dir[1]*tz + fSourcePos[1])<fFaceY ) {
      hit = true;
    }
    nthrows++;
  }

  if ( fVerbose>=1 )
    std::cout << "  direction nthrows: " << nthrows << std::endl;

  if ( fVerbose>=1 ) {
    std::cout << "  dist from source to plane: " << fabs(-fPlaneZ-fSourcePos[2]) << " cm" << std::endl;;
    std::cout << "  max cosz angle: " << maxangle << " deg" << std::endl;
    std::cout << "  generated dir: " << dir[0] << ", " << dir[1] << ", " << dir[2] << ". cosz=" << acos(dir[2])*180.0/3.14159 << " degs" << std::endl;
  }
  
}

void GenGeomDAR::GenPosition( double dir[], double final_pos[], double& path_rate )  {

  // =================================================================
  // Define Neutrino Path through geometry
  std::vector<float> distances; // distances to each boundary
  std::vector<float> densities; // densities over each step
  std::vector< std::string > volnames;
  double pos[3];
  
  // define starting point
  for (int v=0; v<3; v++)
    pos[v] = fSourcePos[v] + dir[v]*fRmin;

  gdml->SetCurrentTrack( 0 );
  gdml->SetCurrentDirection( dir );
  gdml->SetCurrentPoint( pos );
  
  TGeoNode* startNode = gdml->FindNode();
  TGeoNode* next_node = startNode;
  const Double_t* current_pos;
  double current_radius, next_radius;
  if ( fVerbose>=1 )
    std::cout << "  Start: " << pos[0] << ", " << pos[1] << ", " << pos[2] << ": " << startNode->GetVolume()->GetName() << std::endl;
  
  while (next_node) {
    double density = next_node->GetMedium()->GetMaterial()->GetDensity();
    std::string volname = next_node->GetVolume()->GetName();
    next_node = gdml->FindNextBoundaryAndStep();
    current_pos = gdml->GetCurrentPoint();
    double stepdist = 0;
    double srcdist = 0.;
    current_radius = 0.;
    next_radius = 0.;
    for (int v=0; v<3; v++) {
      stepdist += (current_pos[v]-pos[v])*(current_pos[v]-pos[v]);
      current_radius = (pos[v]-fSourcePos[v])*(pos[v]-fSourcePos[v]);
      next_radius = (current_pos[v]-fSourcePos[v])*(current_pos[v]-fSourcePos[v]);
      pos[v] = current_pos[v];
    }
    stepdist = sqrt(stepdist);
    current_radius = sqrt(current_radius);
    next_radius = sqrt(next_radius);
    if ( fVerbose>=1 ) {
      std::cout << "  STEP ------- " << std::endl;
      std::cout << "    current pos: " << current_pos[0] << ", " <<  current_pos[1]  << ", " <<  current_pos[2] << std::endl;
      std::cout << "    step: dist=" << stepdist << " density=" << density << " r1=" << current_radius << " r2=" << next_radius << " dr=" << next_radius-current_radius << std::endl;
      std::cout << "    current node: " << volname << std::endl;
      if ( next_node) {
	std::cout << "    next node: " << next_node->GetVolume()->GetName() << std::endl;
      }
      else
	std::cout << "    next node: out of bounds" << std::endl;
    }
    
    if ( current_radius>=fRmin && current_radius<fRmax && next_radius>=fRmin && next_radius<fRmax ) {
      // easy case
      distances.push_back( stepdist );
      densities.push_back( density );
    }
    else if ( current_radius<fRmin && next_radius>=fRmin && next_radius<fRmax ) {
      distances.push_back( next_radius-fRmin );
      densities.push_back( density );
    }
    else if ( current_radius>=fRmin && current_radius<fRmax && next_radius>fRmax ) {
      distances.push_back( fRmax-current_radius );
      densities.push_back( density );
    }
    else if ( current_radius<=fRmin && next_radius>=fRmax ) {
      distances.push_back( fRmax-fRmin );
      densities.push_back( density );
    }
    
    if ( fVerbose>=1 )
      std::cout << "    segment vector: " << distances.at( distances.size()-1 ) << ", " << density << std::endl;
    volnames.push_back( volname );
  }//end of while loop

  // ===========================================================================
  // now use path to build select interaction point along ray, within segment
  if ( fVerbose>=1 )
    std::cout << "  Choose interaction point along ray" << std::endl;

  int nsegments = distances.size();
  double prob[nsegments+1];
  prob[0] = 0.0;
  double totaldist = 0.;
  double totalrate = 0.;
  for ( int iseg=0; iseg<nsegments; iseg++) {
    totaldist += distances.at(iseg);
    totalrate += distances.at(iseg)*densities.at(iseg);
    prob[iseg+1] = totalrate;
  }
  path_rate = totalrate;
  for (int iseg=0; iseg<nsegments; iseg++)  {
    prob[iseg+1] /= totalrate;
    if ( fVerbose>=1 )
      std::cout << "  Segment  [" << iseg << ", " << iseg+1 << "] = " << prob[iseg+1] << " p=" << prob[iseg+1]-prob[iseg] << " " << volnames.at(iseg) << std::endl;
  }
  double p = rand->Uniform();
  double rtraveled = 0;
  for (int iseg=0; iseg<nsegments; iseg++) {
    if ( p>=prob[iseg] && p<prob[iseg+1] ) {
      rtraveled += distances.at(iseg)*rand->Uniform();
      break;
    }
    else {
      rtraveled += distances.at(iseg);
    }
  }
  double final_radius = fRmin + rtraveled;
  for (int v=0; v<3; v++)
    final_pos[v] = fSourcePos[v] + dir[v]*final_radius;
  TGeoNode* final_node = gdml->FindNode( final_pos[0], final_pos[1], final_pos[2] );
  if ( fVerbose>=1 ) {
    std::cout << "  Selected pos: " << final_pos[0] << ", " << final_pos[1] << ", " << final_pos[2] << ". Dist Traveled=" << final_radius << " (" << fRmin << ", " << fRmax << ")" << std::endl;
    std::cout << "  Node: " << final_node->GetVolume()->GetName() << std::endl;
  }

}

void GenGeomDAR::GetPosition( double final_pos[], double& relative_weight ) {

  if ( fVerbose>=1 ) {
    std::cout << "GenGeomDAR::GetPosition -----------------------" << std::endl;
  }
  double dir[3];
  double pathrate;
  // =================================================================
  // get valid direction
  GetDirection( dir );

  // choose position
  GenPosition( dir, final_pos, pathrate );
  relative_weight = pathrate/fPathRateNorm;
  
  if ( fMakeDiagnosticPlots ) {
    double final_radius = 0.;
    for (int i=0; i<3; i++)
      final_radius += ( fSourcePos[i]-final_pos[i] )*(fSourcePos[i]-final_pos[i] );
    final_radius = sqrt(final_radius);

    hRdist->Fill( final_radius, relative_weight );
    hR2dist->Fill( final_radius, (final_radius/fRmin)*(final_radius/fRmin)*relative_weight );
    hCosz->Fill( dir[2], relative_weight );
    hPhi->Fill( atan2( dir[1], dir[0] ), relative_weight );
    hXY->Fill( final_pos[0], final_pos[1], relative_weight );
    hZY->Fill( final_pos[2], final_pos[1], relative_weight );
    hZX->Fill( final_pos[2], final_pos[0], relative_weight );
  }
  
  if ( fVerbose>=1 )
    std::cout << "---------------------------------------------------" << std::endl;
  
}//end of get position

