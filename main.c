#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include "queue.h"

#define INSTS 5
#define CYLS 30

#define MSG_REQUEST 100
#define MSG_RESPOSE 200
#define MSG_MEETING 300
#define MSG_FREE 400
#define MSG_READY 500

int main(int argc,char **argv)
{
    //PROCES INICJALIZUJE ZMIENNE LOKALNE
    int size,tid;

    MPI_Init(&argc, &argv);
    
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &tid );
    
    srand(clock()*tid);
    
    //PROCES OCZEKUJE NA RESZTĘ PROCESÓW
    
    //MPI_Barrier();
    
    //PROCES LOSUJE NUMER INSTYTUTU
    int i;
    for(i=0;i<5;i++)
    {
      int institute = rand()%(2*INSTS)-INSTS+1;
      institute = institute < 0 ? 0 : institute;
      
    
      printf("[#%d:%d] Narada w instytucie: %d\n", tid+1, i, institute);
      if(institute==0)
      {
	int sleep_time = rand()%10;
	printf("[#%d] SLEEP: %d\n", tid+1,sleep_time);
	sleep(sleep_time);
	continue;
      }
      sleep(2);
    }
    MPI_Finalize();
}
