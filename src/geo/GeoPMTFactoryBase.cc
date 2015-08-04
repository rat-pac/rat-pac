#include <GeoPMTFactoryBase.hh>
#include <RAT/DB.hh>
#include <RAT/Log.hh>

#include <G4SDManager.hh>
#include <G4PVPlacement.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

#include <RAT/Materials.hh>
#include <RAT/GLG4PMTSD.hh>
#include <G4LogicalBorderSurface.hh>
#include <RAT/GLG4PMTSD.hh>
#include <RAT/DetectorConstruction.hh>
#include <RAT/PMTConstruction.hh>
#include <RAT/WaveguideFactory.hh>
#include <RAT/Factory.hh>
#include <vector>
#include <algorithm>

#include "G4FastSimulationManager.hh"
#include "G4VFastSimulationModel.hh"
#include "GLG4PMTOpticalModel.hh"
#include "G4PhysicsOrderedFreeVector.hh"

#include "G4RandomDirection.hh"

#include "iostream"

using namespace std;

namespace RAT {

DS::PMTInfo GeoPMTFactoryBase::pmtinfo;

G4VPhysicalVolume *GeoPMTFactoryBase::ConstructPMTs(DBLinkPtr table, 
                       std::vector<double> pmt_x, std::vector<double> pmt_y, std::vector<double> pmt_z)
{
  // ------------------ Extra PMT settings --------------------
  string volume_name = table->GetIndex();
  string sensitive_detector_name = table->GetS("sensitive_detector");
  
  int start_idx, end_idx;
  try {
    start_idx = table->GetI("start_idx"); //position in this array to start building pmts
  } catch (DBNotFoundError &e) {
    start_idx = 0; // defaults to beginning
  }
  try {
    end_idx = table->GetI("end_idx"); //id of the last pmt to build in this array
  } catch (DBNotFoundError &e) {
    end_idx = pmt_x.size()-1; // defaults to whole array
  }

  string pos_table_name = table->GetS("pos_table");
  DBLinkPtr lpos_table = DB::Get()->GetLink(pos_table_name);
  
  vector<int> pmt_type;
  try {
    pmt_type = lpos_table->GetIArray("type"); // functional type (e.g. inner, veto, etc. - arbitrary integers)
  } catch (DBNotFoundError &e) {
    pmt_type.resize(pmt_x.size());
    fill(pmt_type.begin(),pmt_type.end(),-1); //defaults to type -1 if unspecified
  }

  // read pmt_detector_type
  string pmt_detector_type = table->GetS("pmt_detector_type");

  // flip pmts to face outwards e.g. for use in a veto
  int flip = 0;
  try {  flip = table->GetI("flip"); }
  catch (DBNotFoundError &e) { }
  
  // Find mother
  string mother_name = table->GetS("mother");
  G4LogicalVolume *mother = FindMother(mother_name);
  if (mother == 0)
    Log::Die("Unable to find mother volume " + mother_name + " for "
		+ volume_name);
 
  string pmt_model = table->GetS("pmt_model"); // the form factor of the PMT (physical properties)
  DBLinkPtr lpmt = DB::Get()->GetLink("PMT", pmt_model);

  PMTConstructionParams pmtParam;
  pmtParam.faceGap = 0.1 * CLHEP::mm;
  pmtParam.zEdge = lpmt->GetDArray("z_edge");
  pmtParam.rhoEdge = lpmt->GetDArray("rho_edge");
  pmtParam.zOrigin = lpmt->GetDArray("z_origin");
  pmtParam.dynodeRadius = lpmt->GetD("dynode_radius");
  pmtParam.dynodeTop = lpmt->GetD("dynode_top");
  pmtParam.wallThickness = lpmt->GetD("wall_thickness");
  try {
    pmtParam.prepulseProb = lpmt->GetD("prepulse_prob");
  }
  catch (DBNotFoundError& e) {}

  // Materials
  pmtParam.exterior = mother->GetMaterial();
  pmtParam.glass = G4Material::GetMaterial(lpmt->GetS("glass_material"));
  pmtParam.dynode = G4Material::GetMaterial(lpmt->GetS("dynode_material"));
  pmtParam.vacuum = G4Material::GetMaterial(lpmt->GetS("pmt_vacuum_material")); 
  string pc_surface_name = lpmt->GetS("photocathode_surface");
  pmtParam.photocathode = Materials::optical_surface[pc_surface_name];
  string mirror_surface_name = lpmt->GetS("mirror_surface");
  pmtParam.mirror = Materials::optical_surface[mirror_surface_name];
  pmtParam.dynode_surface=Materials::optical_surface[lpmt->GetS("dynode_surface")];
  
  if (pmtParam.photocathode == 0)
    Log::Die("GeoPMTFactoryBase error: Photocathode surface \""
		+ pc_surface_name + "\" not found");

  // Simplified PMT drawing for faster visualization
  bool vis_simple = false;
  try {
    vis_simple = table->GetI("vis_simple") != 0;
  } catch (DBNotFoundError &e) { }

  // Orientation of PMTs
  bool orient_manual = false;
  try { 
    string orient_str = table->GetS("orientation");
    if (orient_str == "manual")
      orient_manual = true;
    else if (orient_str == "point")
      orient_manual = false;
    else 
      Log::Die("GeoPMTFactoryBase error: Unknown PMT orientation " 
		  + orient_str);
  } catch (DBNotFoundError &e) { }

  vector<double> dir_x, dir_y, dir_z;
  vector<double> orient_point_array;
  G4ThreeVector orient_point;
  bool rescale_radius = false;
  double new_radius = 1.0;
  if (orient_manual) {
    dir_x = lpos_table->GetDArray("dir_x");
    dir_y = lpos_table->GetDArray("dir_y");
    dir_z = lpos_table->GetDArray("dir_z");
  } else {
    //fill with dummy values needed for pmtinfo.AddPMT. They will be redirected towards
    //the proper point afterwards
    dir_x.push_back(9999.);
    dir_y.push_back(9999.);
    dir_z.push_back(9999.);
    orient_point_array = table->GetDArray("orient_point");
    if (orient_point_array.size() != 3)
      Log::Die("GeoPMTFactoryBase error: orient_point must have 3 values");
    orient_point.set(orient_point_array[0], orient_point_array[1],
		     orient_point_array[2]);    
  }

  // Optionally can rescale PMT radius from mother volume center for
  // case where PMTs have spherical layout symmetry
  try {
    new_radius = table->GetD("rescale_radius");
    rescale_radius = true;
  } catch (DBNotFoundError &e) { }
  

  // get pointer to physical mother volume
  G4VPhysicalVolume* phys_mother = FindPhysMother(mother_name);
  if (phys_mother == 0)
    Log::Die("GeoPMTFactoryBase error: PMT mother physical volume " + mother_name
		+" not found");


  // --------------- Start building PMT geometry ------------------

  // PMT sensitive detector
  G4SDManager* fSDman = G4SDManager::GetSDMpointer();  
  GLG4PMTSD* pmtSDInner= new GLG4PMTSD(sensitive_detector_name, end_idx-start_idx+1, pmtinfo.GetPMTCount(), -1 /* evidently unused? */);
  fSDman->AddNewDetector(pmtSDInner);
  pmtParam.detector = pmtSDInner;
  
  // Setup for waveguide
  WaveguideFactory *waveguide_factory = 0;
  try {
    string waveguide = table->GetS("waveguide");
    string waveguide_desc = table->GetS("waveguide_desc");
    string waveguide_table, waveguide_index;
    if (!DB::ParseTableName(waveguide_desc, waveguide_table, waveguide_index))
      Log::Die("GeoPMTFactoryBase: Waveguide descriptor name is not a valid RATDB table: "
               +waveguide_desc);
               
    waveguide_factory = GlobalFactory<WaveguideFactory>::New(waveguide);
    waveguide_factory->SetTable(waveguide_table, waveguide_index);
    pmtParam.faceGap = waveguide_factory->GetZTop();
    pmtParam.minEnvelopeRadius = waveguide_factory->GetRadius();
  } catch (DBNotFoundError &e) { }

  // Set new efficiency correction if requested
  try {
    float efficiency_correction = table->GetD("efficiency_correction");
    pmtParam.efficiencyCorrection = efficiency_correction;
  } catch (DBNotFoundError &e) { }
  
  // Build PMT
  pmtParam.useEnvelope = false; // disable the use of envelope volume for now
  PMTConstruction pmtConstruct(pmtParam);
    
  G4LogicalVolume *logiPMT = pmtConstruct.NewPMT(volume_name, vis_simple);
  G4LogicalVolume *logiWg = 0;
  G4ThreeVector offsetWg;
  
  // Add waveguide if needed
  if (waveguide_factory) {
    waveguide_factory->SetPMTBodySolid(pmtConstruct.NewBodySolid(volume_name+"_waveguide_sub"));
    logiWg = waveguide_factory->Construct(volume_name+"_waveguide_log",
                                                              logiPMT,
                                                              vis_simple);
    offsetWg = waveguide_factory->GetPlacementOffset();
    if (pmtParam.useEnvelope) {
        new G4PVPlacement
    	      ( 0,                   // no rotation
    	        offsetWg,     
    	        logiWg,            // the logical volume
    	        volume_name+"_waveguide_phys", // a name for this physical volume
    	        logiPMT,                // the mother volume
    	        false,               // no boolean ops
    	        0 );                 // copy number
    } 
  }
     
//preparing to calculate magnetic efficiency corrections for all PMTs, if requested
  int BFieldOn=0;
  try{BFieldOn=DB::Get()->GetLink("BField")->GetI("b_field_on");}
  catch (DBNotFoundError &e){}
  string BFieldTableName="";
  string BEffiTableName="";
  string dynorfilename="";
  string BEffiModel="multiplicative";
  vector<pair<int,double> > BEfficiencyCorrection;
  DBLinkPtr BEffiTable;
//  G4PhysicsOrderedFreeVector Bepsix,Bepsiy;
  vector<G4PhysicsOrderedFreeVector> Bepsix,Bepsiy;
  vector<G4ThreeVector> Bpos,Bf;
  vector<G4ThreeVector> Dpos,Dorie;//dynode position&orient

//Force B efficiency<= 1 by default, whatever the input from BEffiTable.
//If we start believing that B may actually help the PMT response, change default to false
  bool CorrBEpsiInput=true;
  int nocorr=0;
  try{nocorr=DB::Get()->GetLink("BField")->GetI("no_b_efficiency_table_correction");}
  catch (DBNotFoundError &e){}
  if(nocorr) CorrBEpsiInput=false;
  else cout<<"Forcing B efficiency<= 1\n";
  bool HaveDynoData=false;

  if(BFieldOn){
    try{BFieldTableName=DB::Get()->GetLink("BField")->GetS("b_field_file");}
    catch (DBNotFoundError &e){}
    try{BEffiTableName=DB::Get()->GetLink("BField")->GetS("b_efficiency_table");}
    catch (DBNotFoundError &e){}
    try{dynorfilename=DB::Get()->GetLink("dynorfile")->GetS("dynorfilename");}
    catch(DBNotFoundError &e){}
    //check if we can calculate B field effect
    if(BFieldTableName=="" || BEffiTableName==""){
      G4cout<<"B field is on, but either B data or B PMT efficiency correction missing.\n"
        <<"Turning B field off.\n";
      BFieldOn=0;
      BEffiTable=NULL;
    }
    else{
      string ExpSubdir=DB::Get()->GetLink("DETECTOR")->GetS("experiment");
      string BFieldTableName1=string(getenv("GLG4DATA"))+"/"+ExpSubdir+"/"+BFieldTableName;//add the experiment subdir
      ifstream Bdata(BFieldTableName1.data());
      if(!Bdata.is_open()){
        BFieldTableName=string(getenv("GLG4DATA"))+"/"+BFieldTableName;
        cout<<"file "<<BFieldTableName1<<" not found, trying "<<BFieldTableName<<"\n";
        Bdata.close();
        Bdata.open(BFieldTableName.data());
        if(!Bdata.is_open()){
          BFieldOn=false;
          BEffiTable=NULL;
          cout<<"also file "<<BFieldTableName<<" not found, magnetic efficiency correction turned off\n";
        }
        if(!Bdata.good())
          Bdata.clear();//for backwards compatibility: g++ 3.4 requires to manually reset the error state flags on opening a new file with the same stream
      }
      string header;
      getline(Bdata,header);
      double xr,yr,zr,bxr,byr,bzr;
      cout<<"about to load B field from file "<<BFieldTableName<<"\n";
      G4ThreeVector posi,field;
      while(!Bdata.rdstate()){
        Bdata>>xr>>yr>>zr>>bxr>>byr>>bzr;
        posi.set(xr,yr,zr);
        field.set(bxr,byr,bzr);
        Bpos.push_back(posi);
        Bf.push_back(field);
      }
      BEffiTable=DB::Get()->GetLink(BEffiTableName);
      vector<double> bpmt;
      vector<double> epsix,epsiy;
      bpmt=BEffiTable->GetDArray("b");
      epsix=BEffiTable->GetDArray("deltax");
      epsiy=BEffiTable->GetDArray("deltay");
      G4PhysicsOrderedFreeVector* QBepsix;G4PhysicsOrderedFreeVector* QBepsiy;
      QBepsix=new G4PhysicsOrderedFreeVector();QBepsiy=new G4PhysicsOrderedFreeVector();
      for(int i=0;i<int(bpmt.size());i++){
          QBepsix->InsertValues(bpmt[i],epsix[i]);
          QBepsiy->InsertValues(bpmt[i],epsiy[i]);
      }
      Bepsix.push_back(*QBepsix);
      Bepsiy.push_back(*QBepsiy);
      //no. of datasheets transcribed in the db table of the same PMT model
      int nsheets=0;
      try{nsheets=BEffiTable->GetI("nsheets");}
      catch (DBNotFoundError &e){}
      if(nsheets>1){
        char c[12];
        string name;
        for(int is=1;is<nsheets;is++){
          sprintf(c,"%i",is);
          name=string("deltax")+c;
          epsix=BEffiTable->GetDArray(name);
          name=string("deltay")+c;
          epsiy=BEffiTable->GetDArray(name);
          delete QBepsix;
          delete QBepsiy;
          QBepsix=new G4PhysicsOrderedFreeVector();
          QBepsiy=new G4PhysicsOrderedFreeVector();
          for(int i=0;i<int(bpmt.size());i++){
            QBepsix->InsertValues(bpmt[i],epsix[i]);
            QBepsiy->InsertValues(bpmt[i],epsiy[i]);
          }
          Bepsix.push_back(*QBepsix);
          Bepsiy.push_back(*QBepsiy);
        }
      }

  //try to load PMT orientation table from file
      dynorfilename=string(getenv("GLG4DATA"))+"/"+ExpSubdir+"/"+dynorfilename;
      ifstream dynorfile(dynorfilename.data());
      if(!dynorfile.is_open()){
        cout<<"Failed to open "<<dynorfilename.data()<<", will assume random dynode orientations\n";
        dynorfile.close();
      }
      else{
        getline(dynorfile,header);
        double xd,yd,zd,dynox,dynoy,dynoz;
        G4ThreeVector dor;
        while(!dynorfile.rdstate()){
          dynorfile>>xd>>yd>>zd>>dynox>>dynoy>>dynoz;
          posi.set(xd,yd,zd);
          dor.set(dynox,dynoy,dynoz);
          Dpos.push_back(posi);
          Dorie.push_back(dor);
        }
      }
      Bdata.close();
      dynorfile.close();
      if(!Dorie.empty() && Dorie.size()==Dpos.size())
        HaveDynoData=true;
      else
        cout<<"No dynode orientation datafile or error in the data, randomizing dynode orientations\n";
    }
    try{BEffiModel=DB::Get()->GetLink("BField")->GetS("b_efficiency_model");}
    catch(DBNotFoundError &e){}
    cout<<"\nSelected "<<BEffiModel.data()<<" B Efficiency Model\n";
  }
  else
    BEffiTable=NULL;
  
  //PMTINFO is always in global coordinates - so calculate the local offset first
  G4ThreeVector offset = G4ThreeVector(0.0,0.0,0.0);
  for (string parent_name = mother_name; parent_name != ""; ) {
     G4VPhysicalVolume *parent_phys = FindPhysMother(parent_name);
     offset += parent_phys->GetFrameTranslation();
     DBLinkPtr parent_table = DB::Get()->GetLink("GEO",parent_name);
     parent_name = parent_table->GetS("mother");
  }
  
  // Place physical PMTs
  // idx - the element of the particular set of arrays we are reading
  // id - the nth pmt that GeoPMTFactoryBase has built
  for (int idx = start_idx, id = pmtinfo.GetPMTCount(); idx <= end_idx; idx++, id++) {
  
    string pmtname = volume_name + ::to_string(id); //internally PMTs are represented by the nth pmt built, not pmtid
    
    // position
    G4ThreeVector pmtpos(pmt_x[idx], pmt_y[idx], pmt_z[idx]);
    pmtpos += offset;
    if (rescale_radius)
      pmtpos.setMag(new_radius);

    // direction
    G4ThreeVector pmtdir;
    if (orient_manual)
      pmtdir.set(dir_x[idx], dir_y[idx], dir_z[idx]);
    else
      pmtdir = orient_point - pmtpos;
    pmtdir = pmtdir.unit();
    if (flip == 1) pmtdir = -pmtdir; 

    // Write the real (perhaps calculated) PMT positions and directions.
    // This goes into the DS by way of Gsim
    pmtinfo.AddPMT(
        TVector3(pmtpos.x(), pmtpos.y(), pmtpos.z()),
        TVector3(pmtdir.x(), pmtdir.y(), pmtdir.z()),
        pmt_type[idx],
        pmt_model);

    // if requested, generates the magnetic efficiency corrections as the PMTs are created
    if(BFieldOn){
      //finds the point of the B grid closest to the current PMT, and attributes it that Bfield
      double MinDist=DBL_MAX;
      int imin=-1;
      for(int i=0;i<int(Bpos.size());i++){
        if(MinDist>(pmtpos-Bpos[i]).mag()){
          MinDist=(pmtpos-Bpos[i]).mag();
          imin=i;
        }
      }
      if(imin<0)
        cout<<"can't find a point close to the "<<id<<"-th pmt; MinDist is "<<MinDist<<"\n";
      else{
        G4ThreeVector bfield=Bf[imin].perpPart(pmtdir);
        G4ThreeVector dynorient;
        if(HaveDynoData){
          int mini=-1;
          double MinDiff=DBL_MAX;
          for(int i=0;i<int(Dorie.size());i++)
            if((pmtpos.unit()-Dpos[i].unit()).mag()<MinDiff){
              MinDiff=(pmtpos.unit()-Dpos[i].unit()).mag();
              mini=i;
            }
          if(mini<0){
            cout<<"can't find the orientation of the "<<id<<"-th pmt's dynode; MinDiff is "<<MinDiff<<"\n"
              <<"Throwing a random dynode orientation\n";
            dynorient=G4RandomDirection();
            dynorient=dynorient.perpPart(pmtdir);
          }
          else
            dynorient=Dorie[mini];
        }
        else{
//random dynode orientation
          dynorient=G4RandomDirection();
      //dynode orthogonal to PMT axis
          dynorient=dynorient.perpPart(pmtdir);
        }
        if(dynorient.mag()==0){
          for(int di=0;di<100 && dynorient.mag()==0;di++){
            dynorient=G4RandomDirection();
            dynorient=dynorient.perpPart(pmtdir);
          }
          if(dynorient.mag()==0)
            cout<<"Warning: tried 100 times to generate a random dynode orientation for "<<id<<"-th PMT and failed. dynorient "<<dynorient(0)<<","<<dynorient(1)<<","<<dynorient(2)<<"\n";
        }
        dynorient=dynorient.unit();
    //build BEfficiencyCorrection table. PMT x axis is dynorient
        bool isOutRange;//just a flag, not really used in the G4 code
        double BeffiComp;
        int sheetno=0;
        if(Bepsix.size()>1){
          double chooser=G4UniformRand();
          if(chooser<1./3) sheetno=0;
          else if(chooser<2./3) sheetno=1;
          else sheetno=2;
        }
        if(BEffiModel=="multiplicative"){
          BeffiComp=Bepsix[sheetno].GetValue(bfield*dynorient,isOutRange)*Bepsiy[sheetno].GetValue(bfield*(pmtdir.cross(dynorient)).unit(),isOutRange);
          if(CorrBEpsiInput && BeffiComp>1)
            BEfficiencyCorrection.push_back(pair<int,double>(id,1));
          else
            BEfficiencyCorrection.push_back(pair<int,double>(id,BeffiComp));
        }
        else
          if(BEffiModel=="additive"){
            BeffiComp=Bepsix[sheetno].GetValue(bfield*dynorient,isOutRange)+Bepsiy[sheetno].GetValue(bfield*(pmtdir.cross(dynorient)).unit(),isOutRange)-1.;
          if(CorrBEpsiInput && BeffiComp>1)
            BEfficiencyCorrection.push_back(pair<int,double>(id,1));
          else
            BEfficiencyCorrection.push_back(pair<int,double>(id,BeffiComp));
          }
          else
            cout<<"\nError: undefined B Efficiency Model\n";
      }
    }
    
    // rotation required to point in direction of pmtdir
    double angle_y = (-1.0)*atan2(pmtdir.x(), pmtdir.z());
    double angle_x = atan2(pmtdir.y(),
			   sqrt(pmtdir.x()*pmtdir.x()+pmtdir.z()*pmtdir.z()));
    
    G4RotationMatrix* pmtrot = new G4RotationMatrix();
    pmtrot->rotateY(angle_y);
    pmtrot->rotateX(angle_x);
    // ****************************************************************
    // * Use the constructor that specifies the PHYSICAL mother, since
    // * each PMT occurs only once in one physical volume.  This saves
    // * the GeometryManager some work. -GHS.
    // ****************************************************************
    G4PVPlacement* thisPhysPMT = new G4PVPlacement(pmtrot,
                      pmtpos,
                      pmtname,
                      logiPMT,
                      phys_mother,
                      false,
                      id);
    if (!pmtParam.useEnvelope) {
      // If not using envelope volume, the PMT optical surfaces have NOT been set
      // and we must do so NOW.
      pmtConstruct.SetPMTOpticalSurfaces(thisPhysPMT,pmtname); 
    }
    
    if (!pmtParam.useEnvelope && logiWg) {
      // If not using envelope volume, the waveguide must be placed in a separate
      // operation
      
      // pmtrot is a passive rotation, but we need an active one to put offsetWg
      // into coordinates of mother
      G4ThreeVector offsetWg_rot = pmtrot->inverse()(offsetWg);
      G4ThreeVector waveguidepos = pmtpos + offsetWg_rot;
      new G4PVPlacement(
        pmtrot,               
        waveguidepos,      
        pmtname+"_waveguide", // a name for this physical volume
        logiWg,               // the logical volume
        phys_mother,          // the mother volume
        false,                // no boolean ops
        id);  // copy number
    }
    
  } // end loop over id

//finally pass the lookup table to GLG4PMTOpticalModel
  if(BFieldOn){
    const G4String modname(volume_name+"_optical_model");
//    G4cout<<"Setting B Efficiency Correction Table for PMT optical model "<<modname<<"\n";
    
    if(logiPMT->GetFastSimulationManager()->GetFastSimulationModelList()[0]->GetName()==modname)
      ((GLG4PMTOpticalModel *)logiPMT->GetFastSimulationManager()->GetFastSimulationModelList()[0])->SetBEfficiencyCorrection(BEfficiencyCorrection);
    else{
      for(int i=0;i<int(logiPMT->GetFastSimulationManager()->GetFastSimulationModelList().size());i++)
        if(logiPMT->GetFastSimulationManager()->GetFastSimulationModelList()[i]->GetName()==modname){
          ((GLG4PMTOpticalModel *)logiPMT->GetFastSimulationManager()->GetFastSimulationModelList()[i])->SetBEfficiencyCorrection(BEfficiencyCorrection);
          G4cout<<"trying to set B efficiency for "<<((GLG4PMTOpticalModel *)logiPMT->GetFastSimulationManager()->GetFastSimulationModelList()[i])->GetName()<<"\n";
          break;
        }
    }
/*    for(int i=0;i<int(logiPMT->GetFastSimulationManager()->GetFastSimulationModelList().size());i++)
      ((GLG4PMTOpticalModel *)logiPMT->GetFastSimulationManager()->GetFastSimulationModelList()[i])->DumpBEfficiencyCorrectionTable();*/
  }

  return 0; // There is no specific physical volume to return
}

} // namespace RAT
