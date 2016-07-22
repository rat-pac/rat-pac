#include <stdio.h>
#include <stdlib.h>                                                
#include "RAT/BONSAI/vertexfit.h"
#include "RAT/BONSAI/binfile.h"

namespace BONSAI {

// hit selection parameter
#define TCLOSE      3.      // timing difference to pass unchecked
#define TRES        3.      // timing resolution

// **********************************************
// load PMT positions; decide fit volume and
// test point radii
// **********************************************
void vertexfit::loadgeom(void)
{
  int     *sizes,*numbers;
  void    **starts;
  binfile bf((char*)"geom.bin",'r');

  npmt=bf.read(sizes,numbers,starts);
  if (npmt!=1) 
    {
      printf("Invalid Geometry file: %d Arrays\n",npmt);
      exit(1);
    }
  if (sizes[0]!=4) 
    {
      printf("Invalid Geometry file: Array of size %d\n",sizes[0]);
      exit(1);
    }
  pmts=(float *)starts[0];
  delete(sizes);
  delete(starts);
  npmt=numbers[0]/3;
  delete(numbers);
}

void vertexfit::set(float resolution)
{
  int     i;
  float   r2,z;

  rmax=0;
  zmax=0;
  for(i=0; i<npmt; i++)
    {
      r2=pmts[3*i]*pmts[3*i]+pmts[3*i+1]*pmts[3*i+1];
      z=fabs(pmts[3*i+2]);
      if (rmax<r2) rmax=r2;
      if (zmax<z) zmax=z;
    }
  rmax=sqrt(rmax);
  printf("Loaded %d PMT locations, maximum r=%f, maximum z=%f\n",
	 npmt,rmax,zmax);
  set_cylinder(rmax,zmax,resolution);
  set_hitsel(0.357*(rmax+zmax),0.01*(rmax+zmax),
             0.09*sqrt(rmax*rmax+zmax*zmax),TRES,TCLOSE);
  set_radius(0.43*zmax,0.43*zmax,0.86*zmax,zmax);
}

vertexfit::vertexfit(float resolution)
{
  loadgeom();
  set(resolution);
}
vertexfit::vertexfit(int np,float *ps,float resolution)
{
  int i;

  npmt=np;
  pmts=new float[3*npmt];
  for(i=0; i<3*npmt; i++)
    pmts[i]=ps[i];
  set(resolution);
}

// **********************************************
// do everything!
// **********************************************
vertexfit::vertexfit(float resolution,
		     int n_raw,int *cable,int *bad_ch,
		     float *tim_raw,float *chg_raw)
{
  loadgeom();
  set(resolution);
  loadhits(n_raw,cable,bad_ch,tim_raw,chg_raw);
}

vertexfit::~vertexfit(void)
{
  delete(pmts);
}

}
