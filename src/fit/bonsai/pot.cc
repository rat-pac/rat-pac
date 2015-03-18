#include <stdio.h>
#include "RAT/BONSAI/pot.h"

//#define FOURNEIGHBOR  60 // above: all PMTs must be neighbors in time
//#define THREENEIGHBOR 32 // above: three PMTs must be neighbors in time
//#define TWONEIGHBOR   26 // above: two PMTs must be neighbors in time
#define FOURNEIGHBOR  30 // above: all PMTs must be neighbors in time
#define THREENEIGHBOR 16 // above: three PMTs must be neighbors in time
#define TWONEIGHBOR   13 // above: two PMTs must be neighbors in time

namespace BONSAI {
// **********************************************
// try fit new vertices based on 4-hit combinations
// **********************************************
inline void pot::add4vert(short int fast,int *cab,double *ffit)
{
  int   nsol=event_hits->hitsel::vertex4(cab,ffit);

  if ((nsol==1) || (nsol==2))
    //new_vertex(ffit[0],ffit[1],ffit[2],ffit[3],2.6,4.4,rinit);
       new_vertex(fast,ffit[0],ffit[1],ffit[2],rinit);
  if (nsol==2)
    //  new_vertex(ffit[4],ffit[5],ffit[6],ffit[7],2.6,4.4,rinit);
  new_vertex(fast,ffit[4],ffit[5],ffit[6],rinit);
}

// **********************************************
// find best starting position by trying 4-hit
// combinations of selected hits or positions in front of PMTs
// **********************************************
void pot::search()
{
  int    cab[4];
  int    nind;
  double ffit[8];

  nind=event_hits->nselected();
  reset();
  if (nind>=4) {
    if (nind>FOURNEIGHBOR)
      { // all four PMTs must be temporal neighbors
	checksize(2*(nind-3)+nind);
	for(cab[0]=0; cab[0]<nind-3; cab[0]++)
	  {
	    cab[1]=cab[0]+1;
	    cab[2]=cab[0]+2;
	    cab[3]=cab[0]+3;
	    add4vert(1,cab,ffit);
	  }
      }
    else if(nind>THREENEIGHBOR)
      { // three PMTs must be temporal neighbors
	checksize(2*(nind-4)*(nind-2)+nind);
	for(cab[0]=0; cab[0]<nind-2; cab[0]++)
	  {
	    cab[1]=cab[0]+1;
	    cab[2]=cab[0]+2;
	    for(cab[3]=0; cab[3]<cab[0]-1; cab[3]++)
	      add4vert(1,cab,ffit);
	    for(cab[3]=cab[2]+1; cab[3]<nind; cab[3]++)
	      add4vert(1,cab,ffit);
	  }
      }
    else if (nind>TWONEIGHBOR)
      { // two PMTs must be temporal neighbors
	checksize((nind-4)*(nind-2)*(nind-1)+nind);
	for(cab[0]=0; cab[0]<nind-2; cab[0]++)
	  {
	    cab[1]=cab[0]+1;
	    for(cab[2]=0; cab[2]<cab[0]-1; cab[2]++)
	      {
		for(cab[3]=cab[2]+2; cab[3]<cab[0]-1; cab[3]++)
		  add4vert(1,cab,ffit);
		for(cab[3]=cab[0]+2; cab[3]<nind; cab[3]++)
		  add4vert(1,cab,ffit);
	      }
	    for(cab[2]=cab[0]+2; cab[2]<nind; cab[2]++)
	      for(cab[3]=cab[2]+1; cab[3]<nind; cab[3]++)
		add4vert(1,cab,ffit);
	  }
      }
    else
      { // use all combinations
	checksize(nind*(nind-1)*(nind-2)*(nind-3)/12+nind);
	for(cab[0]=0; cab[0]<nind-3; cab[0]++)
	  for(cab[1]=cab[0]+1; cab[1]<nind-2; cab[1]++)
	    for(cab[2]=cab[1]+1; cab[2]<nind-1; cab[2]++)
	      for(cab[3]=cab[2]+1; cab[3]<nind; cab[3]++)
		add4vert(1,cab,ffit);
      }
  }
  add_pmt_vertices(1,rpmt,rcent,top);
  return;
}
}
