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
  int         cables[1000],cables_win[1000];
  float       times[1000];
  float       charges[1000];
    
  // generate BONSAI geometry object, the likelihood and the maximizer
  if (!bonsai_geometry)
    {
      RAT::DS::PMTInfo *pmtinfo = run->GetPMTInfo();
      int   i,n;
      n=pmtinfo->GetPMTCount();
      float xyz[3*n+1];

      printf("have %d PMTs for BONSAI\n",n);
      for(i=0; i<n; i++)
	{
	  TVector3 pos=pmtinfo->GetPosition(i);
	  xyz[3*i]=pos[0]*0.1;
	  xyz[3*i+1]=pos[1]*0.1;
	  xyz[3*i+2]=pos[2]*0.1;
	  //printf("%d: %lf %lf %lf\n",i,xyz[3*i],xyz[3*i+1],xyz[3*i+2]);
	}
      bonsai_geometry=new pmt_geometry(n,xyz);
      bonsai_likelihood=new likelihood(bonsai_geometry->cylinder_radius(),
				       bonsai_geometry->cylinder_height());
      bonsai_fit=new bonsaifit(bonsai_likelihood);
    }
    
  int nhit=ev->GetPMTCount();

  if ((nhit>0) && (nhit<1000))
    {
      int hit;
      goodness    *bonsai_hits;
      fourhitgrid *grid;

      for(hit=0; hit<nhit; hit++)
	{
	  RAT::DS::PMT *pmt=ev->GetPMT(hit);
	  cables[hit]=pmt->GetID()+1;
	  times[hit]=pmt->GetTime()+800;
	  charges[hit]=pmt->GetCharge();
	}
      int npmt=pmtinfo->GetPMTCount();
      float darkrate=1000*npmt;
      float tmin=-500+800;
      float tmax=1000+800;
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
	  result->SetPosition(TVector3(bonsai_vtxfit[0]*10.0,
				       bonsai_vtxfit[1]*10.0,
				       bonsai_vtxfit[2]*10.0));
	  result->SetTime(bonsai_vtxfit[3]-800);
	  result->SetGoodness(goodn[0]);
	  result->SetShapeGoodness(goodn[1]);
	  result->SetLogLike0(maxlike[0]);
	  result->SetLogLike(maxlike[1]);
	  result->SetNfit(nsel[2]);
	  result->SetNsel(nsel[0]);
	  result->SetN9(nsel[1]);
	  result->SetBonsaiDir(bonsai_dirfit);	  
	  int cfit[1000],hit;
	  float tfit[1000],qfit[1000],apmt[3*1000],av[4];
	  float agoodn,aqual,cosscat;
	  int   nscat;
	  float adir[3];
	  int n10=nwin(pmtinfo,
		       10,bonsai_vtxfit,nhit,cables,times,cables_win);
	  for(hit=0; hit<n10; hit++)
	    {
	      TVector3 pos=pmtinfo->GetPosition(cables_win[hit]-1);
	      apmt[3*hit]=pos.X()*0.1;
	      apmt[3*hit+1]=pos.Y()*0.1;
	      apmt[3*hit+2]=pos.Z()*0.1;
	    }
	  lfariadne_(bonsai_vtxfit,&n10,apmt,
		     adir,&agoodn,&aqual,&nscat,&cosscat);
	  result->SetDirection(TVector3(adir[0],adir[1],adir[2]));
	  result->SetDirGoodness(agoodn);
	}

      bonsai_likelihood->set_hits(NULL);


  /*DS::BonsaiFit *result = ev->GetBonsaiFit();
      TVector3 pos(bonsai_fit->xfit()*10.0,
		   bonsai_fit->yfit()*10.0,
		   bonsai_fit->zfit()*10.0);
      result->SetPosition(pos);

      bonsai_fit->fitresult();
      result->SetTime(bonsai_likelihood->get_zero()-800.0);
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
      dx=pos.X()*0.1-v[0];
      dy=pos.Y()*0.1-v[1];
      dz=pos.Z()*0.1-v[2];
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
