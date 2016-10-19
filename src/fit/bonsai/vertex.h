#ifndef VERTEX
#define VERTEX
#include "timefit.h"
#include "plato.h"

// **********************************************
// manage an array of tested vertices
// **********************************************
class vertex: public timefit
{
  float        *vert,*bestfit,*worstfit;
  float        rpmt,zpmt,r2cut,zcut,cang0,plusdang,minusdang;
  float        dislimit,timlimit,twin,tres,tclose;
  int          nvert,maxvert;
  dodecahedron plat;

  inline int closest_dist2(float *v,float lim);
  inline void set(float x,float y,float z,float r);
  inline void set(float x,float y,float z,float t,float r);
  inline void set(float ll);
  inline float test(short int fast);
  inline float test(float tmi,float tpl);

 protected:
  float  timefactor;
         void loadhits(int n_raw,int *cable,int *bad_ch,
	              float *pmt_loc,float *tim_raw,float *chg_raw);
  inline void set_cylinder(float rp,float zp,float res);
  inline void set_hitsel(float dlim,float tlim,float tw,float tr,float tc);
  inline void checksize(int newsize);
  inline void addsize(int add);
  inline void new_vertex(short int fast,float x,float y,float z,float r);
  inline void new_vertex(float x,float y,float z,float t,float tmi,float tpl,float r);
  inline void add_pmt_vertices(short int fast,float rpmt,float rcent,float zmax);
  inline void reset(void);

 public:
  vertex(void);
  vertex(int n_raw,int *cable,int *bad_ch,float *pmt_loc,
	 float *tim_raw,float *chg_raw);
  ~vertex(void);

  inline void set_cang(float cang,float pang,float mang);
  inline int toobad(int index);
  inline int nsel();
  inline int ismax(int index);
  int surround(int index,int *&newi);
  float bestwall(float dwall,float rmin,float *wallv);

  inline void print(int index);
  inline int bestid(void);
  inline void bestvertex(float *v);
  inline void getvertex(float *v,int id);
  inline float bestradius();
  inline float maxll(void);
  inline float maxll(int id);
  inline float depth(void);
  inline int ntest(void);
  inline int max(void);
  inline int inside_cylinder(float x,float y,float z);
  inline void addvertex(float *v,float rad);
  void purge(void);
};
#include "vertex.inline"
#endif
