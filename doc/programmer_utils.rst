Extra Utilities
---------------

.. _simulated-annealing:

SimulatedAnnealing
``````````````````
Included in ``src/util/SimulatedAnnealing.hh`` is a general purpose algorithm for globally minimizing a D dimensional continuous
function that contains many local minima. Effectively this is a hybrid between the
`Nelder-Mead downhill simplex method <https://en.wikipedia.org/wiki/Nelder%E2%80%93Mead_method>`_ 
and `simulated annealing <https://en.wikipedia.org/wiki/Simulated_annealing>`_ as
described in section 10.9 of ``Numerical Recipes in 'C'`` but reimplemented in a 
less confusing way for C++. 


Usage
=====
The ``SimulatedAnnealing`` class is templated to the dimensionality of function 
to be minimized. The constructor ::

    SimulatedAnnealing(Minimizable *func)

takes a single argument that is an object implementing the pure virtual ``Minimizable`` class. 

``Minimizable`` only requires that the object implement the call operator as follows ::

    virtual double operator()(double *args);
    
where the result is the value of the function evaluated at function(args[0],...,args[D]).

After creating the ``SimulatedAnnealing`` object, set the initial simplex using ::

    void SetSimplexPoint(size_t pt, std::vector<double> &point)
    
where ``pt`` specifies the index of the point you are setting [0, D], i.e. D+1 required points,
and the simplex is copied from the D length vector ``point``.

Once the initial D+1 simplex points are specified, call the ``Anneal`` method to actually minimize ::

    void Anneal(double temp0, size_t nAnneal, size_t nEval, double alpha)
    
where ``temp0`` is the initial temperature and ``alpha`` controls the temperature according to the 
annealing schedule T = ``temp0`` * (1 - ``cycle`` / ``nAnneal`` ) ^ ``alpha``. The annealing is run at ``nAnneal``
different temperatures ( ``cycle`` ) where each ``cycle`` tests ``nEval`` new points.

Once the algorithm has finished ``GetBestPoint`` will return the tested point with the lowest function
value in the last ``Anneal`` cycle ::

    void GetBestPoint(std::vector<double> &point)

where the best point will be copied into the D length vector ``point``.

Example
=======

As a concrete example, to minimize f(x,y) = x^A+y^B with A=B=2, first implement the ``Minimizable`` function ::

    class Func : public Minimizable {
        public:
        double A,B;
        Func(double _A, double _B) : A(_A), B(_B) { };
        virtual double operator()(double *params) {
            return pow(params[0],A) + pow(params[1],B);
        }
    }

Then the code to minimize would look something like this ::

    Func func(2.0,2.0); //A = B = 2.0
    SimulatedAnnealing<2> anneal(func);
    
    vector<double> point(2), seed(2);
    
    seed[0] = seed[1] = -1.0;
    anneal.SetSimplexPoint(0,seed); // Point0 -> (-1,-1)
    seed[0] = 1.0;
    anneal.SetSimplexPoint(1,seed); // Point1 -> (1,-1)
    seed[1] = 1.0;
    anneal.SetSimplexPoint(2,seed); // Point2 -> (1,1)
    
    anneal.Anneal(10,150,50,4.0); // Minimize
    
    anneal.GetBestPoint(point);
    
    cout << point[0] << ',' << point[1] << endl;
    
    
    
