#include <math.h>
#include <TRandom.h>
#include <RAT/SimulatedAnnealing.hh>


#define GET_SIMPLEXSUM \
for (n=1;n<=D;n++) {\
for (sum=0.0,m=1;m<=mpts;m++) sum += simplex[m][n];\
simplex_sum[n]=sum;}


namespace RAT {

    template <int D>
    SimulatedAnnealing<D>::SimulatedAnnealing(Minimizable *funk_) : funk(funk_) { 
    
    }
    
    template <int D>
    SimulatedAnnealing<D>::~SimulatedAnnealing() {
    
    }
    
    template <int D>
    void SimulatedAnnealing<D>::SetSimplexPoint(size_t pt, std::vector<double> &point) {
        for (size_t i = 0; i < D; i++) {
            simplex[pt][i] = point[i];
        }
        val[pt] = funk(simplex[pt]);
    }
    
    template <int D>
    void SimulatedAnnealing<D>::Anneal(double temp0, size_t nAnneal, size_t nEval, double alpha, double ftol) {
        for (size_t cycle = 0; cycle < nAnneal; cycle++) {
            temp = temp0*pow(1-(double)cycle/nAnneal,alpha);
            int iter = nEval;
            amebsa(ftol,&iter);
        }
    }

    //From Numerical Recipies in C (Section 10.9)
    //Multidimensional minimization of the function funk(x) where x[1..ndim] is a vector in
    //ndim dimensions, by simulated annealing combined with the downhill simplex method of Nelder
    //and Mead. The input matrix simplex[1..ndim+1][1..ndim] has ndim+1 rows, each an ndimdimensional
    //vector which is a vertex of the starting simplex. Also input are the following: the
    //vector val[1..ndim+1], whose components must be pre-initialized to the values of funk evaluated
    //at the ndim+1 vertices (rows) of simplex; ftol, the fractional convergence tolerance to be
    //achieved in the function value for an early return; iter, and temptr. The routine makes iter
    //function evaluations at an annealing temperature temptr, then returns. You should then de-
    //crease temptr according to your annealing schedule, reset iter, and call the routine again
    //(leaving other arguments unaltered between calls). If iter is returned with a positive value,
    //then early convergence and return occurred. If you initialize val_best to a very large value on the first
    //call, then val_best and simplex_best[1..ndim] will subsequently return the best function value and point ever
    //encountered (even if it is no longer a point in the simplex).
    template <int D>
    void SimulatedAnnealing<D>::amebsa(double ftol, int *iter) {

        int i,ihi,ilo,j,m,n,mpts=D+1;
        double rtol,sum,swap,val_hi,val_lo,val_next_hi,val_save,val_thermal,val_try;

        GET_SIMPLEXSUM
        for (;;) {
            ilo=1; //Determine which point is the highest (worst),
            ihi=2; //next-highest, and lowest (best).
            val_next_hi=val_lo=val[1]+(-temp)*log(gRandom->Rndm());  //Whenever we “look at” a vertex, it gets
            val_hi=val[2]+(-temp)*log(gRandom->Rndm());       //a random thermal fluctuation.
            if (val_lo > val_hi) {
                ihi=1;
                ilo=2;
                val_next_hi=val_hi;
                val_hi=val_lo;
                val_lo=val_next_hi;
            }
            for (i=3;i<=mpts;i++) { //Loop over the points in the simplex.
                val_thermal=val[i]+(-temp)*log(gRandom->Rndm()); //More thermal fluctuations.
                if (val_thermal <= val_lo) {
                    ilo=i;
                    val_lo=val_thermal;
                }
                if (val_thermal > val_hi) {
                    val_next_hi=val_hi;
                    ihi=i;
                    val_hi=val_thermal;
                } else if (val_thermal > val_next_hi) {
                    val_next_hi=val_thermal;
                }
            }
            rtol=2.0*fabs(val_hi-val_lo)/(fabs(val_hi)+fabs(val_lo));
            // Compute the fractional range from highest to lowest and return if satisfactory.
            if (rtol < ftol || *iter < 0) { //If returning, put best point and value in
                swap=val[1]; //slot 1.
                val[1]=val[ilo];
                val[ilo]=swap;
                for (n=1;n<=D;n++) {
                    swap=simplex[1][n];
                    simplex[1][n]=simplex[ilo][n];
                    simplex[ilo][n]=swap;
                }
                break;
            }
            *iter -= 2;
            //Begin a new iteration. First extrapolate by a factor −1 through the face of the simplex
            //across from the high point, i.e., reflect the simplex from the high point.
            val_try=amotsa(ihi,&val_hi,-1.0);
            if (val_try <= val_lo) {
                //Gives a result better than the best point, so try an additional extrapolation by a factor of 2.
                val_try=amotsa(ihi,&val_hi,2.0);
            } else if (val_try >= val_next_hi) {
                //The reflected point is worse than the second-highest, so look for an intermediate lower point, i.e., do a one-dimensional contraction.
                val_save=val_hi;
                val_try=amotsa(ihi,&val_hi,0.5);
                if (val_try >= val_save) { //Can’t seem to get rid of that high point. Better contract around the lowest (best) point.
                    for (i=1;i<=mpts;i++) {
                        if (i != ilo) {
                            for (j=1;j<=D;j++) {
                                simplex_sum[j]=0.5*(simplex[i][j]+simplex[ilo][j]);
                                simplex[i][j]=simplex_sum[j];
                            }
                            val[i]=(*funk)(simplex_sum);
                        }
                    }
                    *iter -= D;
                    GET_SIMPLEXSUM // Recompute simplex_sum.
                }
            } else ++(*iter); //Correct the evaluation count.
        }
    }

    //From Numerical Recipies in C (Section 10.9)
    //Extrapolates by a factor fac through the face of the simplex across from the high point, tries
    //it, and replaces the high point if the new point is better.
    template <int D>
    double SimulatedAnnealing<D>::amotsa(int ihi, double *val_hi, double fac) {

        int j;
        double fac1,fac2,val_true,val_try;
        
        fac1=(1.0-fac)/D;
        fac2=fac1-fac;
        for (j=1;j<=D;j++)
            simplex_try[j]=simplex_sum[j]*fac1-simplex[ihi][j]*fac2;
        val_try=(*funk)(simplex_try);
        if (val_try <= val_best) { // Save the best-ever.
            for (j=1;j<=D;j++) simplex_best[j]=simplex_try[j];
            val_best=val_try;
        }
        val_true=val_try-(-temp)*log(gRandom->Rndm());  //We added a thermal fluctuation to all the current
                                        //vertices, but we subtract it here, so as to give
                                        //the simplex a thermal Brownian motion: It
                                        //likes to accept any suggested change.
        if (val_true < *val_hi) {
            val[ihi]=val_try;
            *val_hi=val_true;
            for (j=1;j<=D;j++) {
                simplex_sum[j] += simplex_try[j]-simplex[ihi][j];
                simplex[ihi][j]=simplex_try[j];
            }
        }
        
        return val_true;
    }

}
