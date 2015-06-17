#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <string.h>
#include "queue.h"

#define MSG_REQUEST 100
#define MSG_RESPONSE 200
#define MSG_RELASE 300
#define MSG_READY 400

#define TRUE 1
#define FALSE 0

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
int my_meeting=0;
MPI_Datatype mpi_msg_type;
int size, tid;

void print_all(r_queue* queue)
{
  int i = 0;
  queue_entry* entry = queue->entries;
  entry = entry->next;
  while(entry!=NULL)
  {
    printf("[%d][%d] %d %d %d %d\n", tid, i++,entry->id, entry->lamport_clock, entry->meeting_id, entry->cylon_id);
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
      institutes = atoi(argv[i+1]);
    
    if(strcmp(argv[i],"-c")==0)
      cylons = atoi(argv[i+1]);
  }
  if(institutes<=0)
    institutes=10;
  
  if(cylons<=0)
    cylons=30;
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

void random_institute()
{
  my_institute = rand()%(institutes+1);//(2*institutes)-institutes+1;
  my_institute = my_institute < 0 ? 0 : my_institute;
  //printf("%d : [%d] assigned to institute=%d\n", lamport_clock, tid, my_institute);
}

void send_request()
{
  int i;
  msg message;
  message.id=tid;
  message.lamport=++lamport_clock;
  message.institute=my_institute;
  queue_add(queues[my_institute], tid, lamport_clock);
  //printf("%d : [%d] sending request for institute=%d\n", lamport_clock, tid, my_institute);
  for(i=0;i<size;i++)
  if(i!=tid)
  {
    lamport_clock++;
    MPI_Send( &message, 1, mpi_msg_type, i, MSG_REQUEST, MPI_COMM_WORLD);
  }
}

void collect_requests_and_respond()
{
  msg message;
  message.id=tid;
  int i;
  int sum=0;
  int expected_messages = size-1;
  for(i=1;i<institutes+1;i++)
  {
    sum+=queue_size(queues[i]);
    if(queue_size(queues[i])>0)
      print_all(queues[i]);
  }
  printf("%d : [%d] %d still in queues (%d)\n", lamport_clock, tid, sum, expected_messages-sum+1);
  
  for(i=0;i<expected_messages-sum;i++)
  {
    msg res;
    MPI_Status status;
    lamport_clock++;
    printf("%d : [%d] (%d) requests still expected\n", lamport_clock, tid, expected_messages-i);
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_REQUEST, MPI_COMM_WORLD, &status);
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
    //printf("%d : [%d] recieved request from %d(%d) : institute=%d\n", lamport_clock, tid, res.id, res.lamport, res.institute);
    queue_add(queues[res.institute], res.id, res.lamport);
    
    message.lamport=++lamport_clock;
    MPI_Send( &message, 1, mpi_msg_type, res.id, MSG_RESPONSE, MPI_COMM_WORLD);
    //printf("%d : [%d] sent respond to %d\n", lamport_clock, tid, res.id);
  }
}

void collect_responses()
{
  msg res;
  int i;
  MPI_Status status;
  for(i=0;i<size-1;i++)
  {
    lamport_clock++;
     printf("%d : [%d] (%d) responses still expected\n", lamport_clock, tid, size-1-i);
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_RESPONSE, MPI_COMM_WORLD, &status);
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
    printf("%d : [%d] recieved response from %d\n", lamport_clock, tid, res.id);
  }
  printf("%d : [%d] collected all responses\n", lamport_clock, tid);
}

void calculate_meeting_number()
{
  int i;
  int active_institutes=0;
  int my_rank = 1;
  int my_inst_id, my_inst_lamport;
  
  queue_top(queues[my_institute], &my_inst_id, &my_inst_lamport);
  
  for(i=1;i<institutes+1;i++)
  {
    int inst_id, inst_lamport;
    queue_top(queues[i], &inst_id, &inst_lamport);
    if((inst_lamport<my_inst_lamport) || ((inst_lamport==my_inst_lamport) && (inst_id<my_inst_id)))
      my_rank++;
    if(queue_size(queues[i])>0)
      active_institutes++;
  }
  my_meeting=max_known_meeting_id+my_rank;
  max_known_meeting_id+=active_institutes;
  //printf("%d : [%d] is attempting to meeting number %d (%d) in institute %d\n", lamport_clock, tid, my_meeting, max_known_meeting_id, my_institute);
}

void calculate_cylon_number()
{
  my_cylon=1+queue_position(queues[my_institute], tid);
  //printf("%d : [%d] assigned to cylon %d at meeting %d in institute %d\n", lamport_clock, tid, my_cylon, my_meeting, my_institute);
}

void send_ready()
{
  msg message;
  message.id=tid;
  int i;
  int q_size = queue_count_below_limit(queues[my_institute], cylons);
  for(i=0;i<q_size;i++)
    {
      int needed_id = queue_get_id(queues[my_institute], i);
      if(needed_id!=tid)
      {
	message.lamport=++lamport_clock;
	MPI_Send( &message, 1, mpi_msg_type, needed_id, MSG_READY, MPI_COMM_WORLD);
      }
    }
   // printf("%d : [%d] ready to meeting number %d\n", lamport_clock, tid, my_meeting);
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
   //  printf("%d : [%d] (%d) readys still expected\n", lamport_clock, tid, q_size-1-i);
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_READY, MPI_COMM_WORLD, &status);  
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;
  }
 // printf("%d : [%d] all participants confirmed being ready to meeting %d\n", lamport_clock, tid, my_meeting);
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
  //printf("%d : [%d] relased request to visit institute %d\n", lamport_clock, tid, my_institute);
}

void collect_relases()
{
  int q_size=0;
  int i;
  for(i=1;i<institutes+1;i++)
    q_size+=queue_count_below_limit(queues[i], cylons);
  for(i=0;i<q_size;i++)
  {  
    msg res;
    MPI_Status status;
    lamport_clock++;
    MPI_Recv( &res, 1, mpi_msg_type, MPI_ANY_SOURCE, MSG_RELASE, MPI_COMM_WORLD, &status);
    lamport_clock=res.lamport > lamport_clock ? res.lamport : lamport_clock;  
    queue_remove(queues[res.institute], res.id);
    printf("%d : [%d] relased request from %d\n", lamport_clock, tid,res.id);
  }
}

void main_loop()
{
  int phase_get_institute = TRUE;
  int phase_send_request = FALSE;
  while(1)
  {
    if(phase_get_institute==TRUE)
    {
      random_institute();
      if(my_institute>0)
      {
	phase_get_institute=FALSE;
      }
    }
    
    if(phase_send_request==TRUE)
    {
      send_request();//BLOKUJĄCA
      phase_send_request=FALSE;
    }
    
    collect_requests_and_respond();
    
    collect_responses();
    
    calculate_meeting_number();
    
    if(my_institute!=0)
    {
      while(queue_position(queues[my_institute], tid)>=cylons)
      {
	printf("%d : [%d] failed to attempt meeting %d in institute %d; Waiting for relases...\n", lamport_clock, tid, my_meeting, my_institute);
	collect_relases();
	
	collect_requests_and_respond();
	
	calculate_meeting_number();
      }
	
     calculate_cylon_number();
      
     send_ready();
     
     collect_readys();
     
    // printf("%d : [%d] meeting %d started\n", ++lamport_clock, tid, my_meeting);
     sleep(rand()%5);
   //  printf("%d : [%d] meeting %d ended\n", ++lamport_clock, tid, my_meeting);
     
     send_relase();
    }
    else
     printf("%d : [%d] not invited to any institute\n", ++lamport_clock, tid);
   
    queue_remove(queues[my_institute], tid);
    
    collect_relases();
    
    queue_clean(queues[0]);
    
    my_institute=0;
    my_cylon=0;
    my_meeting=0;
    
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
