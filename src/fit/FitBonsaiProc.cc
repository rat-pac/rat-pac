#include <RAT/FitBonsaiProc.hh>
#include <RAT/DS/BonsaiFit.hh>
#include <RAT/DS/RunStore.hh>
#include <RAT/DS/Run.hh>

#include <BONSAI/goodness.h>
#include <BONSAI/searchgrid.h>
#include <BONSAI/fourhitgrid.h>

using namespace RAT;
using namespace std;
//using namespace BONSAI;

#include <stdio.h>
#include <stdlib.h>

//Need to seperate the Inner-Detector tubes from the Outer-Detector tubes
static const int innerPMTcode   = 1;
static const int vetoPMTcode    = 2;

//minHit should be 1 or above
static const int minHit         = 4;
static const int maxHit         = 1000;

static const double timeOffset  = 800.;
static const double darkRateHz = 1000.;

static const double mm2cm       = 0.1;
static const double cm2mm       = 10.0;

extern "C"
{
    void lfariadne_(float *av,int *anhit,float *apmt,float *adir,float *amsg,
                    float *aratio,int *anscat,float *acosscat);
}

int nwin(RAT::DS::PMTInfo *pmtinfo,
         float twin,float *v,int nfit,int *cfit,float *tfit,int *cwin);

FitBonsaiProc::FitBonsaiProc() : Processor::Processor("BONSAI"), bonsai_geometry(NULL), bonsai_likelihood(NULL), bonsai_fit(NULL) {
    
}

FitBonsaiProc::~FitBonsaiProc() {
    if (bonsai_geometry) delete bonsai_geometry;
    if (bonsai_likelihood) delete bonsai_likelihood;
    if (bonsai_fit) delete bonsai_fit;
}


Processor::Result FitBonsaiProc::Event(DS::Root *ds, DS::EV *ev)
{
    DS::Run *run = DS::RunStore::Get()->GetRun(ds);
    DS::PMTInfo *pmtinfo = run->GetPMTInfo();
    int         cables[maxHit],cables_win[maxHit];
    float       times[maxHit];
    float       charges[maxHit];
    
    // generate BONSAI geometry object, the likelihood and the maximizer
    if (!bonsai_geometry)
    {
        RAT::DS::PMTInfo *pmtinfo = run->GetPMTInfo();
        int   i,n;
        n=pmtinfo->GetPMTCount();
        float xyz[3*n+1];
        int   pmtCode[n];
        int totInner=0,totVeto=0;
        for(i=0; i<n; i++)
        {
            TVector3 pos=pmtinfo->GetPosition(i);
//            pmtCode[i] = pmtinfo->GetType(i);
            if(pmtinfo->GetType(i)==innerPMTcode){
                totInner+=1;
            }else if(pmtinfo->GetType(i)==vetoPMTcode){
                totVeto+=1;
            }else{
                printf("Issue with type : %d\n",pmtinfo->GetType(i));
            }
            xyz[3*i]=pos[0]*mm2cm;
            xyz[3*i+1]=pos[1]*mm2cm;
            xyz[3*i+2]=pos[2]*mm2cm;
            //printf("%d: %lf %lf %lf\n",i,xyz[3*i],xyz[3*i+1],xyz[3*i+2]);
        }
        printf("have %d PMTs for BONSAI (%d inner pmts + %d veto pmts)\n",n,totInner,totVeto);

        bonsai_geometry=new pmt_geometry(n,xyz);
        bonsai_likelihood=new likelihood(bonsai_geometry->cylinder_radius(),
                                         bonsai_geometry->cylinder_height());
        bonsai_fit=new bonsaifit(bonsai_likelihood);
    }
    
    int nhitMAX= ev->GetPMTCount(); // Includes Inner and Veto PMTs
    int nhit = 0, nhitVeto =0,count, id;
    float IDCharge = 0.0, ODCharge = 0.;
    for (count =0; count < nhitMAX; count++) {
        RAT::DS::PMT *pmt=ev->GetPMT(count);
        id = pmt->GetID();
        if ( pmtinfo->GetType(id)==innerPMTcode) {
            nhit+=1;
            IDCharge+=pmt->GetCharge();
        }else if(pmtinfo->GetType(id)==vetoPMTcode){
            nhitVeto+=1;
            ODCharge+=pmt->GetCharge();
        }else{
            printf("Something is wrong with PMT type (%d,%d)\n",count,pmtinfo->GetType(id));
        }
    }
//    printf("Event had (inner,veto) = (%4d [%4.1f pe], %4d [%4.1f pe],%4d) PMTs\n",nhit,IDCharge,nhitVeto,ODCharge,nhitMAX);
    

    
    if ((nhit>=minHit) && (nhit<maxHit))
    {
        int hit;
        goodness    *bonsai_hits;
        fourhitgrid *grid;
        count = 0;
        for(hit=0; hit<nhitMAX; hit++)
        {
            RAT::DS::PMT *pmt=ev->GetPMT(hit);
            id = pmt->GetID();
            if (pmtinfo->GetType(id)==innerPMTcode) { //Type 1 is inner PMTs, type 2 veto PMTs
                cables[count]=pmt->GetID()+1;
                times[count]=pmt->GetTime()+timeOffset;
                charges[count]=pmt->GetCharge();
                count+=1;
            }
        }
        if (count!=nhit) {
            printf("%d %d\n",count,nhit);
        }
        int npmt=pmtinfo->GetPMTCount();
        float darkrate=darkRateHz*npmt;
        float tmin=-500+timeOffset;
        float tmax=1000+timeOffset;
        int ndark,darkhit;
        ndark=rnd.Poisson((tmax-tmin)*1e-9*darkrate);
        /*printf("# pmts=%d, dark rate=%lf, tmin=%lf, tmax=%lf, expect %lf ndark=%d\n",
         npmt,darkrate,tmin,tmax,(tmax-tmin)*1e-9*darkrate,ndark);*/
        for(darkhit=0; darkhit<ndark; darkhit++)
        {
            int darkcable=(int) (npmt*rnd.Rndm()+1);
            float darkt=tmin+(tmax-tmin)*rnd.Rndm();
            //printf("dark hit %d %lf ",darkcable,darkt);
            for(hit=0; hit<nhit; hit++)
                if (cables[hit]==darkcable) break;
            if (hit==nhit)
            {
                //printf("add it to the hit list!\n");
                cables[hit]=darkcable;
                times[hit]=darkt;
                charges[hit]=1;
                nhit++;
            }
            else
            {
                //printf("dark hit %d %lf ",darkcable,darkt);
                //printf("already have hit %lf/%lf ",times[hit],charges[hit]);
                if (darkt<times[hit]) times[hit]=darkt;
                charges[hit]++;
                //printf("now %lf/%lf\n",times[hit],charges[hit]);
            }
        }
        bonsai_hits=new goodness(bonsai_likelihood->sets(),
                                 bonsai_likelihood->chargebins(),
                                 bonsai_geometry,nhit,cables,times,charges);
        grid=new fourhitgrid(bonsai_geometry->cylinder_radius(),
                             bonsai_geometry->cylinder_height(),
                             bonsai_hits);
        bonsai_likelihood->set_hits(bonsai_hits);
        bonsai_likelihood->maximize(bonsai_fit,grid);
        
        if ((nsel[2]=bonsai_likelihood->nfit())>0)
        {
            *bonsai_vtxfit=bonsai_fit->xfit();
            bonsai_vtxfit[1]=bonsai_fit->yfit();
            bonsai_vtxfit[2]=bonsai_fit->zfit();
            gdn=new float[bonsai_likelihood->sets()];
            goodn[1]=bonsai_likelihood->goodness(maxlike[1],bonsai_vtxfit,gdn);
            delete gdn;
            bonsai_likelihood->tgood(bonsai_vtxfit,0,goodn[0]);
            nsel[1]=bonsai_likelihood->nwind(bonsai_vtxfit,-3,6);
            *maxlike=bonsai_fit->maxq();
            bonsai_fit->fitresult();
            bonsai_vtxfit[3]=bonsai_likelihood->get_zero();
            bonsai_likelihood->get_dir(bonsai_dirfit);
            *maxlike=bonsai_likelihood->get_ll0();
            
            DS::BonsaiFit *result=ev->GetBonsaiFit();
            result->SetPosition(TVector3(bonsai_vtxfit[0]*cm2mm,
                                         bonsai_vtxfit[1]*cm2mm,
                                         bonsai_vtxfit[2]*cm2mm));
            result->SetTime(bonsai_vtxfit[3]-timeOffset);
            result->SetGoodness(goodn[0]);
            result->SetShapeGoodness(goodn[1]);
            result->SetLogLike0(maxlike[0]);
            result->SetLogLike(maxlike[1]);
            result->SetNfit(nsel[2]);
            result->SetNsel(nsel[0]);
            result->SetN9(nsel[1]);
            result->SetBonsaiDir(bonsai_dirfit);
            int cfit[maxHit],hit;
            float tfit[maxHit],qfit[maxHit],apmt[3*maxHit],av[4];
            float agoodn,aqual,cosscat;
            int   nscat;
            float adir[3];
            int n10=nwin(pmtinfo,
                         10,bonsai_vtxfit,nhit,cables,times,cables_win);
            for(hit=0; hit<n10; hit++)
            {
                TVector3 pos=pmtinfo->GetPosition(cables_win[hit]-1);
                apmt[3*hit]=pos.X()*mm2cm;
                apmt[3*hit+1]=pos.Y()*mm2cm;
                apmt[3*hit+2]=pos.Z()*mm2cm;
            }
            lfariadne_(bonsai_vtxfit,&n10,apmt,
                       adir,&agoodn,&aqual,&nscat,&cosscat);
            result->SetDirection(TVector3(adir[0],adir[1],adir[2]));
            result->SetDirGoodness(agoodn);
            
            
            result->SetIDHit(nhit);
            result->SetODHit(nhitVeto);
            result->SetIDCharge(IDCharge);
            result->SetODCharge(ODCharge);
            
        }
        
        bonsai_likelihood->set_hits(NULL);
        
        
        /*DS::BonsaiFit *result = ev->GetBonsaiFit();
         TVector3 pos(bonsai_fit->xfit()*cm2mm,
         bonsai_fit->yfit()*cm2mm,
         bonsai_fit->zfit()*cm2mm);
         result->SetPosition(pos);
         
         bonsai_fit->fitresult();
         result->SetTime(bonsai_likelihood->get_zero()-timeOffset.0);
         bonsai_likelihood->get_dir(bonsai_dirfit);
         TVector3 direct(bonsai_dirfit[0],bonsai_dirfit[1],bonsai_dirfit[2]);
         result->SetDirection(direct);
         result->SetLogLike(bonsai_likelihood->get_ll());
         result->SetLogLike0(bonsai_likelihood->get_ll0());
         
         //reset likelihood
         bonsai_likelihood->set_hits(NULL);*/
        delete grid;
        delete bonsai_hits;
    }
    else
    {
        DS::BonsaiFit *result = ev->GetBonsaiFit();
        
        TVector3 pos(-10000,-10000,-10000);
        result->SetPosition(pos);
        result->SetTime(-10000);
        result->SetLogLike(-100);
        result->SetLogLike0(-100);
        
        TVector3 direct(0,0,0);
        result->SetDirection(direct);
        result->SetDirGoodness(-1);
        result->SetGoodness(-1);

        
        result->SetIDHit(nhit);
        result->SetODHit(nhitVeto);
        result->SetIDCharge(IDCharge);
        result->SetODCharge(ODCharge);
    }
    return OK;
}

int nwin(RAT::DS::PMTInfo *pmtinfo,
         float twin,float *v,int nfit,int *cfit,float *tfit,int *cwin)
{
    if (nfit<=0) return(0);
    
    float ttof[nfit],tsort[nfit],dx,dy,dz;
    int   hit,nwin=0,nwindow,hstart_test,hstart,hstop;
    
    // calculate t-tof for each hit
    for(hit=0; hit<nfit; hit++)
    {
        TVector3 pos=pmtinfo->GetPosition(cfit[hit]-1);
        dx=pos.X()*mm2cm-v[0];
        dy=pos.Y()*mm2cm-v[1];
        dz=pos.Z()*mm2cm-v[2];
        tsort[hit]=ttof[hit]=tfit[hit]-sqrt(dx*dx+dy*dy+dz*dz)*CM_TO_NS;
    }
    sort(tsort,tsort+nfit);
    
    // find the largest number of hits in a time window <= twin
    nwindow=1;
    hstart_test=hstart=0;
    while(hstart_test<nfit-nwindow)
    {
        hstop=hstart_test+nwindow;
        while((hstop<nfit) && (tsort[hstop]-tsort[hstart_test]<=twin))
        {
            hstart=hstart_test;
            nwindow++;
            hstop++;
        }
        hstart_test++;
    }
    hstop=hstart+nwindow-1;
    for(hit=0; hit<nfit; hit++)
    {
        if (ttof[hit]<tsort[hstart]) continue;
        if (ttof[hit]>tsort[hstop]) continue;
        cwin[nwin++]=cfit[hit];
    }
    if (nwin!=nwindow) printf("nwin error %d!=%d\n",nwin,nwindow);
    return(nwindow);
}
