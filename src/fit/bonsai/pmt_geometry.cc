#include <stdlib.h>
#include "binfile.h"
#include "pmt_geometry.h"

// **********************************************
// * load PMT positions                         *
// **********************************************
void pmt_geometry::loadgeom(void)
{
  int     *sizes,*numbers;
  void    **starts;
  binfile bf("geom.bin",'r');

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

void pmt_geometry::set(void)
{
  int   i;
  float r2,z;

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
}
