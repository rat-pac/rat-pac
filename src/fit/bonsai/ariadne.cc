#include "RAT/BONSAI/ariadne.h"
#define COS_CUT   0.88
#define COS_CUT2  0.65
#define CUT_FRAC1 0.80
#define CUT_FRAC2 0.95
#define TTOFMIN   -6
#define TTOFMAX   +6
#define COS_THETA 0.719

#include "stdlib.h"

namespace BONSAI {

void ariadne(float *vertex,hits *event,float *direct,float &goodness)
{
  int   hit,row,column,ntot,nsel,ndir;
  float *ttof,*dirs,*directions,dx,dy,dz,dr2,cos_alpha;
  float sx,sy,sz,cx,cy,cz,ratio,sin_gamma2,sin_alpha2;
  char  *active;
  float *clus_dir,maxmag,limit,limit0;
  float dirx,diry,dirz,sum,sumx,sumy,sumz;
  int   clus_index,max_index,iter,nclus,nrow;

  ntot=event->ntot();
  printf("allocate ttof float %d\n",4*ntot);
  ttof=new float[4*ntot];
  dirs=ttof+ntot;
  for(nsel=hit=0; hit<ntot; hit++)
    {
      ttof[nsel]=event->hittime(hit)-event->tof(vertex,dirs+3*nsel,hit)
		-vertex[3];
      if ((ttof[nsel]>=TTOFMIN) && (ttof[nsel]<=TTOFMAX))
        nsel++;
    }
  printf("allocate directions float %d\n",3*nsel*(nsel+1));
  directions=new float[3*nsel*(nsel+1)];
  for(ndir=row=0; row<nsel; row++)
    {
      dx=dirs[3*row];
      dy=dirs[3*row+1];
      dz=dirs[3*row+2];
      dr2=dx*dx+dy*dy+dz*dz;
      for(column=row+1; column<nsel; column++)
        {
          cos_alpha=dx*dirs[3*column]+dy*dirs[3*column+1]+dz*dirs[3*column+2];
	  if ((cos_alpha>=1) || (cos_alpha<=-1)) continue;
	  sx=dx+dirs[3*column];
	  sy=dy+dirs[3*column+1];
	  sz=dz+dirs[3*column+2];
/****************************************************************************
 * Algorithm: cos(alpha)=axis1 dot axis2
 *            cos(gamma)=cos(theta)/cos(alpha/2)(theta: Cherenkov angle)
 *            b1=axis1+axis2 (normalized)
 *            b2=axis1xaxis2 (normalized)
 *            intersect=b1*cos(gamma)+-b2*sin(gamma)
 * norm.:     ratio=cos(gamma)/||axis1+axis2||
 *                 =cos(theta)/(cos(alpha/2)*sqrt(2*(1+cos(alpha)))
 *                 =cos(theta)/sqrt(0.5*(1+cos(alpha)))*sqrt(2*(1+cos(alpha)))
 *                 =cos(theta)/(1+cos(alpha))
 *            sin(gamma)^2=1-cos(theta)^2/cos(alpha/2)^2
 *                        =1-2*cos(theta)^2/(1+cos(alpha))
 *                        =1-2*cos(theta)*ratio
 ***************************************************************************/

	  ratio=COS_THETA/(1+cos_alpha);
	  sin_gamma2=1-2*COS_THETA*ratio;

	  if (sin_gamma2<0) continue; // no solution
	  if (sin_gamma2==0)          // one solution
	    {
	      directions[3*ndir]=sx*ratio;
	      directions[3*ndir+1]=sy*ratio;
	      directions[3*ndir+2]=sz*ratio;
	      /*printf("0 %3d %3d %f %f (%f)\n",row,column,
		     directions[3*ndir]*dirs[3*row]+directions[3*ndir+1]*dirs[3*row+1]+directions[3*ndir+2]*dirs[3*row+2],
		     directions[3*ndir]*dirs[3*column]+directions[3*ndir+1]*dirs[3*column+1]+directions[3*ndir+2]*dirs[3*column+2],
		     directions[3*ndir]*directions[3*ndir]+directions[3*ndir+1]*directions[3*ndir+1]+directions[3*ndir+2]*directions[3*ndir+2]);*/
	      ndir++;
	      continue;
	    }
// two solutions
	  sx*=ratio;
	  sy*=ratio;
	  sz*=ratio;
	  sin_alpha2=dr2*(dirs[3*column]*dirs[3*column]+dirs[3*column+1]*dirs[3*column+1]+dirs[3*column+2]*dirs[3*column+2])-cos_alpha*cos_alpha;
	  if (sin_alpha2==0) continue;
// now do cross product with normalization:
//(axb).(axb)=((axb)xa).b=a(b.b)(a.a)-(a.b)^2
          ratio=sqrt(sin_gamma2/sin_alpha2);
	  cx=ratio*(dy*dirs[3*column+2]-dz*dirs[3*column+1]);
	  cy=ratio*(dz*dirs[3*column]-dx*dirs[3*column+2]);
	  cz=ratio*(dx*dirs[3*column+1]-dy*dirs[3*column]);
// find two solutions
	  directions[3*ndir]=sx+cx;
	  directions[3*ndir+1]=sy+cy;
	  directions[3*ndir+2]=sz+cz;
	  directions[3*ndir+3]=sx-cx;
	  directions[3*ndir+4]=sy-cy;
	  directions[3*ndir+5]=sz-cz;
	  /*printf("1 %3d %3d %f %f %f (%f %f %f)\n",row,column,
		 directions[3*ndir],directions[3*ndir+1],directions[3*ndir+2],
		 directions[3*ndir]*dirs[3*row]+directions[3*ndir+1]*dirs[3*row+1]+directions[3*ndir+2]*dirs[3*row+2],
		 directions[3*ndir]*dirs[3*column]+directions[3*ndir+1]*dirs[3*column+1]+directions[3*ndir+2]*dirs[3*column+2],
		 directions[3*ndir]*directions[3*ndir]+directions[3*ndir+1]*directions[3*ndir+1]+directions[3*ndir+2]*directions[3*ndir+2]);
	  printf("2 %3d %3d %f %f %f (%f %f %f)\n",row,column,
		 directions[3*ndir+3],directions[3*ndir+4],directions[3*ndir+5],
		 directions[3*ndir+3]*dirs[3*row]+directions[3*ndir+4]*dirs[3*row+1]+directions[3*ndir+5]*dirs[3*row+2],
		 directions[3*ndir+3]*dirs[3*column]+directions[3*ndir+4]*dirs[3*column+1]+directions[3*ndir+5]*dirs[3*column+2],
		 directions[3*ndir+3]*directions[3*ndir+3]+directions[3*ndir+4]*directions[3*ndir+4]+directions[3*ndir+5]*directions[3*ndir+5]);*/
	  ndir+=2;
      }
    }
  printf("allocate active char %d\n",ndir);
  active=new char[ndir];
  printf("allocate clus_dir float %d\n",4*ndir);
  clus_dir=new float[4*ndir];
  for(row=0; row<ndir; row++) active[row]=1;
/***********************************************************************
 * loop through direction and try to add as many directions
 * as possible (with a large angualar window). don't loop through the
 * directions already added
 **********************************************************************/
  maxmag=2.5;
  limit0=limit=2.5; //FIXME is this okay
  nclus=0;
  nrow=ndir;
  for(iter=0; iter<3; iter++)
    {
      printf("start iter %d\n",iter+1);
      max_index=-2;
      for(row=0; row<nrow; row++)
        {
	  if (iter==0) // for first iteration, check all active directions
	    {
	      if (active[row])
	        {
		  dirx=directions[3*row];
		  diry=directions[3*row+1];
		  dirz=directions[3*row+2];
		  clus_index=nclus;
		}
	      else continue;
	    }
	  else // for other iterations, check clusters of sufficient goodness
	    {
	      if (clus_dir[4*row]>limit0)
	        {
		  dirx=clus_dir[4*row+1];
		  diry=clus_dir[4*row+2];
		  dirz=clus_dir[4*row+3];
		  clus_index=row;
		}
	      else
	        {
		  clus_dir[4*row]=0;
		  continue;
		}
	    }
	  if (dirx*dirx+diry*diry+dirz*dirz>2)
	    {
	      printf("iter=%d check %3d %9.6f %9.6f %9.6f\n",
		     iter,row,dirx,diry,dirz);
	      exit(1);
	    }
	  if (iter>0) printf("iter=%d check %3d %9.6f %9.6f %9.6f ",
		 iter,row,dirx,diry,dirz);
	  // sum up all directions that have cos(theta)>COS_CUT
	  for(sumx=sumy=sumz=column=0; column<ndir; column++)
	    if (dirx*directions[3*column]+diry*directions[3*column+1]+
	        dirz*directions[3*column+2]>COS_CUT)
	      {
	        active[column]=0;
		sumx+=directions[3*column];
		sumy+=directions[3*column+1];
		sumz+=directions[3*column+2];
	      }
	  sum=sqrt(sumx*sumx+sumy*sumy+sumz*sumz);
	  if (iter>0) printf("sum=%f(%f)\n",sum,limit);
/***********************************************************************
 * store all directions that have at least 80% of the maximal goodness,
 * find the maximal goodness, find maximal goodness                    */
	  if (sum>=limit)
	    {
	      if (sum>maxmag)
	        {
		  printf("(mm=%f)",maxmag);
		  maxmag=sum;
		  *direct=sumx/sum;
		  direct[1]=sumy/sum;
		  direct[2]=sumz/sum;
		  printf("set %f %f %f\n",dirx,diry,dirz);
		  if (iter==0)
		    {
		      nclus++;
		      limit=maxmag*CUT_FRAC1;
		      max_index=clus_index;
		    }
		  else
		    {
		      limit=maxmag*CUT_FRAC2;
		      max_index=row;
		    }
		  printf("max %9.6f %9.6f %9.6f %f (%d) limit=%f\n",
		         sumx/sum,sumy/sum,sumz/sum,sum,nclus,limit);
		}
	      else
	        {
		  printf("not max but %f %f %f %f\n",sumx/sum,sumy/sum,sumz/sum,sum);
		  nclus++;
		}
	      //if ((iter==0) || (sum==maxmag))
	        {
		  clus_dir[4*clus_index]=sum;
		  sum=1/sum;
		  clus_dir[4*clus_index+1]=sumx*sum;
		  clus_dir[4*clus_index+2]=sumy*sum;
		  clus_dir[4*clus_index+3]=sumz*sum;
		}
	    }
	  else clus_dir[4*clus_index]=0;
	}
      nrow=nclus;
      limit0=limit;
      if (max_index<0) break;
      int k;
      for(k=0; k<nrow; k++)
        printf("%d %f %f %f %f\n",k,clus_dir[4*k],clus_dir[4*k+1],
	       clus_dir[4*k+2],clus_dir[4*k+3]);
      if (iter<2) maxmag=1.5;
    }
  if (maxmag<2.5)
    {
      return;
    }
  goodness=2*maxmag/(nsel*(nsel+1));
  dirx=*direct;
  diry=direct[1];
  dirz=direct[2];
  printf("final sum %f %f %f\n",dirx,diry,dirz);
  for(sumx=sumy=sumz=column=0; column<ndir; column++)
    if (dirx*directions[3*column]+diry*directions[3*column+1]+
	dirz*directions[3*column+2]>COS_CUT)
      {
        sumx+=directions[3*column];
	sumy+=directions[3*column+1];
	sumz+=directions[3*column+2];
      }
  sum=sqrt(sumx*sumx+sumy*sumy+sumz*sumz);
  *direct=sumx/sum;
  direct[1]=sumy/sum;
  direct[2]=sumz/sum;
  printf("final %f %f %f %f %f\n",*direct,direct[1],direct[2],maxmag,goodness);
  printf("delete clus_dir\n");
  delete(clus_dir);
  printf("delete active\n");
  delete(active);
  printf("delete directions\n");
  delete(directions);
  printf("delete ttof\n");
  delete(ttof);
/***********************************************************************
 * reprocess the best directions with successively smaller angualar windows*/
  /*limit=maxmag*CUT_FRAC1;
  maxmag=1.5;
  dirx=clus_dir[4*max_index+1];
  diry=clus_dir[4*max_index+2];
  dirz=clus_dir[4*max_index+3];
  max_index=-2;

  for(index=0; index<clus_index; index++)
    if (clus_dir[4*index]>limit)
      {
	
         DO i=1, clus_index
            IF (clusdir(4,i) .GT. limit) THEN
               cosine=clusdir(1,i)*tempdir(1)+clusdir(2,i)*tempdir(2)+
     +                clusdir(3,i)*tempdir(3)
               IF (cosine .LT. cos_cut2) THEN
                  nrscat=nrscat+1
                  IF (cosine .LT. cos_scat) THEN
                     cos_scat=cosine
                  ENDIF
               ENDIF
               CALL sum_dir(tempdir,nradd,clusdir(1,i),cos_cut,nrdir)
               IF (tempdir(4) .LT. maxmag*cut_frac2) THEN
                  clusdir(4,i)=0
               ELSEIF (tempdir(4) .GT. maxmag) THEN
                  maxmag=tempdir(4)
                  max_index=i
                  clusdir(1,i)=tempdir(1)
                  clusdir(2,i)=tempdir(2)
                  clusdir(3,i)=tempdir(3)
                  clusdir(4,i)=maxmag
               ENDIF
            ELSE
               clusdir(4,i)=0
            ENDIF
         ENDDO
         IF (max_index .EQ. -1) THEN
            RETURN
         ENDIF
C-------------------------------------------------------------------
C one last pass to improve the core resolution, calculate goodness
C and quality from the best direction found
         limit=maxmag*cut_frac2
         maxmag=1.5
         DO i=1, 3
            aris_direction(i)=clusdir(i,max_index)
         ENDDO
         max_index=-1
         DO i=1, clus_index
            IF (clusdir(4,i) .GT. limit) THEN
               CALL sum_dir(tempdir,nradd,clusdir(1,i),cos_cut,nrdir)
               IF (tempdir(4) .LT. maxmag*cut_frac2) THEN
                  clusdir(4,i)=0
               ELSEIF (tempdir(4) .GT. maxmag) THEN
                  maxmag=tempdir(4)
                  max_index=i
                  DO j=1, 4
                     clusdir(j,i)=tempdir(j)
                  ENDDO
               ENDIF
            ELSE
               clusdir(4,i)=0
            ENDIF
         ENDDO
         IF (max_index .EQ. -1) THEN
            RETURN
         ENDIF
c         DO i=1,3
c            arit_direction(i)=clusdir(i,max_index)
c         ENDDO
         goodness=clusdir(4,max_index)/quality
         CALL sum_dir(tempdir,nradd,clusdir(1,max_index),
     ,                cos_cut,nrdir)
         quality=nradd/quality
         IF (nradd .LT. 3) RETURN
         DO i=1, 3
            direction(i)=tempdir(i)
         ENDDO
         RETURN*/
}

}
