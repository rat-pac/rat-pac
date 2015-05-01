#include <RAT/GeoPMTCoverageFactory.hh>
#include <vector>
#include <RAT/Log.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {
  
  G4VPhysicalVolume *GeoPMTCoverageFactory::Construct(DBLinkPtr table){
    G4double coverage=0.0, pmtRadius=0.0, pmtDiameter; // coverage is percent solid angle
                                            // pmt radius is distance pmt to center
                                            // pmt geometry diameter
    try{
      coverage = table->GetD("coverage");
      pmtRadius = table->GetD("rescale_radius");
    }catch(DBNotFoundError &e){
      Log::Die("GeoPMTCoverageFactory: coverage or rescale_radius variables unset."); 
    }

    try{
      pmtDiameter = table->GetD("pmtdiameter");
    }catch(DBNotFoundError &e){
      pmtDiameter = 201.6;
    }

    // Below is almost completely stolen from ReactorFsim since it was
    // a feature people wanted ported.  Logic by MW. Wrapper CDT.
    int Ncosbins, Nphibins, Npmt;

    G4double num_pmt = 16*pmtRadius*pmtRadius*coverage/pmtDiameter/pmtDiameter;   
    Ncosbins = int(sqrt(num_pmt/CLHEP::pi));
    Nphibins = int(sqrt(num_pmt*CLHEP::pi));
    Npmt = Ncosbins*Nphibins;

    info << "GeoPMTCoverageFactory: Generated " << Npmt << "PMTs" << endl;

    vector<double> xpmt(Npmt);
    vector<double> ypmt(Npmt);
    vector<double> zpmt(Npmt);

    G4double dphi = CLHEP::twopi / Nphibins;
    G4double dz = 2.0 / Ncosbins;

    for(int i=0;i<Ncosbins;i++){
      G4double z = -1+i*dz+dz/2;
      for(int j=0;j<Nphibins;j++){
        G4double phi = j*dphi+dphi/2;
        xpmt[i*Nphibins+j] = pmtRadius*sqrt(1-z*z)*cos(phi);
        ypmt[i*Nphibins+j] = pmtRadius*sqrt(1-z*z)*sin(phi);
        zpmt[i*Nphibins+j] = pmtRadius*z;
        //      cout<<"pmt location"<<i*Nphibins+j<<" "<<xpmt[i*Nphibins+j]<<" "<<ypmt[i*Nphibins+j]<<" "<<zpmt[i*Nphibins+j]<<endl;
        //      cout << "z,phi  " << z << " " << phi << endl;
      }
    }

    return ConstructPMTs(table, xpmt, ypmt, zpmt);

  }
  
  
} // namespace RAT
