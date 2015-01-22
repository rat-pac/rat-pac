#include <cmath>
#include <string>
#include <vector>
#include <TVector3.h>
#include <RAT/DS/Centroid.hh>
#include <RAT/DS/EV.hh>
#include <RAT/DS/PMT.hh>
#include <RAT/DS/Root.hh>
#include <RAT/DS/Run.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/Processor.hh>
#include <RAT/FitPathProc.hh>

#include <RAT/SimulatedAnnealing.hh>

using namespace std;

namespace RAT {

FitPathProc::FitPathProc() : Processor("fitpath") { 
    fCherenkovMultiplier = 2.0; // nCherenkov/nHits
    fLightSpeed = 300; // mm/ns 
    fDirectProb = 0.879; // fraction of direct detected light
    fOtherProb = 0.121; // fraction of late detected light
    fPhotocathodeArea = 72000.0; // area of photocathode mm^2
}

///Arguments are event {position},{direction unit},time
///Excuse my intentional use of basic types and explicit variables - optomization in progress
double FitPathProc::Probability(double x, double y, double z, double dx, double dy, double dz, double t) {
    const double c = fLightSpeed;
    const double prob_direct = fDirectProb;
    const double prob_other = fOtherProb;
    const size_t nHits = fHits.size();
    const double nCherenkov = nHits*fCherenkovMultiplier;
    const double sensitive_area = fPhotocathodeArea;
    
    double prob = 1.0;
    
    for (size_t i = 0; i < nHits; i++) {
        const hit &cur = fHits[i]; //constant reference for speed
        
        const double rx = dx-cur.x, ry = dy-cur.y, rz = dz-cur.z; //vector r points from event to hit 
        const double dist = sqrt(rx*rx+ry*ry+rz*rz); //distance from event to hit
        const double nx = rx/dist, ny = ry/dist, nz = rz/dist; //vector n is the unit vector of r
        
        const double trel = cur.t - t; //time of hit relative to event time
        const double tresid = trel - dist/c; //hit time corrected by time of flight
        
        const double prob_directtime = PDFDirectTime(tresid); // probability of detecting direct light with this time residual
        const double prob_othertime = PDFOtherTime(tresid); // probability of detecting other light with this time residual
        
        const double cosalpha = nx*dx+ny*dy+nz*dz; // cosine of angle formed by hit-event-direction
        
        const double solidangle = sensitive_area/(dist*dist)*(nx*cur.px+ny*cur.py+nz*cur.pz); //solid angle correction to PMT hit probability
        const double prob_directangle = nCherenkov*PDFCherenkovAngle(cosalpha)*solidangle/(2.0*M_PI); //probability of detecting direct light at this angle
        
        prob *= prob_direct*prob_directtime*prob_directangle + prob_other*prob_othertime;
    }
    
    return prob;
}

//x,y,z,costheta,phi,t
double FitPathProc::operator()(double *params) {
    const double sintheta = sqrt(1-params[3]*params[3]);
    return -log(Probability(params[0],params[1],params[2],sintheta*cos(params[4]),sintheta*sin(params[4]),params[3],params[5]));
}

Processor::Result FitPathProc::Event(DS::Root* ds, DS::EV* ev) {

    fHits.resize(ev->GetPMTCount());

    DS::Run* run = DS::RunStore::Get()->GetRun(ds);
    DS::PMTInfo* pmtinfo = run->GetPMTInfo();

    for (int i=0; i < ev->GetPMTCount(); i++) {
        DS::PMT *pmt = ev->GetPMT(i);
        TVector3 pmtpos = pmtinfo->GetPosition(pmt->GetID());
        TVector3 pmtdir = pmtinfo->GetDirection(pmt->GetID());
        fHits[i].x = pmtpos.X();
        fHits[i].y = pmtpos.Y();
        fHits[i].z = pmtpos.Z();
        fHits[i].px = pmtdir.X();
        fHits[i].py = pmtdir.Y();
        fHits[i].pz = pmtdir.Z();
        fHits[i].t = pmt->GetTime();
        
    }
    
    SimulatedAnnealing<6> anneal(this);
    vector<double> seed, point;
    
    point = seed;
    anneal.SetSimplexPoint(0,point);
    point = seed;
    point[0] += fPositionVar;
    anneal.SetSimplexPoint(1,point);
    point = seed;
    point[1] += fPositionVar;
    anneal.SetSimplexPoint(2,point);
    point = seed;
    point[2] += fPositionVar;
    anneal.SetSimplexPoint(3,point);
    point = seed;
    point[3] += fCosThetaVar;
    anneal.SetSimplexPoint(4,point);
    point = seed;
    point[4] += fPhiVar;
    anneal.SetSimplexPoint(5,point);
    point = seed;
    point[5] += fTimeVar;
    anneal.SetSimplexPoint(6,point);
    
    anneal.Anneal(fTemp0,fNumCycles,fNumEvals,fAlpha,fFTol);

    //FIXME: get and set result

    return Processor::OK;
}

} // namespace RAT

