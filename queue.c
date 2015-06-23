#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

/*
typedef struct{
  int id;
  int lamport_clock;
  int meeting_id;
  int cylon_id;
  queue_entry* next;
} queue_entry;


typedef struct{
  int id;
  queue_entry* entries;
} r_queue;
*/

//RULES
/*
 * 1. If entry's meeting_id is set than it's nomovable.
 * 2. First possible position for new entry is below last entry with set meeting_id and cylon_id.
 */
//QUEUE FUNCTIONS

r_queue* queue_init(int id)
{
   r_queue* queue;
   queue = malloc(sizeof(r_queue));
   
   if(queue==NULL)
    return queue;
   
   queue->id = id;
   queue->entries=malloc(sizeof(queue_entry));
   queue->entries->id=-1;
   queue->entries->lamport_clock=-1;
   queue->entries->next=NULL;
   return queue;
}

int queue_free(r_queue* queue)
{
  queue_entry* field = queue->entries;
  while(field!=NULL)
  {
    queue_entry* rest = field->next;
    free(field);
    field=rest;
  }
  free(queue);
  return 0;
}

int queue_add(r_queue* queue, int send_id, int lamport_clock)
{
  queue_entry* entry = malloc(sizeof(queue_entry));
  int i=1;
  
  if(entry==NULL)
    return -1;
  
  entry->id=send_id;
  entry->lamport_clock=lamport_clock;
  entry->meeting_id=-1;
  entry->cylon_id=-1;
  entry->next=NULL;
  
  queue_entry* field = queue->entries;
  
  while((field->next!=NULL) && 
    ((field->next->lamport_clock<lamport_clock) || ((field->next->lamport_clock==lamport_clock) && (field->next->id<send_id))))
  {
    field=field->next;
    i++;
  }
  
  entry->next = field->next;
  field->next = entry;
  
  return i;
}

int queue_set_meeting(r_queue* queue, int id, int meeting)
{
  int i=0;
  queue_entry* field = queue->entries->next;
  queue_entry* entry = queue->entries->next;
  if(entry==NULL)
  while(field!=NULL && i++!=id)
    field=field->next;
  if(field==NULL)
    return -1;
  field->meeting_id=meeting;
  return 0;
}

int queue_set_meeting_by_id(r_queue* queue, int id, int meeting)
{
  int i=0;
  queue_entry* field = queue->entries->next;queue_entry* entry = queue->entries->next;
  if(entry==NULL)
  while(field!=NULL && field->id!=id)
    field=field->next;
  if(field==NULL)
    return -1;
  field->meeting_id=meeting;
  return 0;
}


int queue_set_cylon(r_queue* queue, int id, int cylon)
{
  queue_entry* field = queue->entries->next;
  while(field!=NULL && field->id!=id)
    field=field->next;
  if(field==NULL)
    return -1;
  field->cylon_id=cylon;
  return 0;
}

void queue_sort(r_queue* queue)
{
  
}

queue_entry* queue_get(r_queue* queue, int id)
{
  queue_entry* field = queue->entries->next;
  while(field!=NULL && field->id!=id)
    field=field->next;
  return field;
}

queue_entry queue_get_top(r_queue* queue)
{
  queue_entry top;
  queue_entry* entry = queue->entries->next;
  if(entry==NULL)
    top.id = -1;
  else
  {
    top.id = entry->id;
    top.lamport_clock=entry->lamport_clock;
    top.cylon_id = entry->cylon_id;
  }
  return top;
}

int queue_get_top_meeting(r_queue* queue)
{
  int top;
  queue_entry* entry = queue->entries->next;
  if(entry==NULL)
    top = -1;
  else
    top = entry->meeting_id;
  return top;
}

int queue_remove(r_queue* queue, int id)
{
  queue_entry* field = queue->entries;
  while(field->next!=NULL && field->next->id!=id)
    field=field->next;
  if(field->next==NULL)
    return -1;
  queue_entry* entry = field->next;
  field->next = entry->next;
  free(entry);
  return 0;
}

int queue_to_array(r_queue* queue, queue_entry array[])
{
  int q_size = queue_size(queue);
  int a_size = sizeof(array)/sizeof(array[0]);
  int i = 0;
  if(q_size<a_size)
    return -1;
  queue_entry* entry = queue->entries->next;
  while(entry!=NULL)
  {
    array[i]=*entry;
    array[i++].next=NULL;
    entry=entry->next;
  }
  return 0;
}

int queue_size(r_queue* queue)
{
  int i=-1;
  
  queue_entry* entry = queue->entries;
  while(entry!=NULL)
  {
    i++;
    entry=entry->next;
  }
  
  return i;
}

int queue_top(r_queue* queue, int *id, int *lamport)
{
  queue_entry* entry = queue->entries->next;
  if(entry==NULL)
    return -1;
  *id=entry->id;
  *lamport=entry->lamport_clock;
  return 0;
}

int queue_position(r_queue* queue, int id)
{
  int i=0;
  queue_entry* field = queue->entries->next;
  while(field!=NULL && field->id!=id)
  {
    field=field->next;
    i++;
  }
  if(field==NULL)
    return -1;
  return i;
}

int queue_get_id(r_queue* queue, int num)
{
  int i=-1;
  
  queue_entry* entry = queue->entries;
  while(entry!=NULL && i!=num)
  {
    i++;
    entry=entry->next;
  }
  
  return entry!=NULL ? entry->id : -1;
}

int queue_count_with_meeting(r_queue* queue)
{
  int i=0;
  
  queue_entry* entry = queue->entries;
  while(entry!=NULL)
  {
    if(entry->meeting_id>0)
      i++;
    entry=entry->next;
  }
  return i;
}

int queue_count_below_limit(r_queue* queue, int cylons)
{
  int size = queue_size(queue);
  return size > cylons ? cylons : size;
}

int queue_clean(r_queue* queue)
{
  queue_entry* field = queue->entries->next;
  queue->entries->next=NULL;
  while(field!=NULL)
  {
    queue_entry* rest = field->next;
    free(field);
    field=rest;
  }
  return 0;
}

int queue_get_lamport(r_queue* queue, int num)
{
  int i=-1;
  
  queue_entry* entry = queue->entries;
  while(entry!=NULL && i!=num)
  {
    i++;
    entry=entry->next;
  }
  
  return entry!=NULL ? entry->lamport_clock : -1;
}