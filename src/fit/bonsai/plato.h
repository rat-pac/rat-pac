#include "RAT/BONSAI/centroid.h"
#ifndef PLATO
#define PLATO

namespace BONSAI {
    
    class plato
    {
    protected:
        double *vec;
        int    nvec;
        
    public:
        inline plato(void)
        { nvec=0; }
        inline ~plato(void)
        { if (nvec>0) delete vec; }
        inline int ndir(void)
        { return(nvec); }
        inline void get_vectors(double *v)
        {
            int i;
            
            for(i=0; i<3*nvec; i++)
                v[i]=vec[i];
        }
        inline void get_vectors(double *v,double n1,double n2,double n3)
        {
            double u[9];
            int    i,j;
            u[1]=sqrt(n1*n1+n3*n3);
            u[0]=-n1*n2/u[1];                         u[2]=-n3*n2/u[1];
            u[3]=n3/u[1];     u[4]=0;                 u[5]=-n1/u[1];
            u[6]=n1;          u[7]=n2;                u[8]=n3;
            printf("nvec=%d\n",nvec);
            for(i=0; i<3*nvec; i+=3)
            {
                for(v[i]=vec[i]*u[0],j=1; j<3; j++)
                    v[i]+=vec[i+j]*u[j];
                for(v[i+1]=vec[i]*u[3],j=1; j<3; j++)
                    v[i+1]+=vec[i+j]*u[3+j];
                for(v[i+2]=vec[i]*u[6],j=1; j<3; j++)
                    v[i+2]+=vec[i+j]*u[6+j];
            }
        }
        inline void move(float *v,int i,float r)
        {
            if (i<nvec)
            {
                i*=3;
                (*v++)+=r*vec[i++];
                (*v++)+=r*vec[i++];
                (*v)+=r*vec[i];
            }
        }
        inline void surround(float *v,float r)
        {
            int i;
            
            for(i=3*nvec-1; i>=3; i--)
            {
                v[i]=v[2]+r*vec[i]; i--;
                v[i]=v[1]+r*vec[i]; i--;
                v[i]=*v+r*vec[i];
            }
            *v+=*vec*r;
            v[1]+=r*vec[1];
            v[2]+=r*vec[2];
        }
        inline void surround(float *v,axes &ax,float r)
        {
            int i;
            
            for(i=3*nvec-3; i>=3; i-=3)
            {
                v[i]=*v+r*ax.getx(vec+i);
                v[i+1]=v[1]+r*ax.gety(vec+i);
                v[i+2]=v[2]+r*ax.getz(vec+i);
            }
            *v+=r*ax.getx(vec);
            v[1]+=r*ax.gety(vec);
            v[2]+=r*ax.getz(vec);
        }
    };
    
    class dodecahedron: public plato
    {
        static double rt5,irt5,sp,sm,rsp,rsm;
    public:
        dodecahedron(void);
        void interpolate(float *values,float *inter);
    };
    
    class icosahedron: public plato
    {
    public:
        icosahedron(void);
    };
}
#endif
