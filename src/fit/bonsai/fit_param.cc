#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "RAT/BONSAI/fit_param.h"

#define FLOAT_TYPE           1
#define INT_TYPE             2
#define SHORT_INT_TYPE       3
#define ARRAY_FLOAT_TYPE     4
#define ARRAY_INT_TYPE       5
#define ARRAY_SHORT_INT_TYPE 6

#define PI180 0.01745329252

namespace BONSAI {


char *    fit_param::numbers[MAX_NUMBER];
float     fit_param::tres;
float     fit_param::tcoin;
float     fit_param::dlim;
float     fit_param::tlim;
short int fit_param::nselall=0;
float     fit_param::gcon0;
float     fit_param::dw4hit;
float     fit_param::dwfit;
float     fit_param::clusgtheta0;
float     fit_param::clusgthetaplus;
float     fit_param::clusgthetaminus;
float     fit_param::clusgdirweight;
short int fit_param::np;
float *   fit_param::cdwall;
float *   fit_param::gdiff;
float     fit_param::lastdiff;
float *   fit_param::gfrac;
float *   fit_param::clustheta0;
float *   fit_param::clusthetaplus;
float *   fit_param::clusthetaminus;
float *   fit_param::clusdirweight;
float     fit_param::lastfrac;
float     fit_param::bgrid;
float     fit_param::oc0;
float     fit_param::obgdiff;
float     fit_param::blastdiff;
float     fit_param::obgfrac;
float     fit_param::cbgfrac;
float     fit_param::obrmin;
float     fit_param::obrstop;
float     fit_param::bdwall;
float     fit_param::bdwfit;
float     fit_param::cgrid;
float     fit_param::tim0;
float *   fit_param::time;
float *   fit_param::rmin;
float *   fit_param::rstop;
float     fit_param::blastfrac;
float     fit_param::opangle;
float     fit_param::omangle;
float     fit_param::cc0;
float     fit_param::bc0;
float     fit_param::cpangle;
float     fit_param::bpangle;
float     fit_param::cmangle;
float     fit_param::bmangle;
float     fit_param::cbgdiff;
float     fit_param::cbrmin;
float     fit_param::cbrstop;

// *************************************************************
// * identify up to two key words and read into appropriate    *
// * array or variable                                         *
// *************************************************************
char fit_param::parse(char *line,unsigned char length,
		      char *key1,char *key2,char *key3,
		      char type,void *var)
{
  unsigned char key_length1=strlen(key1),key_length2=strlen(key2);
  unsigned char key_length3=strlen(key3),pos1,pos2,pos3,ar,nl;

  // if array, find array position using for number words
  // adjust variable pointer accordingly (assume float or int)
  if ((type==ARRAY_FLOAT_TYPE) || (type==ARRAY_INT_TYPE) ||
      (type==ARRAY_SHORT_INT_TYPE))
    for(ar=0; ar<MAX_NUMBER; ar++)
      {
	nl=strlen(numbers[ar])-1;
	for(pos1=0; pos1<length-nl; pos1++)
	  if (!strncmp(line+pos1,numbers[ar]+1,nl))
	    break;
	if (pos1<length-nl)
	  {
	    var=(float *)var+ar;
	    line+=pos1+nl;
	    length-=pos1+nl;
            break;
	  }
      }
  // check key1, if not an empty string
  if (key_length1>0)
    {
      for(pos1=0; pos1<length-key_length1; pos1++)
	if (!strncmp(line+pos1,key1,key_length1)) break;
      pos1+=key_length1;
      if (pos1>=length) return(0); // return, if not found
    }
  else
    pos1=0;
  // check key2, if not an empty string
  if (key_length2>0)
    {
      for(pos2=0; pos2<length-key_length2; pos2++)
	if (!strncmp(line+pos2,key2,key_length2)) break;
      pos2+=key_length2;
      if (pos2>=length) return(0); // return, if not found
    }
  else
    pos2=0;
  // check key2, if not an empty string
  if (key_length3>0)
    {
      for(pos3=0; pos3<length-key_length3; pos3++)
	if (!strncmp(line+pos3,key3,key_length3)) break;
      pos3+=key_length3;
      if (pos3>=length) return(0); // return, if not found
    }
  else
    pos3=0;

  // reset string pointer to after both keywords
  if (pos1>pos2)
    {
      if (pos1>pos3)
	{
	  line+=pos1;
	  length-=pos1;
	}
      else
	{
	  line+=pos3;
	  length-=pos3;
	}
    }
  else
    {
      if (pos2>pos3)
	{
	  line+=pos2;
	  length-=pos2;
	}
      else
	{
	  line+=pos3;
	  length-=pos3;
	}
    }
  // find end of description
  for(pos1=0; (pos1<length) && (line[pos1]!=':'); pos1++);
  if (++pos1>=length) return(0);
  // eliminate empty spaces
  for(; (pos1<length) && (line[pos1]==' '); pos1++);
  if (pos1>=length) return(0);
  line+=pos1;
  length-=pos1;
  // check none
  for(pos3=0; pos3<length-4; pos3++)
    if (!strncmp(line+pos3,(char*)"none",4))
      {
	switch(type)
	  {
	  case FLOAT_TYPE:
	  case ARRAY_FLOAT_TYPE:     *((float *)var)=FIT_PARAM_NONE; return(1);
	  case INT_TYPE:
	  case ARRAY_INT_TYPE:       *((int *)var)=FIT_PARAM_NONE; return(1);
	  case SHORT_INT_TYPE:
	  case ARRAY_SHORT_INT_TYPE: *((short int *)var)=FIT_PARAM_NONE;
	                             return(1);
	  }
      }
  // accept + and - character
  if ((*line=='+') || (*line=='-'))
    pos1=1;
  else
    pos1=0;
  // find end of number
  for(; (pos1<length) && 
	((line[pos1]=='.') || ((line[pos1]>='0') && (line[pos1]<='9')));
      pos1++);
  // end the string at the end of the number
  if (pos1<length) line[pos1]=0;
  // read into variable
  switch(type)
    {
    case FLOAT_TYPE:
    case ARRAY_FLOAT_TYPE:     sscanf(line,(char*)"%f",(float *)var); return(1);
    case INT_TYPE:
    case ARRAY_INT_TYPE:       sscanf(line,(char*)"%d",(int *)var); return(1);
    case SHORT_INT_TYPE:
    case ARRAY_SHORT_INT_TYPE: sscanf(line,(char*)"%hd",(short int *)var); return(1);
    }
  return(0);
}

// *************************************************************
// * check all variables and read in values; initialize arrays *
// * where possible                                            *
// *************************************************************
inline void fit_param::parse(char *line)
{
  unsigned char length=strlen(line),ar;

  if (parse(line,length,(char*)"PMT",(char*)"ime",(char*)"esolution",FLOAT_TYPE,&tres)) return;
  if (parse(line,length,(char*)"PMT",(char*)"oincidence",(char*)"ime",FLOAT_TYPE,&tcoin)) return;
  if (parse(line,length,(char*)"air",(char*)"istance",(char*)"raction",FLOAT_TYPE,&dlim)) return;
  if (parse(line,length,(char*)"air",(char*)"ime",(char*)"raction",FLOAT_TYPE,&tlim)) return;
  if (parse(line,length,(char*)"aximum",(char*)"its",(char*)"ombin",SHORT_INT_TYPE,&nselall)) return;
  if (parse(line,length,(char*)"nitial",(char*)"rid",(char*)"onstant",FLOAT_TYPE,&gcon0)) return;
  
  if (parse(line,length,(char*)"lusfit",(char*)"rid",(char*)"onstant",
			    ARRAY_FLOAT_TYPE,&cgrid)) return;
  if (parse(line,length,(char*)"all",(char*)"istance",(char*)"ertices",FLOAT_TYPE,&dw4hit)) return;
  if (parse(line,length,(char*)"all",(char*)"lusfit",(char*)"ertex",FLOAT_TYPE,&dwfit)) return;
  if (parse(line,length,(char*)"nitial",(char*)"Clusfit",(char*)"pening",FLOAT_TYPE,&clusgtheta0))
    return;
  if (parse(line,length,(char*)"nitial",(char*)"Clusfit",(char*)"ositive",
	    FLOAT_TYPE,&clusgthetaplus)) return;
  if (parse(line,length,(char*)"nitial",(char*)"Clusfit",(char*)"egative",
	    FLOAT_TYPE,&clusgthetaminus)) return;
  if (parse(line,length,(char*)"nitial",(char*)"Clusfit",(char*)"eight",FLOAT_TYPE,&clusgdirweight))
    return;
  if ((np==-1) && (parse(line,length,(char*)"umber",(char*)"lusfit",(char*)"asses",SHORT_INT_TYPE,&np)))
    {
      if (np<1) { np=-1; return; }
      if (np>10) np=10;
      cdwall=new float[np];
      time=new float[np];
      gdiff=new float[np];
      gfrac=new float[np];
      rmin=new float[np];
      rstop=new float[np];
      clustheta0=new float[np];
      clusthetaplus=new float[np];
      clusthetaminus=new float[np];
      clusdirweight=new float[np];
      for(ar=0; ar<np; ar++)
	{	 
	  cdwall[ar]=time[ar]=gdiff[ar]=gfrac[ar]=rmin[ar]=rstop[ar]=-1;
	  clustheta0[ar]=clusthetaplus[ar]=clusthetaminus[ar]=clusdirweight[ar]=-1;
	}
      return;
    }
  if (parse(line,length,(char*)"inal",(char*)"oodness",(char*)"ifference",FLOAT_TYPE,&lastdiff))
    return;
  if ((np!=-1) && (parse(line,length,(char*)"all",(char*)"nvoke",(char*)"iner",
                         ARRAY_FLOAT_TYPE,cdwall))) return;
  if ((np!=-1) && (parse(line,length,(char*)"oodness",(char*)"ifference",(char*)"kim",
			 ARRAY_FLOAT_TYPE,gdiff))) return;
  if (parse(line,length,(char*)"inal",(char*)"oodness",(char*)"raction",FLOAT_TYPE,&lastfrac))
    return;
  if ((np!=-1) && (parse(line,length,(char*)"oodness",(char*)"kim",(char*)"raction",
			 ARRAY_FLOAT_TYPE,gfrac))) return;
  if (parse(line,length,(char*)"rid",(char*)"ime",(char*)"indow",FLOAT_TYPE,&tim0)) return;
  if ((np!=-1) && (parse(line,length,(char*)"oodness",(char*)"ime",(char*)"indow",
			 ARRAY_FLOAT_TYPE,time))) return;
  if ((np!=-1) && (parse(line,length,(char*)"lusfit",(char*)"inimum",(char*)"adius",
			 ARRAY_FLOAT_TYPE,rmin))) return;
  if ((np!=-1) && (parse(line,length,(char*)"lusfit",(char*)"top",(char*)"adius",
			 ARRAY_FLOAT_TYPE,rstop))) return;
  if (parse(line,length,(char*)"Clusfit",(char*)"pening",(char*)"ngle",ARRAY_FLOAT_TYPE,clustheta0))
    return;
  if (parse(line,length,(char*)"Clusfit",(char*)"ositive",(char*)"ngle",
	    ARRAY_FLOAT_TYPE,clusthetaplus)) return;
  if (parse(line,length,(char*)"Clusfit",(char*)"egative",(char*)"ngle",
	    ARRAY_FLOAT_TYPE,clusthetaminus)) return;
  if (parse(line,length,(char*)"Clusfit",(char*)"ngle",(char*)"eight",ARRAY_FLOAT_TYPE,clusdirweight))
    return;
  if (parse(line,length,(char*)"BONSAI",(char*)"rid",(char*)"onstant",FLOAT_TYPE,&bgrid)) return;
  if (parse(line,length,(char*)"all",(char*)"BONSAI",(char*)"ertex",FLOAT_TYPE,&bdwfit)) return;
  if (parse(line,length,(char*)"all",(char*)"nvoke",(char*)"ine",FLOAT_TYPE,&bdwall)) return;
  if (parse(line,length,(char*)"nitial",(char*)"pening",(char*)"ngle",FLOAT_TYPE,&bc0)) return;
  if (parse(line,length,(char*)"nitial",(char*)"ngle",(char*)"ositive",FLOAT_TYPE,&bpangle)) return;
  if (parse(line,length,(char*)"nitial",(char*)"ngle",(char*)"egative",FLOAT_TYPE,&bmangle)) return;
  if (parse(line,length,(char*)"inal",(char*)"ikelihood",(char*)"ifference",FLOAT_TYPE,&blastdiff))
    return;
  if (parse(line,length,(char*)"inal",(char*)"ikelihood",(char*)"raction",FLOAT_TYPE,&blastfrac))
    return;
  if (parse(line,length,(char*)"oarse",(char*)"pening",(char*)"ngle",FLOAT_TYPE,&oc0)) return;
  if (parse(line,length,(char*)"oarse",(char*)"ngle",(char*)"ositive",FLOAT_TYPE,&opangle)) return;
  if (parse(line,length,(char*)"oarse",(char*)"ngle",(char*)"egative",FLOAT_TYPE,&omangle)) return;
  if (parse(line,length,(char*)"oarse",(char*)"ikelihood",(char*)"iff",FLOAT_TYPE,&obgdiff)) return;
  if (parse(line,length,(char*)"oarse",(char*)"ikelihood",(char*)"raction",FLOAT_TYPE,&obgfrac))
    return;
  if (parse(line,length,(char*)"oarse",(char*)"tart",(char*)"adius",FLOAT_TYPE,&obrmin)) return;
  if (parse(line,length,(char*)"oarse",(char*)"top",(char*)"adius",FLOAT_TYPE,&obrstop)) return;
  if (parse(line,length,(char*)"ine",(char*)"pening",(char*)"ngle",FLOAT_TYPE,&cc0)) return;
  if (parse(line,length,(char*)"ine",(char*)"ngle",(char*)"ositive",FLOAT_TYPE,&cpangle)) return;
  if (parse(line,length,(char*)"ine",(char*)"ngle",(char*)"egative",FLOAT_TYPE,&cmangle)) return;
  if (parse(line,length,(char*)"ine",(char*)"ikelihood",(char*)"iff",FLOAT_TYPE,&cbgdiff)) return;
  if (parse(line,length,(char*)"ine",(char*)"ikelihood",(char*)"raction",FLOAT_TYPE,&cbgfrac))
    return;
  if (parse(line,length,(char*)"ine",(char*)"inimum",(char*)"adius",FLOAT_TYPE,&cbrmin)) return;
  if (parse(line,length,(char*)"ine",(char*)"top",(char*)"adius",FLOAT_TYPE,&cbrstop)) return;
}

// *************************************************************
// * convert angular constraint from theta to cos(theta)       *
// *************************************************************
inline void fit_param::ang_constr(float &theta_0,
				  float &theta_plus,float &theta_minus)
{
  if (theta_0==FIT_PARAM_NONE)
    {
      theta_plus=theta_minus=FIT_PARAM_NONE;
      return;
    }
  if (theta_plus==FIT_PARAM_NONE)
    {
      if (theta_minus==FIT_PARAM_NONE)
	{
	  theta_0=cos(theta_0*PI180);
	  return;
	}
      float ang_m=(theta_0-theta_minus)*PI180;
      theta_0=cos(theta_0*PI180);
      theta_plus=cos(ang_m)-theta_0;
      theta_plus=0.5/(theta_plus*theta_plus);
      theta_minus=FIT_PARAM_NONE;
      return;
    }
  if (theta_minus==FIT_PARAM_NONE)
    {
      float ang_p=(theta_0+theta_plus)*PI180;
      theta_0=cos(theta_0*PI180);
      theta_plus=FIT_PARAM_NONE;
      theta_minus=theta_0-cos(ang_p);
      theta_minus=0.5/(theta_minus*theta_minus);
      return;
    }
  float ang_p=(theta_0+theta_plus)*PI180;
  float ang_m=(theta_0-theta_minus)*PI180;
  theta_0=cos(theta_0*PI180);
  theta_plus=cos(ang_m)-theta_0;
  theta_plus=0.5/(theta_plus*theta_plus);
  theta_minus=theta_0-cos(ang_p);
  theta_minus=0.5/(theta_minus*theta_minus);
}

// *************************************************************
// * print n number of spaces                                  *
// *************************************************************
inline void fit_param::printspace(int n)
{
  int i;

  for(i=0; i<n; i++) printf(" ");
}

fit_param::fit_param(void)
{
  if (nselall) return;
  printf("reading fit_param.dat...\n");

  FILE          *pfile=fopen("fit_param.dat",(char*)"r");

  char          line[256];
  unsigned char ar;

  numbers[0]=(char*)"First";
  numbers[1]=(char*)"Second";
  numbers[2]=(char*)"Third";
  numbers[3]=(char*)"Fourth";
  numbers[4]=(char*)"Fifth";
  numbers[5]=(char*)"Sixth";
  numbers[6]=(char*)"Seventh";
  numbers[7]=(char*)"Eighth";
  numbers[8]=(char*)"Ninth";
  numbers[9]=(char*)"Tenth";
  opangle=omangle=oc0=-1;
  bgrid=cgrid=obgdiff=-1;
  blastdiff=blastfrac=obrmin=obrstop=-1;
  cc0=cmangle=-1;
  cpangle=bpangle=cbgdiff=-1;
  cbrmin=cbrstop=obgfrac=cbgfrac=-1;
  bdwall=bdwfit=bc0=bmangle=-1;
  tres=tcoin=dlim=tlim=gcon0=-1;
  clusgtheta0=clusgthetaplus=clusgthetaminus=clusgdirweight=-1;
  nselall=np=-1;
  dw4hit=dwfit=lastdiff=lastfrac=tim0=-1;
  for(fgets(line,255,pfile);!feof(pfile);fgets(line,255,pfile))
    parse(line);
  fclose(pfile);
  if (clusgtheta0==-1) exit(1);
  if (clusgthetaplus==-1) exit(1);
  if (clusgthetaminus==-1) exit(1);
  ang_constr(clusgtheta0,clusgthetaplus,clusgthetaminus);
  for(ar=0; ar<np; ar++)
    {
      if (cdwall[ar]==-1) exit(1);
      if (clustheta0[ar]==-1) exit(1);
      if (clusthetaplus[ar]==-1) exit(1);
      if (clusthetaminus[ar]==-1) exit(1);
      ang_constr(clustheta0[ar],clusthetaplus[ar],clusthetaminus[ar]);
    }
  if (bc0==-1) exit(1);
  if (bpangle==-1) exit(1);
  if (bmangle==-1) exit(1);
  ang_constr(bc0,bpangle,bmangle);
  if (oc0==-1) exit(1);
  if (opangle==-1) exit(1);
  if (omangle==-1) exit(1);
  ang_constr(oc0,opangle,omangle);
  if (cc0==-1) exit(1);
  if (cpangle==-1) exit(1);
  if (cmangle==-1) exit(1);
  ang_constr(cc0,cpangle,cmangle);
  print();
  if (clusgdirweight==-1) exit(1);
  if (cbrstop==-1) exit(1);
  if (tres==-1) exit(1);
  if (tcoin==-1) exit(1);
  if (dlim==-1) exit(1);
  if (tlim==-1) exit(1);
  if (nselall==-1) exit(1);
  if (gcon0==-1) exit(1);
  if (dw4hit==-1) exit(1);
  if (dwfit==-1) exit(1);
  if (np==-1) exit(1);
  if (tim0==-1) exit(1);
  for(ar=0; ar<np; ar++)
    {
      if (gdiff[ar]==-1) exit(1);
      if (gfrac[ar]==-1) exit(1);
      if (time[ar]==-1) exit(1);
      if (rmin[ar]==-1) exit(1);
      if (rstop[ar]==-1) exit(1);
      if (clusdirweight[ar]==-1) exit(1);
    }
  if (lastfrac==-1) exit(1);
  if (lastdiff==-1) exit(1);
  if (cgrid==-1) exit(1);
  if (bgrid==-1) exit(1);
  if (bdwall==-1) exit(1);
  if (blastdiff==-1) exit(1);
  if (obgfrac==-1) exit(1);
  if (cbgfrac==-1) exit(1);
  if (obrmin==-1) exit(1);
  if (obrstop==-1) exit(1);
  if (bdwfit==-1) exit(1);
  if (blastfrac==-1) exit(1);
  if (cbrmin==-1) exit(1);
}

// *************************************************************
// * print out all parameters                                  *
// *************************************************************
void fit_param::print(void)
{
  unsigned char ar;

  printf("BONSAI and Clusfit Shared Parameters\n");
  printf("-------------------------------------------------------\n");
  printf("PMT time resolution:                       %8.2f ns\n",tres);
  printf("PMT coincidence time difference:           %8.2f ns\n",tcoin);
  printf("PMT pair maximal distance fraction:          %8.4f\n",dlim);
  printf("PMT pair maximal time difference fraction:   %8.4f\n",tlim);
  printf("Maximum # of hits to do all 4-hit combin.: %5d\n",nselall);
  printf("Initial grid constant:                     %8.2f cm\n",gcon0);
  printf("Minimum wall distance for 4-hit vertices:  %8.2f cm\n\n",dw4hit);

  printf("Clusfit Parameters\n");
  printf("-------------------------------------------------------\n");
  printf("Clusfit grid constant:                     %8.2f cm\n",cgrid);

  printf("Minimum wall distance for Clusfit vertex:  %8.2f cm\n",dwfit);
  if (clusgtheta0==FIT_PARAM_NONE)
    {
      printf("Initial Clusfit Cherenkov cone opening angle:none\n");
      printf("Initial Clusfit angle positive deviation:   none\n");
      printf("Initial Clusfit angle negative deviation:   none\n");
    }
  else
    {
      printf("Initial Clusfit Cherenkov cone opening angle:%6.2f deg\n",
	     acos(clusgtheta0)/PI180);
      if (clusgthetaminus>0)
        printf("Initial Clusfit angle positive deviation:  %8.2f deg\n",
	       (acos(clusgtheta0-sqrt(0.5/clusgthetaminus))-
	        acos(clusgtheta0))/PI180);
      else
        printf("Initial Clusfit angle positive deviation:   none\n");
      if (clusgthetaplus>0)
        printf("Initial Clusfit angle negative deviation:  %8.2f deg\n",
	       (acos(clusgtheta0)-
	        acos(clusgtheta0+sqrt(0.5/clusgthetaplus)))/PI180);
      else
        printf("Initial Clusfit angle negative deviation:   none\n");
    }
  printf("Initial Clusfit angle goodness weight:     %8.2f\n",
	 clusgdirweight);
  printf("Number of Clusfit passes:                  %5d\n",np);
  for(ar=0; ar<np; ar++)
    {
      printf("%s wall distance to invoke finer search:",numbers[ar]);
      printspace(6-strlen(numbers[ar]));
      printf("%7.2f cm\n",cdwall[ar]);
    }
  for(ar=0; ar<np; ar++)
    {
      printf("%s minimum goodness difference to skim:",numbers[ar]);
      printspace(6-strlen(numbers[ar]));
      printf("%8.2f\n",gdiff[ar]);
    }
  printf("Final minimum goodness difference to skim: %8.2f\n",lastdiff);
  for(ar=0; ar<np; ar++)
    {
      printf("%s goodness skim fraction:",numbers[ar]);
      printspace(19-strlen(numbers[ar]));
      printf("%8.2f\n",gfrac[ar]);
    }
  printf("Final goodness skim fraction:              %8.2f\n",lastfrac);
  printf("Grid search goodness time window:          %8.2f ns\n",tim0);
  for(ar=0; ar<np; ar++)
    {
      printf("%s search goodness time window:",numbers[ar]);
      printspace(14-strlen(numbers[ar]));
      printf("%8.2f ns\n",time[ar]);
    }
  for(ar=0; ar<np; ar++)
    {
      printf("%s search Clusfit minimum radius:",numbers[ar]);
      printspace(12-strlen(numbers[ar]));
      printf("%8.2f cm\n",rmin[ar]);
    }
  for(ar=0; ar<np; ar++)
    {
      printf("%s search Clusfit stop radius:",numbers[ar]);
      printspace(15-strlen(numbers[ar]));
      printf("%8.2f cm\n",rstop[ar]);
    }
  for(ar=0; ar<np; ar++)
    {
      printf("%s Clusfit Cherenkov cone opening angle:",numbers[ar]);
      printspace(7-strlen(numbers[ar]));
      printf("%6.2f deg\n",acos(clustheta0[ar])/PI180);
      printf("%s Clusfit angle positive deviation:",numbers[ar]);
      printspace(9-strlen(numbers[ar]));
      if (clusthetaminus[ar]>0)
	printf("%8.2f deg\n",(acos(clustheta0[ar]-sqrt(0.5/clusthetaminus[ar]))-
		acos(clustheta0[ar]))/PI180);
      else
	printf(" none\n");
      printf("%s Clusfit angle negative deviation:",numbers[ar]);
      printspace(9-strlen(numbers[ar]));
      if (clusthetaplus[ar]>0)
	printf("%8.2f deg\n",(acos(clustheta0[ar])-
		acos(clustheta0[ar]+sqrt(0.5/clusthetaplus[ar])))/PI180);
      else
	printf(" none\n");
      printf("%s Clusfit angle goodness weight:",numbers[ar]);
      printspace(14-strlen(numbers[ar]));
      printf("%6.2f\n",clusdirweight[ar]);
    }
  printf("\n");
  printf("BONSAI Parameters\n");
  printf("-------------------------------------------------------\n");
  printf("BONSAI grid constant:                      %8.2f cm\n",bgrid);
  printf("Minimum wall distance for BONSAI vertex:   %8.2f cm\n",bdwfit);
  printf("Wall distance to invoke fine search:       %8.2f cm\n",bdwall);
  printf("Initial Cherenkov cone opening angle:      %8.2f deg\n",
	 acos(bc0)/PI180);
  if (bmangle>0)
    printf("Initial Cherenkov angle positive deviation:%8.2f deg\n",
	   (acos(bc0-sqrt(0.5/bmangle))-acos(bc0))/PI180);
  else
    printf("Initial Cherenkov angle positive deviation: none\n");
  if (bpangle>0)
    printf("Initial Cherenkov angle negative deviation:%8.2f deg\n",
	   (acos(bc0)-acos(bc0+sqrt(0.5/bpangle)))/PI180);
  else
    printf("Initial Cherenkov angle negative deviation: none\n");
  printf("Final minimum likelihood difference to skim:%7.2f\n",blastdiff);
  printf("Final likelihood skim fraction:            %8.2f\n",blastfrac);
  printf("Coarse Search Parameters-------------------------------\n");
  printf("Coarse search Cherenkov cone opening angle:%8.2f deg\n",
	 acos(oc0)/PI180);
  if (omangle>0)
    printf("Coarse search positive angle deviation:    %8.2f deg\n",
	   (acos(oc0-sqrt(0.5/omangle))-acos(oc0))/PI180);
  else
    printf("Coarse search positive angle deviation:     none\n");
  if (opangle>0)
    printf("Coarse search negative angle deviation:    %8.2f deg\n",
	   (acos(oc0)-acos(oc0+sqrt(0.5/opangle)))/PI180);
  else
    printf("Coarse search negative angle deviation:     none\n");
  printf("Coarse search minimum likelihood diff to skim:%5.2f\n",obgdiff);
  printf("Coarse search likelihood skim fraction:    %8.2f\n",obgfrac);
  printf("Coarse search start radius:                %8.2f cm\n",obrmin);
  printf("Coarse search stop radius:                 %8.2f cm\n",obrstop);
  printf("Fine Search Parameters---------------------------------\n");
  printf("Fine search Cherenkov cone opening angle:  %8.2f deg\n",
	 acos(cc0)/PI180);
  if (cmangle>0)
    printf("Fine search positive angle deviation:      %8.2f deg\n",
	   (acos(cc0-sqrt(0.5/cmangle))-acos(cc0))/PI180);
  else
    printf("Fine search positive angle deviation:       none\n");
  if (cpangle>0)
    printf("Fine search negative angle deviation:      %8.2f deg\n",
	   (acos(cc0)-acos(cc0+sqrt(0.5/cpangle)))/PI180);
  else
    printf("Fine search negative angle deviation:       none");
  printf("Fine search minimum likelihood diff to skim:%7.2f\n",cbgdiff);
  printf("Fine search likelihood skim fraction:      %8.2f\n",cbgfrac);
  printf("Fine search search minimum radius:         %8.2f cm\n",cbrmin);
  printf("Fine search search stop radius:            %8.2f cm\n",cbrstop);
}

}
