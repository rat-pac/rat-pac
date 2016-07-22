#include <math.h>
#include <stdlib.h>
#include "RAT/BONSAI/fourhitgrid.h"
namespace BONSAI {

// *************************************************************
// * print last possible hit for each hit starting a combin.   *
// *************************************************************
inline void fourhitgrid::print_ranges(short int &hit,int &n3comb)
{
  // print out chosen time window and desired number of combinations
  printf("required time window: %8.2fns, %3d combinations\n",twin,ncombo);
  // loop through each hit that can start a combination
  for(hit=0; hit<nsel-3; hit++)
    {
      // if this hit starts only one the one `guaranteed' combination 
      // print an asterisk in front of this combination
      if (times[end[hit]]-times[hit]>twin)
	printf("*");
      else
	printf(" ");
      // print starting hit and the last hit where comb. are allowed
      printf("%3d-%3d ",hit+1,end[hit]+1);
      // print corresponding times and maximal time difference
      printf("(%8.2f-%8.2f): %8.2fns ",
	     times[hit],times[end[hit]],times[end[hit]]-times[hit]);
      // calculate and print out number of comb. with this starting hit
      n3comb=end[hit]-hit;
      printf("%3d hits, %3d combinations\n",n3comb+1,
	     n3comb*(n3comb-1)*(n3comb-2)/6);
    }
}

// *************************************************************
// * set last allowed hit for each starting hit using half     *
// * of the largest possible time window                       *
// *************************************************************
inline void fourhitgrid::set_half_range(short int &start,short int &stop,
				        int &n3comb)
{
  // set time window to 50% of the largest possible time window
  twin=0.5*(times[nsel-1]-times[0]);
  // loop through all hits, summing up the combinations
  for(ncombo=start=0,stop=3; start<nsel-3; start++)
    {
      // search for the last hit that still within the window
      for(; stop<nsel; stop++)
	if (times[stop]-times[start]>twin)
	  break;
      // store this hit; guarantee at least one
      // combination with this hit as starting hit
      if (stop>start+3)
	end[start]=stop-1;
      else
	end[start]=start+3;
      if (ncombo==2147483647) continue;
      // calculate number of combinations with this starting hit
      // and add it to the total number of combinations
      n3comb=end[start]-start;
      if (n3comb<1291) n3comb*=(n3comb-1)*(n3comb-2); else n3comb=2146687710;
      n3comb/=6;
      if (ncombo<2147483647-n3comb) ncombo+=n3comb;
      else ncombo=2147483647;
    }
}

// *************************************************************
// * calculate number of combinations if the window has changed*
// *************************************************************
inline void fourhitgrid::adjust_range(short int &hit,short int &not_expanded,
				      int &n3comb)
{
  // loop through all starting hits and sum up the number of combinations
  for(ncombo=hit=0; hit<nsel-3; hit++)
    {
      not_expanded=1;
      // search for an additional hit within the window
      while((end[hit]<nsel-1) && (times[end[hit]+1]-times[hit]<=twin))
	{
	  not_expanded=0;
	  end[hit]++;
	}
      // if there is no additional hit, test last allowed
      // hit and shrink the range, if necessary
      if (not_expanded)
	while((end[hit]>hit+3) && (times[end[hit]]-times[hit]>twin))
	  end[hit]--;
      // calculate number of combinations and add it to the total
      n3comb=end[hit]-hit;
      if (n3comb<1291) n3comb*=(n3comb-1)*(n3comb-2); else n3comb=2146687710;
      n3comb/=6;
      if (ncombo<2147483647-n3comb) ncombo+=n3comb;
      else { ncombo=2147483647; return; }
    }
}

// *************************************************************
// * define allowed ranges of hit numbers using absolute timing*
// *************************************************************
inline void fourhitgrid::find_ranges(short int nreq)
{
  float     tlow,thigh,bestwin;
  short int start,stop;
  int       n3comb,dev,mindev;

  // find maximum number of combinations; if less than required,
  // set ranges to include all combinations and return
  if (nsel<478)
    {
      ncombo=nsel;   n3comb=nsel-1;
      dev=nsel-2;   mindev=nsel-3;
      if (ncombo%4==0) ncombo/=4;
      else if (n3comb%4==0) n3comb/=4;
      else if (dev%4==0) dev/=4;
      else if (mindev%4==0) mindev/=4;
      if (ncombo%3==0) ncombo/=3;
      else if (n3comb%3==0) n3comb/=3;
      else if (dev%3==0) dev/=3;
      else if (mindev%3==0) mindev/=3;
      if (ncombo%2==0) ncombo/=2;
      else if (n3comb%2==0) n3comb/=2;
      else if (dev%2==0) dev/=2;
      else if (mindev%2==0) mindev/=2;
      ncombo*=n3comb;
      ncombo*=dev;   ncombo*=mindev;
      if (ncombo<nreq)
        {
	  for(start=0; start<nsel-3; start++)
	    end[start]=nsel-1;
	  return;
	}
    }
  else
    ncombo=2147483647;
  // set smallest and largest possible timing window,
  // test the middle point; adjust lower and upper bounds
  tlow=0;
  thigh=times[nsel-1]-times[0];
  set_half_range(start,stop,n3comb);
  mindev=abs(ncombo-nreq);
  bestwin=twin;
  if (nreq>ncombo) tlow=twin; else thigh=twin;
  while(thigh-tlow>0.1)
    {
      // for large deviations, cut the interval in halves
      // for small deviations, assume twin^3 behaviour and
      // do first order Taylor approximation
      if (mindev>0.5*nreq)
	twin=0.5*(tlow+thigh);
      else
        {
	  twin*=(float)(2*ncombo+nreq)/(3*ncombo);
	  if ((twin<tlow+0.05) || (twin>thigh-0.05))
	    twin=0.5*(tlow+thigh);
	}
      adjust_range(start,stop,n3comb);
      if (nreq==ncombo) return; // if exactly as wanted, return
      // adjust boundaries, check deviation
      if (nreq>ncombo) tlow=twin; else thigh=twin;
      dev=abs(ncombo-nreq);
      if (dev<mindev)
	{
	  mindev=dev;
	  bestwin=twin;
	}
    }
  if (twin!=bestwin) // adjust, if latest window leads not to smallest dev.
    {
      twin=bestwin;
      adjust_range(start,stop,n3comb);
    }
  //print_ranges(start,n3comb);
}

// *************************************************************
// * compute all four-hit combination within end[start] range  *
// *************************************************************
inline void fourhitgrid::fourcombo(hitsel *hits)
{
  int       cab[4];
  short int start,stop,sec,thir,four,nsol;
  double    testpoint[8];

  // loop over all starting hits;
  for(start=0; start<nsel-3; start++)
    {
      stop=end[start];
      cab[0]=hits->sel(start);
      // step through all allowed combinations for the other three
      for(sec=start+1; sec<=stop-2; sec++)
	{
	  cab[1]=hits->sel(sec);
	  for(thir=sec+1; thir<stop; thir++)
	    {
	      cab[2]=hits->sel(thir);
	      for(four=thir+1; four<=stop; four++)
		{
		  cab[3]=hits->sel(four);
		  // calculate the corresponding vertex and add to array
		  nsol=hits->hits::vertex4(cab,testpoint);
		  if (nsol>0) add_point(testpoint[0],testpoint[1],testpoint[2]);
		  if (nsol>1) add_point(testpoint[4],testpoint[5],testpoint[6]);
		}
	    }
	}
    }
}

// *************************************************************
// * compute all four-hit combination within end[start] range  *
// *************************************************************
inline void fourhitgrid::fourcombo(hitsel *event_hits,float tsig,
				   float cyl_radius,float cyl_height,float dwallmin,
				   float &goodn)
{
  int       *cab = new int[4+event_hits->ntot()];
  short int start,stop,sec,thir,four,nsol,nstore,h1,h2,h3,h4;
  int       gstart,gend;
  double    testpoint[8];
  float     tmin,tmax,t,gdn[2],gmax;

  gmax=4;
  for(nstore=0; nstore<event_hits->nselected(); nstore++)
    cab[4+nstore]=event_hits->sel(nstore);
  tmin=event_hits->hits::hittime(cab[6])-220-3*tsig;
  tmax=event_hits->hits::hittime(cab[nstore])+220+3*tsig;
  for(h1=0; h1<event_hits->ntot(); h1++)
    {
      t=event_hits->hits::hittime(h1);
      if ((t>=tmin) && (t<=tmax))
        {
          if (t>=event_hits->hits::hittime(cab[3+nstore]))
            {
              if (h1!=cab[3+nstore]) cab[4+nstore++]=h1;
              continue;
            }
          if (t<=event_hits->hits::hittime(cab[4]))
            {
              if (h1==cab[4]) continue;
              h2=0;
            }
          else
            {
              h3=0;
              h4=nstore-1;
              while(h4-h3>1)
                {
                  h2=(h3+h4)/2;
                  if (t==event_hits->hits::hittime(cab[4+h2]))
                    {
                      h3=h4=h2;
                      break;
                    }
                  if (t<event_hits->hits::hittime(cab[4+h2]))
                    h4=h2;
                  else
                    h3=h2;
                }
              if (t>event_hits->hits::hittime(cab[4+h3]))
                h2=h4;
              else
                h2=h3;
            }
          if (h1==cab[4+h2]) continue;
          for(h3=nstore; h3>h2; h3--) cab[4+h3]=cab[3+h3];
          cab[4+h2]=h1;
          nstore++;
        }
    }
  gstart=0;
  gend=nstore-1;
  // loop over all starting hits;
  for(start=0; start<nsel-3; start++)
    {
      stop=end[start];
      cab[0]=event_hits->sel(start);
      // step through all allowed combinations for the other three
      for(sec=start+1; sec<=stop-2; sec++)
	{
	  cab[1]=event_hits->sel(sec);
	  for(thir=sec+1; thir<stop; thir++)
	    {
	      cab[2]=event_hits->sel(thir);
	      for(four=thir+1; four<=stop; four++)
		{
		  cab[3]=event_hits->sel(four);
		  // calculate the corresponding vertex and add to array
		  if (gstart>=0)
		    {
		      nsol=event_hits->hits::vertex4(cab,cab+4,nstore,
						     gstart,gend,tsig,
						     cyl_radius,cyl_height,
						     dwallmin,testpoint,gdn);
		      
		      if (nsol>0)
			{
			  add_point(testpoint[0],testpoint[1],testpoint[2]);
			  if (*gdn>gmax) gmax=*gdn;
			}
		      if (nsol>1)
			{
			  add_point(testpoint[4],testpoint[5],testpoint[6]);
			  if (gdn[1]>gmax) gmax=gdn[1];
			}
		      if (gmax>=goodn) gstart=-1;
		    }
		  else
		    {
		      nsol=event_hits->hits::vertex4(cab,testpoint);
		      if (nsol>0) add_point(testpoint[0],testpoint[1],testpoint[2]);
		      if (nsol>1) add_point(testpoint[4],testpoint[5],testpoint[6]);
		    }
		}
	    }
	}
    }
  goodn=gmax;
  delete [] cab;
}

// *************************************************************
// * initialize grid using four-hit combinations               *
// *************************************************************
fourhitgrid::fourhitgrid(double r,double z,hitsel *hits):
  searchgrid(r,z,dwall4hit())
{
  short int hit,n3c;
  float     pos[3],rat;

  nsel=hits->nselected();
  if (nsel<4) 
    {
      nsel=0;
      return;
    }
  // define arrays for allowed hit combination ranges and
  // copy ordered (absolute) hit times in an array
  end=new short int[nsel-3];
  times=new float[nsel];
  for(hit=0; hit<nsel; hit++)
    times[hit]=hits->hittime(hits->sel(hit));
  // formula used for desired number of hits, if nsel>nthreshold
  n3c=nsel_allcombo()*(nsel_allcombo()-1)*(nsel_allcombo()-2)/6;
  rat=(nsel_allcombo()-3.)/(nsel-3.);
  rat*=rat;
  find_ranges((short int)(0.5+(nsel-3)*(1+rat*(0.25*n3c-1))));
  // generate space in searchgrid for up to twice the number of
  // combinations; add a testpoint in front of each hit
  expand_size(2*ncombo+nsel);
  for(hit=0; hit<nsel; hit++)
    {
       hits->frontof(pos,hits->sel(hit),z);
       add_point(pos[0],pos[1],pos[2]);
    }
  close(); // start a new set, so that those testpoints will not average
  fourcombo(hits); // calculate four-hit combinations
  sparsify(init_grid_constant());
  if(bongrid()<clusgrid())
    {
      sparsify(bongrid());
      sparsify(clusgrid());
    }
  else
    {
      sparsify(clusgrid());
      sparsify(bongrid());
    }
  return;
}

fourhitgrid::fourhitgrid(double r,double z,hitsel *hits,float tsig,float dwallmin,
			 float &goodn):
  searchgrid(r,z,dwall4hit())
{
  short int hit,n3c;
  float     pos[3],rat;

  nsel=hits->nselected();
  if (nsel<4) 
    {
      nsel=0;
      return;
    }
  // define arrays for allowed hit combination ranges and
  // copy ordered (absolute) hit times in an array
  end=new short int[nsel-3];
  times=new float[nsel];
  for(hit=0; hit<nsel; hit++)
    times[hit]=hits->hittime(hits->sel(hit));
  // formula used for desired number of hits, if nsel>nthreshold
  n3c=nsel_allcombo()*(nsel_allcombo()-1)*(nsel_allcombo()-2)/6;
  rat=(nsel_allcombo()-3.)/(nsel-3.);
  rat*=rat;
  find_ranges((short int)(0.5+(nsel-3)*(1+rat*(0.25*n3c-1))));
  // generate space in searchgrid for up to twice the number of
  // combinations; add a testpoint in front of each hit
  expand_size(2*ncombo+nsel);
  for(hit=0; hit<nsel; hit++)
    {
       hits->frontof(pos,hits->sel(hit),z);
       add_point(pos[0],pos[1],pos[2]);
    }
  close(); // start a new set, so that those testpoints will not average
 // calculate four-hit combinations
  fourcombo(hits,tsig,r,z,dwallmin,goodn);
  sparsify(init_grid_constant());
  if(bongrid()<clusgrid())
    {
      sparsify(bongrid());
      sparsify(clusgrid());
    }
  else
    {
      sparsify(clusgrid());
      sparsify(bongrid());
    }
  return;
}

fourhitgrid::fourhitgrid(void *buffer,double r,double z,hitsel *hits):
  searchgrid((float *)buffer)
{
  short int hit,n3c;
  float     pos[3],rat;

  nsel=hits->nselected();
  if (nsel<4) 
    {
      nsel=0;
      return;
    }
  // define arrays for allowed hit combination ranges and
  // copy ordered (absolute) hit times in an array
  end=new short int[nsel-3];
  times=new float[nsel];
  for(hit=0; hit<nsel; hit++)
    times[hit]=hits->hittime(hits->sel(hit));
  // formula used for desired number of hits, if nsel>nthreshold
  n3c=nsel_allcombo()*(nsel_allcombo()-1)*(nsel_allcombo()-2)/6;
  rat=(nsel_allcombo()-3.)/(nsel-3.);
  rat*=rat;

  expand_size(nsel);
  // add points in front of selected hits
  for(hit=0; hit<hits->nselected(); hit++)
    {
      hits->frontof(pos,hits->sel(hit),z);
      add_point(pos[0],pos[1],pos[2]);
    }
  // start a new set, so that those testpoints will not average
  close();
  if (add_point(buffer)>0) return; // add points from structure

  // unpacking from structure failed:
  // --------------------------------
  find_ranges((short int)(0.5+(nsel-3)*(1+rat*(0.25*n3c-1))));
  // generate space in searchgrid for up to twice the number of
  // combinations; add a testpoint in front of each hit
  expand_size(2*ncombo);
  fourcombo(hits); // calculate four-hit combinations
  sparsify(init_grid_constant());
  if(bongrid()<clusgrid())
    {
      sparsify(bongrid());
      sparsify(clusgrid());
    }
  else
    {
      sparsify(clusgrid());
      sparsify(bongrid());
    }
  return;
}
 
}
