#include <iostream>
#include <stdio.h>
#include <string.h>
#include "mpi.h" // message passing interface
using namespace std;

// New compile and run commands for MPI!
// mpicxx -o blah file.cpp
// mpirun -q -np 32 blah

int main (int argc, char * argv[]) {

	int my_rank;			// my CPU number for this process
	int p;					/* number of CPUs that we have */
	int source;				/* rank of the sender */
	int dest;				/* rank of destination */
	int tag = 0;			// message number
	char message[100];		// message itself
	MPI_Status status;		// return status for receive
	
	// Start MPI
	MPI_Init(&argc, &argv);
	
	// Find out my rank!
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	// Find out the number of processes!
	MPI_Comm_size(MPI_COMM_WORLD, &p);
	
	// THE REAL PROGRAM IS HERE//
    // setup for problem
	int seed = 71911;
	srand(seed);
	int n = 1000000;
	int * k = new int[n];
    int max = 0;
    int min = k[0];
    long int sum = 0;

    //fill arrays
    if (my_rank == 0)
		for (int x = 0; x < n; x++)
            k[x] = rand() % 50 +1; //set to generate random number 1-50

    //break it up
    int localn = n/p; 
    int * localk = new int[localn];

    MPI_Scatter(k, localn, MPI_INT, localk, localn, MPI_INT, 0, MPI_COMM_WORLD);

    //local work
    //Max and Min
    int localMax = 0;
    int localMin = localk[0];
    long int localSum = 0;
    for(int i=0; i<localn; i++)
    {
        if(localk[i]>localMax)
            localMax = localk[i];

        if(localk[i]<localMin)
            localMin = localk[i];

        localSum+=localk[i];
    }

    //cout << my_rank << " Max: " << localMax << endl;
    //cout << my_rank << " Min: " << localMin << endl;
    //cout << my_rank << " Sum: " << localSum << endl;

    //bring everything back
    MPI_Reduce(&localMax, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&localMin, &min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&localSum, &sum, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if(my_rank==0)
    {
        cout << "Max: " << max << endl;
        cout << "Min: " << min << endl;
        cout << "Average: " << sum/n << endl;
    }

    //clean up cuz mem leaks are bad
    delete [] k;
    delete [] localk;

	// Shut down MPI
	MPI_Finalize();

	return 0;
}
