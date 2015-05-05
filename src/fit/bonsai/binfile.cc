#include "RAT/BONSAI/binfile.h"
#include <stdlib.h>
#include <string>

#include <RAT/DB.hh>
#include <RAT/DBLink.hh>

namespace BONSAI {

inline void binfile::order2(char *point)
{
  short test=258;
  int  i,j;
  char *testp=(char *) &test;

  for(i=0; i<2; i++)
    for(j=1; j<=2; j++)
      if (testp[i]==j) point[i]=j-1;
}

inline void binfile::order4(char *point)
{
  long test=16909060;
  int i,j;
  char *testp=(char *) &test;

  for(i=0; i<4; i++)
    for(j=1; j<=4; j++)
      if (testp[i]==j) point[i]=j-1;
}

inline void binfile::order8(char *point)
{

  double test=1212121212121212.;
  int   i;
  char *testp=(char *) &test;

  for(i=0; i<8; i++)
    switch(testp[i])
      {
      case -16: point[i]=7; break;
      case -63: point[i]=6; break;
      case  23: point[i]=5; break;
      case -39: point[i]=4; break;
      case -85: point[i]=3; break;
      case  57: point[i]=2; break;
      case  17: point[i]=1; break;
      case  67: point[i]=0; break;
      default:  point[i]=-1;
      }
  for(i=0; i<8; i++) if (point[i]<0)
    {
      printf("%d: ",i);
      printf("Fatal Error: Unknown double precision format!\n");
      exit(1);
    }
}

binfile::binfile(char *name,char mode)
{
  char allmode[]=" b";

  order2(offset);
  order4(offset+2);
  order8(offset+6);
  allmode[0]=mode;
  
  RAT::DB *db = RAT::DB::Get();
  RAT::DBLinkPtr table = db->GetLink("BONSAI");
  std::string path = table->GetS("data_path");
  fp=fopen((std::string(getenv("GLG4DATA"))+"/" + path + "/"+std::string(name)).c_str(),allmode); //FIXME this is probably not good enough

  if (fp==NULL)
    {
      printf("Unable to open file %s as %s!\n",name,allmode);
      exit(1);
    }
}

binfile::binfile(FILE *f)
{
  fp=f;
}

binfile::~binfile(void)
{
  fclose(fp);
}

int binfile::read(int *&sizes,int *&numbers,void **&starts)
{
  char primer[900];
  int  narray,i,j,k;
  char *datp=NULL,*buffp;

  buffp=primer;
  for(narray=0; narray<300; narray++,buffp+=3)
    {
      if (fread(buffp,1,3,fp)!=3) return(-1);
      if (*buffp & 0x80) break;
    }
  *buffp=*buffp & 0x7f;
  narray++;
  sizes=new int[narray];
  numbers=new int[narray];
  starts=new void *[narray];
  buffp=primer;
  for(i=0; i<narray; i++,buffp+=3)
    {
      numbers[i]=0;
      datp=(char *)(numbers+i);
      switch(*buffp & 96)
      {
      case 0:
	sizes[i]=1;
	datp[(int)offset[3]]=*buffp;
	break;
      case 32:
	sizes[i]=2;
	datp[(int)offset[3]]=*buffp-32;
	break;
      case 64:
	sizes[i]=4;
	datp[(int)offset[3]]=*buffp-64;
	*buffp-=64;
	break;
      case 96:
	sizes[i]=8;
	datp[(int)offset[3]]=*buffp-96;
	break;
      }
      datp[(int)offset[4]]=buffp[1];
      datp[(int)offset[5]]=buffp[2];
    }
  for(i=0; i<narray; i++)
    {
      switch(sizes[i])
	{
	case 1:
	  datp=new char[numbers[i]];
	  break;
	case 2:
	  datp=(char *) new short[numbers[i]];
	  break;
	case 4:
	  datp=(char *)new long[numbers[i]];
	  break;
	case 8:
	  datp=(char *)new double[numbers[i]];
	  break;
	}
      starts[i]=datp;
      for(j=0; j<numbers[i]; j++,datp+=sizes[i])
	{
	  if (fread(primer,1,sizes[i],fp)!=(unsigned int)sizes[i])	 
	    {
	      printf("Unable to read record from file!\n");
	      exit(1);
	    }
	  switch(sizes[i])
	    {
	    case 1:
	      *datp=*primer;
	      break;
	    case 2:
	      datp[(int)offset[0]]=*primer;
	      datp[(int)offset[1]]=primer[1];
	      break;
	    case 4:
	      for(k=2; k<6; k++)
		datp[(int)offset[k]]=primer[k-2];
	      break;
	    case 8: 
	      for(k=6; k<14; k++)
		datp[(int)offset[k]]=primer[k-6];
	      break;
	    }
	}
    }
  return(narray);
}

void binfile::write(int *sizes,int *numbers,void **starts)
{
  int  narray,totsize,i,j,k;
  char *datp,*buffer,*buffp;

  for(i=0; sizes[i]>=0; i++)
    {
      if (numbers[i]>2097151) numbers[i]=2097151;
      if (numbers[i]<0) numbers[i]=0;
      switch(sizes[i])
	{
	case 1:
	case 2:
	case 4:
	case 8:
	  break;
	default:
	  sizes[i]=4;
	}
    }
  if ((narray=i)==0) return;
  for(i=1,totsize=*sizes*(*numbers); i<narray; i++)
    totsize+=sizes[i]*numbers[i];
  buffer=new char[totsize+3*narray];
  buffp=buffer+3*narray;
  for(i=0; i<narray; i++)
    {
      datp=(char *)(numbers+i);
      buffer[3*i]=datp[(int)offset[3]];
      buffer[3*i+1]=datp[(int)offset[4]];
      buffer[3*i+2]=datp[(int)offset[5]];
      datp=(char *) (starts[i]);
      switch(sizes[i])
	{
	case 1:
	  for(j=0; j<numbers[i]; j++)
	    *buffp++=*datp++;
	  break;
	case 2:
	  buffer[3*i]+=32;
	  for(j=0; j<numbers[i]; j++,datp+=2)
	    {
	      *buffp++=datp[(int)offset[0]];
	      *buffp++=datp[(int)offset[1]];
	    }
	  break;
	case 4:
	  buffer[3*i]+=64;
	  for(j=0; j<numbers[i]; j++,datp+=4)
	    for(k=2; k<6; k++)
	      *buffp++=datp[(int)offset[k]];
	  break;
	case 8:
	  buffer[3*i]+=96;
	  for(j=0; j<numbers[i]; j++,datp+=8)
	    for(k=6; k<14; k++)
	      *buffp++=datp[(int)offset[k]];
	  break;
	}
      if (sizes[i+1]<0) buffer[3*i]=buffer[3*i] | 0x80;
    }
  if (fwrite(buffer,1,totsize+3*narray,fp)!=
      (unsigned int)(totsize+3*narray))
    {
      printf("Unable to write record to file!\n");
      exit(1);
    }
  delete(buffer);
}
}
