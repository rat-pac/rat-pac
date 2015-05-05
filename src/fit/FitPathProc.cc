#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <TVector3.h>
#include <RAT/DS/PathFit.hh>
#include <RAT/DS/EV.hh>
#include <RAT/DS/PMT.hh>
#include <RAT/DS/Root.hh>
#include <RAT/DS/Run.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/Processor.hh>
#include <RAT/FitPathProc.hh>
#include <RAT/DB.hh>
#include <RAT/SimulatedAnnealing.hh>

#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnMinimize.h>
#include <Minuit2/MnSimplex.h>
#include <Minuit2/MnUserParameters.h>
#include <Minuit2/FunctionMinimum.h>

using namespace ROOT::Minuit2;

using namespace std;

namespace RAT {

FitPathProc::FitPathProc() : Processor("fitpath") { 
    DBLinkPtr ftp = DB::Get()->GetLink("FTP");
    
    fSeed = ftp->GetDArray("seed_pos");
    fSeed.resize(6);
    fSeed[3] = ftp->GetD("seed_theta");
    fSeed[4] = ftp->GetD("seed_phi");
    fSeed[5] = ftp->GetD("seed_time");
    
    fNumCycles = ftp->GetI("num_cycles");
    fNumEvals = ftp->GetI("num_evals");
    fPosSigma0 = ftp->GetD("pos_sigma0");
    fPosSigma1 = ftp->GetD("pos_sigma1");
    fThetaSigma = ftp->GetD("theta_sigma");
    fPhiSigma = ftp->GetD("phi_sigma");
    fTimeSigma0 = ftp->GetD("time_sigma0");
    fTimeSigma1 = ftp->GetD("time_sigma1");
    fTemp0 = ftp->GetD("temp0");
    fTemp1 = ftp->GetD("temp1");
    fAlpha = ftp->GetD("alpha");
    
    fMigrad = ftp->GetI("migrad") == 0 ? false : true;
    
    fCherenkovMultiplier = ftp->GetD("cherenkov_multiplier");
    fLightSpeed = ftp->GetD("light_speed");
    fDirectProb = ftp->GetD("direct_prob");
    fOtherProb = ftp->GetD("other_prob");
    fPhotocathodeArea = ftp->GetD("photocathode_area");
    
    fDirectTime0 = ftp->GetD("direct_time_first");
    fDirectTimeStep = ftp->GetD("direct_time_step");
    fDirectTimeProb = ftp->GetDArray("direct_time_prob");
    
    double integral = 0.0;
    for (size_t i = 0; i < fDirectTimeProb.size(); i++) {
        integral += fDirectTimeProb[i];
    }
    integral *= fDirectTimeStep;
    for (size_t i = 0; i < fDirectTimeProb.size(); i++) {
        fDirectTimeProb[i] /= integral;
    }
    
    fOtherTime0 = ftp->GetD("other_time_first");
    fOtherTimeStep = ftp->GetD("other_time_step");
    fOtherTimeProb = ftp->GetDArray("other_time_prob");
    
    integral = 0.0;
    for (size_t i = 0; i < fOtherTimeProb.size(); i++) {
        integral += fOtherTimeProb[i];
    }
    integral *= fOtherTimeStep;
    for (size_t i = 0; i < fOtherTimeProb.size(); i++) {
        fOtherTimeProb[i] /= integral;
    }
    
    fCosAlpha0 = ftp->GetD("cosalpha_first");
    fCosAlphaStep = ftp->GetD("cosalpha_step");
    fCosAlphaProb = ftp->GetDArray("cosalpha_prob");
    
    integral = 0.0;
    for (size_t i = 0; i < fCosAlphaProb.size(); i++) {
        integral += fCosAlphaProb[i];
    }
    integral *= fCosAlphaStep;
    for (size_t i = 0; i < fCosAlphaProb.size(); i++) {
        fCosAlphaProb[i] /= integral;
    }
}

///Arguments are event {position},{direction unit},time
///Excuse my intentional use of basic types and explicit variables - optomization in progress
double FitPathProc::FTPProbability(const double x, const double y, const double z, const double dx, const double dy, const double dz, const double t) const {
    const double c = fLightSpeed;
    const double prob_direct = fDirectProb;
    const double prob_other = fOtherProb;
    const size_t nHits = fHits.size();
    const double nCherenkov = nHits*fCherenkovMultiplier;
    const double sensitive_area = fPhotocathodeArea;
    
    double prob = 1.0;
    //cout << x << ' ' << y << ' ' << z << ' ' << dx << ' ' << dy << ' ' << dz << ' ' << t << ") = ";
    for (size_t i = 0; i < nHits; i++) {
        const hit &cur = fHits[i]; //constant reference for speed
        
        const double rx = cur.x-x, ry = cur.y-y, rz = cur.z-z; //vector r points from event to hit 
        const double dist = sqrt(rx*rx+ry*ry+rz*rz); //distance from event to hit
        const double nx = rx/dist, ny = ry/dist, nz = rz/dist; //vector n is the unit vector of r
        
        const double trel = cur.t - t; //time of hit relative to event time
        const double tresid = trel - dist/c; //hit time corrected by time of flight
        
        const double prob_directtime = PDFDirectTime(tresid); // probability of detecting direct light with this time residual
        const double prob_othertime = PDFOtherTime(tresid); // probability of detecting other light with this time residual
        
        //cout << '{' << prob_directtime << ',' << prob_othertime << "} ";
        
        const double cosalpha = nx*dx+ny*dy+nz*dz; // cosine of angle formed by hit-event-direction
        
        const double solidangle = -sensitive_area/(dist*dist)*(nx*cur.px+ny*cur.py+nz*cur.pz); //solid angle correction to PMT hit probability
        const double prob_directangle = nCherenkov*PDFCherenkovAngle(cosalpha)*solidangle/(2.0*M_PI); //probability of detecting direct light at this angle
        
        const double hitprob = prob_direct*prob_directtime*prob_directangle + prob_other*prob_othertime;
        if (hitprob > 1e-50) prob *= hitprob; else prob *= 1e-50;
    }
    //cout << prob << endl;
    return prob < 1e-200 ? 1e-200 : prob;
}

double FitPathProc::AvgSquareTimeResid(double x, double y, double z, double t) {
    const double c = fLightSpeed;
    const size_t nHits = fHits.size();
    
    double sum = 0.0;
    //if (t < -10) cout << '(' << x << ',' << y << ',' << z << ',' << t << ')';
    for (size_t i = 0; i < nHits; i++) {
        const hit &cur = fHits[i]; //constant reference for speed
        
        const double rx = cur.x-x, ry = cur.y-y, rz = cur.z-z; //vector r points from event to hit 
        const double dist = sqrt(rx*rx+ry*ry+rz*rz); //distance from event to hit
        
        const double trel = cur.t - t; //time of hit relative to event time
        const double tresid = trel - dist/c; //hit time corrected by time of flight
        
        sum += tresid*tresid;
    }
    //if (t < -10) cout << " = " << sum/nHits << endl;
    return sum/nHits;
}


double FitPathProc::operator()(const std::vector<double>& lParams ) const {
    const double costheta = cos(lParams[3]);
    const double sintheta = sqrt(1-costheta*costheta);
    return -log(FTPProbability(lParams[0],lParams[1],lParams[2],sintheta*cos(lParams[4]),sintheta*sin(lParams[4]),costheta,lParams[5]));
}


//x,y,z,costheta,phi,t
double FitPathProc::operator()(double *params) {
    switch (fStage) {
        case 0:
            return AvgSquareTimeResid(params[0],params[1],params[2],params[3]);
        case 1: {
            const double costheta = cos(params[3]);
            const double sintheta = sqrt(1-costheta*costheta);
            return -log(FTPProbability(params[0],params[1],params[2],sintheta*cos(params[4]),sintheta*sin(params[4]),costheta,params[5]));
        }
        default:
            cout << fStage << "!" << endl;
            return 0.0;
    }
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
    
    DS::PathFit* fit = ev->GetPathFit();
    
    vector<double> point,seed;
    
    fStage = 0;
    SimulatedAnnealing<4> stage0(this);
    
    seed.resize(4);
    seed[0] = fSeed[0];
    seed[1] = fSeed[1];
    seed[2] = fSeed[2];
    seed[3] = fSeed[5];
    
    point = seed;
    stage0.SetSimplexPoint(0,point);
    point = seed;
    point[0] += fPosSigma0;
    stage0.SetSimplexPoint(1,point);
    point = seed;
    point[1] += fPosSigma0;
    stage0.SetSimplexPoint(2,point);
    point = seed;
    point[2] += fPosSigma0;
    stage0.SetSimplexPoint(3,point);
    point = seed;
    point[3] += fTimeSigma0;
    stage0.SetSimplexPoint(4,point);
    
    stage0.Anneal(fTemp0,fNumCycles,fNumEvals,fAlpha);
    stage0.GetBestPoint(seed);
    
    TVector3 pos0(seed[0],seed[1],seed[2]);
    fit->SetPos0(pos0);
    fit->SetTime0(seed[3]);
    
    fStage = 1;
    SimulatedAnnealing<6> stage1(this);
    
    seed.resize(6);
    seed[5] = seed[3];
    seed[3] = fSeed[3];
    seed[4] = fSeed[4];
    
    point = seed;
    stage1.SetSimplexPoint(0,point);
    point = seed;
    point[0] += fPosSigma1;
    stage1.SetSimplexPoint(1,point);
    point = seed;
    point[1] += fPosSigma1;
    stage1.SetSimplexPoint(2,point);
    point = seed;
    point[2] += fPosSigma1;
    stage1.SetSimplexPoint(3,point);
    point = seed;
    point[3] += fThetaSigma;
    stage1.SetSimplexPoint(4,point);
    point = seed;
    point[4] += fPhiSigma;
    stage1.SetSimplexPoint(5,point);
    point = seed;
    point[5] += fTimeSigma1;
    stage1.SetSimplexPoint(6,point);
    
    stage1.Anneal(fTemp1,fNumCycles,fNumEvals,fAlpha);
    stage1.GetBestPoint(point);
    
    if (fMigrad) {
    
        seed = point;
        vector<double> errors(6);
        errors[0] = errors[1] = errors[2] = 1000.0;
        errors[3] = errors[4] = 2.0;
        errors[5] = 10.0; 
        
        gErrorIgnoreLevel = 1001;
        
        MnUserParameters mnParams = MnUserParameters(seed,errors);
        MnUserTransformation trafo;
        MinimumState minState(seed.size());
        MinimumSeed minSeed(minState, trafo);
        FunctionMinimum theMin(minSeed, 1.);
        MnMigrad migrad( *this, mnParams );
        
        theMin = migrad(0,0.1);
        
        MnUserParameters result = theMin.UserParameters();
        point = result.Params();
        
    }
    
    fFitPos = TVector3(point[0],point[1],point[2]);
    fFitTime = point[5];
    
    const double costheta = cos(point[3]);
    const double sintheta = sqrt(1-costheta*costheta);
    TVector3 dir(sintheta*cos(point[4]),sintheta*sin(point[4]),costheta);
    
    fit->SetPosition(fFitPos);
    fit->SetTime(fFitTime);
    fit->SetDirection(dir);

    return Processor::OK;
}

} // namespace RAT

