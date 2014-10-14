#ifndef PMT_GEOMETRY
#define PMT_GEOMETRY
#include <math.h>

namespace BONSAI {

typedef struct
{
  float  pmt_position[14000][3];  /*RECALL:C array's 0-n, f77 1-n  */
  float  pmt_direction[14000][3];
  int    pmt_type [14000];
  double pmt_r[14000];
  double pmt_theta[14000];
  int    n_pmts;
} comtype;

class pmt_geometry
{
  int   npmt;
  float *pmts,rmax,zmax; // PMT positions, maximal r, |z|
  void loadgeom(void);
  void set(void);

 public:
  inline pmt_geometry(void)
    {
      loadgeom();
      set();
    };
  inline pmt_geometry(int np,float *p)
    {
      pmts=new float[3*np];
      for(npmt=0; npmt<np; npmt++)
	{
	  pmts[3*npmt]=p[3*npmt];
	  pmts[3*npmt+1]=p[3*npmt+1];
	  pmts[3*npmt+2]=p[3*npmt+2];
	}
      set();
    };
  inline pmt_geometry(comtype *itgeom)
    {
      pmts=new float[3*itgeom->n_pmts];
      for(npmt=0; npmt<itgeom->n_pmts; npmt++)
	{
	  pmts[3*npmt]=itgeom->pmt_position[npmt][0];
	  pmts[3*npmt+1]=itgeom->pmt_position[npmt][1];
	  pmts[3*npmt+2]=itgeom->pmt_position[npmt][2];
	}
      set();
    };
  inline ~pmt_geometry(void)
    {
      delete(pmts);
      npmt=0;
    }
  inline int npmts(void)
    {
      return(npmt);
    }
  inline void position(float *pos,int index)
    {
      index*=3;
      *pos=pmts[index++];
      pos[1]=pmts[index++];
      pos[2]=pmts[index];
    }
  inline void position(float &px,float &py,float &pz,int index)
    {
      index*=3;
      px=pmts[index++];
      py=pmts[index++];
      pz=pmts[index];
    }
  inline float cylinder_radius(void)
    {
      return(rmax);
    }
  inline float cylinder_height(void)
    {
      return(zmax);
    }
  inline float cylinder_diagonal(void)
    {
      return(2*sqrt(rmax*rmax+zmax*zmax));
    }
  inline float cylinder_circonference(void)
    {
      return(2*(rmax+zmax));
    }
};
}
#endif
