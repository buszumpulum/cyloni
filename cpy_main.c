#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <math.h>

#define ROOT 0
#define MSG_TAG 100
#define ROUNDS 1000

int main(int argc,char **argv)
{
    int size,tid;

    MPI_Init(&argc, &argv); 

    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &tid );
    printf("Whatever...\n");

    srand( tid );

    int res;

    if ( tid == 0 ) {
	MPI_Status status;
        int i;
        long double pi = 0.0;
	int hits = 0;
	for(i=1;i<size;i++)
	{
		MPI_Recv( &res, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		hits += res;
		double temp = 1.0f*hits;
		pi = 4. * temp/(ROUNDS*i);
        	printf("Przybliżenie pi po zebraniu danych od %d procesów wynosi %Lf\n", i, pi);
	}
    } else {
	int hits = 0;
	long i;
	for(i=0;i<ROUNDS;i++)
	{
		long double x = ((long double)rand() / (RAND_MAX)) * 2 - 1;
		long double y = ((long double)rand() / (RAND_MAX)) * 2 - 1;
		x *= x;
		y *= y;
		if( x+y <= 1.0)
			hits++;
	}
	res=hits;
	MPI_Send( &res, 1, MPI_INT, ROOT, MSG_TAG, MPI_COMM_WORLD );
    }

    MPI_Finalize();
}
