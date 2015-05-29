#include <stdio.h>
#include <stdlib.h>

long l_clock;
struct message rcv_message;

struct message
{
 int clock;
};

void init()
{
 l_clock=0;
 rcv_message.clock=10;
}

void lamport_send_message()
{
 l_clock++;
 //MPI_send(...);     
}

void lamport_recive_message()
{
 
 //MPI_recive(...);
 if(l_clock>rcv_message.clock){l_clock++;}
 else {l_clock=rcv_message.clock+1;}    
}

int main(int argc, char *argv[])
{
  init();
  printf("%d\n",l_clock);
  lamport_send_message();
  printf("%d\n",l_clock);
  lamport_recive_message();
  printf("%d\n",l_clock);     
  system("PAUSE");	
  return 0;
}
