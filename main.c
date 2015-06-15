#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <string.h>
#include "queue.h"

#define DEF_INSTITUTE 10
#define DEF_CYLON 30

#define MSG_REQUEST 100
#define MSG_RESPONSE 200
#define MSG_MEETING 300
#define MSG_FREE 400
#define MSG_READY 500

typedef struct msg{
  int id;
  int lamport;
  int institute;
  int meeting;
} msg;

r_queue **queues;
int lamport_clock=0;
int max_known_meeting_id=0;

void print_all(r_queue* queue, int id)
{
  int i = 0;
  queue_entry* entry = queue->entries;
  entry = entry->next;
  while(entry!=NULL)
  {
    printf("[%d][%d] %d %d %d %d\n", id, i++,entry->id, entry->lamport_clock, entry->meeting_id, entry->cylon_id);
    entry=entry->next;
  }
}

//PHASE 1 INITIALIZATION
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
  
  queues = malloc((size_t)((*insts)+1)*sizeof(r_queue*));
  int i;
  for(i=0;i<(*insts)+1;i++)
    queues[i] = queue_init(i);
  
  srand(clock()*(*tid));
  MPI_Barrier(MPI_COMM_WORLD);
}

//PHASE 2: INSTITUTE DRAWING
int rand_institute(int institutes)
{
  int institute = rand()%(institutes+1);//(2*institutes)-institutes+1;
  return institute < 0 ? 0 : institute;
}



void main_loop(int size, int tid, int institutes, int cylons, MPI_Datatype msg_struct)
{
  //BEGIN PHASE INDICATORS
    //BEGIN STEPS
  int institute_draw=1;
  int request_send,respond_recieve,meeting_setting=0;
    //END STEPS
    //BEGIN COMMUNICATION
  
    //END COMMUNICATION
  //END PHASE INDICATORS
  int i=0; 
  while(i<2)
  {
    //SET INSTITUTE
    int institute = rand_institute(institutes);
      
    //printf("[#%d:%d] Narada w instytucie: %d\n", tid+1, i, institute);
    
    msg message;
    message.id=tid;
    message.lamport=lamport_clock;
    message.institute=institute;
    message.meeting=0;
    queue_add(queues[institute], message.id, message.lamport);
    
    //SEND REQUEST TO ALL
    int j;
    lamport_clock++;
    for(j=0;j<size;j++)
      if(j!=tid)
	MPI_Send( &message, 1, msg_struct, j, MSG_REQUEST, MPI_COMM_WORLD);
     
    //REVIEVE REQUESTS FROM ALL AND SEND RESPONDS
    for(j=0;j<size;j++)
    {
      if(j==tid) continue;
      msg res;
      MPI_Status status;
      lamport_clock++;
      MPI_Recv( &res, 1, msg_struct, MPI_ANY_SOURCE, MSG_REQUEST, MPI_COMM_WORLD, &status);
      lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
      //printf("[%4d:%4d] Recv request id:%4d lamport:%4d institute:%4d meeting:%4d\n", lamport_clock, tid, res.id, res.lamport, res.institute, res.meeting);
      queue_add(queues[res.institute], res.id, res.lamport);
      message.lamport=++lamport_clock;
      MPI_Send( &message, 1, msg_struct, j, MSG_RESPONSE, MPI_COMM_WORLD);
    }
    
    //RECIEVE RESPONDS FROM ALL
    for(j=0;j<size;j++)
    {
      if(j==tid) continue;
      msg res;
      MPI_Status status;
      lamport_clock++;
      MPI_Recv( &res, 1, msg_struct, MPI_ANY_SOURCE, MSG_RESPONSE, MPI_COMM_WORLD, &status);
      lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
      //printf("[%4d:%4d] Recv respond id:%4d lamport:%4d\n", lamport_clock, tid, res.id, res.lamport);
    }
    
    //COMPARE TOPS RANKS
    int my_rank = 1;
    int my_inst_id, my_inst_lamport;
    queue_top(queues[institute], &my_inst_id, &my_inst_lamport);
    for(j=1;j<institutes+1;j++)
    {
      int inst_id, inst_lamport;
      queue_top(queues[j], &inst_id, &inst_lamport);
      if((inst_lamport<my_inst_lamport) || ((inst_lamport==my_inst_lamport) && (inst_id<my_inst_id)))
	my_rank++;
    }
    //printf("[%4d:%4d] My_rank:%4d, institute:%4d\n", lamport_clock, tid, my_rank, institute);
    
    //SET MEETING ID
    int my_meeting = max_known_meeting_id+my_rank;
    
    //SET MAX_KNOWN_MEETING_ID
    max_known_meeting_id+=my_rank;
    //printf("[%4d:%4d] institute:%4d meeting:%4d\n", lamport_clock, tid, institute, my_meeting);
    message.meeting=my_meeting;
    
    //SEND MEETING NUMBER TO ALL
    lamport_clock++;
    message.lamport=lamport_clock;
    for(j=0;j<size;j++)
      if(j!=tid)
	MPI_Send( &message, 1, msg_struct, j, MSG_MEETING, MPI_COMM_WORLD);
    
    //RECIEVE MEETINGS NUMBERS FROM ALL
    for(j=0;j<size;j++)
    {
      if(j==tid) continue;
      msg res;
      MPI_Status status;
      lamport_clock++;
      MPI_Recv( &res, 1, msg_struct, MPI_ANY_SOURCE, MSG_MEETING, MPI_COMM_WORLD, &status);
      lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
      if(res.institute != 0)
      max_known_meeting_id=res.meeting > max_known_meeting_id ? res.meeting : max_known_meeting_id;
      //printf("[%4d:%4d] Recv meeting id:%4d lamport:%4d meeting:%4d\n", lamport_clock, tid, res.id, res.lamport, res.meeting);
      queue_set_meeting(queues[res.institute], res.id, res.meeting);
    }
    
    //if(institute==1)
    //  print_all(queues[institute], tid);
    
    //SET CYLON ID
    int my_cylon=queue_position(queues[institute], tid)+1;
    if(institute!=0)
    printf("[ %3d %3d ] institute:%4d meeting:%4d cylon:%4d\n", lamport_clock, tid, institute, my_meeting, my_cylon);
    
    //SEND READY TO ALL COMPANIONS
    int q_size = queue_size(queues[institute]);
    for(j=0;j<q_size;j++)
    {
      int needed_id = queue_get_id(queues[institute], j);
      if(needed_id!=tid)
	MPI_Send( &message, 1, msg_struct, needed_id, MSG_READY, MPI_COMM_WORLD);
    }

    
    //RECIEVE READY FROM ALL COMPANIONS
    for(j=0;j<q_size-1;j++)
    {
      msg res;
      MPI_Status status;
      lamport_clock++;
      MPI_Recv( &res, 1, msg_struct, MPI_ANY_SOURCE, MSG_READY, MPI_COMM_WORLD, &status);
      lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
    }
    
    //ACTUAL MEETING
    sleep(rand()%4);
    
    //SEND FREE TO ALL
    queue_remove(queues[institute], tid);
    lamport_clock++;
    message.lamport=lamport_clock;
    for(j=0;j<size;j++)
      if(j!=tid)
	MPI_Send( &message, 1, msg_struct, j, MSG_FREE, MPI_COMM_WORLD);
    
    //RECIEVE FREE FROM ALL
    for(j=0;j<size;j++)
    {
      if(j==tid) continue;
      msg res;
      MPI_Status status;
      lamport_clock++;
      MPI_Recv( &res, 1, msg_struct, MPI_ANY_SOURCE, MSG_FREE, MPI_COMM_WORLD, &status);
      lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
      queue_remove(queues[res.institute], res.id);
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
    
    //printf("%d %d\n", institutes_count, cylons_count);
    
    main_loop(size, tid, institutes_count, cylons_count, msg_struct);
    
    MPI_Type_free(&msg_struct);
    
    int i;
    for(i=0;i<institutes_count+1;i++)
      queue_free(queues[i]);
    MPI_Finalize();
    return 0;
}
