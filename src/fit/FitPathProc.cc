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
        
        //fTimeSigma0=0.2;
        //fThetaSigma=0.01;
        
        //fTemp0=200.0;
        //fTemp1=200.0;
        //fNumEvals=400;
        //fNumCycles=200;
        
        //std::cout << "aaa fNumCycles is " << fNumCycles << ", fNumEvals is " << fNumEvals << ", fTemp0 is "<< fTemp0 << ", fTemp1 is " << fTemp1 << "\n";
        
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
        double exponent=0.0;
        double expTracker=0.0;
        //std::cout << "in FTPProbability prob_direct is " << prob_direct << ", prob_other is " << prob_other << ", nHits is " << nHits << ", nCherenkov is " << nCherenkov << ", sensitive_area is " << sensitive_area << "\n";
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
            
            //original
            const double solidangle = -sensitive_area/(dist*dist)*(nx*cur.px+ny*cur.py+nz*cur.pz); //solid angle correction to PMT hit probability
            //new
            //const double solidangle = sensitive_area/(dist*dist);
            
            const double prob_directangle = nCherenkov*PDFCherenkovAngle(cosalpha)*solidangle/(2.0*M_PI); //probability of detecting direct light at this angle
            //std::cout << "nCherenkov is " << nCherenkov << ", PDFCherenkovAngle(" << cosalpha << ") is " << PDFCherenkovAngle(cosalpha) << ", solidangle is " << solidangle << ", 2.0*M_PI is " << 2.0*M_PI << "\n";
            
            //original
            double hitprob = prob_direct*prob_directtime*prob_directangle + prob_other*prob_othertime;
            //new
            //double hitprob = 100.0*(prob_direct*prob_directtime*prob_directangle + prob_other*prob_othertime);
            //if (hitprob<0.00001) hitprob=0.00001;
            
            
            //std::cout <<"in FTPProbability prob_direct is " << prob_direct << ", prob_directtime is " << prob_directtime << ", prob_directangle is " << prob_directangle << ", prob_othertime is " << prob_othertime << ", prob_other is " << prob_other << ", hitprob is "<< hitprob << "\n";
            if (hitprob > 1e-50) prob *= hitprob; else prob *= 1e-50;
            exponent=log10(prob);
            //expTracker=expTracker+exponent;
            if (exponent<-100) {
                prob=1.0;
                expTracker=expTracker+exponent;
            }
            //std::cout <<"in FTPProbability hitprob is " << hitprob << ", combined prob is " << prob << ", expTracker is " << expTracker << "\n";
        }
        //std::cout <<"in FTPProbability return value is " << prob << "\n";
        //cout << prob << endl;
        //return prob < 1e-200 ? 1e-200 : prob;
        expTracker=expTracker+log10(prob);
        //std::cout << "in FTPProbability returning expTracker " << expTracker << "\n";
        return expTracker;
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
        //return -log(FTPProbability(lParams[0],lParams[1],lParams[2],sintheta*cos(lParams[4]),sintheta*sin(lParams[4]),costheta,lParams[5]));
        return -(FTPProbability(lParams[0],lParams[1],lParams[2],sintheta*cos(lParams[4]),sintheta*sin(lParams[4]),costheta,lParams[5]));
        
    }
    
    
    //x,y,z,costheta,phi,t
    double FitPathProc::operator()(double *params) {
        //std::cout << "in FitPathProc::operator, params[0] is " << params[0] << " params[1] is " << params[1] << " params[2] is " << params[2] << " params[3] is " << params[3] << " params[4] is " << params[4] << " params[5] is " << params[5] << "\n";
        switch (fStage) {
            case 0:
                return AvgSquareTimeResid(params[0],params[1],params[2],params[3]);
            case 1: {
                const double costheta = cos(params[3]);
                const double sintheta = sqrt(1-costheta*costheta);
                //std::cout << "costheta is "<< costheta << ", sintheta is " << sintheta << "\n";
                //return -log(FTPProbability(params[0],params[1],params[2],sintheta*cos(params[4]),sintheta*sin(params[4]),costheta,params[5]));
                return -(FTPProbability(params[0],params[1],params[2],sintheta*cos(params[4]),sintheta*sin(params[4]),costheta,params[5]));
            }
            default:
                cout << fStage << "!" << endl;
                return 0.0;
        }
    }
    
    Processor::Result FitPathProc::Event(DS::Root* ds, DS::EV* ev) {
        
        //  std::cout << "in Processor::Result FitPathProc::Event, num hits is " << ev->GetPMTCount() << "\n";
        
        fHits.resize(ev->GetPMTCount());
        //std::cout << "in Processor::Result FitPathProc::Event2\n";
        
        DS::Run* run = DS::RunStore::Get()->GetRun(ds);
        //std::cout << "in Processor::Result FitPathProc::Event3\n";
        DS::PMTInfo* pmtinfo = run->GetPMTInfo();
        //std::cout << "in Processor::Result FitPathProc::Event4, ev->GetPMTCount() is " << ev->GetPMTCount() << "\n";
        
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
        
        //std::cout << "in Processor::Result FitPathProc::Event5\n";
        
        DS::PathFit* fit = ev->GetPathFit();
        
        if (ev->GetPMTCount()==0) {
            fit->SetPosition(TVector3(-100000,-100000,-100000));
            fit->SetTime(-1);
            fit->SetDirection(TVector3(-1,0,0));
            return Processor::OK;
        }
        
        
        //std::cout << "in Processor::Result FitPathProc::Event6\n";
        vector<double> point,seed;
        
        //std::cout << "in Processor::Result FitPathProc::Event7\n";
        
        fStage = 0;
        SimulatedAnnealing<4> stage0(this);
        
        
        //std::cout << "in Processor::Result FitPathProc::Event8\n";
        
        seed.resize(4);
        seed[0] = fSeed[0];
        seed[1] = fSeed[1];
        seed[2] = fSeed[2];
        seed[3] = fSeed[5];
        
        //std::cout << "now setting up for stage 0 anneal, seed[0] is " << seed[0] << ", seed[1] is " << seed[1] << ", seed[2] is " << seed[2] << ", seed[3] is " << seed[3] << "\n";
        
        point = seed;
        stage0.SetSimplexPoint(0,point);
        //std::cout << "in Processor::Result FitPathProc::Event9.1\n";
        point = seed;
        point[0] += fPosSigma0;
        stage0.SetSimplexPoint(1,point);
        //std::cout << "in Processor::Result FitPathProc::Event9.2\n";
        point = seed;
        point[1] += fPosSigma0;
        stage0.SetSimplexPoint(2,point);
        //std::cout << "in Processor::Result FitPathProc::Event9.3\n";
        point = seed;
        point[2] += fPosSigma0;
        stage0.SetSimplexPoint(3,point);
        //std::cout << "in Processor::Result FitPathProc::Event9.4\n";
        point = seed;
        point[3] += fTimeSigma0;
        //    std::cout << "point[3] is " << point[3] << "\n";
        stage0.SetSimplexPoint(4,point);
        //std::cout << "in Processor::Result FitPathProc::Event9.5\n";
        //std::cout << "fTemp0 is " << fTemp0 << "\n";
        //std::cout << "fNumCycles is " << fNumCycles << "\n";
        //std::cout << "fNumEvals is " << fNumEvals << "\n";
        //std::cout << "fAlpha is " << fAlpha << "\n";
        //    std::cout << "now starting stage 0 Anneal\n";
        
        stage0.Anneal(fTemp0,fNumCycles,fNumEvals,fAlpha);
        //std::cout << "in Processor::Result FitPathProc::Event9.51\n";
        stage0.GetBestPoint(seed);
        //std::cout << "in Processor::Result FitPathProc::Event9.6\n";
        
        TVector3 pos0(seed[0],seed[1],seed[2]);
        fit->SetPos0(pos0);
        fit->SetTime0(seed[3]);
        //std::cout << "in Processor::Result FitPathProc::Event9.7\n";
        fStage = 1;
        SimulatedAnnealing<6> stage1(this);
        
        //std::cout << "in Processor::Result FitPathProc::Event10\n";
        //std::cout << "end of stage 0 seed[0] is " << seed[0] << ", seed[1] is " << seed[1] << ", seed[2] is " << seed[2] << ", seed[3] is " << seed[3] << "\n";
        seed.resize(6);
        //std::cout << "after resize seed[0] is " << seed[0] << ", seed[1] is " << seed[1] << ", seed[2] is " << seed[2] << ", seed[3] is " << seed[3] << ", seed[4] is " << seed[4] << ", seed[5] is " << seed[5] << "\n";
        seed[5] = seed[3];
        seed[3] = fSeed[3];
        seed[4] = fSeed[4];
        
        //std::cout << "now setting up for stage 1 anneal, seed[0] is " << seed[0] << ", seed[1] is " << seed[1] << ", seed[2] is " << seed[2] << ", seed[3] is " << seed[3] << ", seed[4] is " << seed[4] << ", seed[5] is " << seed[5] << "\n";
        
        point = seed;
        stage1.SetSimplexPoint(0,point);
        //std::cout << "stage1.SetSimplexPoint(0) point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        point = seed;
        point[0] += fPosSigma1;
        stage1.SetSimplexPoint(1,point);
        //std::cout << "stage1.SetSimplexPoint(1) point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        point = seed;
        point[1] += fPosSigma1;
        stage1.SetSimplexPoint(2,point);
        //std::cout << "stage1.SetSimplexPoint(2) point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        point = seed;
        point[2] += fPosSigma1;
        stage1.SetSimplexPoint(3,point);
        //std::cout << "stage1.SetSimplexPoint(3) point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        point = seed;
        //std::cout << "stage 1 anneal, after 4 SetSimplexPoint statements, seed[0] is " << seed[0] << ", seed[1] is " << seed[1] << ", seed[2] is " << seed[2] << ", seed[3] is " << seed[3] << ", seed[4] is " << seed[4] << ", seed[5] is " << seed[5] << "\n";
        point[3] += fThetaSigma;
        stage1.SetSimplexPoint(4,point);
        //std::cout << "stage1.SetSimplexPoint(4) point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        point = seed;
        point[4] += fPhiSigma;
        stage1.SetSimplexPoint(5,point);
        //std::cout << "stage1.SetSimplexPoint(5) point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        point = seed;
        point[5] += fTimeSigma1;
        stage1.SetSimplexPoint(6,point);
        //std::cout << "stage1.SetSimplexPoint(6) point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        //std::cout << "now starting stage 1 Anneal\n";
        stage1.Anneal(fTemp1,fNumCycles,fNumEvals,fAlpha);
        stage1.GetBestPoint(point);
        
        //std::cout << "after anneal best point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        
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
        
        //std::cout << "in Processor::Result FitPathProc::Event13\n";
        
        fFitPos = TVector3(point[0],point[1],point[2]);
        fFitTime = point[5];
        
        const double costheta = cos(point[3]);
        const double sintheta = sqrt(1-costheta*costheta);
        TVector3 dir(sintheta*cos(point[4]),sintheta*sin(point[4]),costheta);
        
        //std::cout << "after anneal and fMigrad setting the values of best point was " << point[0] << ", " << point[1] << ", " << point[2] << ", " << point[3] << ", " << point[4] << ", " << point[5] << "\n";
        
        fit->SetPosition(fFitPos);
        fit->SetTime(fFitTime);
        fit->SetDirection(dir);
        
        //std::cout << "in Processor::Result FitPathProc::Event14\n";
        
        return Processor::OK;
    }
    
} // namespace RAT

