#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <string.h>
#include "queue.h"

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
int institutes=10;
int cylons=30;
int my_cylon=0;
int my_institute=0;
MPI_Datatype mpi_msg_type;
int size, tid;

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
void init_msg_struct()
{
  int blocklengths[4] = {1,1,1,1};
  MPI_Datatype types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
  MPI_Aint offsets[4];

  offsets[0] = offsetof(msg, id);
  offsets[1] = offsetof(msg, lamport);
  offsets[2] = offsetof(msg, institute);
  offsets[3] = offsetof(msg, meeting);

  MPI_Type_create_struct(4, blocklengths, offsets, types, &mpi_msg_type);
  MPI_Type_commit(&mpi_msg_type);
}

void parse_params(int argc, char **argv)
{
  int i;
  for(i=1;i<argc-1;i++)
  {
    if(strcmp(argv[i],"-i")==0)
    {
      institutes = atoi(argv[i+1]);
    }
    if(strcmp(argv[i],"-c")==0)
    {
      cylons = atoi(argv[i+1]);
    }
  }
}

void initialization(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  MPI_Comm_size( MPI_COMM_WORLD, &size);  
  init_msg_struct();
  MPI_Comm_rank( MPI_COMM_WORLD, &tid);
    
  parse_params(argc, argv);
  
  queues = malloc((size_t)((institutes)+1)*sizeof(r_queue*));
  int i;
  for(i=0;i<(institutes)+1;i++)
    queues[i] = queue_init(i);
  
  srand(clock()*tid);
  
  MPI_Barrier(MPI_COMM_WORLD);
}

void rand_institute()
{
  my_institute = rand()%(institutes+1);//(2*institutes)-institutes+1;
  my_instytute = my_institute < 0 ? 0 : my_institute;
}

void send_request()
{
  int i;
  msg message;
  message.id=tid;
  message.lamport=++lamport_clock;
  message.institute=my_institute;
  for(i=0;i<size;i++)
  if(j!=tid)
  {
    lamport_clock++;
    MPI_Send( &message, 1, mpi_msg_type, j, MSG_REQUEST, MPI_COMM_WORLD);
  }
}

void collect_requests_and_respond()
{
  msg message;
  message.id=tid;
  int i;
  int expected_messages = size-1;
  for(i=i;i<institutes;i++)
    expected_messages-=queue_size(queue[i]);
  
  for(i=0;i<expected_messages;i++)
  {
    msg res;
    MPI_Status status;
    lamport_clock++;
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_REQUEST, MPI_COMM_WORLD, &status);
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
    printf("%d : [%d] recieved request from %d(%d) : institute=%d\n", lamport_clock, tid, res.id, res.lamport, res.institute);
    queue_add(queues[res.institute], res.id, res.lamport);
    
    message.lamport=++lamport_clock;
    MPI_Send( &message, 1, mpi_msg_type, res.id, MSG_RESPONSE, MPI_COMM_WORLD);
    printf("%d : [%d] sended respond to %d\n", lamport_clock, tid, res.id);
  }
}

void collect_responses()
{
  msg res;
  int i;
  MPI_Status status;
  for(i=0;i<institutes;i++)
  {
    lamport_clock++;
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_REQUEST, MPI_COMM_WORLD, &status);
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
  }
  printf("%d : [%d] collected all responses\n", lamport_clock, tid);
}

void calculate_meeting_number()
{
  int active_institutes=0;
  int my_rank = 1;
  int my_inst_id, my_inst_lamport;
  
  queue_top(queues[my_institute], &my_inst_id, &my_inst_lamport);
  
  for(i=1;j<institutes+1;i++)
  {
    int inst_id, inst_lamport;
    queue_top(queues[i], &inst_id, &inst_lamport);
    if((inst_lamport<my_inst_lamport) || ((inst_lamport==my_inst_lamport) && (inst_id<my_inst_id)))
      my_rank++;
    if(queue_size(queue[i]>0)
      active_institutes++;
  }
    
  my_meeting=max_known_meeting_id+my_rank;
  max_known_meeting_id+=active_institutes;
}

void calculate_cylon_number()
{
  my_cylon=1+queue_position(queue[my_institute], tid);
}

void send_ready()
{
  int i;
  int q_size = queue_count_below_limit(queues[my_institute], cylons);
  for(i=0;i<q_size;i++)
    {
      int needed_id = queue_get_id(queues[my_institute], j);
      if(needed_id!=tid)
	MPI_Send( &message, 1, mpi_msg_type, needed_id, MSG_READY, MPI_COMM_WORLD);
    }
}

void collect_readys()
{
  int i;
  int q_size = queue_count_below_limit(queues[my_institute], cylons);
  for(i=0;i<q_size-1;i++)
  {
    msg res;
    MPI_Status status;
    lamport_clock++;
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_READY, MPI_COMM_WORLD, &status);  
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
    }
}

void send_relase()
{
  int i;
  msg message;
  message.id=tid;
  message.institute=my_institute;
  queue_remove(queues[my_institute], tid);
  lamport_clock++;
  message.lamport=lamport_clock;
  for(i=0;i<size;i++)
    if(i!=tid)
      MPI_Send( &message, 1, mpi_msg_type, i, MSG_RELASE, MPI_COMM_WORLD);
}

void collect_relases()
{
  int q_size=0;
  int i;
  for(i=1;i<institutes+1;i++)
    q_size+=queue_count_below_limit(queue[i], cylons);
  for(i=0;i<size;i++)
  {  
    if(j==tid) 
      continue;
    msg res;
    MPI_Status status;
    lamport_clock++;
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_FREE, MPI_COMM_WORLD, &status);
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;  
    queue_remove(queues[res.institute], res.id);
  }
}

void main_loop()
{
  int i=0; 
  while(i<2)
  {
    //SET INSTITUTE
    rand_institute();
    
    //SEND REQUEST
    send_request();
    queue_add(queues[my_institute], message.id, message.lamport);
     
    //REVIEVE REQUESTS AND SEND RESPONDS
    collect_requests_and_respond();
    
    //RECIEVE RESPONSES FROM ALL
    collect_responses();
    
    //RECIEVE READY FROM ALL COMPANIONS
   
    
    //ACTUAL MEETING
    sleep(rand()%4);
    
    //SEND FREE TO ALL
    
    
    //RECIEVE FREE FROM ALL
    
    } 
      
    i++;
  }
}

void cleanup()
{
  MPI_Type_free(&mpi_msg_type);
  
  int i;
    for(i=0;i<institutes+1;i++)
      queue_free(queues[i]);
    MPI_Finalize();
}

int main(int argc,char **argv)
{
    initialization(argc, argv);    
    
    main_loop();
    
    cleanup();
    
    return 0;
}
