#include <iostream>
#include <stdio.h>
#include <string.h>
#include <array>
#include <cmath>
#include "mpi.h" // message passing interface
using namespace std;

// New compile and run commands for MPI!
// mpicxx -o blah file.cpp
// mpirun -q -np 32 blah

void mergesort (int * a, int first, int last);
void smerge(int * a, int * b, int lasta, int lastb, int * output);
int Rank(int * a, int first, int last, int valToFind);
void pmerge(int * a, int * b, int lasta, int lastb, int * output = NULL);

int my_rank;
int p;

int main (int argc, char * argv[]) {
					
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
    srand(69420);

    int n = 0;

	//read in size from user
	if(my_rank == 0)
	{
		cout << "Please enter the size of your array (must be power of 2): ";
		cin >> n;
	}

	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	int * array = new int[n];
    bool used[501] = {false}; //ensures no repeats
	
	//fill array with random numbers
	if(my_rank == 0)
	{
    	//fill it with numbers 1-500
		for (int i=0;i<n;i++) {
        	int number = rand() % 500 + 1;

        	while(used[number])
            	number = rand() % 500 + 1;
        	used[number] = true;

       		array[i] = number;
    	}

		cout << "Unsorted Array: " << endl;
		for(int i=0; i<n; i++)
			cout << array[i] << " ";
		cout << endl;
	}

	//share results with everyone
	MPI_Bcast(array, n, MPI_INT, 0, MPI_COMM_WORLD);

	mergesort(array, 0, n-1);

	if(my_rank == 0)
	{
		cout << "Sorted Array: " << endl;
		for(int i=0; i<n; i++)
		{
			cout << array[i] << " ";
		}
		cout << endl;
	}
    
    //clean up
    delete [] array;

	// Shut down MPI
	MPI_Finalize();

	return 0;
}

void mergesort (int * a, int first, int last)
{
	if(first < last && last-first+1 >= 4)
	{
		int mid = ((first+last)/2);
		mergesort(a, first, mid);
		mergesort(a, mid+1, last);
		pmerge(&a[first], &a[mid+1], mid-first, last-mid-1);
	}
	else
	{
		//if we come in here, there are just two elements. So swap if necessary
		if(a[last] < a[first])
		{
			int temp = a[last];
			a[last] = a[first];
			a[first] = temp;
		}
		return;
	}
}

void smerge(int * a, int * b, int lasta, int lastb, int * output)
{
	int size = lasta+lastb;
	int i=0, j=0, k=0;

	while(i<lasta && j<lastb)
	{
		if(a[i] < b[j]) {
			output[k] = a[i];
			k++; i++;
		}
		else
		{
			output[k] = b[j];
			k++; j++;
		}
	}

	while(i<lasta)
	{
		output[k] = a[i];
		k++; i++;
	}

	while(j<lastb)
	{
		output[k] = b[j];
		k++; j++;
	}
}

int Rank(int * a, int first, int last, int valToFind)
{
	while (first<=last)
    {
        int mid = (first+last) / 2;
        
        if(valToFind == a[mid]) 		//value is present in array
            return mid;
        else if (valToFind > a[mid]) 	//value is on right side
            first = mid+1;
        else                        	//value is on the left side
            last = mid-1;
    }
    return first; //not found in array, so give position where it would go
}

void pmerge(int * a, int * b, int lasta, int lastb, int * output)
{
	//setup
	int size = lasta + lastb +2;

	int win[size] = {0};
	int localWin[size] = {0};
	int numSamples = (size/2)/log2(size/2);

	//all arrays initialized to zero to make sure sum works when reducing
	int ASelect[numSamples] = {0};
	int BSelect[numSamples] = {0};
	int SRankA[numSamples] = {0};
	int SRankB[numSamples] = {0};
	int localRA[numSamples] = {0};
	int localRB[numSamples] = {0};

	//find values for ASelect and BSelect
	for(int i=0;i<numSamples;i++)
	{
		int pos = i * log2(size/2);
		ASelect[i] = a[pos];
		BSelect[i] = b[pos];
	}
		
	//calc SRankA and SRankB in parallel
	for(int i=my_rank; i<numSamples; i+=p)
	{
		localRA[i] = Rank(b,0,size/2-1,ASelect[i]);
		localRB[i] = Rank(a,0,size/2-1,BSelect[i]);

		//place these values into win
		int pos = i * log2(size/2);
		localWin[localRA[i] + pos] = ASelect[i];
		localWin[localRB[i] + pos] = BSelect[i];
	}

	//bring it all back together
	MPI_Allreduce(&localRA, &SRankA, numSamples, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	MPI_Allreduce(&localRB, &SRankB, numSamples, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	//oh hey look, it is time for shapes
	//premise is to sort the pivots and all shapes are bound by two elements within sorted Select
	int Select[numSamples*2] = {0};

	int x = 0;
	int y = 0;

	if(my_rank == 0)
	{
		//sort partitions from ASelect and BSelect
		smerge(&ASelect[0], &BSelect[0],numSamples,numSamples,&Select[0]);
	}

	// Sharing is caring
	MPI_Bcast(&Select, numSamples*2, MPI_INT, 0, MPI_COMM_WORLD);
	
	int startA=0;
	int startB=0;
	
	for(int i=my_rank; i<numSamples*2; i+=p)
	{
			//look thru array to find start point in both arrays
			while(a[startA] <= Select[i] && startA < size/2)
			{
				startA+=1;
			}

			while(b[startB] <= Select[i] && startB < size/2)
			{
				startB+=1;
			}

			int endA = startA;
			int endB = startB;

			// this if is for the final shape
			if(i>= (numSamples*2)-1)
			{
				//look for ending
				while(endA < size/2)
					endA+=1;
				while(endB < size/2)
					endB+=1;

				//smerge the shapes found
				smerge(&a[startA], &b[startB], endA-startA, endB-startB, &localWin[size-((endA-startA)+(endB-startB))]);
			}
			else
			{
				//look for ending
				while(a[endA] < Select[i+1] && endA < size/2)
					endA++;
				while(b[endB] < Select[i+1] && endB < size/2)
					endB++;

				//smerge shapes found				
				smerge(&a[startA], &b[startB], endA-startA, endB-startB, &localWin[startA+startB]);
			}
	}

	MPI_Allreduce(&localWin, &win, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	//place final answer back into a
	for(int i=0; i<size; i++)
		a[i] = win[i];
}
