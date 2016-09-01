#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <string>

#include <RAT/DB.hh>
#include <RAT/DBLink.hh>
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
    
    float* GetFArray(RAT::DBLinkPtr table, std::string field) {
        std::vector<double> temp = table->GetDArray(field);
        float *result = new float[temp.size()];
        for (size_t i = 0; i < temp.size(); i++) {
            result[i] = temp[i];
        }
        return result;
    }
    
    fit_param::fit_param(void) {
        
        if (nselall) return;
        
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
        
        RAT::DB *db = RAT::DB::Get();
        RAT::DBLinkPtr table = db->GetLink("BONSAI");
        
        tres = table->GetD("tres");
        tcoin = table->GetD("tcoin");
        dlim = table->GetD("dlim");
        tlim = table->GetD("tlim");
        nselall = table->GetI("nselall");
        gcon0 = table->GetD("gcon0");
        dw4hit = table->GetD("dw4hit");
        dwfit = table->GetD("dwfit");
        clusgtheta0 = table->GetD("clusgtheta0");
        clusgthetaplus = table->GetD("clusgthetaplus");
        clusgthetaminus = table->GetD("clusgthetaminus");
        clusgdirweight = table->GetD("clusgdirweight");
        
        np = table->GetI("np");
        if (np < 1) {
            np=-1;
        } else {
            if (np > 10) np = 10;
            cdwall = GetFArray(table,"cdwall");
            time = GetFArray(table,"time");
            gdiff = GetFArray(table,"gdiff");
            gfrac = GetFArray(table,"gfrac");
            rmin = GetFArray(table,"rmin");
            rstop = GetFArray(table,"rstop");
            clustheta0 = GetFArray(table,"clustheta0");
            clusthetaplus = GetFArray(table,"clusthetaplus");
            clusthetaminus = GetFArray(table,"clusthetaminus");
            clusdirweight = GetFArray(table,"clusdirweight");
        }
        
        ang_constr(clusgtheta0,clusgthetaplus,clusgthetaminus);
        for (int ar = 0; ar < np; ar++) {
            ang_constr(clustheta0[ar],clusthetaplus[ar],clusthetaminus[ar]);
        }
        
        tim0 = table->GetD("tim0");
        lastdiff = table->GetD("lastdiff");
        lastfrac = table->GetD("lastfrac");
        bgrid = table->GetD("bgrid");
        oc0 = table->GetD("oc0");
        obgdiff = table->GetD("obgdiff");
        blastdiff = table->GetD("blastdiff");
        obgfrac = table->GetD("obgfrac");
        cbgfrac = table->GetD("cbgfrac");
        obrmin = table->GetD("obrmin");
        obrstop = table->GetD("obrstop");
        bdwall = table->GetD("bdwall");
        bdwfit = table->GetD("bdwfit");
        cgrid = table->GetD("cgrid");
        blastfrac = table->GetD("blastfrac");
        opangle = table->GetD("opangle");
        omangle = table->GetD("omangle");
        cc0 = table->GetD("cc0");
        bc0 = table->GetD("bc0");
        cpangle = table->GetD("cpangle");
        bpangle = table->GetD("bpangle");
        cmangle = table->GetD("cmangle");
        bmangle = table->GetD("bmangle");
        cbgdiff = table->GetD("cbgdiff");
        cbrmin = table->GetD("cbrmin");
        cbrstop = table->GetD("cbrstop");
        
        ang_constr(bc0,bpangle,bmangle);
        ang_constr(oc0,opangle,omangle);
        ang_constr(cc0,cpangle,cmangle);
        
        print();
        
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
