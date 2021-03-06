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
  for(i=7;i>2;i--)
  {
    queue_add(queue,i,i%2 == 0 ? 1 : 2);
    printf("\n");
    print_all(queue);
  }
  return 0;
}