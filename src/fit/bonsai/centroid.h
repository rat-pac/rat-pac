#ifndef CENTROID
#define CENTROID

#include "hitsel.h"

class centroid;

class axes
{
  double vectors[9];
 public:
  inline axes(void);
  inline axes(double theta,double phi,double alpha);
  inline void set(double theta,double phi,double alpha);
  inline void set(short int comp,short int vect,double value);
  inline void set(short int vect,double val1,double val2,double val3);
  inline double get(short int comp,short int vect);
  inline void get(double *val);
  inline void get(float *val);
  inline void get(double &theta,double &phi,double &alpha);
  inline void get(float &theta,float &phi,float &alpha);
  inline void get(double *val,short int comp);
  inline void get(float *val,short int comp);
  inline double getx(double *comp);
  inline float getx(float *comp);
  inline double gety(double *comp);
  inline float gety(float *comp);
  inline double getz(double *comp);
  inline float getz(float *comp);
  inline void rotate(short int vect,double si,double co);
  short int align(double *nor);
  inline short int align(centroid &c,double *vec);
  inline short int align(centroid &c,float *vec);
};

// **********************************************
// generate the center of gravity of an array of
// points (matrix[0] to matrix[2]) as well as the
// variation matrix (matrix[3] to matrix[8])
// **********************************************
class centroid
{
  double centr[9],*matrix;

  inline double pythag(double a,double b);
  inline int iszero(int sta);
  void tridiag(axes &ax);
  int planegivens(axes &ax,double shift);

 public:
  centroid(int n,float *points,float *weight);
  centroid(hitsel *h);
  inline double x(void);
  inline double y(void);
  inline double z(void);
  inline void centre(double *vec);
  inline void centre(float *vec);
  inline double xx(void);
  inline double yy(void);
  inline double zz(void);
  inline double trace(void);
  inline double xy(void);
  inline double yz(void);
  inline double xz(void);
  inline double rxy(void);
  inline double r(void);
  void eigen(axes &ax);
  inline void eigen(float *axes);
  inline void eigen(axes &ax,float *values);
  inline void eigen(float *axes,float *values);
};

#include "centroid.inline"
#endif
