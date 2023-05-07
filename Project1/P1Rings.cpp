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
	
	// THE REAL PROGRAM IS HERE
    
    //seperate ring1 from ring2 (odds and evens)
    if(my_rank%2==0) {
        char * ring1 = message;
        sprintf(ring1, "Those worthy enough to carry Ring 1: ");

        if(my_rank==0) {
            cout << "Ring 1 off to... " << my_rank << endl;
            sprintf(ring1+strlen(ring1), "%d - ", my_rank);

            MPI_Send(ring1,strlen(ring1)+1,MPI_CHAR,my_rank+2,tag,MPI_COMM_WORLD);

            if(p%2==0) //with odd number of processors, last even is one away, not two
                MPI_Recv(ring1, 100, MPI_CHAR, p-2, tag, MPI_COMM_WORLD, &status);
            else
                MPI_Recv(ring1, 100, MPI_CHAR, p-1, tag, MPI_COMM_WORLD, &status);

            sprintf(ring1+strlen(ring1), "%d!", my_rank);
            cout << ring1 << endl; //yay
        }
        else {
            MPI_Recv(ring1, 100, MPI_CHAR, my_rank-2, tag, MPI_COMM_WORLD, &status);
            sprintf(ring1+strlen(ring1), "%d - ", my_rank);
            cout << "Ring 1 off to... " << my_rank << endl;

            if((my_rank+2)>p)
                MPI_Send(ring1, strlen(ring1)+1, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
            else
                MPI_Send(ring1, strlen(ring1)+1, MPI_CHAR, (my_rank+2)%p, tag, MPI_COMM_WORLD);
            //cout << ring1 << endl;
        }
    }
    else {
        char * ring2 = message;
        sprintf(ring2, "Those worth enough to carry Ring 2: ");

        if(my_rank==1) {
            cout << "Ring 2 off to... " << my_rank << endl;
            sprintf(ring2+strlen(ring2), "%d - ", my_rank);

            if(p%2==0)
                MPI_Send(ring2,strlen(ring2)+1,MPI_CHAR,p-1,tag,MPI_COMM_WORLD);
            else
                MPI_Send(ring2,strlen(ring2)+1,MPI_CHAR,p-2,tag,MPI_COMM_WORLD);

            MPI_Recv(ring2, 100, MPI_CHAR, my_rank+2, tag, MPI_COMM_WORLD, &status);
            sprintf(ring2+strlen(ring2), "%d!", my_rank);
            cout << ring2 << endl; //yay
        }
        else {
            if((my_rank+2)>=p)
                MPI_Recv(ring2, 100, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &status);
            else
                MPI_Recv(ring2, 100, MPI_CHAR, my_rank+2, tag, MPI_COMM_WORLD, &status);

            sprintf(ring2+strlen(ring2), "%d - ", my_rank);
            cout << "Ring 2 off to... " << my_rank << endl;
            MPI_Send(ring2, strlen(ring2)+1, MPI_CHAR, my_rank-2, tag, MPI_COMM_WORLD);
            //cout << ring1 << endl;
        }
    }

	// Shut down MPI
	MPI_Finalize();

	return 0;
}
