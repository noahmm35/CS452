#include <iostream>
#include <stdio.h>
#include <string.h>
#include <fstream>
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
    int size=0;
	char hold;
	int count[26];

	ifstream initInput("book.txt"); //initial ifstream to get size of book
	if(initInput.is_open())
	{
		while(!initInput.eof())
		{
			initInput >> hold;
			size++;
		}

		size--;
		initInput.close();
		//cout << "Size: " << size << endl;

		char * book = new char [size]; //create char array of given size

		if(my_rank==0) //fill the array
		{
			ifstream input("book.txt"); //second ifstream to actually save book
			if(input.is_open())
			{
				for(int i=0;i<size;i++)
				{
					input >> hold;
					book[i] = hold;
				}
				input.close();
			}
			else
				cout << "Oops... could not open file!!" << endl;
		}

		//lets break it up
		int localSize = size/p; //NOTE: code only works when localSize is an integer (the quotient cannot be decimal)
		char * localBook = new char[localSize];

   		MPI_Scatter(book, localSize, MPI_CHAR, localBook, localSize, MPI_CHAR, 0, MPI_COMM_WORLD);

		//setup for counting letter frequency
		int localCount[26];
		memset(localCount, 0, sizeof(localCount)); //set all indexes in count to 0

		//actual counting
		for (int i = 0; i < localSize; i++)
        	localCount[localBook[i] - 'a']++; 
		
		MPI_Allreduce(&localCount, &count, 26, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

		char place = 'a'; //used to rotate thru letters when printing results
		if(my_rank==0)
		{
			for (int i = 0; i < 26; i++) 
			{
				if(count[i]!=0)
					cout << place << count[i] << " ";
				place++;
			}

			cout << endl;
		}
		
		//clean up
		delete [] book;
		delete [] localBook;
	}
	else
		cout << "Oops... could not open file!" << endl;
	
	// Shut down MPI
	MPI_Finalize();

	return 0;
}
