#include <RAT/WatchmanDetectorFactory.hh>
#include <RAT/Log.hh>
#include <RAT/DB.hh>

#include <math.h>
#include <vector>

using namespace std;

namespace RAT {

    void WatchmanDetectorFactory::DefineDetector(DBLinkPtr detector) {
        const double photocathode_coverage = detector->GetD("photocathode_coverage");
        const double veto_coverage = detector->GetD("veto_coverage");
        const double veto_offset = 700;
        const std::string geo_template = "Watchman/Watchman.geo";
        DB *db = DB::Get();
        if (db->Load(geo_template) == 0) {
            Log::Die("WatchmanDetectorFactory: could not load template Watchman/Watchman.geo");
        }
        
        //calculate the area of the defined inner_pmts
        DBLinkPtr inner_pmts = db->GetLink("GEO","inner_pmts");
        string pmt_model = inner_pmts->GetS("pmt_model");
        DBLinkPtr pmt = db->GetLink("PMT", pmt_model);
        vector<double> rho_edge = pmt->GetDArray("rho_edge");
        double photocathode_radius = rho_edge[0];
        for (size_t i = 1; i < rho_edge.size(); i++) {
            if (photocathode_radius < rho_edge[i]) photocathode_radius = rho_edge[i];
        }
        const double photocathode_area = M_PI*photocathode_radius*photocathode_radius;
        
        DBLinkPtr shield = db->GetLink("GEO","shield");
        const double steel_thickness = shield->GetD("steel_thickness");
        const double shield_thickness = shield->GetD("shield_thickness");
        const double detector_size = shield->GetD("detector_size");
        
        const double cable_radius = detector_size/2.0 - shield_thickness + 4.0*steel_thickness;
        const double pmt_radius = detector_size/2.0 - shield_thickness - 4.0*steel_thickness;
        const double veto_radius = pmt_radius + veto_offset;
        
        const double topbot_offset = detector_size/2.0 - shield_thickness;
        const double topbot_veto_offset = topbot_offset + veto_offset;
        
        const double surface_area = 2.0*M_PI*pmt_radius*pmt_radius + 2.0*topbot_offset*2.0*M_PI*pmt_radius;
        const double required_pmts = ceil(photocathode_coverage * surface_area / photocathode_area);
        const double veto_surface_area = 2.0*M_PI*veto_radius*veto_radius + 2.0*topbot_veto_offset*2.0*M_PI*veto_radius;
        const double required_vetos = ceil(veto_coverage * veto_surface_area / photocathode_area);
        
        const double pmt_space = sqrt(surface_area/required_pmts);
        const double veto_space = sqrt(veto_surface_area/required_vetos);
        
        const size_t cols = (size_t)round(2.0*M_PI*pmt_radius/pmt_space);
        const size_t rows = (size_t)round(2.0*topbot_offset/pmt_space);
        const size_t veto_cols = (size_t)round(2.0*M_PI*veto_radius/veto_space);
        const size_t veto_rows = (size_t)round(2.0*topbot_veto_offset/veto_space);
        
        info << "Generating new PMT positions for:\n";
        info << "\tdesired photocathode coverage " << photocathode_coverage << '\n';
        info << "\ttotal area " << surface_area << '\n';
        info << "\tphotocathode radius " << photocathode_radius << '\n';
        info << "\tphotocathode area " << photocathode_area << '\n';
        info << "\tdesired PMTs " << required_pmts << '\n';
        info << "\tPMT spacing " << pmt_space << '\n';
        
        //make the grid for top and bottom PMTs
        vector<pair<int,int> > topbot;
        vector<pair<int,int> > topbot_veto;
        const int rdim = (int)round(pmt_radius/pmt_space); 
        for (int i = -rdim; i <= rdim; i++) {
            for (int j = -rdim; j <= rdim; j++) {
                if (pmt_space*sqrt(i*i+j*j) <= pmt_radius-pmt_space/2.0) {
                    topbot.push_back(make_pair(i,j));
                }
                if (veto_space*sqrt(i*i+j*j) <= pmt_radius-pmt_space/2.0) { // pmt_* is not a mistake
                    topbot_veto.push_back(make_pair(i,j));
                }
            }
        }
        
        size_t num_pmts = (size_t)(cols*rows + 2*topbot.size());
        size_t num_vetos = (size_t)(veto_cols*veto_rows + 2*topbot_veto.size());
        size_t total_pmts = (size_t)(num_pmts + num_vetos);
        
        info << "Actual calculated values:\n"; 
        info << "\tactual photocathode coverage " << photocathode_area*num_pmts/surface_area << '\n';
        info << "\tgenerated PMTs " << num_pmts << '\n';
        info << "\tcols " << cols << '\n';
        info << "\trows " << rows << '\n';
        info << "\tgenerated Vetos " << num_vetos << '\n';
        info << "\tcols " << veto_cols << '\n';
        info << "\trows " << veto_rows << '\n';
        
        vector<double> x(total_pmts), y(total_pmts), z(total_pmts), dir_x(total_pmts), dir_y(total_pmts), dir_z(total_pmts);
        vector<int> type(total_pmts);
        
        //generate cylinder PMT positions
        for (size_t col = 0; col < cols; col++) {
            for (size_t row = 0; row < rows; row++) {
                const size_t idx = row + col*rows;
                const double phi = 2.0*M_PI*(col+0.5)/cols;
                
                x[idx] = pmt_radius*cos(phi);
                y[idx] = pmt_radius*sin(phi);
                z[idx] = row*2.0*topbot_offset/rows + pmt_space/2.0 - topbot_offset;
                
                dir_x[idx] = -cos(phi);
                dir_y[idx] = -sin(phi);
                dir_z[idx] = 0.0;
                
                type[idx] = 1;
            }
        }
        
        //generate topbot PMT positions
        for (size_t i = 0; i < topbot.size(); i++) {
            const size_t idx = rows*cols+i*2;
            
            //top = idx
            x[idx] = pmt_space*topbot[i].first;
            y[idx] = pmt_space*topbot[i].second;
            z[idx] = topbot_offset;
            
            dir_x[idx] = dir_y[idx] = 0.0;
            dir_z[idx] = -1.0;
            
            type[idx] = 1;
            
            //bot = idx+1
            x[idx+1] = pmt_space*topbot[i].first;
            y[idx+1] = pmt_space*topbot[i].second;
            z[idx+1] = -topbot_offset;
            
            dir_x[idx+1] = dir_y[idx] = 0.0;
            dir_z[idx+1] = 1.0;
            
            type[idx+1] = 1;
        }
        
        //generate cylinder Veto positions
        for (size_t col = 0; col < veto_cols; col++) {
            for (size_t row = 0; row < veto_rows; row++) {
                const size_t idx = num_pmts + row + col*veto_rows;
                const double phi = 2.0*M_PI*col/veto_cols;
                
                x[idx] = veto_radius*cos(phi);
                y[idx] = veto_radius*sin(phi);
                z[idx] = row*2.0*topbot_offset/veto_rows + veto_space/2 - topbot_offset;
                
                dir_x[idx] = cos(phi);
                dir_y[idx] = sin(phi);
                dir_z[idx] = 0.0;
                
                type[idx] = 2;
            }
        }
        
        //generate topbot Veto positions
        for (size_t i = 0; i < topbot_veto.size(); i++) {
            const size_t idx = num_pmts + veto_rows*veto_cols+i*2;
            
            //top = idx
            x[idx] = veto_space*topbot_veto[i].first;
            y[idx] = veto_space*topbot_veto[i].second;
            z[idx] = topbot_veto_offset;
            
            dir_x[idx] = dir_y[idx] = 0.0;
            dir_z[idx] = 1.0;
            
            type[idx] = 2;
            
            //bot = idx+1
            x[idx+1] = veto_space*topbot_veto[i].first;
            y[idx+1] = veto_space*topbot_veto[i].second;
            z[idx+1] = -topbot_veto_offset;
            
            dir_x[idx+1] = dir_y[idx] = 0.0;
            dir_z[idx+1] = -1.0;
            
            type[idx+1] = 2;
        }
        
        //generate cable positions
        vector<double> cable_x(cols), cable_y(cols);
        for (size_t col = 0; col < cols; col++) {
            cable_x[col] = cable_radius*cos(col*2.0*M_PI/cols);
            cable_y[col] = cable_radius*sin(col*2.0*M_PI/cols);
        }
        
        info << "Override default PMTINFO information...\n";
        db->SetDArray("PMTINFO","x",x);
        db->SetDArray("PMTINFO","y",y);
        db->SetDArray("PMTINFO","z",z);
        db->SetDArray("PMTINFO","dir_x",dir_x);
        db->SetDArray("PMTINFO","dir_y",dir_y);
        db->SetDArray("PMTINFO","dir_z",dir_z);
        db->SetIArray("PMTINFO","type",type);
        
        info << "Update geometry fields related to veto PMTs...\n";
        db->SetI("GEO","shield","veto_start",num_pmts);
        db->SetI("GEO","shield","veto_len",num_vetos);
        db->SetI("GEO","veto_pmts","start_idx",num_pmts);
        db->SetI("GEO","veto_pmts","end_idx",total_pmts-1);
        
        info << "Update geometry fields related to normal PMTs...\n";
        db->SetI("GEO","shield","cols",cols);
        db->SetI("GEO","shield","rows",rows);
        db->SetI("GEO","shield","inner_start",0);
        db->SetI("GEO","shield","inner_len",num_pmts);
        db->SetI("GEO","inner_pmts","start_idx",0);
        db->SetI("GEO","inner_pmts","end_idx",num_pmts-1);
        
        info << "Update cable positions to match shield...\n";
        db->SetDArray("cable_pos","x",cable_x);
        db->SetDArray("cable_pos","y",cable_y);
        db->SetDArray("cable_pos","z",vector<double>(cols,0.0));
        db->SetDArray("cable_pos","dir_x",vector<double>(cols,0.0));
        db->SetDArray("cable_pos","dir_y",vector<double>(cols,0.0));
        db->SetDArray("cable_pos","dir_z",vector<double>(cols,1.0));
    }

}
