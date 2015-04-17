#include <RAT/GeoPMTParser.hh>

#include <string>
#include <G4Material.hh>
#include <G4SDManager.hh>
#include <RAT/GLG4PMTSD.hh>
#include <RAT/Materials.hh>
#include <RAT/GeoFactory.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <CLHEP/Units/SystemOfUnits.h>

using namespace std;

namespace RAT {

  GeoPMTParser::GeoPMTParser(DBLinkPtr table, bool useSD)
  {  
    string volume_name = table->GetIndex();

    // Find mother
    string mother_name = table->GetS("mother");
    G4LogicalVolume *mother = GeoFactory::FindMother(mother_name);
    if (mother == 0)
      Log::Die("Unable to find mother volume " + mother_name + " for "
	       + volume_name);

    //////////// Read PMT positions

    string pos_table_name = table->GetS("pos_table");
    DBLinkPtr lpos_table = DB::Get()->GetLink(pos_table_name);
    const vector<double> &pmt_x = lpos_table->GetDArray("x");
    const vector<double> &pmt_y = lpos_table->GetDArray("y");
    const vector<double> &pmt_z = lpos_table->GetDArray("z");
 
    // read max number of pmts to use
    int max_pmts = pmt_x.size(); // default to read all
    try { max_pmts = table->GetI("max_pmts"); }
    catch (DBNotFoundError &e) { }
    if (max_pmts > (int) pmt_x.size())
	max_pmts = pmt_x.size();
  
    // Optionally can rescale PMT radius from mother volume center for
    // case where PMTs have spherical layout symmetry
    bool rescale_radius = false;
    G4double new_radius = 1.0;
    try {
      new_radius = table->GetD("rescale_radius");
      rescale_radius = true;
    } catch (DBNotFoundError &e) { }

    // Load pmt positions as three vectors
    fPos.resize(max_pmts);
    for (int i = 0; i < max_pmts; i++) {
      fPos[i] = G4ThreeVector(pmt_x[i], pmt_y[i], pmt_z[i]);
      if (rescale_radius) fPos[i].setMag(new_radius);
    }

    //////////// Orientation of PMTs

    bool orient_manual = false;
    try { 
      string orient_str = table->GetS("orientation");
      if (orient_str == "manual")
	orient_manual = true;
      else if (orient_str == "point")
	orient_manual = false;
      else 
	Log::Die("GeoBuilder error: Unknown PMT orientation " 
		 + orient_str);
    } catch (DBNotFoundError &e) { }
    
    vector<double> orient_point_array;
    G4ThreeVector orient_point;

    fDir.resize(max_pmts);
    if (orient_manual) {
      const vector<double> &dirx = lpos_table->GetDArray("dir_x");
      const vector<double> &diry = lpos_table->GetDArray("dir_y");
      const vector<double> &dirz = lpos_table->GetDArray("dir_z");

      for (int i = 0; i < max_pmts; i++)
	fDir[i] = G4ThreeVector(dirx[i], diry[i], dirz[i]).unit();
    } else {
      // compute radially inward pointing 
      orient_point_array = table->GetDArray("orient_point");
      if (orient_point_array.size() != 3)
	Log::Die("GeoBuilder error: orient_point must have 3 values");
      orient_point.set(orient_point_array[0], orient_point_array[1],
		       orient_point_array[2]);

      for (int i = 0; i < max_pmts; i++)
	fDir[i] = (orient_point - fPos[i]).unit();
    }
  
    //////////// Read PMT dimensions
    string pmt_model = table->GetS("pmt_model");
    DBLinkPtr lpmt = DB::Get()->GetLink("PMT", pmt_model);
    
    fParam.faceGap = 0.1 * CLHEP::mm;
    fParam.zEdge = lpmt->GetDArray("z_edge");
    fParam.rhoEdge = lpmt->GetDArray("rho_edge");
    fParam.zOrigin = lpmt->GetDArray("z_origin");
    fParam.dynodeRadius = lpmt->GetD("dynode_radius");
    fParam.dynodeTop = lpmt->GetD("dynode_top");
    fParam.wallThickness = lpmt->GetD("wall_thickness");
    try {
      fParam.prepulseProb = lpmt->GetD("prepulse_prob");
    }
    catch (DBNotFoundError& e) {
      fParam.prepulseProb = 0;
    }

    //////////// PMT Materials
    fParam.exterior = mother->GetMaterial();
    fParam.glass = G4Material::GetMaterial(lpmt->GetS("glass_material"));
    fParam.dynode = G4Material::GetMaterial(lpmt->GetS("dynode_material"));
    fParam.vacuum = G4Material::GetMaterial(lpmt->GetS("pmt_vacuum_material"));
    string pc_surface_name = lpmt->GetS("photocathode_surface");
    fParam.photocathode = Materials::optical_surface[pc_surface_name];
    string mirror_surface_name = lpmt->GetS("mirror_surface");
    fParam.mirror = Materials::optical_surface[mirror_surface_name];
    string dynode_surface_name=lpmt->GetS("dynode_surface");
    fParam.dynode_surface=Materials::optical_surface[dynode_surface_name];
    
    // PMT sensitive detector
    if (useSD) {
      string sensitive_detector_name = table->GetS("sensitive_detector");
      G4SDManager* sdman = G4SDManager::GetSDMpointer();
      GLG4PMTSD* pmtSD = new GLG4PMTSD(sensitive_detector_name, max_pmts, 0, 10);
      sdman->AddNewDetector(pmtSD);
      fParam.detector = pmtSD;
    } else
      fParam.detector = 0;


    fParam.useEnvelope = false; // disable the use of envelope volume for now
    fConstruction = new PMTConstruction(fParam);
  }
   
  G4RotationMatrix GeoPMTParser::GetPMTRotation(int i) const
  {
    G4RotationMatrix rot;
    const G4ThreeVector &dir = fDir[i];

    double angle_y = (-1.0)*atan2(dir.x(), dir.z());
    double angle_x = atan2(dir.y(), sqrt(dir.x()*dir.x()+dir.z()*dir.z()));
    
    rot.rotateY(angle_y);
    rot.rotateX(angle_x);

    return rot;
  }

} // namespace RAT
