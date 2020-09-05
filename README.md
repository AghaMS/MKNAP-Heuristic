# MKNAP-Heuristic
A constructive heaursitic to solve the multi-dimensional knapsack problem

The code was originally developed back in 2014 by Mohammed Alagha, coded in C++

The code is now being updated and re-coded in Python in 2020

The multidimensional knapsack problem is considered as one of the NP-hard linear integer
programs. It is one of the challenging combinatorial optimization problems. Several
algorithms have been proposed to solve this problem to optimality ranging between exact
and heuristic algorithms. The main concerns when developing a solution method are the
solution quality and the computational time required to obtain this solution, thus, this work
presents a new hybrid algorithm of linear programming, surrogate columns’ costs and
particle swarm optimization to solve the multidimensional knapsack problem. The proposed
hybrid takes advantage of the valuable information obtained from solving the relaxed linear
program version of the problem to calculate the surrogate column costs. Such valuable
information are the variables’ reduced costs and the constraints’ dual values. It then
incorporates these surrogate columns’ costs into the global search strategies of the particle
swarm algorithm to help guide the search throughout the feasible solution space. A local
search heuristic was embedded in the hybrid algorithm to further enhance the generated
solutions. The local search heuristic employs a repair operator to treat any infeasible
solutions. The proposed hybrid algorithm was tested to validate its performance. Several
computational experiments were performed on general benchmarks from the literature. The
results were compared with those of some state-of-the-art solution techniques used to solve
the multidimensional knapsack problem. The obtained results were found to be promising.
These comparisons show that the proposed hybrid is one of the successful algorithms used
for solving the multidimensional knapsack problem in terms of the quality of solutions
obtained and the CPU time requirements.
