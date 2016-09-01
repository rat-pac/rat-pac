#include "RAT/BONSAI/goodness.h"
namespace BONSAI {
    
    // **********************************************
    // quick sort algorithm to sort the hit ttofs
    // **********************************************
    void goodness::qsort(short int *list,short int n)
    {
        if (n<2) return; // no need to sort for 1 or zero hits
        if (n==2)        // 2 hits are easy to sort ...
        {
            if (ttof[*list]<ttof[list[1]]) return;
            n=list[1];
            list[1]=*list;
            *list=n;
            return;
        }
        
        // split list of hits in 2 sublists, sort sublists
        short int half=n/2;
        short int *first=buffer;
        qsort(list,half);
        qsort(list+half,n-half);
        
        // merge the two sorted sublists
        short int *second=list+half;
        int       m=n-half,firstp,secondp,nfirst=0;
        short int firstel=*list;
        
        // firstp is the index of the current element
        // of the first sublist, secondp of the second
        // exit loop, if second list is exhausted, or
        // first list is exhausted and no elements remain
        // in the storage
        for(firstp=secondp=0;
            (secondp<m) && ((firstp<half) || (nfirst!=0));
            firstp++)
        {
            if (ttof[firstel]<ttof[second[secondp]])
            { // take element of the first sublist
                if (nfirst==0)            // no elements in storage
                    firstel=list[firstp+1]; // next el. of 1. sublist
                else
                { // otherwise, first use storage
                    if (firstp<half)              // if necessary,
                        first[nfirst]=list[firstp]; // store element
                    else
                        nfirst--;      // otherwise decrease storage
                    list[firstp]=firstel; // fill list with el. of 1. sublist
                    firstel=*(++first);   // next el. of 1. sublist from storage
                }
            }
            else
            { // take element of the second sublist
                if (firstp<half)                // if necessary,
                    first[nfirst++]=list[firstp]; // store element
                list[firstp]=second[secondp++]; // fill list with el. of 2.
            }
        }
        // empty storage
        for(; nfirst>0; nfirst--)
            list[firstp++]=*(first++);
    }
    
    // *************************************************************
    // * bubble sort algorithm for time-of-flight subtracted hit   *
    // * ttofs                                                     *
    // *************************************************************
    void goodness::bubble(void)
    {
        short int sweep,loop,swap,begin,end;
        
        // if sort list is a not initialized, initialize and quick sort it
        if (sort_list[0]==-1)
        {
            for(loop=0; loop<nselected(); loop++) sort_list[loop]=loop;
            qsort(sort_list,nselected());
            return;
        }
        
        for(loop=begin=0,end=nselected()-1; loop<nselected()-1; loop++)
        {
            // search from begin to end; if t_sweep>t_sweep+1 swap
            // and remember position of last swap
            for(sweep=begin,swap=-1; sweep<end; sweep++)
                if (ttof[sort_list[sweep]]>ttof[sort_list[sweep+1]])
                {
                    swap=sort_list[sweep];
                    sort_list[sweep]=sort_list[sweep+1];
                    sort_list[sweep+1]=swap;
                    swap=sweep;
                }
            if (swap==-1) return; // if no disorder was found, stop
            // set end of search interval to the last swap; search from end to
            // begin; if t_sweep>t_sweep+1 swap and remember position of first swap
            end=swap;
            for(sweep=end-1,swap=-1; sweep>=begin; sweep--)
                if (ttof[sort_list[sweep]]>ttof[sort_list[sweep+1]])
                {
                    swap=sort_list[sweep];
                    sort_list[sweep]=sort_list[sweep+1];
                    sort_list[sweep+1]=swap;
                    swap=sweep+1;
                }
            if (swap==-1) return; // if no disorder was found, stop
            begin=swap;           // set begin of searchinteval to the first swap
        }
    }
    
    // *************************************************************
    // * calculate vertex goodness using a truncated chi^2:        *
    // * 1-0.5(tau/twin)^2 for |tau/twin|<rt(2)                    *
    // *************************************************************
    float goodness::quality(float *vertex)
    {
        short int start,nwin,beststart,beststop;
        float     tau_begin,tau_end,tau_sum,tau2_sum,g,tav,deviation,tau0;
        
        if (nselected()<2) return(-1); // can't calculate with less than two hits
        
        ngdn++;
        tnorm=1/(1.4142136*twin);
        
        // calculate time-of-flight subtracted hit times, calculate average
        for(tcent=start=0; start<nselected(); start++)
        {
            nwin=sel(start);
            tcent+=(ttof[start]=hittime(nwin)-tof(vertex,hitdir+3*start,nwin));
        }
        tcent/=nselected();
        // center distribution around zero and normalize it by rt(2)*twin
        for(start=0; start<nselected(); start++)
            tau[start]=(ttof[start]-tcent)*tnorm;
        tnorm=1.4142136*twin;
        // sort the time-of-flight subtracted hit times
        bubble();
        // define first window starting at the first hit (just 1 hit)
        start=0;  nwin=0;
        beststart=0; beststop=0;
        tau_begin=tau_end=tau_sum=tau[sort_list[0]];
        tau2_sum=tau_sum*tau_sum;
        gdns=1;
        tau0=0;
        while(1)
        { // calculate goodness for all hits inside the window, if
            // it could be larger than the largest goodness so far
            if (++nwin>gdns)
            {
                tav=tau_sum/nwin; // running average is the time of largest goodness
                if ((deviation=tau_end-1-tav)>0)
                { // if tav is below minimum time (tau_end-1), use minimum time
                    if ((g=nwin*(1+tav*tav-deviation*deviation)-tau2_sum)>gdns)
                    { // if goodness is largerest so far, set t0 and gdns
                        tau0=tau_end-1;
                        beststart=start;
                        beststop=start+nwin;
                        gdns=g;
                    }
                }
                else if ((deviation=tav-tau_begin-1)>0)
                { // if tav is above maximum time (tau_begin+1), use max. time
                    if ((g=nwin*(1+tav*tav-deviation*deviation)-tau2_sum)>gdns)
                    { // if goodness is largerest so far, set t0 and gdns
                        tau0=tau_begin+1;
                        beststart=start;
                        beststop=start+nwin;
                        gdns=g;
                    }
                }
                else
                { // use tav
                    if ((g=nwin*(1+tav*tav)-tau2_sum)>gdns)
                    { // if goodness is largerest so far, set t0 and gdns
                        tau0=tav;
                        beststart=start;
                        beststop=start+nwin;
                        gdns=g;
                    }
                }
                t0=tcent+tnorm*tau0;
            }
            if (start+nwin>=nselected())
            { // if the the end of the window is the end of
                // the hit list, subtract the first time;
                // if the goodness cannot exceed the largest
                // goodness so far, stop
                if (--nwin<gdns) break;
                nwin--;
                tau_sum-=tau_begin;
                tau2_sum-=tau_begin*tau_begin;
                start++;
                tau_begin=tau[sort_list[start]];
            }
            else if ((tav=tau[sort_list[start+nwin]])-tau_begin<=2)
            { // if the next time is still inside the allowed window,
                // extend the window by one
                tau_end=tav;
                tau_sum+=tau_end;
                tau2_sum+=tau_end*tau_end;
            }
            else
            { // if not, remove first time in window
                nwin-=2;
                tau_sum-=tau_begin;
                tau2_sum-=tau_begin*tau_begin;
                start++;
                tau_begin=tau[sort_list[start]];
            }
        }
        // if no direction fit, normalize goodness and return
        if (cosc0==FIT_PARAM_NONE)
        {
            gdns=gdn0=gdns/nselected();
            return(gdns);
        }
        // do direction fit
        float sumdx,sumdy,sumdz,sumg;
        
        sumdx=sumdy=sumdz=sumg=0;
        for(start=beststart; start<beststop; start++)
        {
            nwin=sort_list[start];
            g=tau[nwin]-tau0;
            g=1-g*g;
            nwin*=3;
            sumdx+=hitdir[nwin]*g;
            sumdy+=hitdir[nwin+1]*g;
            sumdz+=hitdir[nwin+2]*g;
            sumg+=g;
        }
        cosc=sqrt(sumdx*sumdx+sumdy*sumdy+sumdz*sumdz);
        phi=atan2(sumdy,sumdz);
        if (sumg>1e-10) //if (gdns>1e-10)	
        {
            cosc/=sumg;             //cosc/=gdns;
            theta=acos(sumdz/sumg); //theta=acos(sumdz/gdns);
        }
        else
        {
            cosc=1;
            if (sumdz>0)
                theta=0;
            else
                theta=3.1415927;
        }
        gdn0=gdns/nselected();
        sumdx=cosc-cosc0;
        if (sumdx>0)
        {
            if (plusdang==FIT_PARAM_NONE)
            {
                gdns=gdn0;
                if (ngdn==1) set_worst(gdns); else check_worst(gdns);
                return(gdns);
            }
            sumdx=1-plusdang*sumdx*sumdx;
        }
        else
        {
            if (minusdang==FIT_PARAM_NONE)
            {
                gdns=gdn0;
                if (ngdn==1) set_worst(gdns); else check_worst(gdns);
                return(gdns);
            }
            sumdx=1-minusdang*sumdx*sumdx;
        }
        if (sumdx>0)
            gdns=((1-dirweight)*gdn0+dirweight*sumdx);
        else
            gdns=(1-dirweight)*gdn0;
        if (ngdn==1) set_worst(gdns); else check_worst(gdns);
        return(gdns);
    }
    
}
