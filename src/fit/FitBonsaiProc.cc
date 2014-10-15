#include <RAT/FitBonsaiProc.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/DS/Run.hh>

using namespace RAT;
using namespace std;
using namespace BONSAI;

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
        for (size_t i = 0; i < totalPMTs; i++) {
            if (pmtinfo->GetType(i) == 1) { //is normal PMT
                TVector3 pos = pmtinfo->GetPosition(i);
                packedPos.push_back(pos.X());
                packedPos.push_back(pos.Y());
                packedPos.push_back(pos.Z());
                
                pmtmap[i] = bonsai_idx++;
            }
        }
        bonsai_geometry = new pmt_geometry(packedPos.size()/3,&packedPos[0]);
        bonsai_likelihood = new likelihood(bonsai_geometry->cylinder_radius(), bonsai_geometry->cylinder_height());
        goodness.resize(bonsai_likelihood->sets());
        bonsai_fit = new bonsaifit(bonsai_likelihood);
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
    
    //goodness hits(bonsai_likelihood->sets(), bonsai_likelihood->chargebins(), bonsai_geometry, nhit, &hit_pmtids[0], &hit_times[0], &hit_charge[0]);
    //fourhitgrid grid(bonsai_geometry->cylinder_radius(), bonsai_geometry->cylinder_height(), &hits);
    
    //bonsai_likelihood->set_hits(&hits);
    
    //actually fit
    
    return OK;
    
}

