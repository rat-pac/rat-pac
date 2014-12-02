#ifndef __SIMULATEDANNEALING__
#define __SIMULATEDANNEALING__

#include <vector>

namespace RAT {

class Minimizable {
    public:
        virtual double operator()(double *args) = 0;
};

template <int D>
class SimulatedAnnealing {

    public:
        SimulatedAnnealing(Minimizable *funk);
        virtual ~SimulatedAnnealing();
        
        void SetSimplexPoint(size_t pt, std::vector<double> &point);
        void Anneal(double temp0, size_t nAnneal, size_t nEval, double alpha, double ftol);
        
    protected:
        double temp; 
        
        Minimizable *funk;
        
        double simplex[D+1][D], val[D+1];
        double simplex_try[D], simplex_sum[D], simplex_best[D], val_best;
        
        //From Numerical Recipies in C (Section 10.9)
        void amebsa(double ftol, int *iter);
        
        //From Numerical Recipies in C (Section 10.9)
        double amotsa(int ihi, double *yhi, double fac);

};

} //namespace RAT

#endif
