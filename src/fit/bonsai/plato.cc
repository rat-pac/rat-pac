#include<stdio.h>
#include <math.h>
#include "BONSAI/plato.h"

double dodecahedron::rt5;
double dodecahedron::irt5;
double dodecahedron::sp;
double dodecahedron::sm;
double dodecahedron::rsp;
double dodecahedron::rsm;


dodecahedron::dodecahedron(void): plato()
{
  // initialize "golden" ratios
  rt5=sqrt(5);     irt5=1/rt5;
  sp=0.5*(1+irt5); sm=0.5*(1-irt5);
  rsp=sqrt(sp);    rsm=sqrt(sm);
  vec=new double[36];
  nvec=12;
  *vec=vec[1]=vec[18]=vec[19]=0; vec[20]=-(vec[2]=1);
  vec[5]=vec[8]=vec[11]=vec[14]=vec[17]=irt5;
  vec[23]=vec[26]=vec[29]=vec[32]=vec[35]=-irt5;
  vec[21]=-(vec[3]= 2*irt5); vec[22]=vec[4]=0;
  vec[24]=-(vec[6]= sm);  vec[25]=-(vec[7]=rsp);
  vec[9] =-(vec[27]=sp);  vec[28]=-(vec[10]=rsm);
  vec[12]=-(vec[30]=sp);  vec[13]=-(vec[31]=rsm);
  vec[33]=-(vec[15]=sm);  vec[34]=-(vec[16]=-rsp);
}

// *************************************************************
// * interpolate the point of highest goodness based on the    *
// * calculated values on the corners of a dodecahedron and    *
// * its center                                                *
// *************************************************************
void dodecahedron::interpolate(float *values,float *inter)
{
  float vap1=values[1]+values[7], vam1=values[1]-values[7];
  float vap2=values[2]+values[8], vam2=values[2]-values[8];
  float vap3=values[3]+values[9], vam3=values[3]-values[9];
  float vap4=values[4]+values[10],vam4=values[4]-values[10];
  float vap5=values[5]+values[11],vam5=values[5]-values[11];
  float vap6=values[6]+values[12],vam6=values[6]-values[12];

  // fit equation f(x)=xT*a*x-2xT*b+c to all thirteen points with a
  // chi^2 minimization
  float axx=0.125*(-vap1+3*vap2+0.5*(vap3+vap4+vap5+vap6
			             -rt5*(vap3-vap4-vap5+vap6)))-values[0];
  float axy=          0.125*rt5*(rsm*(vap3-vap6)-rsp*(vap4-vap5));
  float axz=0.25*vap2+0.125*rt5*( sm*(vap3+vap6)- sp*(vap4+vap5));
  float ayy=0.125*(-vap1-vap2+1.5*(vap3+vap4+vap5+vap6)
	                     +0.5*rt5*(vap3-vap4-vap5+vap6))-values[0];
  float ayz=0.125*rt5*(rsp*(vap3-vap6)+rsm*(vap4-vap5));
  float azz=0.5*vap1-values[0];
  float bx=-0.25*irt5*vam2-0.125*(sm*(vam3+vam6)-sp*(vam4+vam5));
  float by=               -0.125*(rsp*(vam3-vam6)+rsm*(vam4-vam5));
  float bz=-0.125*(vam1+irt5*(vam2+vam3+vam4+vam5+vam6));
  float copy,ayx=axy,azx=axz,azy=ayz;

  // then find the maximum of f(x) by setting derivative to zero:
  // 2*xT(max)*a-2b=0 or x(max)=a^-1*b; solve this with Gauss elimination
  inter[0]=inter[1]=inter[2]=inter[3]=0;
  // first column: find the equation with the largest first element
  if (fabs(axx)<fabs(ayx))
    {
      copy=axx; axx=ayx; ayx=copy;
      copy=axy; axy=ayy; ayy=copy;
      copy=axz; axz=ayz; ayz=copy;
      copy=bx; bx=by; by=copy;
    }
  if (fabs(axx)<fabs(azx))
    {
      copy=axx; axx=azx; azx=copy;
      copy=axy; axy=azy; azy=copy;
      copy=axz; axz=azz; azz=copy;
      copy=bx; bx=bz; bz=copy;
    }
  // first column: dividing the first row by axx
  if (fabs(axx)<=0) return;
  axx=1/axx;
  axy*=axx;
  axz*=axx;
  bx*=axx;
  // first column: mulitply first row by ayx, subtract it from second row
  ayy-=ayx*axy;
  ayz-=ayx*axz;
  by-=ayx*bx;
  // first column: mulitply first row by azx, subtract it from third row
  azy-=azx*axy;
  azz-=azx*axz;
  bz-=azx*bx;

  // second column: find the equation with the larger second element
  if (fabs(ayy)<fabs(azy))
    {
      copy=ayy; ayy=azy; azy=copy;
      copy=ayz; ayz=azz; azz=copy;
      copy=by; by=bz; bz=copy;
    }
  // second column: first divide whats left from second row with ayy
  if (fabs(ayy)<=0) return;
  ayz/=ayy;
  by/=ayy;
  // second column: mulitply second row by axy, subtract it from first row
  axz-=axy*ayz;
  bx-=axy*by;
  // second column: mulitply second row by azy, subtract it from third row
  azz-=azy*ayz;
  bz-=azy*by;
  
  // third column: first divide whats left from third row with azz
  if (fabs(azz)<=0) return;
  bz/=azz;
  inter[2]=bz;
  // third column: mulitply third row by axz, subtract it from first row
  inter[0]=bx-axz*bz;  
  // third column: mulitply third row by ayz, subtract it from second row
  inter[1]=by-ayz*bz;
  inter[3]=sqrt(inter[0]*inter[0]+inter[1]*inter[1]+inter[2]*inter[2]);
  if (inter[3]>1.5)
    {
      inter[3]=1.5/inter[3];
      inter[0]*=inter[3];
      inter[1]*=inter[3];
      inter[2]*=inter[3];
      inter[3]=1.5;
    }
}


icosahedron::icosahedron(void)
{
  double     costhdo=1/sqrt(5),sinthdo=2*costhdo;
  double     con1=1+costhdo,con2=1+2*costhdo;
  double     norico=1/sqrt(3*con2);
  dodecahedron dode;
  double     dodevec[36];
  int    loop,i,j,k,l;
  int    ind[10]={0,3,8,7,2,1,9,-5,-4,6};
  //int    ind[10]={0,1,4,5,2,3,8,-7,-9,6};

  vec=new double[60];
  nvec=20;
  dode.get_vectors(dodevec,                  // rotate dodecahedron
		   sinthdo*sqrt(con2/3)/con1, 
		   sinthdo/(sqrt(3)*con1),
		   sqrt(con2/3));
  // generate icosahedron out of the dodecahedron area vectors
  // which are the icosahedron corner vectors
  for(loop=0; loop<5; loop++)
    {
      i=3*ind[loop];
      j=3*(loop+1);
      if ((k=j+3)>15) k-=15;
      if ((l=j+27)>33) l-=15;
      vec[i]=norico*(dodevec[0]+dodevec[j]+dodevec[k]);
      vec[i+1]=norico*(dodevec[1]+dodevec[j+1]+dodevec[k+1]);
      vec[i+2]=norico*(dodevec[2]+dodevec[j+2]+dodevec[k+2]);
      i=3*ind[loop+5];
      if (i<0)
	{
	  i=-i;
	  vec[i]=-norico*(dodevec[l]+dodevec[j]+dodevec[k]);
	  vec[i+1]=-norico*(dodevec[l+1]+dodevec[j+1]+dodevec[k+1]);
	  vec[i+2]=-norico*(dodevec[l+2]+dodevec[j+2]+dodevec[k+2]);
	}
      else
	{
	  vec[i]=norico*(dodevec[l]+dodevec[j]+dodevec[k]);
	  vec[i+1]=norico*(dodevec[l+1]+dodevec[j+1]+dodevec[k+1]);
	  vec[i+2]=norico*(dodevec[l+2]+dodevec[j+2]+dodevec[k+2]);
	}
    }
  for(i=0; i<30; i++) vec[i+30]=-vec[i];
}
