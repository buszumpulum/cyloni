#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <string.h>
//#include "queue.h"

#define DEF_INSTITUTE 10
#define DEF_CYLON 30

#define MSG_REQUEST 100
#define MSG_RESPOSE 200
#define MSG_MEETING 300
#define MSG_FREE 400
#define MSG_READY 500

typedef struct msg{
  int id;
  int lamport;
  int institute;
  int meeting;
} msg;

void init_msg_struct(MPI_Datatype *mpi_msg_type)
{
  int blocklengths[4] = {1,1, 1, 1};
  MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  MPI_Aint offsets[4];

  offsets[0] = offsetof(msg, id);
  offsets[1] = offsetof(msg, lamport);
  offsets[2] = offsetof(msg, institute);
  offsets[3] = offsetof(msg, meeting);

  MPI_Type_create_struct(4, blocklengths, offsets, types, mpi_msg_type);
  MPI_Type_commit(mpi_msg_type);
}

void parse_params(int argc, char **argv, int *insts, int *cylons)
{
  *insts = DEF_INSTITUTE;
  *cylons = DEF_CYLON;
  int i;
  for(i=1;i<argc-1;i++)
  {
    if(strcmp(argv[i],"-i")==0)
    {
      *insts = atoi(argv[i+1]);
    }
    if(strcmp(argv[i],"-c")==0)
    {
      *cylons = atoi(argv[i+1]);
    }
  }
}

void initialization(int argc, char **argv, int *size ,int *tid, int *insts, int *cylons, MPI_Datatype *data)
{
  MPI_Init(&argc, &argv);
    
  MPI_Comm_size( MPI_COMM_WORLD, size );
    
  init_msg_struct(data);
    
  MPI_Comm_rank( MPI_COMM_WORLD, tid );
    
  parse_params(argc, argv, insts, cylons); 
  srand(clock()*(*tid));
  sleep(rand()%3);
  MPI_Barrier(MPI_COMM_WORLD);
}

void main_loop(int size, int tid, int institutes, int cylons, MPI_Datatype msg_struct)
{
  int i=0; 
  while(i<2)
  {
    int institute = rand()%(2*institutes)-institutes+1;
    institute = institute < 0 ? 0 : institute;
      
    printf("[#%d:%d] Narada w instytucie: %d\n", tid+1, i, institute);
    
    msg message;
    message.id=tid;
    message.lamport=10;
    message.institute=institute;
    message.meeting=0;
    
    int j;
    for(j=0;j<size;j++)
      if(j!=tid)
	MPI_Send( &message, 1, msg_struct, j, MSG_REQUEST, MPI_COMM_WORLD);
      
    for(j=0;j<size;j++)
    {
      if(j==tid) continue;
      msg res;
      MPI_Status status;
      MPI_Recv( &res, 1, msg_struct, MPI_ANY_SOURCE, MSG_REQUEST, MPI_COMM_WORLD, &status);
      printf("[%d] Recv id:%d lamport:%d institute:%d meeting:%d\n", tid, res.id, res.lamport, res.institute, res.meeting);
    }
    
    i++;
  }
}

int main(int argc,char **argv)
{
    //PROCES INICJALIZUJE ZMIENNE LOKALNE
    int size,tid, institutes_count, cylons_count;
    MPI_Datatype msg_struct;
    initialization(argc, argv, &size, &tid, &institutes_count, &cylons_count,&msg_struct);    
    
    
    printf("%d %d\n", institutes_count, cylons_count);
    
    main_loop(size, tid, institutes_count, cylons_count, msg_struct);
    
    MPI_Type_free(&msg_struct);
    MPI_Finalize();
}
