#ifndef __RAT_SimulatedAnnealing__
#define __RAT_SimulatedAnnealing__

#include <iostream>
#include <vector>
#include <cmath>
#include <cfloat>
#include <TRandom.h>

namespace RAT {

class Minimizable {
    public:
        virtual double operator()(double *args) = 0;
};

template <int D>
class SimulatedAnnealing {
        
    protected:
        double temp; 
        
        Minimizable *funk;
        
        double simplex[D+1][D], val[D+1];
        double global_best_pt[D], global_best_val;

    public:
        SimulatedAnnealing(Minimizable *funk_) : funk(funk_) { 
        
        }
        
        ~SimulatedAnnealing() {
        
        }
        
        void SetSimplexPoint(size_t pt, std::vector<double> &point) {
            for (size_t j = 0; j < D; j++) {
                simplex[pt][j] = point[j];
            }
            val[pt] = (*funk)(simplex[pt]);
        }
        
        void Anneal(double temp0, size_t nAnneal, size_t nEval, double alpha) {
            global_best_val = DBL_MAX;
            for (size_t cycle = 0; cycle < nAnneal; cycle++) {
                temp = temp0*pow(1-(double)cycle/nAnneal,alpha);
                int iter = nEval;
                amoeba(&iter);
            }
        }
        
        void GetBestPoint(std::vector<double> &point) {
            for (size_t j = 0; j < D; j++) {
                 point[j] = global_best_pt[j];
            }
        }
     
    protected:
    
        inline double replace_project(const double (&offset)[D], const double (&by)[D], const double factor, double (&into)[D]) {
            for (size_t j = 0; j < D; j++) {
                into[j] = offset[j] + factor*by[j];
            }
            double val_try = (*funk)(into);
            if (val_try < global_best_val) {
                global_best_val = val_try;
                for (size_t j = 0; j < D; j++) {
                    global_best_pt[j] = into[j];
                }
            }
            return val_try;
        }
        
        void amoeba(int *iter) {
        
            //best, worst, and next worst points
            size_t i_best, i_worst, i_nextworst;
            double val_best, val_worst, val_nextworst;
            
            const double alpha = 1.0;
            const double gamma = 2.0;
            const double rho   =-0.5;
            const double sigma = 0.5;
            
            double centroid[D]; //centroid of all but worst
            double dworst[D]; //vector from worst to centroid
            double try_pt[D]; //temporary vector for test point
            
            //std::cout << "*******************************\n";
            
            for ( ; *iter > 0; ) {
                
                //find best, worst, nextworst (can optomize a lot of this later)
                i_best = i_worst = i_nextworst = -1;
                val_best = DBL_MAX, val_worst = val_nextworst = -DBL_MAX;
                for (size_t i = 0; i <= D; i++) {
                    const double val_thermal = val[i] - temp*log(gRandom->Rndm());
                    //std::cout << val[i] << ' ';
                    if (val_thermal < val_best) {
                        i_best = i;
                        val_best = val_thermal;
                    } 
                    if (val_thermal > val_worst) {
                        i_nextworst = i_worst;
                        val_nextworst = val_worst;
                        i_worst = i;
                        val_worst = val_thermal;
                    } else if (val_thermal > val_nextworst) {
                        i_nextworst = i;
                        val_nextworst = val_thermal;
                    }
                }
                //std::cout << "~ " << i_best << '/' << i_nextworst << '/' << i_worst << " & ";
                //std::cout << val_best << '/' << val_nextworst << '/' << val_worst << "\n";
                
                //update centroid, dworst
                for (size_t j = 0; j < D; j++) {
                    centroid[j] = 0.0;
                    for (size_t i = 0; i <= D; i++) {
                        if (i == i_worst) continue;
                        centroid[j] += simplex[i][j];
                    }
                    centroid[j] /= D;
                    dworst[j] = centroid[j] - simplex[i_worst][j];
                }
                
                //std::cout << "+++++++++++\n";
                //definitely replacing the worst point, try with reflecting
                val[i_worst] = replace_project(centroid,dworst,alpha,simplex[i_worst]);
                const double val_ref_therm = val[i_worst] + temp*log(gRandom->Rndm());
                //std::cout << "\tRef: " << val_ref_therm << "\n";
                (*iter)--;
                if (val_ref_therm < val_nextworst && val_ref_therm >= val_best) {
                    // reflected point was better than the two worst and replaced
                    // the worst point, so continue
                    //std::cout << "reflected (okay)" << std::endl;
                    continue;
                } else if (val_ref_therm < val_best) {
                    // reflecting was really good, try further
                    const double val_try = replace_project(centroid,dworst,gamma,try_pt) ;
                    //std::cout << "\tExp: " << val_try << "\n";
                    if (val_try + temp*log(gRandom->Rndm()) < val_ref_therm) {
                        //further is better, replace
                        for (size_t j = 0; j < D; j++) {
                            simplex[i_worst][j] = try_pt[j];
                        }
                        val[i_worst] = val_try;
                        //std::cout << "expanded" << std::endl;
                    } else {
                        //std::cout << "reflected (best)" << std::endl;
                    }
                    (*iter)--;
                } else {
                    // reflection didn't work so well, try contracting
                    const double val_con = replace_project(centroid,dworst,rho,simplex[i_worst]);
                    val[i_worst] = val_con;
                    //std::cout << "\tCon: " << val_con << "\n";
                    if (val_con + temp*log(gRandom->Rndm())  >= val_worst) {
                        // contracting didn't replace worst
                        // we're near the minimum, so shrink towards best
                        for (size_t i = 0; i <= D; i++) {
                            if (i == i_best) continue;
                            for (size_t j = 0; j < D; j++) {
                                simplex[i][j] = (1.0 - sigma)*simplex[i_best][j] + sigma*simplex[i][j];
                            }
                            val[i] = (*funk)(simplex[i]);
                            //std::cout << "\tShr: " << val[i] << "\n";
                        }
                        //std::cout << "shrink" << std::endl;
                    } else {
                        //std::cout << "contracted" << std::endl;
                    }
                    (*iter)--;
                }
                
            }
        }


};

} //namespace RAT

#endif
