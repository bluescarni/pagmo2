.. _algorithms:

List of algorithms (UDAs) available in pagmo/pygmo
==================================================

This is the list of user defined algorithms (UDAs) currently provided with pagmo/pygmo. These are classes that 
can be used to construct a :cpp:class:`pagmo::algorithm`, or :class:`pygmo.algorithm` which will then provide a unified 
interface to acces their functionalities.

Each algorithm can be
associated only to problems of certain types S = Single, M = Multi-objective, C = Constrained, U = Unconstrained, sto = stochastic

Heuristic Global Optimization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
========================================================== ========================================= ========================================= =============== 
Common Name                                                Docs of the C++ class                     Docs of the python class                  Type            
========================================================== ========================================= ========================================= =============== 
The null algorithm                                         :cpp:class:`pagmo::null_algorithm`        :class:`pygmo.null_algorithm`             SM-CU           
Differential Evolution (DE)                                :cpp:class:`pagmo::de`                    :class:`pygmo.de`                         S-U            
Self-adaptive DE (jDE and iDE)                             :cpp:class:`pagmo::sade`                  :class:`pygmo.sade`                       S-U            
Self-adaptive DE (de_1220 aka pDE)                         :cpp:class:`pagmo::de1220`                :class:`pygmo.de1220`                     S-U             
Particle Swarm Optimization (PSO)                          :cpp:class:`pagmo::pso`                   :class:`pygmo.pso`                        S-U             
(N+1)-ES Simple Evolutionary Algorithm                     :cpp:class:`pagmo::sea`                   :class:`pygmo.sea`                        S-U (sto)       
Simple Genetic Algorithm                                   :cpp:class:`pagmo::sga`                   :class:`pygmo.sga`                        S-U (sto)       
Corana's Simulated Annealing (SA)                          :cpp:class:`pagmo::simulated_annealing`   :class:`pygmo.simulated_annealing`        S-U             
Artificial Bee Colony (ABC)                                :cpp:class:`pagmo::bee_colony`            :class:`pygmo.bee_colony`                 S-U             
Covariance Matrix Adaptation Evo. Strategy (CMA-ES)        :cpp:class:`pagmo::cmaes`                 :class:`pygmo.cmaes`                      S-U             
Non-dominated Sorting GA (NSGA2)                           :cpp:class:`pagmo::nsga2`                 :class:`pygmo.nsga2`                      M-U             
Multi-objective EA vith Decomposition (MOEA/D)             :cpp:class:`pagmo::moead`                 :class:`pygmo.moead`                      M-U             
========================================================== ========================================= ========================================= ===============

Meta-algorithms 
^^^^^^^^^^^^^^^
================================== ============================================ ============================================ =============== 
Common Name                        Docs of the C++ class                        Docs of the python class                     Type            
================================== ============================================ ============================================ =============== 
Monotonic Basin Hopping (MBH)      :cpp:class:`pagmo::mbh`                      :class:`pygmo.mbh`                           S-CU           
Cstrs Self-Adaptive                :cpp:class:`pagmo::cstrs_self_adaptive`      :class:`pygmo.cstrs_self_adaptive`           S-C            
Augmented Lagrangian algorithm     :cpp:class:`pagmo::nlopt`                    :class:`pygmo.nlopt`                         S-CU           
================================== ============================================ ============================================ =============== 

Local optimization 
^^^^^^^^^^^^^^^^^^
====================================================== ========================================= ========================================= =============== 
Common Name                                            Docs of the C++ class                     Docs of the python class                             Type           
====================================================== ========================================= ========================================= =============== 
Compass Search (CS)                                    :cpp:class:`pagmo::compass_search`        :class:`pygmo.compass_search`             S-CU           
COBYLA (from NLopt)                                    :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-CU           
BOBYQA (from NLopt)                                    :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
NEWUOA + bound constraints (from NLopt)                :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
PRAXIS (from NLopt)                                    :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
Nelder-Mead simplex (from NLopt)                       :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
sbplx (from NLopt)                                     :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
MMA (Method of Moving Asymptotes) (from NLopt)         :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-CU           
CCSA (from NLopt)                                      :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-CU           
SLSQP (from NLopt)                                     :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-CU           
low-storage BFGS (from NLopt)                          :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
preconditioned truncated Newton (from NLopt)           :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
Shifted limited-memory variable-metric (from NLopt)    :cpp:class:`pagmo::nlopt`                 :class:`pygmo.nlopt`                      S-U            
Ipopt                                                  :cpp:class:`pagmo::ipopt`                 :class:`pygmo.ipopt`                      S-CU           
====================================================== ========================================= ========================================= =============== 
