# Tower Allocation
This is the code for the first assignment of <a href="http://www.cse.iitd.ac.in/~mausam/courses/col333/autumn2016/">COL333:Artificial Intelligence</a>. The problem statement can be found <a href="http://www.cse.iitd.ac.in/~mausam/courses/col333/autumn2016/A1/A1.pdf">here</a>. We have used a Beam Search algorithm with some parallelization introduced to make the search more optimal.  

## Algorithm
After trying HillClimibingWithRandomRestarts, HillClimbingwithTabu, BeamSearch,
BeamSearchWithTabu and BeamSearchWithRandomRestarts; we chose
BeamSearchWithRandomRestarts. This algorithm gave the best costs consistently for the
maximum no of test cases that were given. We also used a tabu list in this of size 20, thus
making the algorithm more general.
The one trick that allowed us to get maximum increase in the quality of results was making a
better neighbourhood function for the states we defined. To generate the neighbours we would
remove any given bid say at position i, after this we add bids from i+1 to n (if possible). This
allowed us to have the maximal state at any given time. However this wasn’t complete as we
ignored the bids from 1 to i-1 and also didn’t try all the permutations of bids from i+1 to n

## Optimizations
We used multi threading, where we ran 3 threads with random initial states and used these
threads to update a common variable maxValue which contained the best of all the running
threads. The three parallel threads made a random initial state more useful, as the randomness
across multiple threads allowed a more general approach to beam search.
We forgot to use the -O3 flag and didn’t calculate the value of the state in a delta fashion ( to
see the incremental change), instead we calculated it from scratch every time. These
optimisations could have made the code perform even better.
