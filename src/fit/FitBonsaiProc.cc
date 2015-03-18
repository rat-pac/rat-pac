#include <RAT/FitBonsaiProc.hh>
#include <RAT/DS/BonsaiFit.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/DS/Run.hh>

#include <RAT/BONSAI/goodness.h>
#include <RAT/BONSAI/searchgrid.h>
#include <RAT/BONSAI/fourhitgrid.h>

using namespace RAT;
using namespace std;

FitBonsaiProc::FitBonsaiProc() : Processor::Processor("BONSAI"), bonsai_geometry(NULL), bonsai_likelihood(NULL), bonsai_fit(NULL) {

}

FitBonsaiProc::~FitBonsaiProc() {
    if (bonsai_geometry) delete bonsai_geometry;
    if (bonsai_likelihood) delete bonsai_likelihood;
    if (bonsai_fit) delete bonsai_fit;
}


Processor::Result FitBonsaiProc::Event(DS::Root *ds, DS::EV *ev) {

    DS::Run *run = DS::RunStore::Get()->GetRun(ds);
    DS::PMTInfo *pmtinfo = run->GetPMTInfo();
    
    if (!bonsai_geometry) {
        const size_t totalPMTs = pmtinfo->GetPMTCount();
        vector<float> packedPos;
        packedPos.reserve(totalPMTs*3);
        size_t bonsai_idx = 0;
	Double_t pmt_x_bonsai = 0.0;
        Double_t pmt_y_bonsai = 0.0;
        Double_t pmt_z_bonsai = 0.0;

        for (size_t i = 0; i < totalPMTs; i++) {
            if (pmtinfo->GetType(i) == 1) { //is normal PMT
                TVector3 pos = pmtinfo->GetPosition(i);
		pmt_x_bonsai = pos.X()/10.0;
 		pmt_y_bonsai = pos.Y()/10.0;
 		pmt_z_bonsai = pos.Z()/10.0;
                packedPos.push_back(pmt_x_bonsai);
                packedPos.push_back(pmt_y_bonsai);
                packedPos.push_back(pmt_z_bonsai);
                
                pmtmap[i] = bonsai_idx++;
            }
        }
        bonsai_geometry = new BONSAI::pmt_geometry(packedPos.size()/3,&packedPos[0]);
        bonsai_likelihood = new BONSAI::likelihood(bonsai_geometry->cylinder_radius(), bonsai_geometry->cylinder_height());
        goodness.resize(bonsai_likelihood->sets());
        bonsai_fit = new BONSAI::bonsaifit(bonsai_likelihood);
    } 
    
    const size_t nhit = ev->GetPMTCount(); 
    hit_time.resize(nhit);
    hit_charge.resize(nhit);
    hit_pmtid.resize(nhit);
    
    for (size_t i = 0; i < nhit; i++) {
        DS::PMT *pmt = ev->GetPMT(i);
        hit_time[i] = pmt->GetTime();
        hit_charge[i] = pmt->GetCharge();
        hit_pmtid[i] = pmtmap[pmt->GetID()];
    }
    
    /*
     * The following is based on the WCsim implementation of BONSAI. Vague comments
     * where comments might be appropriate.
     */
    
    //uses magic to select "good" hits
    BONSAI::goodness hitselection(bonsai_likelihood->sets(), bonsai_likelihood->chargebins(), bonsai_geometry, nhit, &hit_pmtid[0], &hit_time[0], &hit_charge[0]);
    
    //probably something to do with combanatorics 
    BONSAI::fourhitgrid grid(bonsai_geometry->cylinder_radius(), bonsai_geometry->cylinder_height(), &hitselection);
    
    //BONSAI::searchgrid grid(bonsai_geometry->cylinder_radius(), bonsai_geometry->cylinder_height(),0.0);
    
    //use the selected hits
    bonsai_likelihood->set_hits(&hitselection);
    
    //do some sort of fitting
    bonsai_likelihood->maximize(bonsai_fit,&grid);
    
    //we got a position fit result
    float vtx[4]; // (x,y,z,t)
    vtx[0] = bonsai_fit->xfit();
    vtx[1] = bonsai_fit->yfit();
    vtx[2] = bonsai_fit->zfit();
    
    //something internal to bonsai / unsure what this does
    bonsai_fit->fitresult(); 
    
    //now we have a time result
    vtx[3] = bonsai_likelihood->get_zero();
    
    //and a direction result
    float dir[6]; //why 6 / what is dirx,diry,dirz,?,?,ll0
    bonsai_likelihood->get_dir(dir);
    dir[5] = bonsai_likelihood->get_ll0(); //loglikelihood w/o angle constraint (?)
    
    //reset likelihood
    bonsai_likelihood->set_hits(NULL);
    
    /* This time fit won't work properly until calibrated
    
    //now fit the time
    bonsai_likelihood->set_hits(&hitselection);
    float dt; // called dt in bonsai / apparently related to loglikelihood of time fit
    bonsai_likelihood->fittime(0,vtx,dir,dt); //probably modifies vtx[3]
    
    //reset likelihood
    bonsai_likelihood->set_hits(NULL);
    
    */
    
    DS::BonsaiFit *result = ev->GetBonsaiFit();
    
    TVector3 pos(vtx[0]*10.0,vtx[1]*10.0,vtx[2]*10.0);
    result->SetPosition(pos);
    result->SetTime(vtx[3]);
    
    return OK;
    
}

