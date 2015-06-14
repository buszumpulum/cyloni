#include "queue.h"
#include <stdio.h>

void print_all(r_queue* queue)
{
  int i = 0;
  queue_entry* entry = queue->entries;
  entry = entry->next;
  while(entry!=NULL)
  {
    printf("[%d] %d %d %d %d\n", i++,entry->id, entry->lamport_clock, entry->meeting_id, entry->cylon_id);
    entry=entry->next;
  }
}

int main(void)
{
  r_queue* queue;
  queue = queue_init(1);
  int i;
  for(i=2;i<7;i++)
  {
    queue_add(queue,i,i);
    printf("\n");
    print_all(queue);
  }
  printf("position[%d]: %d\n", 8,queue_position(queue, 8));
  queue_free(queue);
  return 0;
}